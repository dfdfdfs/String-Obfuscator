#include <stdio.h>
#include <ftw.h>
#include <sys/stat.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <errno.h>
#include <libgen.h>
#include <dirent.h>
#include <limits.h>
#include <math.h>

const char INDENT[] = "│  ";
const char LINE_END_INDENT[] = "├── ";
const char LAST_ITEM_INDENT[] = "└── ";
const size_t INDENT_LEN = sizeof(INDENT) / sizeof(INDENT[0]);
const size_t LINE_END_INDENT_LEN = sizeof(LINE_END_INDENT) / sizeof(LINE_END_INDENT[0]);
const size_t LAST_ITEM_INDENT_LEN = sizeof(LAST_ITEM_INDENT) / sizeof(LAST_ITEM_INDENT[0]);
const char OFF_T_LEN_MAX = 8 * sizeof(off_t) * (M_LN2 / M_LN10) + 2 + 1;
const char DATE_LEN_MAX = 8 * 3 * sizeof(int) * (M_LN2 / M_LN10) + 2 + 1;
const size_t INDENT_SIZE = PATH_MAX * INDENT_LEN + LINE_END_INDENT_LEN + LAST_ITEM_INDENT_LEN + 1;
const size_t NAME_SIZE_DATE_SIZE = FILENAME_MAX + 1 + OFF_T_LEN_MAX + 1 + DATE_LEN_MAX + 1;
const size_t BUF_SIZE = INDENT_SIZE + NAME_SIZE_DATE_SIZE; 
const int MAX_DATE_STR_SIZE = 20 + 1 + 2 + 1 + 2 + 1;

bool date_str_to_time_t(time_t* time, const char* date_str) {
	struct tm tm = {0};
	if (strptime(date_str, "%Y-%m-%d", &tm) != NULL) {
		*time = mktime(&tm);
		return true;
	}
	return false;
}

bool time_t_to_date_str(char* date_str, const time_t time) {
	return strftime(date_str, BUF_SIZE, "%Y-%m-%d", localtime(&time));
}

struct cmdopts {
	char* dir;
	FILE* outfile;
	off_t min_size, max_size;
	time_t min_ctime, max_ctime;
} options = {NULL, NULL, -1, -1, -1, -1};

int parse_cmdopts(char** argv) {
	options.dir = argv[1];
	options.outfile = fopen(argv[2], "w");

	errno = 0;
	char* leftover = NULL;
	options.min_size = strtol(argv[3], &leftover, 10);
	if (leftover == NULL || *leftover != '\0' || errno != 0) {
		perror("Invalid min size");
		return -1;
	}
	
	errno = 0;
	options.max_size = strtol(argv[4], &leftover, 10);
	if (leftover == NULL || *leftover != '\0' || errno != 0) {
		perror("Invalid max size");
		return -2;
	} else if (options.max_size < options.min_size) {
		perror("Maximum size can't be smaller than minimum size");
		return -21;
	}
	
	errno = 0;
	if(!date_str_to_time_t(&options.min_ctime, argv[5])) {
		perror("Invalid min creation date");
		return -3;
	}
	
	errno = 0;
	if(!date_str_to_time_t(&options.max_ctime, argv[6])) {
		perror("Invalid max creation date");
		return -4;
	} else if (options.max_ctime < options.min_ctime) {
		printf("%li\t%li", options.min_ctime, options.max_ctime);
		perror("Maximum creation time can't be smaller than minimum");
		return -41;
	}
	return 0;
}	

bool check_file(const struct stat* stat) {
	return	(S_ISREG(stat->st_mode)) && //is regular file
			(stat->st_size >= options.min_size) &&
			(stat->st_size <= options.max_size) &&
			(stat->st_ctime >= options.min_ctime) &&
			(stat->st_ctime <= options.max_ctime);
}

bool is_nonempty_dir(const char* dirname) {
	DIR *dir = opendir(dirname);
	if (dir == NULL) { // Not a directory or doesn't exist
		return false;
	}
	int n = 0;
	struct dirent* direntry = NULL;
	while (((direntry = readdir(dir)) != NULL) && (n <= 2)) {
		++n;
	}
	closedir(dir);
	return (n > 2); // . and ..
}

void print_indent(const char* line_end_indent, int count) {
	for (int i = 0; i < count - 1; ++i) {
		fprintf(options.outfile, "%s", INDENT);
		printf("%s", INDENT);
	}
	fprintf(options.outfile, "%s", line_end_indent);
	printf("%s", line_end_indent);
}

int print_entry(const char* path, const struct stat* stat,
			   int info, struct FTW* ftw) {
	static int last_level = 0;
	// print empty line after the last file in directory
	if (ftw->level < last_level) {
		print_indent(INDENT, ftw->level);
		fprintf(options.outfile, "\n");
		printf("\n");
	}
	bool is_non_empty_dir = S_ISDIR(stat->st_mode) && is_nonempty_dir(path);
	bool is_good_file = check_file(stat);
	if (is_non_empty_dir || is_good_file) {
		print_indent(LINE_END_INDENT, ftw->level);
		if (is_good_file) {
			char date_str[MAX_DATE_STR_SIZE];
			if (!time_t_to_date_str(date_str, stat->st_ctime)) {
				perror("Failed to convert time_t to string");
				return -1;
			}
			fprintf(options.outfile, "%s\t%zi\t%s\n",
					path + ftw->base, stat->st_size, date_str);
			printf("%s\t%zi\t%s\n", path + ftw->base, stat->st_size, date_str);
		} else {
			fprintf(options.outfile, "%s\n", path + ftw->base);
			printf("%s\n", path + ftw->base);
		}
	}
	last_level = ftw->level;
	return 0;
}

int main(int argc, char** argv) {
	switch (argc) {
		case 1: perror("Directory name is missing"); return -1;
		case 2: perror("Output file name is missing"); return -2;
		case 3: perror("Min size is missing"); return -3;
		case 4: perror("Max size is missing"); return -4;
		case 5: perror("Min time is missing"); return -5;
		case 6: perror("Max time is missing"); return -6;
	}
	int code = parse_cmdopts(argv);
	if (code) {
		return code;
	}
	errno = 0;
	int fd_limit = 15;
	int flags = 0;
	if (nftw(options.dir, print_entry, fd_limit, flags) == -1) {
		strerror(errno);
		return -1;
	}
	fclose(options.outfile);
	return 0;
}
