#ifndef _RBTREE_H_
#define _RBTREE_H_

#include "container_of.h"
#include "types.h"
#include "err.h"

#define rb_parent(r) ((rb_node *)((r)->__rb_parent_color & ~3))

#define rb_entry(ptr, type, member) container_of(ptr, type, member)

#define RB_EMPTY_ROOT(root) (((root)->rb_node) == NULL)

#define RB_EMPTY_NODE(node)  \
	((node)->__rb_parent_color == (unsigned long)(node))
#define RB_CLEAR_NODE(node)  \
	((node)->__rb_parent_color = (unsigned long)(node))

extern void rb_insert_color(struct rb_node *, struct rb_root *);
extern void rb_erase(struct rb_node *, struct rb_root *);

extern struct rb_node *rb_next(const struct rb_node *);
extern struct rb_node *rb_prev(const struct rb_node *);
extern struct rb_node *rb_first(const struct rb_root *);
extern struct rb_node *rb_last(const struct rb_root *);

extern struct rb_node *rb_first_postorder(const struct rb_root *);
extern struct rb_node *rb_next_postorder(const struct rb_node *);

extern void rb_replace_node(struct rb_node *victim, struct rb_node *newL,
			    struct rb_root *root);
extern void rb_replace_node_rcu(struct rb_node *victim, struct rb_node *newL,
				struct rb_root *root);

static inline void rb_link_node(rb_node *node, rb_node *parent, rb_node **rb_link)
{
	node->__rb_parent_color = (unsigned long)parent;
	node->rb_left = node->rb_right = (rb_node *)(0);

	*rb_link = node;
}

#endif // _RBTREE_H_