
#define TRUE (1)
#define FALSE (0)

#define FILE_IDX (0)
#define MAYX_IDX (1)
#define MUST_IDX (2)
#define MSNT_IDX (3)

#define NUM_RIVERS (4)

/* #define DEBUG_CRUFT 2 */
#define DEBUG_HEADER	"File\n" \
			"| May exist\n" \
			"|/ Must exist\n" \
			"||/ Must NOT exist\n" \
			"|||/\n" \
			"||||\n"

void cruft_debug(const char *fmt, ...);

