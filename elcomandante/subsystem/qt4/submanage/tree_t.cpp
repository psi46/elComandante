/**
 * tree_t.cpp
 *
 * Dennis Terhorst
 * Tue Jul 22 15:12:54 CEST 2008
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "tree_t.h"

/**
 * \brief create a new tree with given name
 *
 * \param name
 */
tree_t::tree_t(char* name) {
	if (DEBUG_TREE_T) printf("new node @%p constructor start \"%s\"\n", this, name);
	if (name == NULL || strlen(name) == 0) {
		name="unknown";
		printf("WARNING: Creating tree_t node with empty name! Using \"%s\" instead.\n", name);
	}
	strncpy(node, name, MAX_NODENAME); node[MAX_NODENAME-1]=0;
	subnode = NULL;
	branchsize=0;
	subnodes = 0;
	if (DEBUG_TREE_T) printf("new node @%p constructor done \"%s\"\n", this, node);
}

/**
 * \brief recursively copy to a new tree
 */
tree_t::tree_t(const tree_t* copy) {
	if (DEBUG_TREE_T) printf(">> new node @%p copy constructor start (copy from %p \"%s\")\n", this, copy, copy->Name());
	data = copy->data;			// copy data pointer FIXME check this
	strncpy(node, copy->node, MAX_NODENAME);// copy name
	subnode = NULL;
	branchsize=0;
	subnodes = 0;
	resize(copy->branchsize);		// copy subtree
	if (DEBUG_TREE_T) printf("%p: adding %d subnodes\n", this, copy->subnodes);
	for (int i=0; i<copy->subnodes; i++) {
		if (DEBUG_TREE_T) printf("%p: subnode add %p\n", this, copy->subnode[i]);
		subnode[subnodes] = new tree_t(copy->subnode[i]);
		subnodes++;
	}
	if (DEBUG_TREE_T) printf("<< new node @%p copy constructor done \"%s\"\n", this, node);
}

/**
 * \brief recursively remove this tree
 */
tree_t::~tree_t() {
	while (subnodes--) {
		delete subnode[subnodes];
	}
	if (DEBUG_TREE_T) printf("%p: del this node \"%s\"\n", this, node);
}

const char* tree_t::Name() const {
	return node;
}
/**
 * \brief add a new node to the tree
 *
 * This addition checks for name breaks and double tree entries and is supposed to be the main add-method.
 * \param name new node path name
 */
int tree_t::add(char* name) {
	if (DEBUG_TREE_T) printf("%p: add(\"%s\") to \"%s\"\n", this, name, node);
	for (int i=0; i<MAX_NODENAME; i++) {
		if (i==(int)strlen(node) && i==(int)strlen(name)) return 0; // already in list
		if (i==(int)strlen(node)) {		// end of this node
			int ret;
			for (int s=0; s<subnodes; s++) {	// pass rest of name to subnode
				if ( (ret=subnode[s]->add(&(name[i]))) >= 0) return ret;
			}
			add(new tree_t(&(name[i])));		// add here if no subnode found
			return 1;
		}
		if (i==(int)strlen(name)) {		// end in name
			int oldlen=strlen(node);
			for (int c=i; c<oldlen; c++) {	// make this nodename rest of node[]
				node[c-i] = node[c];
				node[c-i+1] = 0; // ensure termination
			}

			tree_t* old = new tree_t(node);	// create new empty node
			old->subnode = subnode;		// move all belongings to new node
			old->data = data;
			old->branchsize = branchsize;
			old->subnodes = subnodes;

			strncpy(node, name, MAX_NODENAME);	// truncate name
			branchsize=0;		// reuse this with old as first subtree
			subnode=NULL;
			subnodes=0;
			resize(+BRANCHRESIZE);		// add old tree as subtree
			subnode[subnodes++] = old;
			//if (subnodes==branchsize) resize(+BRANCHRESIZE);
			//isubnode[subnodes++] = new tree_t(&(name[i])); // add this as new node on this level
			return 1;
		}
		if (node[i] != name[i]) {	// difference
			if (i==0) { 	// wrong branch (added element is a sibling)
				return -1;
			}
			tree_t* old = new tree_t(&(node[i]));	// create new node
			old->subnode = subnode;		// move all belongings to new node
			old->data = data;
			old->branchsize = branchsize;
			old->subnodes = subnodes;

			node[i]=0;		// truncate name
			branchsize=0;		// reuse this with old as first subtree
			subnode=NULL;
			subnodes=0;
			resize(+BRANCHRESIZE);
			subnode[subnodes++] = old;
			if (subnodes==branchsize) resize(+BRANCHRESIZE);
			subnode[subnodes++] = new tree_t(&(name[i])); // add this as new node on this level
			return 1;
		}
	}
	return -1;
}

/**
 * \brief add the a tree by copy
 *
 * no double entry checking is done here!
 *
 * \param subtree tree_t structure which is to be copied into the current tree
 */
