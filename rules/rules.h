#ifndef WOJ_SANDBOX_RULES_H
#define WOJ_SANDBOX_RULES_H

#include "../utils/list.h"

#include "seccomp.h"

struct rule {
    char *name;
    void (*setup)(scmp_filter_ctx);
    struct list_head list;
};

void register_rule(struct rule *rule);
void setup_rule(char *name, scmp_filter_ctx ctx);
void dump_rules(void);

#endif // WOJ_SANDBOX_RULES_H
