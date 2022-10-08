#ifndef WOJ_SANDBOX_LIST_H
#define WOJ_SANDBOX_LIST_H

struct list_head {
    struct list_head *next;
};

#define LIST_HEAD(name) struct list_head name = {&(name)};

static inline void list_add(struct list_head *new, struct list_head *head) {
    new->next  = head->next;
    head->next = new;
}

#define list_for_each(pos, head)      for (pos = (head)->next; pos != (head); pos = pos->next)
#define list_entry(ptr, type, member) ((type *)((char *)(ptr) - (unsigned long)(&((type *)0)->member)))

#endif // WOJ_SANDBOX_LIST_H
