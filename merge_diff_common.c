#include <stdio.h>
#include <stdarg.h>
#include "merge_diff_common.h"

void cruft_debug(const char *fmt, ...)
{
#if DEBUG_CRUFT > 1
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
#endif
}

