#include <stdio.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <ctype.h>

#include "strbuf.h"

#ifdef _MSC_VER
#   include <io.h>

#   define strncasecmp strnicmp
#   define ssize_t     size_t
#   define IS_FILE(m)  ((m) & _S_IFREG)
#else
#   include <unistd.h>

#   define IS_FILE(m) (S_ISREG( (m) ) || S_ISLNK( (m) ))
#endif

static int          readall_regfile( strbuf_t * buf, FILE * fp, size_t size );
static int          readall_stream ( strbuf_t * buf, int fd );

static const char * do_find        ( const strbuf_t * buf, const char * str,
                                     size_t pos, int (*cmp)( const char *, const char *, size_t ) );

static const char * do_rfind       ( const strbuf_t * buf, const char * str,
                                     size_t pos, int (*cmp)( const char *, const char *, size_t ) );

static int          do_replace     ( strbuf_t * buf, const char * find, const char * replace,
                                     size_t times, int (*cmp)( const char *, const char *, size_t ) );

static int          do_rreplace    ( strbuf_t * buf, const char * find, const char * replace,
                                     size_t times, int (*cmp)( const char *, const char *, size_t ) );

static int          do_replacen    ( strbuf_t * buf, char const * const * find, char const * const * replace, size_t n,
                                     int (*cmp)( const char *, const char *, size_t ) );

int strbuf_init( strbuf_t * buf, size_t initsize ) {
   buf->data = malloc( initsize );

   if( buf->data ) {
      buf->size   = initsize;
      buf->curpos = 0;
      return 0;
   }
   else {
      return errno = ENOMEM;
   }
}

void strbuf_destroy( strbuf_t * buf ) {
   /* buf->data könnte NULL sein (wenn strbuf_strextr() vorher aufgerufen wurde). */
   if( buf->data ) {
      free( buf->data );
   }
}

int strbuf_addc( strbuf_t * buf, char c ) {
   int errnum = 0;

   if( buf->size == buf->curpos ) {
      if( (errnum = strbuf_increase( buf, STRBUF_BLOCKSIZE )) != 0 ) {
         return errnum;
      }
   }

   buf->data[ buf->curpos ++ ] = c;

   return 0;
}

int strbuf_addstr( strbuf_t * buf, const char * str ) {
   return strbuf_adddata( buf, str, strlen( str ) );
}

int strbuf_addstrs( strbuf_t * buf, ... ) {
   int errnum = 0;
   va_list ap;

   va_start( ap, buf );
   errnum = strbuf_vaddstrs( buf, ap );
   va_end( ap );

   return errnum;
}

int strbuf_vaddstrs( strbuf_t * buf, va_list ap ) {
   int errnum = 0;
   const char * str = NULL;

   while( (str = va_arg( ap, const char * )) ) {
      if( (errnum = strbuf_addstr( buf, str )) )
         break;
   }

   return errnum;
}

int strbuf_adddata( strbuf_t * buf, const void * data, size_t size ) {
   int errnum = strbuf_ensure( buf, size > STRBUF_BLOCKSIZE ? size : STRBUF_BLOCKSIZE );

   if( errnum == 0 ) {
      memcpy( strbuf_curptr( buf ), data, size );
      buf->curpos += size;
   }

   return errnum;
}

int strbuf_addbuf( strbuf_t * buf, const strbuf_t * src ) {
   return strbuf_adddata( buf, src->data, STRBUF_USED( src ) );
}

const char * strbuf_str( strbuf_t * buf ) {
   int errnum = 0;

   /* strbuf_addc() wird nicht verwendet, zumal buf->curpos hier nicht erhöht werden soll. */
   if( (errnum = strbuf_ensure( buf, 1 )) != 0 ) {
      errno = errnum;
      return NULL;
   }

   buf->data[ buf->curpos ] = '\0';

   return buf->data;
}

char * strbuf_strdup( const strbuf_t * buf ) {
   char * str = malloc( strbuf_used( buf ) + 1 );

   if( str ) {
      memcpy( str, buf->data, strbuf_used( buf ) );
      str[ buf->curpos ] = '\0';
   }
   else {
      errno = ENOMEM;
   }

   return str;
}