int tree_t::add(tree_t* subtree) {
	if (DEBUG_TREE_T) printf("%p: add(%p)\n", this, subtree);
	if (subnodes == branchsize) resize(+BRANCHRESIZE);
	if (subnodes == branchsize) return -1;	// resize failed
	if (DEBUG_TREE_T) printf("%p: new subnode[%d]:\n", this, subnodes);
	subnode[subnodes] = new tree_t(subtree);
	subnodes++;
	for (int i=0; i<subnodes; i++) 
		if (DEBUG_TREE_T) printf("%p: now has subnode[%d] = %p:\n", this, i, subnode[i]);
	return 1;
}

/**
 * \brief remove a subtree
 *
 * Remove the subtree with the given key. Note that this function cannot remove *this node.
 * \return -1 if node not found, -2 if node is this
 */
int tree_t::remove(char* name) {
	if (DEBUG_TREE_T) printf("%p: search   \"%s\" for removal of \"%s\"\n", this, node, name);

	if (strlen(name)<strlen(node)) return -1;
	if ( this->is(name) ) return -2; // cannot remove myself!
	if (strncmp(node, name, strlen(node)) != 0 ) return -1;
	for (int i=0; i<subnodes; i++) {
		if ( subnode[i]->remove(&(name[strlen(node)])) > 0 ) return 1;
	}
	return -1;
}

/**
 * \brief remove a subnode
 */
/*
int tree_t::remove(tree_t* ptr) {
	
}
*/

/**
 * \brief check for existance of a node name
 * \param name node to search for
 * \return Zero if node is not found,
 * 	   1 if node is found in tree,
 * 	   2 if node IS the requested one
 */
int tree_t::has(char* name) const {
	if (DEBUG_TREE_T) printf("%p: search   \"%s\"\n", this, name);
	if (strlen(name)<strlen(node)) return 0;
	for (int i=0; i<(int)strlen(node); i++) {
		if (node[i] != name[i]) return 0;
	}
	if (strlen(name)==strlen(node) ) { return 2; } // found me
	for (int i=0; i<subnodes; i++) {
		if ( subnode[i]->has(&(name[strlen(node)])) >0 ) return 1; // found child
	}
	return 0;
}

/**
 * \brief check if this node is the one with name 
 */
int tree_t::is(char* name) const {
	if (strlen(name) == strlen(node) && strcmp(name, node) == 0) return 1;
	return 0;
}

/**
 * \brief print a tree view to stdout
 */
void tree_t::list(int level) const {
	if (level==0) printf("list:\n");

	for (int j=0; j<level;j++) printf("  "); printf("%p \"%s\" [%d]\n", this, node, subnodes);

	for (int i=0; i<subnodes;i++)	{
		subnode[i]->list(level+1);
	}
	return;
}

/**
 * \brief retrieve a subtree pointer
 *
 * This function retrieves a pointer to the node for the
 * key \param name or NULL otherwise.
 *
 * Note: "return this" cannot be used with "tree_t* get() const;"
 */
tree_t* tree_t::get(char* name) {
	tree_t* found=NULL;
	if (DEBUG_TREE_T) printf(" get %s from %s...\n", name, node);
	if (strlen(name)<strlen(node)) return NULL;
	if (DEBUG_TREE_T) printf("  could be...\n");
	for (int i=0; i<(int)strlen(node); i++) {
		if (node[i] != name[i]) return NULL;
	}
	if (DEBUG_TREE_T) printf("   me?...\n");
	if (strlen(name)==strlen(node)) return this;	// this pointer not const
	if (DEBUG_TREE_T) printf("    or child?...\n");
	for (int i=0; i<subnodes; i++) {
		if ( ( found=subnode[i]->get(&(name[strlen(node)]))) != NULL ) return found;
	}
	if (DEBUG_TREE_T) printf("     no, not found.\n");
	return NULL;
}

/**
 * \brief normalize the tree
 *
 * >>> NOT JET IMLEMENTED! <<<
 * This function modifies the tree so that it does not contain
 * duplicate entries on a single level and no nodes with exactly
 * one subnode. In the latter case the single childs name is
 * appended to the parent node name.
 *
 * \return negative on error
 */
int tree_t::normalize() { printf("tree_t::normalize(): function not implemented!\n"); return -1; }

tree_t* tree_t::getFirstChild() {
	child = 0;
	return getNextChild();
}

tree_t* tree_t::getNextChild() {
	if (child >= subnodes) return NULL;
	return subnode[child++];
}

/// set user data pointer
int tree_t::setData(void*  Data) {  data = Data; return 0; }
/// get user data pointer
int tree_t::getData(void** Data) const { *Data = data; return 0; }

/**
 * \brief resize the internal storage
 *
 * \param diff storage size is increased or decreased by this number of elements
 */
int tree_t::resize(int diff) {
	tree_t** oldtree = subnode;

	if (diff == 0) return branchsize;
	if (branchsize + diff < 0) return -1;

	subnode = (tree_t**)realloc(subnode, sizeof(tree_t*)*(branchsize+diff));
	if ( subnode == NULL ) { // allocation failed
		subnode = oldtree;
		return -1;
	}
	branchsize += diff;
	if (DEBUG_TREE_T) printf("%p: resized to %d\n", this,  branchsize);
	return branchsize;
}


