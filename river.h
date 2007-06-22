#include "fn_stream.h"

/*
 * A "river" is a bunch of (I/O) streams. It also has some additional data
 * which go with it.
 */
typedef struct river {
	/* Just a descriptive name */
	char*      name;
	/* Number of input streams open. The number of output streams open is
	 * the same. */
	int        n;
	/* The offset of the input stream whose line to be read compares as
	 * lexicographically smallest to lines of other input streams in this
	 * river.
	 */
	int        least;
	/* Input streams. */
	fn_stream* in[100];
	/* Output streams. */
	FILE*      out[100];
	/* The prefix of filenames which will be read by this river. */
	char*      input_prefix;
	/* The prefix of filenames which will be written by this river. */
	char*      output_prefix;
	/* NOTE: the length of both prefixes must be the same. See
	 * open_streams(). */
} river;

int rivers_open_files(char *spool_dir_name, river *rivers);
int rivers_prepare_round(river *rivers);
void river_scroll(river *r);
char *river_least_line(river *r);
int river_has_line(river *r);
int rivers_compare(int prev, int cur, river *rivers);
int river_output_line(river *r);
int rivers_close(river *rivers);