char * strbuf_strextr( strbuf_t * buf ) {
   char * reply = NULL;
   int errnum = 0;

   if( (errnum = strbuf_ensure( buf, 1 )) != 0 ) {
      errno = errnum;
      return NULL;
   }

   buf->data[ buf->curpos ] = '\0';

   reply = buf->data;

   buf->data   = NULL;
   buf->size   = 0;
   buf->curpos = 0;

   return reply;
}

int strbuf_resize( strbuf_t * buf, size_t size ) {
   char * data = realloc( buf->data, size );

   if( !data ) {
      return errno = ENOMEM;
   }
   else {
      buf->data = data;
      buf->size = size;

      if( buf->curpos > size ) {
         /* könnt ja auch verkleinert werden */
         buf->curpos = size;
      }

      return 0;
   }
}

int strbuf_readto( strbuf_t * buf, int fd, char c ) {
   int     errnum = 0;
   char    sym    = 0;
   ssize_t count  = 0;

   count = read( fd, &sym, 1 );

   if( count == 0 ) {
      errnum = EOF;
   }
   else if( count < 0 ) {
      errnum = errno;
   }
   else {
      while( count == 1 ) {
         errnum = strbuf_addc( buf, sym );

         if( errnum != 0 ) {
            return errnum;
         }

         if( sym == c ) {
            return 0;
         }

         count = read( fd, &sym, 1 );
      }

      if( count < 0 ) {
         return errno;
      }
   }

   return errnum;
}

int strbuf_readln( strbuf_t * buf, int fd ) {
   int     errnum = 0;
   char    sym    = 0;
   ssize_t count  = 0;

   count = read( fd, &sym, 1 );

   /* EOF? */
   if( count == 0 ) {
      errnum = EOF;
   }
   /* read-error? */
   else if( count < 0 ) {
      errnum = errno;
   }
   else {
      /* solange ein Zeichen gelesen werden konnte... */
      while( count == 1 ) {
         /* unix lineend? */
         if( sym == '\n' ) {
            return 0;
         }
#ifdef STRBUF_WIN_ENDL
         /* windows lineend? */
         else if( sym == '\r' ) {
            /* nächstes Zeichen einlesen */
            count = read( fd, &sym, 1 );

            /* read-error? */
            if( count < 0 ) {
               return errno;
            }
            /* ein Zeichen gelesen? */
            else if( count == 1 ) {
               /* vollständiges Windows lineend? */
               if( sym == '\n' ) {
                  return 0;
               }
               else {
                  /* das \r von vorhin adden */
                  errnum = strbuf_addc( buf, '\r' );

                  if( errnum != 0 ) {
                     return errnum;
                  }
               }
            }
            /* damit das strbuf_addc() unten das vorhin gelesene \r noch added.
               hmmm, das wird nicht benötigt, zumal ja read() hier ja nix lesen konnte!
            else {
               sym = '\r';
            }
             */
         }
#endif
         errnum = strbuf_addc( buf, sym );

         if( errnum != 0 ) {
            return errnum;
         }
         count = read( fd, &sym, 1 );
      }

      /* read-error? */
      if( count < 0 ) {
         return errno;
      }
      /* EOF beendet an dieser Stelle die Zeile ebenfalls. */
   }

   return errnum;
}

int strbuf_readn( strbuf_t * buf, int fd, size_t n ) {
   int errnum = 0;

   if( (errnum = strbuf_ensure( buf, n )) == 0 ) {
      ssize_t count = read( fd, strbuf_curptr( buf ), n );

      if( count < 0 ) {
         errnum = errno;
      }
      else {
         buf->curpos += count;
      }
   }

   return errnum;
}

