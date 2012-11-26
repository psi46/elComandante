#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <stdio.h>

#include "conf.h"

#define BUFSIZE    512
#define HEX_1ST(b) ((b) >> 4)
#define HEX_2ND(b) ((b) & 0xF)

static void   group_free       ( void * group );
static void   fprint_esc       ( FILE * fp, const char * str );
static char * fscan_esc        ( FILE * fp );
static int    fpeek            ( FILE * fp );
static int    is_name_char     ( int c );
static int    is_name          ( const char * name );
static void   skip_ws          ( FILE * fp );
static int    hex2byte         ( int x1, int x2 );
static int    conf_read_group  ( conf_t * conf, FILE * fp );
static int    group_read_values( group_t * group, FILE * fp );

int x2b( int x ) {
	if( isdigit( x ) ) {
		return x - '0';
	}
	else if( x >= 'a' && x <= 'f' ) {
		return x - 'a' + 10;
	}
	else if( x >= 'A' && x <= 'F' ) {
		return x - 'A' + 10;
	}
	else {
		return -1;
	}
}

int hex2byte( int x1, int x2 ) {
	x1 = x2b( x1 );
	x2 = x2b( x2 );

	if( x1 < 0 || x2 < 0 ) {
		return -1;
	}
	else {
		return (x1 << 4) | x2;
	}
}

int fpeek( FILE * fp ) {
	int c = fgetc( fp );
	ungetc( c, fp );
	return c;
}

void fprint_esc( FILE * fp, const char * str ) {
	fputc( '\"', fp );

	while( *str ) {
		if( isascii( *str ) ) {
			switch( *str ) {
				case '\\': fprintf( fp, "\\\\" ); break;
				case '\"': fprintf( fp, "\\\"" ); break;
				case '\'': fprintf( fp, "\\\'" ); break;
				case '\n': fprintf( fp, "\\n" ); break;
				case '\r': fprintf( fp, "\\r" ); break;
				case '\t': fprintf( fp, "\\t" ); break;
				case '\v': fprintf( fp, "\\v" ); break;
				default: fputc( *str, fp );
			}
		}
		else {
			fprintf( fp, "\\x%X%X", HEX_1ST(*str), HEX_2ND(*str) );
		}

		++ str;
	}

	fputc( '\"', fp );
}

int is_name_char( int c ) {
	return isalnum( c ) || strchr( ".-_", c );
}

int is_name( const char * name ) {
	while( *name ) {
		if( ! is_name_char( *name ) ) {
			return 0;
		}
		++ name;
	}

	return 1;
}

void skip_ws( FILE * fp ) {
	int c = EOF;
	
	for(;;) {
		c = fpeek( fp );

		if( c == '#' ) {
			while( ! feof( fp ) && fgetc( fp ) != '\n' );
		}
		else if( isspace( c ) ) {
			fgetc( fp );
		}
		else {
			break;
		}
	}
}

void group_free( void * group ) {
	tree_destroy( (group_t*)group );
	free( group );
}

void conf_init( conf_t * conf ) {
	tree_init( &conf->groups, (compare_t)strcmp, free, group_free );
	tree_init( &conf->global, (compare_t)strcmp, free, free );
}

void conf_destroy( conf_t * conf ) {
	tree_destroy( &conf->groups );
	tree_destroy( &conf->global );
}

void conf_clear( conf_t * conf ) {
	tree_clear( &conf->groups );
	tree_clear( &conf->global );
}

/*

 comments: #.*$
   
 group := group_name value_list

 group_name := "[" identifier "]\n"

 identifier := [a-zA-Z][a-zA-Z0-9]*
 
 value_list := ( value_name "=" value "\n" )*

 value_name := identifier

 value := ( escaped_value | unescaped_value | e )
 
*/
int conf_read( conf_t * conf, FILE * fp ) {
	int errnum = group_read_values( &conf->global, fp );
	
	while( errnum == 0 && ! feof( fp ) ) {
		errnum = conf_read_group( conf, fp );
	}
	
	return errnum;
}

