#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdlib.h>
#include <unistd.h>

#define FALSE 0
#define TRUE  1

int shellexp(char*, char*);

int main(int argc, char **argv) {
    char filename[1000];
    char buffer[1000];
    int i;
    DIR* dpkg_info;
    struct dirent* pde;

    dpkg_info = opendir("/usr/lib/cruft/filters/" );
    if ( dpkg_info == NULL ) {
	perror( "/usr/lib/cruft/filters" );
	exit(EXIT_FAILURE);
    }

    chdir("/usr/lib/cruft/filters");
    while( (pde = readdir( dpkg_info )) ) {
	FILE* f;
	f = fopen( pde->d_name, "r" );
	if ( !f ) continue;
	while( fgets( buffer, 1000, f ) ) {
	    if ( buffer[strlen(buffer)-1] == '\n' )
		buffer[strlen(buffer)-1] = '\0';
	    
	    for ( i = 1; i < argc; i++ ) {
		/* printf( "shellexp( \"%s\", \"%s\" )\n", argv[i], buffer );*/
		if ( shellexp( argv[i], buffer ) ) {
		    strcpy( filename, pde->d_name );
		    if (printf("%s: %s\n", filename, argv[i]) < 0)
		    	return EXIT_FAILURE; 
		}
	    }
	}
	fclose(f);
    }
    closedir( dpkg_info );
    if (fclose(stdout) != 0)
    	return EXIT_FAILURE;
    return EXIT_SUCCESS;
}