int strbuf_readall( strbuf_t * buf, int fd ) {
   long   pos    = 0;
   int    errnum = 0;
   struct stat st;

   if( fstat( fd, &st ) != 0 ) {
      errnum = errno;
   }
   else if( IS_FILE( st.st_mode ) ) {
      int    fdcpy = dup( fd );
      FILE * fp    = NULL;

      if( fdcpy < 0 ) {
         errnum = errno;
      }
      else {
         fp = fdopen( fdcpy, "r" );

         if( !fp ) {
            close( fdcpy );
            errnum = errno;
         }
         else {
            /* vieleicht mit lseek( fd, 0, SEEK_CUR )? */
            if( (pos = ftell( fp )) < 0 ) {
               errnum = errno;
            }
            else {
               /* + 1 in weißer voraussicht um nicht
                  nur für \0 nacher noch resizen zu müssen */
               errnum = readall_regfile( buf, fp, st.st_size - pos + 1 );
            }

            fclose( fp );
         }
      }
   }
   else {
      errnum = readall_stream( buf, fd );
   }

   return errnum;
}

int strbuf_freadto( strbuf_t * buf, FILE * fp, char c ) {
   int  errnum = 0;
   char sym    = 0;

   sym = fgetc( fp );

   /* EOF? */
   if( sym == EOF ) {
      if( ferror( fp ) ) {
         return errno;
      }
      errnum = EOF;
   }
   else {
      while( sym != EOF ) {
         errnum = strbuf_addc( buf, sym );

         if( errnum != 0 ) {
            return errnum;
         }

         if( sym == c ) {
            return 0;
         }

         sym = fgetc( fp );
      }

      /* read-error? */
      if( ferror( fp ) ) {
         errnum = errno;
      }
   }

   return errnum;
}

int strbuf_freadln( strbuf_t * buf, FILE * fp ) {
   int errnum = 0;
   int sym    = 0;

   sym = fgetc( fp );

   /* EOF? */
   if( sym == EOF ) {
      if( ferror( fp ) ) {
         errnum = errno;
      }
      else {
         errnum = EOF;
      }
   }
   else {
      while( sym != EOF ) {
         /* unix lineend? */
         if( sym == '\n' ) {
            return 0;
         }
#ifdef STRBUF_WIN_ENDL
         /* windows lineend? */
         else if( sym == '\r' ) {
            /* nächstes Zeichen einlesen */
            sym = fgetc( fp );

            if( sym == EOF ) {
               if( ferror( fp ) ) {
                  return errno;
               }
               else {
                  sym = '\r';
               }
            }
            else if( sym == '\n' ) {
               return 0;
            }
            else {
               /* das \r von vorhin adden */
               errnum = strbuf_addc( buf, '\r' );

               if( errnum != 0 ) {
                  return errnum;
               }
            }
         }
#endif
         errnum = strbuf_addc( buf, (char)sym );

         if( errnum != 0 ) {
            return errnum;
         }

         sym = fgetc( fp );
      }

      /* read-error? */
      if( ferror( fp ) ) {
         errnum = errno;
      }
   }

   return errnum;
}

int strbuf_freadn( strbuf_t * buf, FILE * fp, size_t n ) {
   int errnum = 0;

   if( (errnum = strbuf_ensure( buf, n )) == 0 ) {
      size_t count = fread( strbuf_curptr( buf ), 1, n, fp );

      if( ferror( fp ) ) {
         errnum = errno;
      }
      else {
         buf->curpos += count;
      }
   }

   return errnum;
}

int strbuf_freadall( strbuf_t * buf, FILE * fp ) {
   int    errnum = 0;
   long   pos    = 0;
   struct stat st;

   if( fstat( fileno( fp ), &st ) != 0 || (pos = ftell( fp )) < 0 ) {
      errnum = errno;
   }
   else if( IS_FILE( st.st_mode ) ) {
      /* könnt ja auch mit fdopen() auf nen socket erzeugt wirden sein, oder? */
      /* + 1 in weißer voraussicht um nicht
         nur für \0 nacher noch resizen zu müssen */
      errnum = readall_regfile( buf, fp, st.st_size - pos + 1 );
   }
   else {
      errnum = readall_stream( buf, fileno( fp ) );
   }

   return errnum;
}

