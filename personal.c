#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <memory.h>
#include <ftw.h>

bool compare_files(const char* filename1, const char* filename2) {
	FILE* file1 = fopen(filename1, "r");
	FILE* file2 = fopen(filename2, "r");
	fseek(file1, 0, SEEK_END);

	long file1_size = ftell(file1);
	rewind(file1);
	fseek(file2, 0, SEEK_END);
	
	long file2_size = ftell(file2);
	rewind(file2);
	if (file1_size != file2_size) {
		printf("File sizes differ, %li vs. %li\n", file1_size, file2_size);
		return false;
	}

	char tmp1, tmp2;
	for (int i = 0; i < file1_size; ++i) {
		fread(&tmp1, 1, 1, file1);
		fread(&tmp2, 1, 1, file2);
		if (tmp1 != tmp2) {
			printf("%x: tmp1 0x%x != tmp2 0x%x\n", i, tmp1, tmp2);
			return false;
		}
	}
	return true;
}

int file_count = 0;

int count_file(const char* path, const struct stat* stat,
			   int info, struct FTW* ftw) {
	if (S_ISREG(stat->st_mode)) {
		++file_count;
	}
	return 0;
}

int count_files_nftw(const char* dirname) {
	const int FD_LIMIT = 15;
	const int FLAGS = 0;
	errno = 0;
	file_count = 0;
	if (nftw(dirname, count_file, FD_LIMIT, FLAGS) == -1) {
		perror("nftw error");
		return errno;
	}
	printf("nftw: %i\n", file_count);
	return file_count;
}

void getlist(int* _i, char** _str, char* dirn, int* _fsize) {
	DIR* dirs;
	dirs = opendir(dirn);
	if (dirs) {
		struct dirent* dirstr;
		while ((dirstr = readdir(dirs))) {
			char* fnm = dirstr->d_name;
			char* fullnm = (char*)alloca(strlen(dirn) + strlen(fnm) + 1);
			strcpy(fullnm, dirn);
			strcat(fullnm, "/");
			strcat(fullnm, fnm);
			struct stat fst;
			stat(fullnm, &fst);
			if (S_ISREG(fst.st_mode) && !S_ISLNK(fst.st_mode)) {
				strcpy(_str[*_i], fullnm);//копирование полного имени файла в массив
				*(_fsize+*_i) = fst.st_size;//заполняем массив размера файла
				(*_i)++;
			}
			if (S_ISDIR(fst.st_mode) &&
				strcmp(fnm, ".") &&
				strcmp(fnm, "..") &&
				!S_ISLNK(fst.st_mode)) {
				getlist(_i, _str, fullnm, _fsize);
			}
		}
		closedir(dirs);
	}
	else printf("Couldn't open directory - %s\n",dirn);
}

int xfork(const char* fnm1, const char* fnm2, int* _fs) {
	int ppid = fork();
	if (ppid == 0) {
		FILE* fd1 = fopen(fnm1,"r");
		FILE* fd2 = fopen(fnm2,"r");
		int t = 0;
		while (abs(fgetc(fd1)) == fgetc(fd2)) ++t;
		printf("\n_____ PID: %d ____\n1> %s\n2> %s\n === Result: %d of %d bytes match ===\n", getpid(), fnm1, fnm2, t, *_fs);
		fclose(fd1);
		fclose(fd2);
	}
	return ppid;
}

int main(int argc, char** argv) {
	switch (argc) {
		case 0: perror("Something really nasty happened!!!"); return -1;
		case 1: perror("Missing dir1"); return -2;
		case 2: perror("Missing dir2"); return -3;
		case 3: perror("Missing max process count"); return -4;
	}

	errno = 0;
	int N = 0;
	if(!sscanf(argv[3], "%d", &N)) {
		perror("Not whole number");
		return errno;
	}

	errno = 0;
	int ksize1 = count_files_nftw(argv[1]);
	int ksize2 = count_files_nftw(argv[2]);
	printf("Total files found: %d ", ksize1 + ksize2);
	
	//Memory allocation
	char** str1 = malloc(ksize1 * sizeof(char*));
	char** str2 = malloc(ksize2 * sizeof(char*));
	int* fsize1 = malloc(ksize1 * sizeof(int));
	int* fsize2 = malloc(ksize2 * sizeof(int));
	for (int i = 0; i < ksize1; ++i) {
		str1[i] = malloc(512 * sizeof(char));
	}
	for (int i = 0; i < ksize2; ++i) {
		str2[i] = malloc(512 * sizeof(char));
	}

	ksize1 = 0;
	ksize2 = 0;
	getlist(&ksize1, str1, argv[1], fsize1);
	getlist(&ksize2, str2, argv[2], fsize2);
	printf("Total files read: %d\n",ksize1 + ksize2);

	int fr = 1; //fork result
	int n = 0; // переменные счетчики
	for (int i = 0; i < ksize1; ++i) {
		for (int k = 0; k < ksize2; ++k) {
			if (fr > 0) {
				if (fsize1[i] == fsize2[k]) {
					if (n++ > N) {
						wait(NULL);
					}
					fr = xfork(str1[i], str2[k], fsize1 + i);
				}
			} else {
				break;
			}
		}
	}
	if (fr > 0) {
		while (wait(NULL) > 0);
		printf("\nErrno result: %s \n", strerror(errno));
	}

	// Free memory
	for (int i = 0; i < ksize1; ++i) {
		free(str1[i]);
	}
	for (int i = 0; i < ksize2; ++i) {
		free(str2[i]);
	}
	free(str1);
	free(str2);
	free(fsize1);
	free(fsize2);
	return 0;
}
