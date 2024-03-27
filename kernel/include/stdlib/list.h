#ifndef _LIST_H_
#define _LIST_H_

#include "types.h"
#include "err.h"
#include "container_of.h"
#include "extra.h"

#define LIST_POISON1  ((void *) 0x100)
#define LIST_POISON2  ((void *) 0x122)

static inline void INIT_LIST(DLL *list)
{
    list->next = list;
    list->prev = list;
}

static inline void _list_add(DLL *newL, DLL *prev, DLL *next)
{
    next->prev = newL;
    newL->next = next;
    newL->prev = prev;
    prev->next = newL;
}

static inline void list_add(DLL *newL, DLL *head)
{
    _list_add(newL, head, head->next);
}

static inline void list_add_tail(DLL *newL, DLL *head)
{
    _list_add(newL, head->prev, head);
}

static inline void _list_del(DLL *prev, DLL *next)
{
    next->prev = prev;
    prev->next = next;
}

static inline void _list_del_entry(DLL *entry)
{
    _list_del(entry->prev, entry->next);
}

static inline void list_del(DLL *entry)
{
    _list_del_entry(entry);
    entry->prev = NLIST;
    entry->next = NLIST;
}

static inline void list_replace(DLL *old, DLL *newL)
{
    newL->next = old->next;
    newL->next->prev = newL;
    newL->prev = old->prev;
    newL->prev->next = newL;
}

static inline void list_swap(DLL *entry1, DLL *entry2)
{
    DLL *pos = entry2->prev;

    list_del(entry2);
    list_replace(entry1, entry2);
    if (pos == entry1)
        pos = entry2;
    list_add(entry1, pos);
}

static inline void list_move(DLL *list, DLL *head)
{
    _list_del_entry(list);
    list_add(list, head);
}

static inline void list_move_tail(DLL *list, DLL *head)
{
    _list_del_entry(list);
    list_add_tail(list, head);
}

static inline void list_bulk_move_tail(DLL *head, DLL *first, DLL *last)
{
    first->prev->next = last->next;
    last->next->prev = first->prev;

    head->prev->next = first;
    first->prev = head->prev;

    last->next = head;
    head->prev = last;
}

static inline int list_is_first(const DLL *list, const DLL *head)
{
    return list->prev == head;
}

static inline int list_is_last(const DLL *list, const DLL *head)
{
    return list->next == head;
}

static inline int list_is_head(const DLL *list, const DLL *head)
{
    return list == head;
}

static inline int list_is_empty(const DLL *head)
{
    return head->next == head;
}

static inline void list_rotate_left(DLL *head)
{
    DLL *first;

    if (!list_is_empty(head)) {
        first = head->next;
        list_move_tail(first, head);
    }
}

static inline void list_rotate_to_front(DLL *list, DLL *head)
{
    list_move_tail(head, list);
}

static inline int list_is_singular(const DLL *head)
{
    return !list_is_empty(head) && (head->next == head->prev);
}

static inline void _list_cut_position(DLL *list, DLL *head, DLL *entry)
{
    DLL *new_first = entry->next;
    list->next = head->next;
	list->next->prev = list;
	list->prev = entry;
	entry->next = list;
	head->next = new_first;
	new_first->prev = head;
}

static inline void list_cut_position(DLL *list, DLL *head, DLL *entry)
{
    if (list_is_empty(head))
        return;
    if (list_is_singular(head) && !list_is_head(entry, head) && (entry != head->next))
        return;
    if (list_is_head(entry, head))
        INIT_LIST(list);
    else
        _list_cut_position(list, head, entry);
}

static inline void list_cut_before(DLL *list, DLL *head, DLL *entry)
{
    if (head->next == entry) {
		INIT_LIST(list);
		return;
	}
	list->next = head->next;
	list->next->prev = list;
	list->prev = entry->prev;
	list->prev->next = list;
	head->next = entry;
	entry->prev = head;
}

static inline void _list_splice(const DLL *list, DLL *prev, DLL *next)
{
    DLL *first = list->next;
    DLL *last = list->prev;

    first->prev = prev;
	prev->next = first;

	last->next = next;
	next->prev = last;
}

static inline void list_splice(const DLL *list, DLL *head)
{
    if (!list_is_empty(list))
        _list_splice(list, head, head->next);
}

static inline void list_splice_tail(DLL *list, DLL *head)
{
    if (!list_is_empty(list))
        _list_splice(list, head->prev, head);
}

static inline void list_splice_init(DLL *list, DLL *head)
{
    if (!list_is_empty(list)) {
        _list_splice(list, head, head->next);
        INIT_LIST(list);
    }
}

