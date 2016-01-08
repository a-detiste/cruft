#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "river.h"

/*
 * Please note that the adjective smaller/smallest in this program does not
 * have anything to do with its size. Instead it means "a river/stream, whose
 * line to be read on input compares as lexicographically smaller than lines to
 * be read from other rivers/streams".
 */

static void scroll_smallest(int *smallest, river *rivers)
{
	unsigned int idx;
	for (idx = 0; idx < NUM_RIVERS; idx++)
		if (smallest[idx])
			river_scroll(&rivers[idx]);
}

static int at_least_one_is(int *smallest)
{
	unsigned int idx;
	for (idx = 0; idx < NUM_RIVERS; idx++)
		if (smallest[idx])
			return 1;
	return 0;
}

static void debug_current_state(int *smallest, river *rivers)
{
	char *line = "?";
	unsigned int idx;
	for (idx = 0; idx < NUM_RIVERS; idx++) {
		cruft_debug("%c", smallest[idx] ? '*' : '-');
		if (smallest[idx])
			line = river_least_line(&rivers[idx]);
	}
	cruft_debug(" %s (", line);
	for (idx = 0; idx < NUM_RIVERS; idx++) {
		cruft_debug("%s%s", river_least_line(&rivers[idx]), idx == NUM_RIVERS - 1 ? "" : " ");
	}
	cruft_debug(")\n");
}

/*
 * Finds out which river(s) in rivers[] have the smallest line(s) on input and
 * sets matching elements of smallest[] appropriately.
 */
static void pick_smallest_rivers(int *smallest, river *rivers)
{
	int prev = -1;
	int cur;
	for (cur = 0; cur < NUM_RIVERS; cur++) {
		int smaller = rivers_compare(prev, cur, rivers);
		if (prev == smaller)
			; /* cur is larger, so nothing changes */
		else if (cur == smaller) {
			/* cur is smaller than anything before, so first wipe
			 * the table up and including the previous element */
			memset(smallest, 0, sizeof(int) * cur);
			/* and mark the new smallest one */
			smallest[cur] = 1;
			prev = smaller;
		} else
			/* prev and cur have the same line on input, so just
			 * mark cur as being smallest as well */
			smallest[cur] = 1;
			/* prev stays unchanged, as it matters in case of EOFs */
	}

	/* some assertions */
	{
		unsigned int idx;
		/* a river must not be smallest and empty at the same time */
		for (idx = 0; idx < NUM_RIVERS; idx++)
			assert(! (smallest[idx] && (! river_has_line(&rivers[idx]))));
		/* some river must be smallest, if at least one is not empty */
		assert(at_least_one_is(smallest));
	}
}

int merge_diff(char *spool_dir_name)
{
	river rivers[NUM_RIVERS] = {
		{ "file",           0, -1, { NULL }, { NULL }, "file_", "unex" },
		{ "may exist",      0, -1, { NULL }, { NULL }, "mayx_", "whte" },
		{ "must exist",     0, -1, { NULL }, { NULL }, "must_", "miss" },
		{ "must not exist", 0, -1, { NULL }, { NULL }, "msnt_", "frbn" }
	};

	if (! rivers_open_files(spool_dir_name, rivers))
		return EXIT_FAILURE;

	cruft_debug(DEBUG_HEADER);

	for(;;) {
		int smallest[NUM_RIVERS];
		memset(smallest, 0, sizeof(int) * NUM_RIVERS);

		if (! rivers_prepare_round(rivers))
			break;

		pick_smallest_rivers(smallest, rivers);

		debug_current_state(smallest, rivers);

		/* must be, but isn't */
		if (smallest[MUST_IDX] && ! smallest[FILE_IDX])
			if (! river_output_line(&rivers[MUST_IDX]))
				return EXIT_FAILURE;

		/* is, but not explained by must nor may */
		if (smallest[FILE_IDX] && (! smallest[MUST_IDX] && ! smallest[MAYX_IDX]))
			if (! river_output_line(&rivers[FILE_IDX]))
				return EXIT_FAILURE;

		/* must not be, but is */
		if (smallest[MSNT_IDX] && smallest[FILE_IDX])
			if (! river_output_line(&rivers[MSNT_IDX]))
				return EXIT_FAILURE;

		if (smallest[MSNT_IDX]) {
			if (smallest[MAYX_IDX])
				fprintf(stderr, "Explain script (may and mustn't) conflict: [%s]\n", river_least_line(&rivers[MSNT_IDX]));
			if (smallest[MUST_IDX])
				fprintf(stderr, "Explain script (must and mustn't) conflict: [%s]\n", river_least_line(&rivers[MSNT_IDX]));
		}

		scroll_smallest(smallest, rivers);
	}

	if (! rivers_close(rivers))
		return EXIT_FAILURE;

	return EXIT_SUCCESS;
}

