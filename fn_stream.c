#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "fn_stream.h"

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

fn_stream* init_fn_stream(char *filename)
{
	fn_stream* s = malloc(sizeof(fn_stream));
	if (! s) {
		perror("malloc");
		return NULL;
	}
	
	if ((s->file = fopen(filename, "r")) == NULL) {
		free(s);
		perror(filename);
		return NULL;
	}

	next_line(s);
	return s;
}

