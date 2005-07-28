#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define FALSE 0
#define TRUE  1

/* 0 on no match, non-zero on match */
int shellexp( char* string, char* pattern ) {
    /*  printf( "...matching( \"%s\", \"%s\" )\n", string, pattern ); */
    
    switch( pattern[0] ) {
    case '\0':
	return string[0] == '\0';
    case '?':
	switch( string[0] ) {
	case '\0':
	case '/':
	    return FALSE;
	default:
	    return shellexp( string+1, pattern+1 );
	}
    case '/':
	if ( pattern[1] == '*' && pattern[2] == '*' ) {
	    char* pch = string;
	    if ( *pch != '/' ) return FALSE;                
	    if ( pattern[3] == '\0' ) return TRUE;
	    if ( pattern[3] != '/' ) {
		fprintf( stderr, "Bad expression.\n" );
		return -1;
	    }
	    while ( *pch != '\0' ) {
		if ( *pch == '/' ) {
		    if ( shellexp( pch, pattern + 3 ) ) return TRUE;
		}
		pch++;
	    }
	    return FALSE;
	}
	return string[0] == '/' && shellexp( string+1, pattern+1 );
    case '*':
	if ( string[0] == '/' ) return shellexp( string, pattern+1 );
	return shellexp( string, pattern+1 ) || 
	    (string[0] != '\0' ? shellexp( string + 1, pattern ) : FALSE );
    case '[': 
	if ( string[0] == '\0' ) return FALSE;
	{
	    int not = FALSE;
	    int okay = FALSE;
	    pattern++;
	    if ( pattern[0] == '!' || pattern[0] == '^' ) {
		not = TRUE;
		pattern++;
	    }

	    if ( pattern[0] == ']' ) {
		if ( string[0] == ']' ) { okay = TRUE; }
		pattern++;
	    }
	    
	    while( pattern[0] != ']' && pattern[0] != '\0' ) {
		if ( pattern[0] == string[0] ) {
		    okay = TRUE;
		} else if ( pattern[1] == '-' && pattern[2] != ']' ) {
		    if ( pattern[0] <= string[0] && string[0] <= pattern[2] ) {
			okay = TRUE;
		    }
		    pattern += 2;
		}
		pattern++;
	    }
	    
	    if ( pattern[0] == '\0' ) {
		fprintf( stderr, "Bad shell expression\n" );
		return -1;
	    }
	    
	    return (not ? !okay : okay) && shellexp( string + 1, pattern + 1 );
	}
    default:
	return pattern[0] == string[0] && shellexp( string + 1, pattern + 1 );
    }
}

