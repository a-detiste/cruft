#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define FALSE 0
#define TRUE 1

/*
 * This program reads filenames from standard input (one per line), and tries
 * to canonicalize them by chdir()-ing into their directories and calling
 * getcwd()
 */

/*
 * Realloc provided buffers to double their size.
 */
int double_buffers( size_t s, char** b1, char** b2, char** b3, char** b4 ) {
    char* n;

    n = realloc( *b1, s*2 );
    if ( n == NULL ) return FALSE;
    else             *b1 = n;

    n = realloc( *b2, s*2 );
    if ( n == NULL ) return FALSE;
    else             *b2 = n;

    n = realloc( *b3, s*2 );
    if ( n == NULL ) return FALSE;
    else             *b3 = n;

    n = realloc( *b4, s*2 );
    if ( n == NULL ) return FALSE;
    else             *b4 = n;

    return TRUE;
}

int main(void) {
    /* Prepare buffers */
    size_t sz_buffs = 4;

    char *buffer = malloc(sz_buffs);   /* user's input */
    char *cwd = malloc(sz_buffs);      /* expanded dirname */
    char *basename = malloc(sz_buffs); /* basename */
    char *last = malloc(sz_buffs);     /* last unexpanded dirname */

    char *pch;

    if ( buffer == NULL )   return EXIT_FAILURE;
    if ( cwd == NULL )      return EXIT_FAILURE;
    if ( basename == NULL ) return EXIT_FAILURE;
    if ( last == NULL )     return EXIT_FAILURE;
  
    strcpy( last, "" );

    /* Read lines from standard input */
    for(;;) {
        /* Read whole line, expanding the buffer if necessary */
	if ( fgets(buffer, sz_buffs, stdin) == NULL ) break;
	for ( pch = buffer + strlen(buffer) - 1; 
	      *pch != '\n' && pch == buffer + sz_buffs - 2; 
	      pch += strlen(pch) - 1 ) 
	{
	    assert( pch[1] == '\0' );
	    if ( double_buffers( sz_buffs, &buffer, &cwd, 
				 &basename, &last ) ) 
	    {
		sz_buffs *= 2;
	    } else {
		break;
	    }
	    pch = buffer + strlen(buffer) - 1;
	    if ( fgets(pch+1, sz_buffs/2+1, stdin) == NULL ) break;
	}

	if ( *pch == '\n' ) *pch = '\0';

	if ( buffer[0] != '/' ) {
		/* don't even try working with relative paths */
		printf("%s\n", buffer );
		continue;
	}

        /* Find last component */
	for ( ; *pch != '/'; pch-- ) 
	    ;

	/* Save it as basename */
	strcpy( basename, pch + 1 );
	/* And cut the buffer */
	*(pch+1) = '\0';

        /* Use the last result, if the directory was the same */
	if ( strcmp( last, buffer ) != 0 ) {
	    strcpy( last, buffer );

            /* Try to enter the directory */
	    if ( 0 == chdir( buffer ) ) {
                /* If succeeded, get current canonical directory
		 * expanding the buffers as necessary */
		while ( NULL == getcwd( cwd, sz_buffs ) ) {	
		    if ( double_buffers( sz_buffs, &buffer, &cwd,
					 &basename, &last ) ) 
		    {
			sz_buffs *= 2;
		    } else {
			break;
		    }
		}
	    } else {
		/* If failed for some reason, just use the read path, as there
		 * is no way to canonicalize it (maybe it is already deleted or
		 * didn't exist) */
		strcpy( cwd, buffer );
	    }
	    /* Append a slash if it does not end with one */
            pch = cwd + strlen(cwd) - 1;
            if ( *pch != '/' ) {
                pch[1] = '/';
		pch[2] = '\0';
            }
	}	

        /* Finally print the canonical path */
	printf( "%s%s\n", cwd, basename );
    }

    free(cwd); 
    free(buffer);
    free(basename);
    free(last);
    
    return 0;
}
