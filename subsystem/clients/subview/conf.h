#ifndef CONF_H__
#define CONF_H__

#include <stdio.h>

#include "tree.h"
#include "strbuf.h"

#ifdef __cplusplus
extern "C" {
#endif

// typedef tree_t      conf_t;
typedef tree_t      group_t;
typedef tree_node_t group_iter_t;
typedef tree_node_t value_iter_t;

typedef struct conf_s {
	tree_t  groups;
	group_t global;
} conf_t;


void           conf_init     ( conf_t * conf );
void           conf_destroy  ( conf_t * conf );
void           conf_clear    ( conf_t * conf );
int            conf_read     ( conf_t * conf, FILE * fp );
void           conf_write    ( conf_t * conf, FILE * fp );
group_t *      conf_get_group( conf_t * conf, const char * group );
group_t *      conf_add_group( conf_t * conf, const char * group );
int            conf_del_group( conf_t * conf, const char * group );
group_iter_t * conf_begin    ( conf_t * conf );

const char   * group_get_value( group_t * group, const char * name );
int            group_set_value( group_t * group, const char * name, const char * value );
int            group_del_value( group_t * group, const char * name );
value_iter_t * group_begin    ( group_t * group );

#define NEXT(iter)   ((iter) = tree_node_successor( (iter) ))
#define PREV(iter)   ((iter) = tree_node_predecessor( (iter) ))
#define IS_END(iter) ((iter) == NULL)
#define NAME(iter)   ((const char *)(iter)->key)
#define VALUE(iter)  ((const char *)(iter)->data)
#define GROUP(iter)  ((group_t *)(iter)->data)

#ifdef __cplusplus
}
#endif

#endif /* CONF_H__ */
