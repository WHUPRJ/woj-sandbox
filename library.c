#include "resource.h"
#include "rules/lang.h"
#include "sandbox.h"
#include "utils/log.h"

#include <fcntl.h>
#include <unistd.h>

static __attribute__((constructor)) void inject(void) {
    char    comm[64];
    int     fd  = open("/proc/self/comm", O_RDONLY);
    ssize_t len = read(fd, comm, sizeof(comm));
    len         = len > 0 ? len - 1 : 0;
    comm[len]   = '\0';
    close(fd);

    LOG_INFO("Setting up sandbox for %s(%d)", comm, getpid());

    register_lang_c_cpp();
    setup_rlimit();
    setup_seccomp();
}