int conf_read_group( conf_t * conf, FILE * fp ) {
	int errnum = 0;
	strbuf_t buf;

	strbuf_init( &buf, BUFSIZE );

	skip_ws( fp );
	
	if( fpeek( fp ) == '[' ) {
		int c = 0;

		fgetc( fp );
		
		while( (c = fpeek( fp )) != EOF && is_name_char( c )  ) {
			errnum = strbuf_addc( &buf, fgetc( fp ) );

			if( errnum != 0 ) {
				strbuf_destroy( &buf );
				return errnum;
			}
		}
		
		if( fpeek( fp ) == ']' ) {
			char * group_name = strbuf_strextr( &buf );

			fgetc( fp );
				
			if( group_name ) {
				group_t * group = conf_add_group( conf, group_name );
			
				if( group ) {
					errnum = group_read_values( group, fp );
				}
				else {
					free( group_name );
					errnum = errno; /* kann nur ENOMEM sein */
				}
			}
			else {
				errnum = errno; /* kann nur ENOMEM sein */
			}
		}
		else {
			errnum = EINVAL;
		}
	}
	else {
		errnum = EINVAL;
	}
	
	strbuf_destroy( &buf );

	return errnum;
}

void conf_write( conf_t * conf, FILE * fp ) {
	group_iter_t * group = conf_begin( conf );
	value_iter_t * value = group_begin( &conf->global );

	// write global values:
	while( ! IS_END( value ) ) {
		fprintf( fp, "%s = ", NAME( value ) );
		fprint_esc( fp, VALUE( value ) );
		fprintf( fp, "\n" );
			
		NEXT( value );
	}
	
	fprintf( fp, "\n" );

	while( ! IS_END( group ) ) {
		value = group_begin( GROUP( group ) );

		fprintf( fp, "[%s]\n", NAME( group ) );

		while( ! IS_END( value ) ) {
			fprintf( fp, "%s = ", NAME( value ) );
			fprint_esc( fp, VALUE( value ) );
			fprintf( fp, "\n" );
			
			NEXT( value );
		}

		fprintf( fp, "\n" );

		NEXT( group );
	}
}

group_t * conf_get_group( conf_t * conf, const char * group ) {
	if( group ) {
		tree_node_t * ptr = tree_search( &conf->groups, group );

		if( ptr ) {
			return (group_t*)ptr->data;
		}
		else {
			return NULL;
		}
	}
	else {
		return &conf->global;
	}
}

group_t * conf_add_group( conf_t * conf, const char * group ) {
	group_t * ptr = NULL;

	if( is_name( group ) ) {
		char * key = strdup( group );

		if( key ) {
			ptr = malloc(sizeof(tree_t));

			if( ptr ) {
				tree_node_t * node = NULL;
			
				tree_init( ptr, (compare_t)strcmp, free, free );
				node = tree_insert( &conf->groups, key, ptr );

				if( !node ) {
					free( key );
					free( ptr );
					ptr   = NULL;
					errno = ENOMEM;
				}
			}
			else {
				free( key );
			}
		}
	}
	else {
		errno = EINVAL;
	}

	return ptr;
}

int conf_del_group( conf_t * conf, const char * group ) {
	tree_node_t * ptr = tree_search( &conf->groups, group );

	if( ptr ) {
		tree_erase( &conf->groups, ptr );
		return 0;
	}

	return EINVAL;
}

group_iter_t * conf_begin( conf_t * conf ) {
	return tree_min( &conf->groups );
}

	
const char * group_get_value( group_t * group, const char * name ) {
	tree_node_t * ptr = tree_search( group, name );

	if( ptr ) {
		return (const char *)ptr->data;
	}
	else {
		return NULL;
	}
}

