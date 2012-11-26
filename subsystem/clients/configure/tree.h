#ifndef TREE_H__
#define TREE_H__

/*
example:

tree_t tree;
tree_init( &tree, strcmp, free, free );

tree_insert( &tree, strdup("key1"), strdup("foo") );
tree_insert( &tree, strdup("key2"), strdup("bar") );
tree_insert( &tree, strdup("key3"), strdup("baz") );

tree_node_t * node = tree_search( &tree, "key1" );
tree_erase( &tree, node );

tree_destroy( &tree );
*/

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct tree_node_s {
	struct tree_node_s * parent;
	struct tree_node_s * left;
	struct tree_node_s * right;

	void * data;
	void * key;

	long height;
} tree_node_t;

typedef int  (*compare_t)( const void * left, const void * right );
typedef void (*destroy_t)( void * data );
typedef void (*transvert_t)( void * key, void * data, void * arg );

typedef struct tree_s {
	tree_node_t * root;
	
	compare_t compare;
	destroy_t key_destroy;
	destroy_t data_destroy;
} tree_t;

#define TREE_INIT( cmp, kdestr, ddestr ) { .root = NULL, .compare = (cmp), .key_destroy = (kdestr), .data_destroy = (ddestr) }

void          tree_init       ( tree_t * tree,
                                compare_t compare,
                                destroy_t key_destroy,
                                destroy_t data_destroy );
void          tree_destroy    ( tree_t * tree );
void          tree_clear      ( tree_t * tree );
void          tree_inorder    ( tree_t * tree, transvert_t transvert, void * arg );
void          tree_preorder   ( tree_t * tree, transvert_t transvert, void * arg );
void          tree_postorder  ( tree_t * tree, transvert_t transvert, void * arg );
tree_node_t * tree_min        ( tree_t * tree );
tree_node_t * tree_max        ( tree_t * tree );
tree_node_t * tree_insert     ( tree_t * tree, void * key, void * data );
tree_node_t * tree_search     ( tree_t * tree, const void * key );
tree_node_t * tree_lower_then ( tree_t * tree, const void * key );
void          tree_erase      ( tree_t * tree, tree_node_t * node );
size_t        tree_count      ( const tree_t * tree );
size_t        tree_count_leafs( const tree_t * tree );
long          tree_height     ( const tree_t * tree );
long          tree_balance    ( const tree_t * tree );
int           tree_empty      ( const tree_t * tree );

void          tree_node_init       ( tree_node_t * node, void * key, void * data );
void          tree_node_destroy    ( tree_t * tree, tree_node_t * node );
void          tree_node_inorder    ( tree_node_t * node,
                                     transvert_t transvert,
                                     void * arg );
void          tree_node_preorder   ( tree_node_t * node,
                                     transvert_t transvert,
                                     void * arg );
void          tree_node_postorder  ( tree_node_t * node,
                                     transvert_t transvert,
                                     void * arg );
tree_node_t * tree_node_min        ( tree_node_t * node );
tree_node_t * tree_node_max        ( tree_node_t * node );
tree_node_t * tree_node_predecessor( tree_node_t * node );
tree_node_t * tree_node_successor  ( tree_node_t * node );
size_t        tree_node_count      ( const tree_node_t * node );
size_t        tree_node_count_leafs( const tree_node_t * node );
long          tree_node_height     ( const tree_node_t * node );
long          tree_node_balance    ( const tree_node_t * node );

tree_node_t * tree_node_rotate_left      ( tree_node_t * node );
tree_node_t * tree_node_rotate_right     ( tree_node_t * node );
tree_node_t * tree_node_rotate_left_right( tree_node_t * node );
tree_node_t * tree_node_rotate_right_left( tree_node_t * node );

#ifdef __cplusplus
}
#endif

#endif /* TREE_H__ */