int strbuf_ensure( strbuf_t * buf, size_t freesize ) {
   int    errnum    = 0;
   size_t remaining = STRBUF_REMAINING( buf );

   if( remaining < freesize ) {
      errnum = STRBUF_INCREASE( buf, (freesize - remaining) );
   }

   return errnum;
}

void strbuf_clear( strbuf_t * buf ) {
   buf->curpos = 0;
}

int strbuf_erase( strbuf_t * buf, size_t from, size_t len ) {
   int errnum = 0;

   if( from >= STRBUF_CURPOS( buf ) ) {
      errno = errnum = EINVAL;
   }
   else if( len >= STRBUF_CURPOS( buf ) - from ) {
      buf->curpos = from;
   }
   else {
      memmove( buf->data + from, buf->data + from + len, STRBUF_CURPOS( buf ) - from - len );
      buf->curpos -= len;
   }

   return errnum;
}

int strbuf_copy( const strbuf_t * buf, strbuf_t * dest ) {
   if( buf == dest ) {
      return 0;
   }
   else {
      strbuf_clear( dest );
      return strbuf_addbuf( dest, buf );
   }
}

int readall_regfile( strbuf_t * buf, FILE * fp, size_t size ) {
   int errnum = 0;

   if( (errnum = strbuf_ensure( buf, size )) == 0 ) {
      size_t count = fread( strbuf_curptr( buf ), 1, size, fp );

      if( ferror( fp ) ) {
         errnum = errno;
      }
      else {
         buf->curpos += count;
      }
   }

   return errnum;
}

int readall_stream( strbuf_t * buf, int fd ) {
   int errnum = 0;

   if( (errnum = strbuf_ensure( buf, STRBUF_BLOCKSIZE )) == 0 ) {
      ssize_t count = 0;

      while( (count = read( fd, strbuf_curptr( buf ), STRBUF_BLOCKSIZE )) > 0 ) {
         buf->curpos += count;

         if( (errnum = strbuf_ensure( buf, STRBUF_BLOCKSIZE )) != 0 ) {
            return errnum;
         }
      }
   }

   return errnum;
}

#if defined(STRBUF_HAS_PRINTF)

int strbuf_printf( strbuf_t * buf, const char * fmt, ... ) {
   int ret = -1;
   va_list ap;

   va_start( ap, fmt );
   ret = strbuf_vprintf( buf, fmt, ap );
   va_end( ap );

   return ret;
}

int strbuf_vprintf( strbuf_t * buf, const char * fmt, va_list ap ) {
   int ret    = -1;
#if defined(STRBUF_HAS_GNUC3) && defined(STRBUF_USE_ASPRINTF)
   char * str = NULL;

   ret = vasprintf( &str, fmt, ap );

   if( ret != -1 ) {
      int errnum = strbuf_addstr( buf, str );

      free( str );
   }
#else
   int errnum = 0;

   /* wozu das hier eigentlich?
   if( (errnum = strbuf_ensure( buf, STRBUF_BLOCKSIZE )) != 0 ) {
      errno = errnum;
      return -1;
   }
   */

   if( (ret = vsnprintf( strbuf_curptr( buf ), STRBUF_REMAINING( buf ), fmt, ap )) < 0 ) {
      return ret;
   }

   if( (size_t)ret >= STRBUF_REMAINING( buf ) ) {
      /* + 1 wegen Abschlussnull */
      if( (errnum = strbuf_ensure( buf, ret + 1 )) != 0 ) {
         return -1;
      }

	  /* sprintf schreibt ja eine abschlussnull.
	   * in ret is die aber net mitgezählt, somit muss ich
	   * genügend speicher auch für die null haben und sprintf
	   * auch sagen, das der platz da ist!!!
	   */
      if( (ret = vsnprintf( STRBUF_CURPTR( buf ), ret + 1, fmt, ap )) < 0 ) {
         return ret;
      }
   }

   buf->curpos += ret;
#endif

   return ret;
}

#endif