static inline void list_splice_tail_init(DLL *list, DLL *head)
{
    if (!list_is_empty(list)) {
        _list_splice(list, head->prev, head);
        INIT_LIST(list);
    }
}

#define list_entry(ptr, type, member) \
    container_of(ptr, type, member)

#define list_first_entry(ptr, type, member) \
	list_entry((ptr)->next, type, member)

#define list_last_entry(ptr, type, member) \
	list_entry((ptr)->prev, type, member)

#define list_first_entry_or_null(ptr, type, member) ({ \
	DLL *head__ = (ptr); \
	DLL *pos__ = head__->next; \
	pos__ != head__ ? list_entry(pos__, type, member) : NULL; \
})

#define list_next_entry(pos, member) \
	list_entry((pos)->member.next, __typeof_(*(pos)), member)

#define list_next_entry_circular(pos, head, member) \
	(list_is_last(&(pos)->member, head) ? \
	list_first_entry(head, __typeof_(*(pos)), member) : list_next_entry(pos, member))

#define list_prev_entry(pos, member) \
    list_entry((pos)->member.prev, __typeof_(*(pos)), member)

#define list_prev_entry_circular(pos, head, member) \
	(list_is_first(&(pos)->member, head) ? \
	list_last_entry(head, __typeof_(*(pos)), member) : list_prev_entry(pos, member))

#define list_for_each(pos, head) \
	for (pos = (head)->next; !list_is_head(pos, (head)); pos = pos->next)

#define list_for_each_continue(pos, head) \
	for (pos = pos->next; !list_is_head(pos, (head)); pos = pos->next)

#define list_for_each_prev(pos, head) \
	for (pos = (head)->prev; !list_is_head(pos, (head)); pos = pos->prev)

#define list_for_each_safe(pos, n, head) \
	for (pos = (head)->next, n = pos->next; \
	     !list_is_head(pos, (head)); \
	     pos = n, n = pos->next)

#define list_for_each_prev_safe(pos, n, head) \
	for (pos = (head)->prev, n = pos->prev; \
	     !list_is_head(pos, (head)); \
	     pos = n, n = pos->prev)

#define list_entry_is_head(pos, head, member)				\
	(&pos->member == (head))

#define list_for_each_entry(pos, head, member)				\
	for (pos = list_first_entry(head, __typeof_(*pos), member);	\
	     !list_entry_is_head(pos, head, member);			\
	     pos = list_next_entry(pos, member))

#define list_for_each_entry_reverse(pos, head, member)			\
	for (pos = list_last_entry(head, __typeof_(*pos), member);		\
	     !list_entry_is_head(pos, head, member); 			\
	     pos = list_prev_entry(pos, member))

#define list_prepare_entry(pos, head, member) \
	((pos) ? : list_entry(head, __typeof_(*pos), member))

#define list_for_each_entry_continue(pos, head, member) 		\
	for (pos = list_next_entry(pos, member);			\
	     !list_entry_is_head(pos, head, member);			\
	     pos = list_next_entry(pos, member))

#define list_for_each_entry_continue_reverse(pos, head, member)		\
	for (pos = list_prev_entry(pos, member);			\
	     !list_entry_is_head(pos, head, member);			\
	     pos = list_prev_entry(pos, member))

#define list_for_each_entry_from(pos, head, member) 			\
	for (; !list_entry_is_head(pos, head, member);			\
	     pos = list_next_entry(pos, member))

#define list_for_each_entry_from_reverse(pos, head, member)		\
	for (; !list_entry_is_head(pos, head, member);			\
	     pos = list_prev_entry(pos, member))

#define list_for_each_entry_safe(pos, n, head, member)			\
	for (pos = list_first_entry(head, __typeof_(*pos), member),	\
		n = list_next_entry(pos, member);			\
	     !list_entry_is_head(pos, head, member); 			\
	     pos = n, n = list_next_entry(n, member))

#define list_for_each_entry_safe_continue(pos, n, head, member) 		\
	for (pos = list_next_entry(pos, member), 				\
		n = list_next_entry(pos, member);				\
	     !list_entry_is_head(pos, head, member);				\
	     pos = n, n = list_next_entry(n, member))

#define list_for_each_entry_safe_from(pos, n, head, member) 			\
	for (n = list_next_entry(pos, member);					\
	     !list_entry_is_head(pos, head, member);				\
	     pos = n, n = list_next_entry(n, member))

#define list_for_each_entry_safe_reverse(pos, n, head, member)		\
	for (pos = list_last_entry(head, __typeof_(*pos), member),		\
		n = list_prev_entry(pos, member);			\
	     !list_entry_is_head(pos, head, member); 			\
	     pos = n, n = list_prev_entry(n, member))

#define list_safe_reset_next(pos, n, member)				\
	n = list_next_entry(pos, member)

