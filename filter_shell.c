#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define FALSE 0
#define TRUE  1

#define BUFFER_LEN 1000
#define NUM_STRINGS 4000

int shellexp(char*, char*);

int main(int argc, char **argv) {
    char (*strings)[BUFFER_LEN] = calloc(NUM_STRINGS, BUFFER_LEN);
    char buffer[BUFFER_LEN];
    FILE* exp_list;
    int n_strings = 0;
    int i;
    
    if (!strings)
    {
        return EXIT_FAILURE;
    }
    /* Read into memory all patterns from the files specified as command line
     * arguments */
    for ( i = 1; i < argc; i++ ) {
	exp_list = fopen( argv[i], "r" );
	if ( exp_list == NULL ) {
	    perror( argv[i] );
	    continue;
	}
	
	while( fgets( strings[n_strings], BUFFER_LEN, exp_list ) ) {
	    char*pch;
	    char*buffer = strings[n_strings];
	    
	    for ( pch = buffer; *pch != '\0'; pch++ ) {
		/* Skip comments */
		if ( *pch == '#' ) {
		    *pch = '\0';
		    break;
		}
	    }
	    
	    /* Trim trailing whitespace */
	    for ( pch = pch - 1;
		  pch >= buffer && isspace(*pch); 
		  pch-- )
		;
	    
	    
	    *(pch+1) = '\0';
	    
	    n_strings++;
	    if ( n_strings == NUM_STRINGS ) {
		fprintf(stderr, "You have more than %d patterns defined. Change NUM_STRINGS definition in "__FILE__" and recompile.\n", NUM_STRINGS);
		exit(EXIT_FAILURE);
	    }
	    continue;
	    /* XXX: the following code is unreachable */
	    /* Trim leading whitespace */
	    if ( isspace(buffer[0]) ) { 
		for ( pch = buffer + 1; isspace(*pch) && *pch; pch++ ) ;
		memmove( buffer, pch, strlen(pch) + 1 );
	    }
	    
	    if ( buffer[0] == '\0' ) continue;
	    n_strings++;
	}
	fclose( exp_list );
    } 
    
    /* Copy lines from standard input to standard output, skipping the ones
     * which matched at least one of the loaded patterns */
    while( fgets( buffer, BUFFER_LEN, stdin ) ) {
	int match;
	match = FALSE;
	
	if ( buffer[strlen(buffer)-1] == '\n' ) 
	    buffer[strlen(buffer)-1]='\0';
	
	for ( i = 0; !match && i < n_strings; i++ ) {
	    match = shellexp( buffer, strings[i] );
	}
	if ( !match ) {
	    if (printf("%s\n", buffer) < 0)
            {
                free(strings);
	    	return EXIT_FAILURE;
            }
	} 
    }
    free(strings);
    if (fclose(stdout) != 0)
    	return EXIT_FAILURE;
    return EXIT_SUCCESS;
}
