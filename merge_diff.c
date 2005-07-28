#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>

#define TRUE (1)
#define FALSE (0)

typedef struct fn_stream {
    FILE* file;
    char  line[1000];
} fn_stream;

typedef struct collection {
    fn_stream* s[100];
    int        n;
} collection;

void next_line( fn_stream* s ) {
    assert(s != NULL);
    assert(s->file != NULL);

    if ( NULL == fgets( s->line, 1000, s->file ) ) {
	fclose(s->file);
	s->file = NULL;
    } else {
	int x = strlen(s->line) - 1;
	if ( s->line[x] == '\n' ) s->line[x] = '\0';
    }
}

fn_stream* init_fn_stream( char* filename ) {
    fn_stream* s = malloc( sizeof( fn_stream ) );
    if ( !s ) return NULL;

    s->file = fopen( filename, "r" );
    if ( !s->file ) {
	free(s);
	return NULL;
    }

    next_line(s);

    return s;
}

int find_least( collection* c ) {
    int least = -1;
    int i;

    for ( i = 0; i < c->n; i++ ) {
	if ( NULL == c->s[i]->file ) {
	    continue;
	} else if ( least == -1 ) { 
	    least = i;
	} else {
	    int x = strcmp( c->s[least]->line, c->s[i]->line );
	    if ( x == 0 ) {
		next_line(c->s[i]);
	    } else if ( x > 0 ) {
		least = i;
	    }
	}
    }

    return least;
}  

void close_collection( collection* c ) {
    int i;
    
    for ( i = 0; i < c->n; i++ ) {
	if ( c->s[i]->file ) fclose( c->s[i]->file );
	free(c->s[i]);
    }

    c->n = 0;
}


int main(void) {
    collection expl;
    FILE*      miss[100];
    collection file;
    FILE*      unex[100];
    collection need;
    FILE*      want[100];
    int i;
    char filename[1000];

    DIR* spool_dir;
    struct dirent* pde;

    expl.n = 0;
    file.n = 0;
    need.n = 0;
  
    spool_dir = opendir( "/var/spool/cruft" );
    if ( spool_dir == NULL ) {
	perror( "/var/spool/cruft" );
	return 1;
    }

    chdir( "/var/spool/cruft" );
    while( (pde = readdir( spool_dir )) ) {
	if ( strncmp( pde->d_name, "expl_", 5 ) == 0 ) {
	    strcpy( filename, pde->d_name );
	    strncpy( filename, "miss", 4 );      
	    miss[expl.n] = fopen( filename, "w"  );
	    expl.s[expl.n++] = init_fn_stream( pde->d_name );
	} else if ( strncmp( pde->d_name, "file_", 5 ) == 0 ) {
	    strcpy( filename, pde->d_name );
	    strncpy( filename, "unex", 4 );      
	    unex[file.n] = fopen( filename, "w" );
	    file.s[file.n++] = init_fn_stream( pde->d_name );
	} else if ( strncmp( pde->d_name, "need_", 5 ) == 0 ) {
	    strcpy( filename, pde->d_name );
	    strncpy( filename, "want", 4 );      
	    want[need.n] = fopen( filename, "w" );
	    need.s[need.n++] = init_fn_stream( pde->d_name );
	}
    }
    closedir(spool_dir);
    
    for(;;) {
	int least_expl = find_least( &expl );
	int least_file = find_least( &file );
	int least_need = find_least( &need );
	int x[3];
	int y;

	if ( least_expl < 0 && least_file < 0 && least_need < 0 ) break;

	x[0] = least_expl >= 0;
	x[1] = least_file >= 0;
	x[2] = least_need >= 0;

	if ( x[0] && x[1] ) {
	    y = strcmp( expl.s[least_expl]->line, file.s[least_file]->line );
	    if ( y < 0 ) {
		x[1] = FALSE;
		if ( x[2] ) {
		    y = strcmp( expl.s[least_expl]->line, 
				need.s[least_need]->line );
		    if ( y < 0 ) {
			x[2] = FALSE;
		    } else if ( y > 0 ) {
			x[0] = FALSE;
		    }
		}   
	    } else if ( y > 0 ) {
		x[0] = FALSE;
		if ( x[2] ) {
		    y = strcmp( file.s[least_file]->line, 
				need.s[least_need]->line );
		    if ( y < 0 ) {
			x[2] = FALSE;
		    } else if ( y > 0 ) {
			x[1] = FALSE;
		    }
		}
	    } else {
		if ( x[2] ) {
		    y = strcmp( expl.s[least_expl]->line, 
				need.s[least_need]->line );
		    if ( y < 0 ) {
			x[2] = FALSE;
		    } else if ( y > 0 ) {
			x[0] = FALSE;
			x[1] = FALSE;
		    }
		}
	    }
	}

	assert(x[0] || x[1] || x[2]);

	assert( !x[0] || least_expl >= 0 );
	assert( !x[1] || least_file >= 0 );
	assert( !x[2] || least_need >= 0 );


#if 0
fprintf( stderr, "%c%c%c %s (%s %s %s)\n", 
	 x[0] ? '*' : '-',
	 x[1] ? '*' : '-',
	 x[2] ? '*' : '-',
	 x[0] ? expl.s[least_expl]->line : 
	     x[1] ? file.s[least_file]->line :
                 need.s[least_need]->line,
	 least_expl >= 0 ? expl.s[least_expl]->line : "(eof)",
	 least_file >= 0 ? file.s[least_file]->line : "(eof)",
	 least_need >= 0 ? need.s[least_need]->line : "(eof)"
	 );
#endif

	if ( x[0] && !x[1] ) { /* explained, but not there */
	    fprintf( miss[least_expl], "%s\n", expl.s[least_expl]->line );
	}
	
	if ( x[1] && !x[0] ) { /* there, but not explained */
	    fprintf( unex[least_file], "%s\n", file.s[least_file]->line ); 
	}

	if ( x[2] && !x[1] ) { /* needed but not there */
	    fprintf( want[least_need], "%s\n", need.s[least_need]->line ); 
	}

	if ( x[0] ) next_line( expl.s[least_expl] );
	if ( x[1] ) next_line( file.s[least_file] );
	if ( x[2] ) next_line( need.s[least_need] );

    }

    for ( i = 0; i < expl.n; i++ ) fclose( miss[i] );
    close_collection(&expl);

    for ( i = 0; i < file.n; i++ ) fclose( unex[i] );
    close_collection(&file);

    for ( i = 0; i < need.n; i++ ) fclose( want[i] );
    close_collection(&need);

    return 0;
}



