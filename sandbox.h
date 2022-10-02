#ifndef WOJ_SANDBOX_SANDBOX_H
#define WOJ_SANDBOX_SANDBOX_H

#include <seccomp.h>
#include <stdint.h>

// Configuration Environment Variables
#define SANDBOX_TEMPLATE "SANDBOX_TEMPLATE"
#define SANDBOX_ACTION   "SANDBOX_ACTION"

void setup_seccomp(void);
void add_syscall_name(const char *syscall_name, scmp_filter_ctx ctx,
                      uint32_t action);
void add_syscall_nr(int syscall_nr, scmp_filter_ctx ctx, uint32_t action);

#endif // WOJ_SANDBOX_SANDBOX_H