void strbuf_upper( strbuf_t * buf ) {
	size_t i;

	for( i = 0; i < STRBUF_USED( buf ); ++ i ) {
		buf->data[ i ] = toupper( buf->data[ i ] );
	}
}

void strbuf_lower( strbuf_t * buf ) {
	size_t i;

	for( i = 0; i < STRBUF_USED( buf ); ++ i ) {
		buf->data[ i ] = tolower( buf->data[ i ] );
	}
}

const char * do_find( const strbuf_t * buf, const char * str, size_t pos, int (*cmp)( const char *, const char *, size_t ) ) {
	size_t len     = strlen( str );
	size_t end_pos = STRBUF_USED( buf ) - len;

	if( pos <= end_pos && STRBUF_USED( buf ) > 0 ) {
		if( len == 0 ) {
			return STRBUF_STR_AT( buf, pos );
		}
		else if( STRBUF_USED( buf ) >= len ) {
			size_t i;

			for( i = pos; i <= end_pos; ++ i ) {
				if( (*cmp)( buf->data + i, str, len ) == 0 ) {
					return buf->data + i;
				}
			}
		}
	}

	return NULL;
}

const char * do_rfind( const strbuf_t * buf, const char * str, size_t pos, int (*cmp)( const char *, const char *, size_t ) ) {
	if( STRBUF_USED( buf ) > 0 ) {
		size_t len = strlen( str );

		if( pos >= STRBUF_USED( buf ) ) {
			pos = STRBUF_CURPOS( buf ) - 1;
		}

		if( len == 0 ) {
//			puts("len == 0");
			return STRBUF_STR_AT( buf, pos );
		}
		else if( pos >= (len - 1) ) {
			const char * ptr;

			// XXX: stimmt das so?
//			puts("SEARCHING...");
			for( ptr = STRBUF_STR_AT( buf, pos - len + 1 ); ptr >= STRBUF_DATA( buf ); -- ptr ) {
//				printf( "(pos: %u) [ %u ] = \"", pos, STRBUF_PTR2POS( buf, ptr ) );
//				fwrite( ptr, len, 1, stdout );
//				putchar('\"');

				if( (*cmp)( ptr, str, len ) == 0 ) {
//					puts(" <-- FOUND");
					return ptr;
				}
//				putchar('\n');
			}
		}
	}
//	puts("NOT FOUND");

	return NULL;
}

/* hmmm... memcmp verwenden um besser bin daten zu behandeln? */
const char * strbuf_find( const strbuf_t * buf, const char * str, size_t pos ) {
	return do_find( buf, str, pos, strncmp );
}

const char * strbuf_rfind( const strbuf_t * buf, const char * str, size_t pos ) {
	return do_rfind( buf, str, pos, strncmp );
}

const char * strbuf_find_ic( const strbuf_t * buf, const char * str, size_t pos ) {
	return do_find( buf, str, pos, strncasecmp );
}

const char * strbuf_rfind_ic( const strbuf_t * buf, const char * str, size_t pos ) {
	return do_rfind( buf, str, pos, strncasecmp );
}

int do_replace( strbuf_t * buf, const char * find, const char * replace, size_t times, int (*cmp)( const char *, const char *, size_t ) ) {
	int    errnum = 0;
	size_t flen   = strlen( find );

	if( flen == 0 ) {
		// XXX: oder einfach nix tun?
		errno = errnum = EINVAL;
	}
	else if( STRBUF_USED( buf ) >= flen ) {
		size_t       done_times = 0;
		size_t       rlen       = strlen( replace );
		const char * found      = buf->data;

		while( found != NULL ) {
			found = do_find( buf, find, STRBUF_PTR2POS( buf, found ), cmp );

			if( found ) {
				errnum = strbuf_overwrite( buf, replace, STRBUF_PTR2POS( buf, found ), flen );

				if( errnum == 0 ) {
					found += rlen;

					if( times && ++ done_times == times ) {
						break;
					}
				}
				else {
					break;
				}
			}

		}
	}

	return errnum;
}

