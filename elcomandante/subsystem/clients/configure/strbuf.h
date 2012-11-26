#ifndef STRBUF_H__
#define STRBUF_H__

#include <stdio.h>
#include <stdarg.h>

#define STRBUF_VERSION          1.01
#define STRBUF_BLOCKSIZE        512
#define STRBUF_ALLWAYS_WIN_ENDL 1
/* #define STRBUF_USE_ASPRINTF */

#if defined(_MSC_VER) || defined(__CYGWIN__) || defined(__WIN32__) || defined(WIN32) || defined(STRBUF_ALLWAYS_WIN_ENDL)
#   define STRBUF_WIN_ENDL
#endif

#if !defined(_MSC_VER)
#   define STRBUF_HAS_PRINTF 1
#   define STRBUF_HAS_SCANF  1
#endif

#if defined(__GNUC__)
#   if __GNUC__ >= 3
#      define STRBUF_HAS_GNUC3 1
#   endif
#else
#  define  __attribute__( x )  /* nothing */
#endif

#ifndef MIN
#   define MIN( arg1, arg2 ) ( (arg1) < (arg2) ? (arg1) : (arg2) )
#endif

#ifndef MAX
#   define MAX( arg1, arg2 ) ( (arg1) > (arg2) ? (arg1) : (arg2) )
#endif

