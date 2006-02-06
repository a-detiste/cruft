#include <stdio.h>
#include <stdlib.h>

int shellexp( char* string, char* pattern );

void usage()
{
	printf("Usage: shellexptest <pattern> <string>\n");
	exit(1);
}

int main(int argc, char **argv)
{
	if (argc != 3)
		usage();
	exit(shellexp(argv[2], argv[1]));
}