int group_set_value( group_t * group, const char * name, const char * value ) {
	tree_node_t * ptr = tree_search( group, name );
	char        * str = NULL;

	if( ptr ) {
		str = strdup( value );

		if( value ) {
			free( ptr->data );
			ptr->data = str;
		}
		else {
			return ENOMEM;
		}
	}
	else {
		char * key = strdup( name );
		
		if( !key ) {
			return ENOMEM;
		}
		else if( (str = strdup( value )) == NULL ) {
			free( key );
			return ENOMEM;
		}
		else {
			value_iter_t * iter = tree_insert( group, key, str );

			if( !iter ) {
				free( key );
				free( str );
				return ENOMEM;
			}
		}
	}

	return 0;
}

int group_del_value( group_t * group, const char * name ) {
	tree_node_t * ptr = tree_search( group, name );

	if( ptr ) {
		tree_erase( group, ptr );
		return 0;
	}

	return EINVAL;
}

value_iter_t * group_begin( group_t * group ) {
	return tree_min( group );
}

int group_read_values( group_t * group, FILE * fp ) {
	int errnum = 0;
	int c      = 0;

	const char * name  = NULL;
	const char * value = NULL;
	
	strbuf_t buf;

	strbuf_init( &buf, BUFSIZE );
	
	skip_ws( fp );
		
	while( ! feof( fp ) && fpeek( fp ) != '[' && errnum == 0 ) {
		name  = NULL;
		value = NULL;

		strbuf_clear( &buf );

		while( (c = fpeek( fp )) != EOF && is_name_char( c ) ) {
			errnum = strbuf_addc( &buf, fgetc( fp ) );

			if( errnum != 0 ) {
				strbuf_destroy( &buf );
				return errnum;
			}
		}

		skip_ws( fp );

		if( fpeek( fp ) == '=' ) {
			fgetc( fp );
			
			name = strbuf_str( &buf );

			if( name ) {
				skip_ws( fp );
				
				strbuf_clear( &buf );
					
				if( fpeek( fp ) == '\"' ) {
					value = fscan_esc( fp );
				}
				else {
					errnum = strbuf_freadln( &buf, fp );

					if( errnum == 0 ) {
						value = strbuf_str( &buf );
					}
				}
	
				if( value ) {
					errnum = group_set_value( group, name, value );
				}
				else {
					errnum = errno;
				}
			}
			else {
				errnum = ENOMEM;
			}
		}
		else {
			errnum = EINVAL;
		}
		skip_ws( fp );
	}
	
	strbuf_destroy( &buf );

	return errnum;
}

char * fscan_esc( FILE * fp ) {
	strbuf_t buf;
	char * str = NULL;
	int    c   = 0;
	int    x1  = 0;
	int    x2  = 0;

	strbuf_init( &buf, BUFSIZE );

	if( fpeek( fp ) == '\"' ) {
		fgetc( fp );

		while( (c = fpeek( fp )) != EOF && c != '\"' && c != '\n' ) {
			if( fgetc( fp ) == '\\' ) {
				switch( fpeek( fp ) ) {
					case '\\': c = '\\'; fgetc( fp ); break;
					case '\"': c = '\"'; fgetc( fp ); break;
					case '\'': c = '\''; fgetc( fp ); break;
					case 'n':  c = '\n'; fgetc( fp ); break;
					case 'r':  c = '\r'; fgetc( fp ); break;
					case 't':  c = '\t'; fgetc( fp ); break;
					case 'v':  c = '\v'; fgetc( fp ); break;
					case 'x':
						fgetc( fp ); // friss das 'x'
						
						x1 = fpeek( fp );
						if( x1 == EOF || x1 == '\"' || x1 == '\n' )
							break;
						fgetc( fp );
						
						x2 = fpeek( fp );
						if( x2 == EOF || x2 == '\"' || x2 == '\n' )
							break;
						fgetc( fp );
						
						c = hex2byte( x1, x2 );

						if( c < 0 ) {
							c = 'x';
						}
				}
			}
			
			if( strbuf_addc( &buf, c ) != 0 ) {
				strbuf_destroy( &buf );
				return str;
			}
		}

		fgetc( fp );

		str = strbuf_strextr( &buf );

		fgetc( fp );
	}
	else {
		errno = EINVAL;
	}

	strbuf_destroy( &buf );
	return str;
}

