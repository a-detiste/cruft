#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdarg.h>

#define TRUE (1)
#define FALSE (0)

#define EXPL_IDX (0)
#define FILE_IDX (1)
#define NEED_IDX (2)

#define NUM_INPUT (3)
static const int contestants[] = { -1, EXPL_IDX, FILE_IDX, NEED_IDX };

#define DEBUG_CRUFT 0

typedef struct fn_stream {
    FILE* file;
    char  line[1000];
} fn_stream;

typedef struct collection {
    char*      name;
    int        n;
    int        least;
    fn_stream* in[100];
    FILE*      out[100];
    char*      input_prefix;
    char*      output_prefix;
} collection;

static void cruft_debug(const char *fmt, ...)
{
#if DEBUG_CRUFT > 1
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
#endif
}

static char *least_line(collection *c)
{
	if (c->least < 0)
		return "(eof)";
	else
		return c->in[c->least]->line;
}

static void next_line( fn_stream* s ) {
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

static void scroll(collection *c)
{
	next_line(c->in[c->least]);
}

static fn_stream* init_fn_stream( char* filename ) {
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

static int find_least( collection* c ) {
    int i;
    c->least = -1;

    for ( i = 0; i < c->n; i++ ) {
	if ( NULL == c->in[i]->file ) {
	    continue;
	} else if ( c->least == -1 ) { 
	    c->least = i;
	} else {
	    int x = strcmp( c->in[c->least]->line, c->in[i]->line );
	    if ( x == 0 ) {
		next_line(c->in[i]);
	    } else if ( x > 0 ) {
		c->least = i;
	    }
	}
    }

    return c->least;
}  

static void close_collection( collection* c )
{
	int i;

	for (i = 0; i < c->n; i++)
	{
		if (c->out[i])
			fclose(c->out[i]);
		if (c->in[i]->file)
			fclose(c->in[i]->file);
		free(c->in[i]);
	}
	c->n = 0;
}

static int compare(int champion, int challenger, collection *collections)
{
	int champion_has_lines;
	int challenger_has_lines;

	if (champion == -1) {
		/* can only win if the challenger has no more lines */
		int winner = collections[challenger].least < 0 ? champion : challenger;
		cruft_debug("\t\t\t\tFake champion, winner = %d\n", winner);
		return winner;
	}

	cruft_debug("\t\t\t\tComparing champion=%d and challenger=%d\n", champion, challenger);
	
	champion_has_lines = collections[champion].least >= 0;
	challenger_has_lines = collections[challenger].least >= 0;

#define A_TIE -2

	if (champion_has_lines && challenger_has_lines) {
		int res = strcmp(least_line(&collections[champion]), least_line(&collections[challenger]));
		cruft_debug("\t\t\t\tBoth have lines: %s\n", res < 0 ? "champion wins" : res > 0 ? "challenger wins" : "a tie");
		return res < 0 ? champion : res > 0 ? challenger : A_TIE;
	} else if (challenger_has_lines) {
		cruft_debug("\t\t\t\tOnly challenger has lines, so wins\n");
		return challenger;
	} else if (champion_has_lines) {
		cruft_debug("\t\t\t\tOnly champion has lines, so wins\n");
		return champion;
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
static void open_streams(char *d_name, collection *c)
{
	/* XXX buffer overflow */
	char filename[1000];
	strcpy(filename, d_name);
	strncpy(filename, c->output_prefix, strlen(c->output_prefix));
	c->out[c->n] = fopen(filename, "w" );
	c->in[c->n] = init_fn_stream(d_name);
	c->n++;
}

/* have all collections been read completly? */
static int any_input_has_lines(collection *input)
{
	unsigned int idx;
	int input_present = 0;
	for (idx = 0; idx < NUM_INPUT; idx++)
		if (input[idx].least >= 0)
			input_present = 1;
	return input_present;
}

static void scroll_winners(int *winners, collection *input)
{
	unsigned int idx;
	for (idx = 0; idx < NUM_INPUT; idx++)
		if (winners[idx])
			scroll(&input[idx]);
}


static void close_collections(collection *input)
{
	unsigned int idx;
	for (idx = 0; idx < NUM_INPUT; idx++)
		close_collection(&input[idx]);
}
static void prepare_round(collection *input)
{
	unsigned int idx;
	for (idx = 0; idx < NUM_INPUT; idx++)
		find_least(&input[idx]);
}

static void output_line(collection *c)
{
	fprintf(c->out[c->least], "%s\n", least_line(c));
}

static int open_files(char *spool_dir_name, collection *input)
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
		for (idx = 0; idx < NUM_INPUT; idx++) {
			collection *c = &input[idx];
			if (strncmp(pde->d_name, c->input_prefix, strlen(c->input_prefix)) == 0)
				open_streams(pde->d_name, c);
		}
	}
	
	closedir(spool_dir);
	
	return 1;
}

static void contest(int *winners, collection *input)
{
	int champion = *contestants;
	int challenger;
	unsigned int current;
	for (current = 1, challenger = contestants[current]; current < sizeof(contestants)/sizeof(int); challenger = contestants[++current]) {
		int winner = compare(champion, challenger, input);
		if (winner == champion)
			; /* nothing changes */
		else if (winner == challenger) {
			memset(winners, 0, sizeof(int) * NUM_INPUT);
			winners[challenger] = 1;
			champion = winner;
		} else
			/* a tie */
			winners[challenger] = 1;
	}

	/* some assertions */
	{
		unsigned int idx;
		assert(winners[EXPL_IDX] || winners[FILE_IDX] || winners[NEED_IDX]);
		for (idx = 0; idx < NUM_INPUT; idx++)
			assert(! (winners[idx] && input[idx].least < 0));
	}
}

int merge_diff(char *spool_dir_name)
{
	collection input[NUM_INPUT] = {
		{ "explain", 0, -1, { NULL }, { NULL }, "expl_", "miss" },
		{ "file",    0, -1, { NULL }, { NULL }, "file_", "unex" },
		{ "need",    0, -1, { NULL }, { NULL }, "need_", "want" }
	};

	if (! open_files(spool_dir_name, input))
		return 1;
	
	cruft_debug("Expl\n" "| File\n" "|/ Need\n" "||/\n" "|||\n");

	for(;;) {
		int winners[NUM_INPUT] = { 0, 0, 0 };
		prepare_round(input);
		
		if (! any_input_has_lines(input))
			break;

		contest(winners, input);

		cruft_debug("%c%c%c %s (%s %s %s)\n",
			winners[EXPL_IDX] ? '*' : '-',
			winners[FILE_IDX] ? '*' : '-',
			winners[NEED_IDX] ? '*' : '-',
			winners[EXPL_IDX] ? least_line(&input[EXPL_IDX]) : 
				winners[FILE_IDX] ? least_line(&input[FILE_IDX]) : least_line(&input[NEED_IDX]),
			least_line(&input[EXPL_IDX]),
			least_line(&input[FILE_IDX]),
			least_line(&input[NEED_IDX])
		);
	
		if (winners[EXPL_IDX] && ! winners[FILE_IDX]) /* explained, but not there */
			output_line(&input[EXPL_IDX]);
		
		if (winners[FILE_IDX] && ! winners[EXPL_IDX]) /* there, but not explained */
			output_line(&input[FILE_IDX]);
	
		if (winners[NEED_IDX] && ! winners[FILE_IDX]) /* needed but not there */
			output_line(&input[NEED_IDX]);
	
		scroll_winners(winners, input);
	}
	
	close_collections(input);

	return 0;
}

