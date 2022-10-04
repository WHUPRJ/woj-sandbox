#include "../sandbox.h"
#include "lang.h"
#include "rules.h"

#include <seccomp.h>

void setup_lang_c(scmp_filter_ctx ctx) {
    int white[] = {
        SCMP_SYS(read),            // 0
        SCMP_SYS(write),           // 1
        SCMP_SYS(fstat),           // 5
        SCMP_SYS(lseek),           // 8
        SCMP_SYS(mmap),            // 9
        SCMP_SYS(munmap),          // 11
        SCMP_SYS(pread64),         // 17
        SCMP_SYS(getpid),          // 39
        SCMP_SYS(futex),           // 202
        SCMP_SYS(newfstatat),      // 262
        SCMP_SYS(clock_gettime),   // 228
        SCMP_SYS(clock_getres),    // 229
        SCMP_SYS(clock_nanosleep), // 230
        SCMP_SYS(exit_group),      // 231
    };
    int white_len = sizeof(white) / sizeof(white[0]);

    for (int i = 0; i < white_len; i++) {
        add_syscall_nr(white[i], ctx, SCMP_ACT_ALLOW);
    }
}

struct rule lang_c_rule = {
    .name  = "lang_c",
    .setup = setup_lang_c,
};

void register_lang_c(void) { register_rule(&lang_c_rule); }
