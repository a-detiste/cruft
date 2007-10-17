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
	int ret = 0;
	if (argc != 3)
		usage();
	ret = shellexp(argv[2], argv[1]);
	if (ret == -1)
		exit(2);
	else
		exit(ret);
}
