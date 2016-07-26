#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define FILE_NAME "test_file.txt"
#define LINK_NAME "test_file.link"
#define TEST_STRING "This is a test of file io.\nThis file should be deleted.\n"
#define LEN 56
#define CWD_BUF_LEN 127

int delete_file(char * filename) {
    if (unlink(filename) == -1) {
        fprintf(stderr, "Could not remove file %s.\n", filename);
        return 0;
    }

    FILE * fp = fopen(filename, "r");
    if (fp != NULL) {
        fprintf(stderr, "File was not actually removed%s.\n", filename);
        fclose(fp);
        return 0;
    }
    return 1;
}

int verify_file(char * filename) {

    FILE * fp = fopen(filename, "r");
    char buf[LEN+1];

    if (fp == NULL) {
        fprintf(stderr, "verify_file: Could not open file: %s", filename);
        return 0;
    }

    buf[LEN] = '\0';
    fread(buf, 1, LEN, fp);
    fclose(fp);

    if (strcmp(TEST_STRING, buf)) {
        fprintf(stderr, "String was not saved/loaded from the test file.\n");
        return 0;
    }
    return 1;
}

int main(int argc, char ** argv)
{
    if (argc != 2) {
        fprintf(stderr, "%s: missing required argument\n\n", argv[0]);
        fprintf(stderr, "Usage: %s DIR\n", argv[0]);
        fprintf(stderr, "Test io & inode capabilities in directory DIR.\n");
        return 1;
    }

    char old_cwd[CWD_BUF_LEN];
    if (getcwd(old_cwd, CWD_BUF_LEN) == NULL) {
        fprintf(stderr, "Could not get CWD.\n");
        return 2;
    }

    if (chdir(argv[1]) == -1) {
        fprintf(stderr, "Chdir failed.\n");
        return 2;
    }

    char cwd[CWD_BUF_LEN];
    if (! strcmp(getcwd(cwd, CWD_BUF_LEN), old_cwd)) {
        printf("'%s'\n", cwd);
        printf("'%s'\n", argv[1]);
        fprintf(stderr, "Could not change directory.\n");
        return 2;
    }

    FILE * fp = fopen(FILE_NAME, "w");
    if (!fp) {
        fprintf(stderr, "Cannot open file for writing.\n");
        return 2;
    }

    fputs(TEST_STRING, fp);

    fclose(fp);

    struct stat st;
    if (stat(FILE_NAME, &st) == -1) {
        fprintf(stderr, "`stat' failed.\n");
        return 2;
    }
    if (st.st_size != LEN) {
        fprintf(stderr, "`stat' reported the wrong size.\n");
        return 2;
    }

    fp = fopen(FILE_NAME, "r");
    if (!fp) {
        fprintf(stderr, "Cannot open file for reading.\n");
        return 2;
    }

    struct stat fst;
    if (fstat(fileno(fp), &fst) == -1) {
        fprintf(stderr, "`fstat' failed.\n");
        return 2;
    }

    if (fseek(fp, 0, SEEK_END) == -1) {
        fprintf(stderr, "Seeking to the end of the file failed.\n");
        return 2;
    }

    long size = ftell(fp);
    fclose(fp);

    if (size == -1) {
        fprintf(stderr, "Could not determe the current position in the file.\n");
        return 2;
    } else if (size != LEN) {
        fprintf(stderr, "Current position in the file is wrong.\n");
        return 2;
    }


    verify_file(FILE_NAME);

    if (link(FILE_NAME, LINK_NAME) == -1) {
        fprintf(stderr, "Could not hard link files.\n");
        return 2;
    }
    if (! verify_file(LINK_NAME)) return 2;
    if (! delete_file(LINK_NAME)) return 2;

    if (symlink(FILE_NAME, LINK_NAME) == -1) {
        fprintf(stderr, "Could not symlink files.\n");
        return 2;
    }
    if (! verify_file(LINK_NAME)) return 2;
    if (! delete_file(LINK_NAME)) return 2;

    // Remove all permissions from the file.
    if (chmod(FILE_NAME, 0) == -1) {
        fprintf(stderr, "Could not change permissions of file.\n");
        return 2;
    }

    printf("all good.\n");

    return 0;
}
