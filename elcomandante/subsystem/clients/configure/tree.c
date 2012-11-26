#include <errno.h>
#include "tree.h"

#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) < (b) ? (a) : (b))

static tree_node_t * tree_node_insert     ( tree_t * tree, tree_node_t ** node, tree_node_t * new_node );
static void          tree_node_calc_height( tree_node_t * node );

void tree_init( tree_t * tree,
                compare_t compare,
                destroy_t key_destroy,
                destroy_t data_destroy ) {
	tree->root         = NULL;
	tree->compare      = compare;
	tree->key_destroy  = key_destroy;
	tree->data_destroy = data_destroy;
}

void tree_destroy( tree_t * tree ) {
	tree_clear( tree );
}

void tree_clear( tree_t * tree ) {
	tree_node_destroy( tree, tree->root );
	tree->root = NULL;
}

void tree_inorder( tree_t * tree, transvert_t transvert, void * arg ) {
	tree_node_inorder( tree->root, transvert, arg );
}

void tree_preorder( tree_t * tree, transvert_t transvert, void * arg ) {
	tree_node_preorder( tree->root, transvert, arg );
}

void tree_postorder( tree_t * tree, transvert_t transvert, void * arg ) {
	tree_node_postorder( tree->root, transvert, arg );
}

tree_node_t * tree_min( tree_t * tree ) {
	if( tree->root ) {
		return tree_node_min( tree->root );
	}
	else {
		return NULL;
	}
}

tree_node_t * tree_max( tree_t * tree ) {
	if( tree->root ) {
		return tree_node_max( tree->root );
	}
	else {
		return NULL;
	}
}

/* AVL-Insert: */
tree_node_t * tree_insert( tree_t * tree, void * key, void * data ) {
	tree_node_t * node = malloc(sizeof(tree_node_t));

	if( node ) {
		tree_node_init( node, key, data );

		node = tree_node_insert( tree, &tree->root, node );
	}
	return node;
}

tree_node_t * tree_search( tree_t * tree, const void * key ) {
	int cmp = 0;
	tree_node_t * p = tree->root;

	while( p != NULL && (cmp = tree->compare( key, p->key )) != 0 ) {
		if( cmp < 0 ) {
			p = p->left;
		}
		else {
			p = p->right;
		}
	}
	return p;
}

tree_node_t * tree_lower_then( tree_t * tree, const void * key ) {
	tree_node_t * ret  = NULL;
	tree_node_t * node = tree->root;

	while( node ) {
		if( tree->compare( node->key, key ) <= 0 ) {
			if( ret ) {
				if( tree->compare( ret->key, node->key ) < 0 ) {
					ret = node;
				}
			}
			else {
				ret = node;
			}

			node = node->right;
		}
		else {
			node = node->left;
		}
	}

	return ret;
}

/* AVL-Erase: */
void tree_erase( tree_t * tree, tree_node_t * node ) {
	tree_node_t * child = NULL;
	tree_node_t * todel = NULL;

	if( node->left == NULL || node->right == NULL ) {
		todel = node;
	}
	else {
		void * tmp = NULL;

		todel = tree_node_successor( node );

		tmp        = node->key;
		node->key  = todel->key;
		todel->key = tmp;

		tmp         = node->data;
		node->data  = todel->data;
		todel->data = tmp;
	}

	if( todel->left != NULL ) {
		child = todel->left;
	}
	else {
		child = todel->right;
	}

	if( child != NULL ) {
		child->parent = todel->parent;
	}

	if( todel->parent == NULL ) {
		tree->root = child;
	}
	else {
		tree_node_t ** ptrback = NULL;

		if( todel == todel->parent->left ) {
			todel->parent->left  = child;
		}
		else {
			todel->parent->right = child;
		}

		node = todel->parent;

		while( node ) {
			tree_node_calc_height( node );

			if( node->parent ) {
				if( node == node->parent->left ) {
					ptrback = &node->parent->left;
				}
				else {
					ptrback = &node->parent->right;
				}
			}
			else {
				ptrback = &tree->root;
			}

			if( tree_node_balance( node ) == 2 ) {
				if( tree_node_height( node->right->right ) >
				    tree_node_height( node->right->left ) ) {
					*ptrback = tree_node_rotate_left( node );
				}
				else {
					*ptrback = tree_node_rotate_right_left( node );
				}
			}
			else if( tree_node_balance( node ) == -2 ) {
				if( tree_node_height( node->left->left ) >
				    tree_node_height( node->left->right ) ) {
					*ptrback = tree_node_rotate_right( node );
				}
				else {
					*ptrback = tree_node_rotate_left_right( node );
				}
			}

			node = node->parent;
		}
	}

	todel->left  = NULL;
	todel->right = NULL;

	tree_node_destroy( tree, todel );
}

size_t tree_count( const tree_t * tree ) {
	return tree_node_count( tree->root );
}

size_t tree_count_leafs( const tree_t * tree ) {
	if( tree->root ) {
		return tree_node_count_leafs( tree->root );
	}
	return 0;
}

long tree_height( const tree_t * tree ) {
	return tree_node_height( tree->root );
}

long tree_balance( const tree_t * tree ) {
	if( tree->root ) {
		return tree_node_balance( tree->root );
	}
	else {
		return 0;
	}
}

int tree_empty( const tree_t * tree ) {
	return tree->root == NULL;
}

void tree_node_init( tree_node_t * node, void * key, void * data ) {
	node->parent = NULL;
	node->left   = NULL;
	node->right  = NULL;

	node->key    = key;
	node->data   = data;
	node->height = 0;
}