// pseudocode:
//
// if strlen( findstr )
//    done_times = 0
//    found      = curptr() - 1
//
//    while found != NULL
//       found = rfind( findstr, ptr2pos( found ) )
//
//       if found
//          overwrite( replacestr, ptr2pos( found ), strlen( findstr ) )
//
//          found -= 1
//
//          if times and ++ done_times == times
//             break
int do_rreplace( strbuf_t * buf, const char * find, const char * replace, size_t times, int (*cmp)( const char *, const char *, size_t ) ) {
	int    errnum = 0;
	size_t flen   = strlen( find );

	if( flen == 0 ) {
		// XXX: oder einfach nix tun?
		errno = errnum = EINVAL;
	}
	else if( STRBUF_USED( buf ) >= flen ) {
		size_t       done_times = 0;
		const char * found      = STRBUF_CURPTR( buf );

		while( found != NULL ) {
			found = do_rfind( buf, find, STRBUF_PTR2POS( buf, found ), cmp );

			if( found ) {
				errnum = strbuf_overwrite( buf, replace, STRBUF_PTR2POS( buf, found ), flen );

				if( errnum == 0 ) {
					-- found;

					if( found < STRBUF_DATA( buf ) || ( times && ++ done_times == times ) ) {
//						printf( "\nfound:%u data:%u pos:%ld times:%u done_times:%u\n\n",
//								(size_t)found, (size_t)buf->data, (long int)found - (long int)buf->data, times, done_times );
						break;
					}
				}
				else {
					break;
				}
			}
		}
	}

	return errnum;
}

int do_replacen( strbuf_t * buf, char const * const * find, char const * const * replace, size_t n, int (*cmp)( const char *, const char *, size_t ) ) {
	int    errnum = 0;
	int    match  = 0;
	size_t pos    = 0;
	size_t * lens = malloc( n * sizeof(size_t) );
	size_t i;

	if( !lens ) {
		errno = errnum = ENOMEM;
	}
	else {
		for( i = 0; i < n; ++ i ) {
			lens[ i ] = strlen( find[ i ] );
		}

		while( errnum == 0 && pos < STRBUF_USED( buf ) ) {
			for( i = 0; i < n; ++ i ) {
				if( STRBUF_USED( buf ) - pos >= lens[ i ] &&
				    (*cmp)( find[ i ], STRBUF_STR_AT( buf, pos ), lens[ i ] ) == 0 ) {
					match  = 1;
					errnum = strbuf_overwrite( buf, replace[ i ], pos, lens[ i ] );
					break;
				}
			}

			if( match ) {
				match = 0;
				pos += strlen( replace[ i ] );
			}
			else {
				++ pos;
			}
		}

		free( lens );
	}

	return errnum;
}

int strbuf_replace( strbuf_t * buf, const char * find, const char * replace, size_t times ) {
	return do_replace( buf, find, replace, times, strncmp );
}

int strbuf_rreplace( strbuf_t * buf, const char * find, const char * replace, size_t times ) {
	return do_rreplace( buf, find, replace, times, strncmp );
}

int strbuf_replacen( strbuf_t * buf, char const * const * find, char const * const * replace, size_t n ) {
	return do_replacen( buf, find, replace, n, strncmp );
}

int strbuf_replace_ic( strbuf_t * buf, const char * find, const char * replace, size_t times ) {
	return do_replace( buf, find, replace, times, strncasecmp );
}

int strbuf_rreplace_ic( strbuf_t * buf, const char * find, const char * replace, size_t times ) {
	return do_rreplace( buf, find, replace, times, strncasecmp );
}

int strbuf_replacen_ic( strbuf_t * buf, char const * const * find, char const * const * replace, size_t n ) {
	return do_replacen( buf, find, replace, n, strncasecmp );
}

int strbuf_cmp( const strbuf_t * buf, const strbuf_t * other ) {
	size_t minlen = MIN( STRBUF_USED( buf ), STRBUF_USED( other ) );
	int    reply  = strncmp( buf->data, other->data, minlen );

	if( reply == 0 ) {
		reply = STRBUF_USED( buf ) - STRBUF_USED( other );
	}

	return reply;
}

