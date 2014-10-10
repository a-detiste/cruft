#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef __USE_BSD
#define __USE_BSD
#endif
#include <unistd.h>

#include <errno.h>

#define FALSE 0
#define TRUE 1

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
    size_t sz_buffs = 1000;

    char *buffer = malloc(sz_buffs);   /* user's input */
    char *cwd = malloc(sz_buffs);      /* expanded dirname */
    char *basename = malloc(sz_buffs); /* basename */
    char *last = malloc(sz_buffs);     /* directory the link was in */

    char *pch;

    if ( buffer == NULL )   return EXIT_FAILURE;
    if ( cwd == NULL )      return EXIT_FAILURE;
    if ( basename == NULL ) return EXIT_FAILURE;

    for(;;) {
	if ( fgets(buffer, sz_buffs, stdin) == NULL ) break;

	for ( pch = buffer + strlen(buffer) - 1; 
	      *pch != '\n' && pch == buffer + sz_buffs - 2; 
	      pch += strlen(pch) - 1 ) 
	{
	    assert( pch[1] == '\0' );
	    if ( double_buffers( sz_buffs, &buffer, &cwd, &basename, &last )) {
		sz_buffs *= 2;
	    } else {
		break;
	    }
	    pch = buffer + strlen(buffer) - 1;
	    if ( fgets(pch+1, sz_buffs/2+1, stdin) == NULL ) break;
	}

	assert( pch == buffer + strlen(buffer) - 1 );

        if ( *pch == '\n' ) *pch = '\0';

	for ( ; pch > buffer && *pch != '/'; pch-- )
	    ;

        if ( buffer[0] != '/' ) { /* don't even try relative paths */
	    if (printf("%s\n", buffer) < 0)
		return EXIT_FAILURE;
	    continue;
        }

	strncpy( last, buffer, pch - buffer + 1 );
	last[pch-buffer+1] = '\0';

	for(;;) {
	    int numchars;
	    
	    /* basename == link */
	    numchars = readlink( buffer, basename, sz_buffs - 1 );
	    if ( numchars != -1 && (unsigned)numchars < sz_buffs - 1 ) {
		chdir(last);
		basename[numchars] = 0;
		strcpy(buffer,basename);
		break;
	    }
	    
	    if ( (unsigned)numchars == sz_buffs - 1 || errno == ENAMETOOLONG ) {
		    if (double_buffers( sz_buffs, &buffer, &cwd, 
					&basename, &last )) {
			sz_buffs *= 2;
		    } else {
			fprintf( stderr, "Error. Out of memory." );
			exit(EXIT_FAILURE);
		    }
	    } else {
		break;
	    }
	}
	
	for ( pch = buffer+strlen(buffer); 
	      pch > buffer && *pch != '/'; 
	      pch-- )
	    ;
	
	if ( *pch != '/' ) { /* then buffer = "xyzzy", w/o a dir */
	    strcpy( basename, buffer );
	    strcpy( buffer, "./" );
	} else {
	    strcpy( basename, pch + 1 );
	    *(pch+1) = '\0';
	}
	
	if ( 0 == chdir( buffer ) ) {
	    while ( NULL == getcwd( cwd, sz_buffs ) ) {	
		if ( double_buffers( sz_buffs, &buffer, &cwd,
				     &basename, &last ) ) 
		{
		    sz_buffs *= 2;
		} else {
		    break;
		}
	    }
	    pch = cwd + strlen(cwd) - 1;
	    if ( *pch != '/' ) {
		pch[1] = '/'; pch[2] = '\0';
	    }
	} else {
	    if ( buffer[0] == '/' ) {
		strcpy( cwd, buffer );
	    } else {
		strcpy( cwd, last );
		strcat( cwd, buffer ); /* may overflow XXXXX */
	    }
	    pch = cwd + strlen(cwd) - 1;
	    if ( *pch != '/' ) {
		pch[1] = '/'; pch[2] = '\0';
	    }
	}
	if (printf("%s%s\n", cwd, basename) < 0)
		return EXIT_FAILURE;
    }
    if (fclose(stdout) != 0)
    	return EXIT_FAILURE;

    free(cwd);
    free(buffer);
    free(basename);
    
    return 0;
}