void tree_node_destroy( tree_t * tree, tree_node_t * node ) {
	if( node != NULL ) {
		tree_node_destroy( tree, node->left );
		tree_node_destroy( tree, node->right );

		tree->key_destroy( node->key );
		tree->data_destroy( node->data );

		free( node );
	}
}

void tree_node_inorder( tree_node_t * node,
                        transvert_t transvert,
                        void * arg ) {
	if( node != NULL ) {
		tree_node_inorder( node->left, transvert,arg );
		transvert( node->key, node->data, arg );
		tree_node_inorder( node->right, transvert,arg );
	}
}

void tree_node_preorder( tree_node_t * node,
                         transvert_t transvert,
                         void * arg ) {
	if( node != NULL ) {
		transvert( node->key, node->data, arg );
		tree_node_inorder( node->left, transvert,arg );
		tree_node_inorder( node->right, transvert,arg );
	}
}

void tree_node_postorder( tree_node_t * node,
                          transvert_t transvert,
                          void * arg ) {
	if( node != NULL ) {
		tree_node_inorder( node->left, transvert,arg );
		tree_node_inorder( node->right, transvert,arg );
		transvert( node->key, node->data, arg );
	}
}

tree_node_t * tree_node_min( tree_node_t * node ) {
	while( node->left != NULL ) {
		node = node->left;
	}
	return node;
}

tree_node_t * tree_node_max( tree_node_t * node ) {
	while( node->right != NULL ) {
		node = node->right;
	}
	return node;
}

tree_node_t * tree_node_predecessor( tree_node_t * node ) {
	if( node->left != NULL ) {
		return tree_node_max( node->left );
	}
	else {
		tree_node_t * q = node->parent;

		while( q != NULL && node == q->left ) {
			node = q;
			q    = q->parent;
		}

		return q;
	}
}

tree_node_t * tree_node_successor( tree_node_t * node ) {
	if( node->right != NULL ) {
		return tree_node_min( node->right );
	}
	else {
		tree_node_t * q = node->parent;

		while( q != NULL && node == q->right ) {
			node = q;
			q    = q->parent;
		}

		return q;
	}
}

size_t tree_node_count( const tree_node_t * node ) {
	if( node ) {
		return tree_node_count( node->left  ) +
		       tree_node_count( node->right ) + 1;
	}
	else {
		return 0;
	}
}

size_t tree_node_count_leafs( const tree_node_t * node ) {
	size_t leafs = 0;

	if( node->left == NULL && node->right == NULL ) {
		return 1;
	}

	if( node->left != NULL ) {
		leafs += tree_node_count_leafs( node->left );
	}

	if( node->right != NULL ) {
		leafs += tree_node_count_leafs( node->right );
	}

	return leafs;
}

long tree_node_height( const tree_node_t * node ) {
	if( node ) {
		return node->height;
	}
	else {
		return -1;
	}
}

long tree_node_balance( const tree_node_t * node ) {
	return tree_node_height( node->right ) -
	       tree_node_height( node->left );
}

tree_node_t * tree_node_insert( tree_t * tree, tree_node_t ** node, tree_node_t * new_node ) {
	int cmp = 0;

	if( *node == NULL ) {
		*node = new_node;
	}
	else {
		if( (cmp = tree->compare( new_node->key, ( *node )->key )) < 0 ) {
			new_node->parent = *node;
			tree_node_insert( tree, & ( *node )->left, new_node );

			if( tree_node_balance( *node ) == -2 ) {
				if( tree_node_height( ( *node )->left->left ) >
				    tree_node_height( ( *node )->left->right ) ) {
					*node = tree_node_rotate_right( *node );
				}
				else {
					*node = tree_node_rotate_left_right( *node );
				}
			}
		}
		else if( cmp > 0 ) {
			new_node->parent = *node;
			tree_node_insert( tree, & ( *node )->right, new_node );

			if( tree_node_balance( *node ) == 2 ) {
				if( tree_node_height( ( *node )->right->right ) >
				    tree_node_height( ( *node )->right->left ) ) {
					*node = tree_node_rotate_left( *node );
				}
				else {
					*node = tree_node_rotate_right_left( *node );
				}
			}
		}
		else {
			/* gleicher schlüssel! */
			void * tmp = NULL;

			tmp             = ( *node )->data;
			( *node )->data = new_node->data;
			new_node->data  = tmp;

			tree_node_destroy( tree, new_node );
		}

		tree_node_calc_height( *node );
	}

	return new_node;
}

tree_node_t * tree_node_rotate_right( tree_node_t * node ) {
	tree_node_t * v = node->left;

	node->left = v->right;

	if( v->right ) {
		v->right->parent = node;
	}

	v->right     = node;
	v->parent    = node->parent;
	node->parent = v;

	tree_node_calc_height( node );
	tree_node_calc_height( v );

	return v;
}

tree_node_t * tree_node_rotate_left( tree_node_t * node ) {
	tree_node_t * v = node->right;

	node->right = v->left;

	if( v->left ) {
		v->left->parent = node;
	}

	v->left      = node;
	v->parent    = node->parent;
	node->parent = v;

	tree_node_calc_height( node );
	tree_node_calc_height( v );

	return v;
}

tree_node_t * tree_node_rotate_left_right( tree_node_t * node ) {
	node->left = tree_node_rotate_left( node->left );
	return tree_node_rotate_right( node );
}

tree_node_t * tree_node_rotate_right_left( tree_node_t * node ) {
	node->right = tree_node_rotate_right( node->right );
	return tree_node_rotate_left( node );
}

void tree_node_calc_height( tree_node_t * node ) {
	long lh = tree_node_height( node->left );
	long rh = tree_node_height( node->right );

	node->height = MAX(lh,rh) + 1;
}