int strbuf_cmp_ic( const strbuf_t * buf, const strbuf_t * other ) {
	size_t minlen = MIN( STRBUF_USED( buf ), STRBUF_USED( other ) );
	int    reply  = strncasecmp( buf->data, other->data, minlen );

	if( reply == 0 ) {
		reply = STRBUF_USED( buf ) - STRBUF_USED( other );
	}

	return reply;
}

int strbuf_overwrite( strbuf_t * buf, const char * str, size_t pos, size_t overwrite ) {
	int    errnum = 0;
	size_t len    = strlen( str );

	if( pos + overwrite > STRBUF_CURPOS( buf ) ) {
		errno = errnum = EINVAL;
	}
	else if( pos + overwrite == STRBUF_CURPOS( buf ) ) {
		errnum = strbuf_erase( buf, pos, overwrite );

		if( errnum == 0 ) {
			errnum = strbuf_addstr( buf, str );
		}
	}
	else {
		if( len > overwrite ) {
			errnum = strbuf_ensure( buf, len - overwrite );
		}

		if( errnum == 0 ) {
			memmove( buf->data + pos + len, buf->data + pos + overwrite, STRBUF_USED( buf ) - ( pos + overwrite ) );
			memcpy ( buf->data + pos, str, len );
			buf->curpos += len - overwrite;
		}
	}

	return errnum;
}

#undef strbuf_null_init

int strbuf_null_init( strbuf_t * buf ) {
   return STRBUF_NULL_INIT( buf );
}

#undef strbuf_shrink

int strbuf_shrink( strbuf_t * buf ) {
   return STRBUF_SHRINK( buf );
}

#undef strbuf_increase

int strbuf_increase( strbuf_t * buf, size_t inc ) {
   return STRBUF_INCREASE( buf, inc );
}

#undef strbuf_remaining

size_t strbuf_remaining( const strbuf_t * buf ) {
   return STRBUF_REMAINING( buf );
}

#undef strbuf_used

size_t strbuf_used( const strbuf_t * buf ) {
   return STRBUF_USED( buf );
}

#undef strbuf_curptr

char * strbuf_curptr( strbuf_t * buf ) {
   return STRBUF_CURPTR( buf );
}

#undef strbuf_curpos

size_t strbuf_curpos( const strbuf_t * buf ) {
   return STRBUF_CURPOS( buf );
}

#undef strbuf_size

size_t strbuf_size( const strbuf_t * buf ) {
   return STRBUF_SIZE( buf );
}

#undef strbuf_data

const char * strbuf_data( const strbuf_t * buf ) {
   return STRBUF_DATA( buf );
}

#if defined(STRBUF_HAS_SCANF)

#undef strbuf_vscanf

int strbuf_vscanf( strbuf_t * buf, const char * fmt, va_list ap ) {
   return STRBUF_VSCANF( buf, fmt, ap );
}

#undef strbuf_scanf

int strbuf_scanf( strbuf_t * buf, const char * fmt, ... ) {
   va_list ap;
   int     ret;

   va_start( ap, fmt );
   ret = STRBUF_VSCANF( buf, fmt, ap );
   va_end( ap );

   return ret;
}

#endif

#undef strbuf_lastc

int strbuf_lastc( const strbuf_t * buf ) {
   return STRBUF_LASTC( buf );
}

#undef strbuf_at

int strbuf_at( const strbuf_t * buf, size_t pos ) {
   return STRBUF_AT( buf, pos );
}

#undef strbuf_str_at

const char * strbuf_str_at( const strbuf_t * buf, size_t pos ) {
	return STRBUF_STR_AT( buf, pos );
}

#undef strbuf_ptr2pos

size_t strbuf_ptr2pos( const strbuf_t * buf, const char * ptr ) {
	return STRBUF_PTR2POS( buf, ptr );
}

#undef strbuf_insert

int strbuf_insert( strbuf_t * buf, const char * str, size_t pos ) {
	return STRBUF_INSERT( buf, str, pos );
}

