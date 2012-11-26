/*
 * tree_t.h
 *
 * self building tree for strings
 * Dennis Terhorst
 * Thu Jul 17 23:54:48 CEST 2008
 */
#ifndef TREE_T_H
#define TREE_T_H

/**
 * \brief string space allocation
 */
#define MAX_NODENAME	32

/**
 * \brief internal storage resize amount
 *
 * Setting this to a higher value will increase the
 * minimum memory requirements for each node, setting this
 * to lower values will decrease performance.
 */
#define BRANCHRESIZE	5

/**
 * DEBUG FLAG
 */
#define DEBUG_TREE_T	0

class tree_t {
private:
	char node[MAX_NODENAME];///< node name
	void* data;		///< user data pointer
	tree_t** subnode;	///< pointers to subnodes;
	int branchsize;		///< allocated *subnode slots;
	int subnodes;		///< used *subnode slots;
	int child;		///< counter for firstChild(), nextChild() mechanism
public:
	// create an empty tree by default
	tree_t(char* name);
	tree_t(const tree_t* copy);
	~tree_t();
	const char* Name() const;

	int add(char* name);
	int add(tree_t* subtree);
	int remove(char* name);
	// int remove(tree_t* ptr);
	int has(char* name) const;
	int is(char* name) const;
	tree_t* get(char* name);
	void list(int level=0) const;

	int normalize();

	tree_t* getFirstChild();
	tree_t* getNextChild();

	int setData(void* data);
	int getData(void** data) const;
private:
	int resize(int diff);
};

#endif // define TREE_T_H
