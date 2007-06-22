#include <stdio.h>
#include "merge_diff_common.h"

typedef struct fn_stream {
    FILE* file;
    char  line[1000];
} fn_stream;

void next_line( fn_stream* s );
fn_stream* init_fn_stream(char *filename);

