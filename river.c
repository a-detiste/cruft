#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include "river.h"

int river_has_line(river *r)
{
	if (r->least >= 0)
		return 1;
	else
		return 0;
}

char *river_least_line(river *r)
{
	if (river_has_line(r))
		return r->in[r->least]->line;
	else
		return "(eof)";
}

void river_scroll(river *r)
{
	next_line(r->in[r->least]);
}

static int river_find_least(river *r)
{
	int i;
	
	r->least = -1;

	for ( i = 0; i < r->n; i++ ) {
		if ( NULL == r->in[i]->file ) {
			continue;
		} else if ( r->least == -1 ) { 
			r->least = i;
		} else {
			int x = strcmp( r->in[r->least]->line, r->in[i]->line );
			if ( x == 0 ) {
				next_line(r->in[i]);
			} else if ( x > 0 ) {
				r->least = i;
			}
		}
	}
	return r->least;
}  

static int river_close(river* r)
{
	int i;

	for (i = 0; i < r->n; i++)
	{
		if (r->out[i])
			if (fclose(r->out[i]) != 0)
				return 0;
		if (r->in[i]->file)
			if (fclose(r->in[i]->file) != 0)
				return 0;
		free(r->in[i]);
	}
	r->n = 0;
	return 1;
}

int rivers_close(river *rivers)
{
	unsigned int idx;
	for (idx = 0; idx < NUM_RIVERS; idx++)
		if (! river_close(&rivers[idx]))
			return 0;
	return 1;
}

/*
 * Compares two rivers, and returns the offset of the smaller one.
 *
 * Also understands the concept of a "fake prev. river", with index=-1, which
 * only compares as smaller than dry rivers (whose inputs is all EOF).  This
 * makes the pick_smallest_rivers algorithm a bit more straightforward.
 */
int rivers_compare(int prev_idx, int cur_idx, river *rivers)
{
	int prev_has_lines;
	int cur_has_lines;

	/* the "fake river" can only be considered smaller, if the other one is
	 * at EOF (has no more lines) */
	if (prev_idx == -1) {
		int smaller = river_has_line(&rivers[cur_idx]) ? cur_idx : prev_idx;
		cruft_debug("\t\t\t\tFake prev, smaller= %d\n", smaller);
		return smaller;
	}

	cruft_debug("\t\t\t\tComparing prev_idx=%d and cur_idx=%d\n", prev_idx, cur_idx);
	
	prev_has_lines = river_has_line(&rivers[prev_idx]);
	cur_has_lines = river_has_line(&rivers[cur_idx]);

#define A_TIE -2

	if (prev_has_lines && cur_has_lines) {
		int res = strcmp(river_least_line(&rivers[prev_idx]), river_least_line(&rivers[cur_idx]));
		cruft_debug("\t\t\t\tBoth have lines: %s\n", res < 0 ? "prev is smaller" :
				res > 0 ? "cur is smaller" : "and are equal");
		return res < 0 ? prev_idx : res > 0 ? cur_idx : A_TIE;
	} else if (cur_has_lines) {
		cruft_debug("\t\t\t\tOnly cur has lines, so wins\n");
		return cur_idx;
	} else if (prev_has_lines) {
		cruft_debug("\t\t\t\tOnly prev has lines, so wins\n");
		return prev_idx;
	} else {
		cruft_debug("\t\t\t\tNone have lines, a tie\n");
		return A_TIE;
	}
}



/*
 * Opens an input file for reading, and an output file (with a changed prefix)
 * for writing.
 * TODO: because of how the filename is transformed, we are currently limited
 * to 4-char prefixes for both input and output filenames.
 */
static int open_streams(char *d_name, river *r)
{
	/* XXX buffer overflow */
	char filename[1000];
	strcpy(filename, d_name);
	strncpy(filename, r->output_prefix, strlen(r->output_prefix));
	if ((r->out[r->n] = fopen(filename, "w")) == NULL) {
		perror(filename);
		return 0;
	}
	if ((r->in[r->n] = init_fn_stream(d_name)) == NULL) {
		fclose(r->out[r->n]);
		return 0;
	}
	r->n++;
	return 1;
}

int river_output_line(river *r)
{
	int ret = fprintf(r->out[r->least], "%s\n", river_least_line(r));
	if (ret < 0) {
		perror("fprintf");
		return 0;
	} else
		return 1;
}

int rivers_open_files(char *spool_dir_name, river *rivers)
{
	DIR* spool_dir;
	struct dirent* pde;

	spool_dir = opendir( spool_dir_name );
	
	if ( spool_dir == NULL ) {
		perror( spool_dir_name );
		return 0;
	}

	if (chdir(spool_dir_name) != 0) {
		perror(spool_dir_name);
		return 0;
	}

	while( (pde = readdir( spool_dir )) ) {
		unsigned int idx;
		for (idx = 0; idx < NUM_RIVERS; idx++) {
			river *r = &rivers[idx];
			if (strncmp(pde->d_name, r->input_prefix, strlen(r->input_prefix)) == 0)
				if (! open_streams(pde->d_name, r))
					return 0;
		}
	}
	
	closedir(spool_dir);
	
	return 1;
}

/* Find the smallest line within each river (among its streams) and return true
 * if there is input available in at least one of the rivers */
int rivers_prepare_round(river *rivers)
{
	int ret = 0;
	unsigned int idx;
	for (idx = 0; idx < NUM_RIVERS; idx++)
		if (river_find_least(&rivers[idx]) >= 0)
			ret = 1;
	return ret;
}

