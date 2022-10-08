#include "sandbox.h"
#include "err.h"
#include "rules/rules.h"
#include "utils/log.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

void add_syscall_nr(int syscall_nr, scmp_filter_ctx ctx, uint32_t action) {
    if (seccomp_rule_add_exact(ctx, action, syscall_nr, 0)) {
        LOG_ERR("Failed to add syscall %d", syscall_nr);
        seccomp_release(ctx);
        exit(ERR_SECCOMP_RESOLVE);
    }
}

void add_syscall_name(const char *syscall_name, scmp_filter_ctx ctx, uint32_t action) {
    int syscall_nr = seccomp_syscall_resolve_name(syscall_name);
    if (syscall_nr == __NR_SCMP_ERROR) {
        LOG_ERR("Failed to resolve syscall %s", syscall_name);
        seccomp_release(ctx);
        exit(ERR_SECCOMP_RESOLVE);
    }

    add_syscall_nr(syscall_nr, ctx, action);
}

#define UNSET_ENV(name)                                               \
    do {                                                              \
        if (unsetenv(name)) {                                         \
            LOG_ERR("Failed to unset environment variable %s", name); \
            if (!disabled) seccomp_release(ctx);                      \
            exit(ERR_UNSETENV);                                       \
        }                                                             \
    } while (0)

void setup_seccomp(void) {
    LOG_INFO("Setting seccomp rules...");

    char *template = getenv(SANDBOX_TEMPLATE);
    char *action   = getenv(SANDBOX_ACTION);

    bool kill     = true;
    bool disabled = false;

    if (action && strncmp(action, "log", sizeof("log")) == 0) kill = false;

    if (action && strncmp(action, "disabled", sizeof("disabled")) == 0) {
        LOG_INFO("Seccomp disabled");
        disabled = true;
        kill     = false;
    }

    if (kill && !template) {
        LOG_ERR("Environment variable %s required", SANDBOX_TEMPLATE);
        dump_rules();
        exit(ERR_SECCOMP_ENV);
    }

    scmp_filter_ctx ctx = seccomp_init(kill ? SCMP_ACT_KILL_PROCESS : SCMP_ACT_LOG);
    if (!ctx) {
        LOG_ERR("Failed to init seccomp context");
        exit(ERR_SECCOMP_INIT);
    }

    if (!disabled && template) setup_rule(template, ctx);

    UNSET_ENV(SANDBOX_TEMPLATE);
    UNSET_ENV(SANDBOX_ACTION);

    if (!disabled && seccomp_load(ctx)) {
        LOG_ERR("Failed to load seccomp context");
        seccomp_release(ctx);
        exit(ERR_SECCOMP_LOAD);
    }
    seccomp_release(ctx);

    LOG_INFO("Preload Done");
}
