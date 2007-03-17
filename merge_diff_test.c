#include <stdio.h>

int merge_diff(char *spool_dir_name);

int main(int argc, char **argv) {
	if (argc < 2) {
		fprintf(stderr, "Usage: merge_diff_test <directory>\n");
		return 1;
	}
	return merge_diff(argv[1]);
}

