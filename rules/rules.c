#include "rules.h"
#include "../err.h"
#include "../utils/log.h"

#include <stdlib.h>
#include <string.h>

LIST_HEAD(seccomp_rules);

void register_rule(struct rule *rule) { list_add(&rule->list, &seccomp_rules); }

void setup_rule(char *name, scmp_filter_ctx ctx) {
    struct list_head *current;
    struct rule      *rule;

    list_for_each(current, &seccomp_rules) {
        rule = list_entry(current, struct rule, list);
        if (strcmp(rule->name, name) == 0) {
            rule->setup(ctx);
            return;
        }
    }

    LOG_ERR("No rule found for %s", name);
    dump_rules();
    exit(ERR_NO_RULE_FOUND);
}

void dump_rules(void) {
    struct list_head *current;
    struct rule      *rule;
    LOG_INFO("Available Rules:");
    list_for_each(current, &seccomp_rules) {
        rule = list_entry(current, struct rule, list);
        LOG_INFO("> %s", rule->name);
    }
}
