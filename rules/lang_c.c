#include "../sandbox.h"
#include "lang.h"
#include "rules.h"

#include <seccomp.h>

void setup_lang_c(scmp_filter_ctx ctx) {
    int white[]   = {SCMP_SYS(read),  SCMP_SYS(write),      SCMP_SYS(getpid),
                     SCMP_SYS(futex), SCMP_SYS(exit_group), SCMP_SYS(newfstatat)};
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
