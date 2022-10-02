#include "resource.h"
#include "rules/lang.h"
#include "sandbox.h"
#include "utils/log.h"

static __attribute__((constructor)) void inject(void) {
    LOG_INFO("Setting up...");
    register_lang_c();
    setup_rlimit();
    setup_seccomp();
}
