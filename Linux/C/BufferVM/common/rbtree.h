//
// Created by ben on 29/12/16.
//

#ifndef PROJECT_RBTREE_H
#define PROJECT_RBTREE_H

typedef struct rb_node rb_node_t;
struct rb_node {
    unsigned long  __rb_parent_color;
    rb_node_t *rb_right;
    rb_node_t *rb_left;
};

typedef struct rb_root {
    rb_node_t *rb_node;
} rb_root_t;

#define	RB_RED		0
#define	RB_BLACK	1

#define __rb_parent(pc)    ((rb_node_t *)(pc & ~3))
#define __rb_color(pc)     ((pc) & 1)
#define __rb_is_black(pc)  __rb_color(pc)
#define __rb_is_red(pc)    (!__rb_color(pc))
#define rb_color(rb)       __rb_color((rb)->__rb_parent_color)
#define rb_is_red(rb)      __rb_is_red((rb)->__rb_parent_color)
#define rb_is_black(rb)    __rb_is_black((rb)->__rb_parent_color)

#define rb_parent(r)   ((rb_node_t *)((r)->__rb_parent_color & ~3))

void rb_link_node(rb_node_t *, rb_node_t *, rb_node_t **);
void rb_insert(rb_node_t *, rb_root_t *);
rb_node_t *rb_erase(rb_node_t *, rb_root_t *);

//VMA rbtree stubs
void vma_rb_gap_propagate(rb_node_t *rb, rb_node_t *stop);
void vma_rb_gap_rotate(rb_node_t *rb_old, rb_node_t *rb_new);
void vma_rb_gap_copy(rb_node_t *rb_old, rb_node_t *rb_new);

#endif //PROJECT_RBTREE_H
