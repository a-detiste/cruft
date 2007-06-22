
int merge_diff(char *spool_dir_name);

int main(void) {
	return merge_diff("/var/spool/cruft");
}