/* * * * *
 * HLIST *
 * * * * */

#define HLIST_HEAD_INIT { .first = NULL }
#define HLIST_HEAD(name) struct hlist_head name = { .first = NULL}
#define INIT_HLIST_HEAD(ptr) ((ptr)->first = (hlist_node *)NULL)

static inline void INIT_HLIST_NODE(struct hlist_node *h)
{
    h->next = (hlist_node *)NULL;
    h->pprev = (hlist_node **)NULL;
}

static inline int hlist_unhashed(const struct hlist_node *h)
{
    return !h->pprev;
}

static inline int hlist_unhashed_lockless(const struct hlist_node *h)
{
    return !READ_ONCE(h->pprev);
}

static inline int hlist_empty(const struct hlist_head *h)
{
    return !READ_ONCE(h->first);
}

static inline void __hlist_del(struct hlist_node *n)
{
    struct hlist_node *next = n->next;
    struct hlist_node **pprev = n->pprev;

    WRITE_ONCE(*pprev, next);
    if (next)
        WRITE_ONCE(next->pprev, pprev);
}

static inline void hlist_del(struct hlist_node *n)
{
    __hlist_del(n);
    n->next = (hlist_node *)LIST_POISON1;
    n->pprev = (hlist_node **)LIST_POISON2;
}

static inline void hlist_del_init(struct hlist_node *n)
{
    if (!hlist_unhashed(n)) {
        __hlist_del(n);
        INIT_HLIST_NODE(n);
    }
}

static inline void hlist_add_head(struct hlist_node *n, struct hlist_head *h)
{
    struct hlist_node *first = h->first;
    WRITE_ONCE(n->next, first);
    if (first)
        WRITE_ONCE(first->pprev, &n->next);
    WRITE_ONCE(h->first, n);
    WRITE_ONCE(n->pprev, &h->first);
}

static inline void hlist_add_before(struct hlist_node *n,
                                    struct hlist_node *next)
{
    WRITE_ONCE(n->pprev, next->pprev);
    WRITE_ONCE(n->next, next);
    WRITE_ONCE(next->pprev, &n->next);
    WRITE_ONCE(*(n->pprev), n);
}

static inline void hlist_add_behind(struct hlist_node *n,
                                    struct hlist_node *prev)
{
    WRITE_ONCE(n->next, prev->next);
    WRITE_ONCE(prev->next, n);
    WRITE_ONCE(n->pprev, &prev->next);

    if (n->next)
        WRITE_ONCE(n->next->pprev, &n->next);
}

static inline void hlist_add_fake(struct hlist_node *n)
{
    n->pprev = &n->next;
}

static inline bool hlist_fake(struct hlist_node *h)
{
    return h->pprev == &h->next;
}

static inline bool
hlist_is_singular_node(struct hlist_node *n,
                        struct hlist_head *h)
{
    return !n->next && n->pprev == &h->first;
}

static inline void hlist_move_list(struct hlist_head *old_h,
                                    struct hlist_head *new_h)
{
    new_h->first = old_h->first;
    if (new_h->first)
        new_h->first->pprev = &new_h->first;
    old_h->first = (hlist_node *)NULL;
}

#define hlist_entry(ptr, type, member) container_of(ptr,type,member)

#define hlist_for_each(pos, head) \
    for (pos = (head)->first; pos; pos = pos->next)

#define hlist_for_each_safe(pos, n, head) \
    for (pos = (head)->firstl pos && ({ n = pos->next; 1; }); \
         pos = n)

#define hlist_entry_safe(ptr, type, member) \
    ({ typeof(ptr) ____ptr = (ptr); \
       ____ptr ? hlist_entry(____ptr, type, member) : NULL; \
    })

#define hlist_for_each_entry(pos, head, member) \
    for (pos = hlist_entry_safe((head)->first, typeof(*(pos)), member_; \
         pos; \
         pos = hlist_entry_safe((pos)->member.next, typeof(*pos)), member))

#define hlist_for_each_entry_continue(pos, member) \
    for (pos = hlist_entry_safe((pos)->member.next, typeof(*(pos)), member);\
         pos;\
         pos = hlist_entry_safe((pos)->member.next, typeof(*(pos)), member))

#define hlist_for_each_entry_from(pos, member)\
    for (; pos;\
         pos = hlist_entry_safe((pos)->member.next, typeof(*(pos)), member))

#define hlist_for_each_entry_safe(pos, n, head, member)\
    for (pos = hlist_entry_safe((head)->first, typeof(*pos), member);\
         pos && ({ n = pos->member.next; 1; });\
         pos = hlist_entry_safe(n, typeof(*pos), member))

#endif // _LIST_H_