#ifdef __cplusplus
namespace strbuf {
   extern "C" {
#endif

typedef struct strbuf_s {
   size_t   size;
   size_t   curpos;
   char   * data;
} strbuf_t;

#define STRBUF_NPOS ((size_t)-1)

/* hmmm... inline support hinzufügen? würd sich das net mit DLLs/SOs haun? */
#if defined(STRBUF_HAS_GNUC3)
#  define STRBUF_STATIC_INIT { .size = 0, .curpos = 0, .data = NULL }
#  define __inline__
#else
#  define STRBUF_STATIC_INIT { 0, 0, NULL }
#  if !defined(_MSC_VER)
#     define inline
#  endif
#endif

int          strbuf_init      (       strbuf_t * buf, size_t initsize );
void         strbuf_destroy   (       strbuf_t * buf );
int          strbuf_addc      (       strbuf_t * buf, char c );
int          strbuf_addstr    (       strbuf_t * buf, const char * str );
int          strbuf_addstrs   (       strbuf_t * buf, ... );
int          strbuf_vaddstrs  (       strbuf_t * buf, va_list ap );
int          strbuf_adddata   (       strbuf_t * buf, const void * data, size_t size );
int          strbuf_addbuf    (       strbuf_t * buf, const strbuf_t * src );
const char * strbuf_str       (       strbuf_t * buf );
char       * strbuf_strdup    ( const strbuf_t * buf );
char       * strbuf_strextr   (       strbuf_t * buf );
int          strbuf_resize    (       strbuf_t * buf, size_t size );
int          strbuf_readto    (       strbuf_t * buf, int fd, char c );
int          strbuf_readln    (       strbuf_t * buf, int fd );
int          strbuf_readn     (       strbuf_t * buf, int fd, size_t n );
int          strbuf_readall   (       strbuf_t * buf, int fd );
int          strbuf_freadto   (       strbuf_t * buf, FILE * fp, char c );
int          strbuf_freadln   (       strbuf_t * buf, FILE * fp );
int          strbuf_freadn    (       strbuf_t * buf, FILE * fp, size_t n );
int          strbuf_freadall  (       strbuf_t * buf, FILE * fp );
int          strbuf_ensure    (       strbuf_t * buf, size_t freesize );
void         strbuf_clear     (       strbuf_t * buf );
int          strbuf_erase     (       strbuf_t * buf, size_t from, size_t len );
int          strbuf_copy      ( const strbuf_t * buf, strbuf_t * dest );

#if defined(STRBUF_HAS_PRINTF)

int          strbuf_printf    (       strbuf_t * buf, const char * fmt, ... )        __attribute__(( format( printf, 2, 3 ) ));
int          strbuf_vprintf   (       strbuf_t * buf, const char * fmt, va_list ap ) __attribute__(( format( printf, 2, 0 ) ));

#endif

int          strbuf_overwrite (       strbuf_t * buf, const char * str, size_t pos, size_t overwrite );
void         strbuf_upper     (       strbuf_t * buf );
void         strbuf_lower     (       strbuf_t * buf );
const char * strbuf_find      ( const strbuf_t * buf, const char * str, size_t pos );
const char * strbuf_rfind     ( const strbuf_t * buf, const char * str, size_t pos );
int          strbuf_replace   (       strbuf_t * buf, const char * find, const char * replace, size_t times );
int          strbuf_replacen  (       strbuf_t * buf, char const*const* find, char const*const* replace, size_t n );
int          strbuf_cmp       ( const strbuf_t * buf, const strbuf_t * other );
int          strbuf_cmp_ic    ( const strbuf_t * buf, const strbuf_t * other );

/* TODO: Implementation */
int          strbuf_rreplace  (       strbuf_t * buf, const char * find, const char * replace, size_t times );

/* ignore case: */
const char * strbuf_find_ic    ( const strbuf_t * buf, const char * str, size_t pos );
const char * strbuf_rfind_ic   ( const strbuf_t * buf, const char * str, size_t pos );
int          strbuf_replace_ic (       strbuf_t * buf, const char * find, const char * replace, size_t times );
int          strbuf_replacen_ic(       strbuf_t * buf, char const*const* find, char const*const* replace, size_t n );

/* TODO: Implementation */
int          strbuf_rreplace_ic(       strbuf_t * buf, const char * find, const char * replace, size_t times );

/* there are also macros for the following functions: */
int          strbuf_null_init(       strbuf_t * buf );
int          strbuf_shrink   (       strbuf_t * buf );
int          strbuf_increase (       strbuf_t * buf, size_t inc );
size_t       strbuf_remaining( const strbuf_t * buf );
size_t       strbuf_used     ( const strbuf_t * buf );
char       * strbuf_curptr   (       strbuf_t * buf );
size_t       strbuf_curpos   ( const strbuf_t * buf );
size_t       strbuf_size     ( const strbuf_t * buf );
const char * strbuf_data     ( const strbuf_t * buf );

#if defined(STRBUF_HAS_SCANF)

int          strbuf_scanf    (       strbuf_t * buf, const char * fmt, ... )        __attribute__(( format( scanf, 2, 3 ) ));
int          strbuf_vscanf   (       strbuf_t * buf, const char * fmt, va_list ap ) __attribute__(( format( scanf, 2, 0 ) ));

#endif

int          strbuf_lastc    ( const strbuf_t * buf );
int          strbuf_at       ( const strbuf_t * buf, size_t pos );
const char * strbuf_str_at   ( const strbuf_t * buf, size_t pos );
size_t       strbuf_ptr2pos  ( const strbuf_t * buf, const char * ptr );
int          strbuf_insert   (       strbuf_t * buf, const char * str, size_t pos );


#if defined(STRBUF_HAS_GNUC3)
#   define STRBUF_ADDSTRS( buf, ... )     strbuf_addstrs( (buf), ## __VA_ARGS__, NULL )
#endif

#define STRBUF_NULL_INIT( buf )           ( (buf)->size = 0, (buf)->curpos = 0, (buf)->data = NULL, /* return */ 0 )
#define STRBUF_SHRINK(    buf )           strbuf_resize( (buf), (buf)->curpos )
#define STRBUF_INCREASE(  buf, inc )      strbuf_resize( (buf), (buf)->size + (inc) )
#define STRBUF_USED(      buf )           ((const size_t)((buf)->curpos))
#define STRBUF_REMAINING( buf )           ((buf)->size - STRBUF_USED( (buf) ))
#define STRBUF_CURPTR(    buf )           ((buf)->data + (buf)->curpos)
#define STRBUF_CURPOS(    buf )           ((const size_t)((buf)->curpos))
#define STRBUF_SIZE(      buf )           ((const size_t)((buf)->size))
#define STRBUF_DATA(      buf )           ((const char *)((buf)->data))

#if defined(STRBUF_HAS_SCANF)

#define STRBUF_VSCANF(    buf, fmt, ap )  vsscanf( strbuf_str( (buf) ), fmt, ap )

#endif

#define STRBUF_LASTC(     buf )           ((const int)( STRBUF_USED( (buf) ) ? *( STRBUF_CURPTR( (buf) ) - 1 ) : EOF ) )
#define STRBUF_AT(        buf, pos )      ((const int)( STRBUF_CURPOS( (buf) ) > (pos) ? STRBUF_DATA( (buf) )[ (pos) ] : EOF ))
#define STRBUF_STR_AT(    buf, pos )      ((const char *)( STRBUF_CURPOS( (buf) ) > (pos) ? STRBUF_DATA( (buf) ) + (pos) : NULL ))
#define STRBUF_PTR2POS(   buf, ptr )      ((size_t)( (ptr) >= STRBUF_DATA( (buf) ) ? (ptr) - STRBUF_DATA( (buf) ) : STRBUF_NPOS ))
#define STRBUF_INSERT(    buf, str, pos ) strbuf_overwrite( (buf), (str), (pos), 0 )
																												 
#if !defined(STRBUF_DONT_USE_MACROS)

#define strbuf_null_init( buf )           STRBUF_NULL_INIT( (buf) )
#define strbuf_shrink(    buf )           STRBUF_SHRINK(    (buf) )
#define strbuf_increase(  buf, inc )      STRBUF_INCREASE(  (buf), (inc) )
#define strbuf_remaining( buf )           STRBUF_REMAINING( (buf) )
#define strbuf_used(      buf )           STRBUF_USED(      (buf) )
#define strbuf_curptr(    buf )           STRBUF_CURPTR(    (buf) )
#define strbuf_curpos(    buf )           STRBUF_CURPOS(    (buf) )
#define strbuf_size(      buf )           STRBUF_SIZE(      (buf) )
#define strbuf_data(      buf )           STRBUF_DATA(      (buf) )

#if defined(STRBUF_HAS_SCANF)

#define strbuf_vscanf(    buf, fmt, ap )  STRBUF_VSCANF(    (buf), (fmt), (ap) )

#endif

#define strbuf_lastc(     buf )           STRBUF_LASTC(     (buf) )
#define strbuf_at(        buf, pos )      STRBUF_AT(        (buf), (pos) )
#define strbuf_str_at(    buf, pos )      STRBUF_STR_AT(    (buf), (pos) )
#define strbuf_ptr2pos(   buf, ptr )      STRBUF_PTR2POS(   (buf), (ptr) )
#define strbuf_insert(    buf, str, pos ) STRBUF_INSERT(    (buf), (str), (pos) )

#  if defined(STRBUF_HAS_GNUC3) && defined(STRBUF_HAS_SCANF)
#     define strbuf_scanf( buf, fmt, ... ) sscanf( strbuf_str( (buf) ), fmt, ## __VA_ARGS__ )
#  endif /* defined(STRBUF_HAS_GNUC3) */

#endif /* !defined(STRBUF_DONT_USE_MACROS) */

#ifdef __cplusplus
   } // extern "C"
} // namespace strbuf
#endif

#if !defined(__GNUC__)
#   undef __attribute__
#endif

#endif /* STRBUF_H__ */
