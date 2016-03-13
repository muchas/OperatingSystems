#define _XOPEN_SOURCE 500
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <ftw.h>
typedef struct dirent dirent_t;
typedef struct stat  stat_t;

char *mode;


int check_mode(const stat_t *file_stat, char *mode_pattern)
{
    int i, bits[9] = {S_IRUSR, S_IWUSR, S_IWUSR, S_IRGRP, S_IWGRP, S_IXGRP,
                      S_IROTH, S_IWOTH, S_IXOTH};

    for(i=0; i<9; i+=1) {
        if(mode_pattern[i] != '-') {
            if(!(file_stat->st_mode & bits[i])) return 0;
        } else if(file_stat->st_mode & bits[i]) return 0;
    }

    return 1;
}

int is_mode_valid(char *mode)
{
    unsigned i;
    char pattern[9] = "rwxrwxrwx";
    
    if(strlen(mode) != 9) return 0;
    for(i=0; i<9; i+=1) {
        if(mode[i] != pattern[i] && mode[i] != '-') return 0;
    }
    return 1;
}


int handle_file_entry(const char *fpath, const stat_t *sb, int typeflag, struct FTW *ftwbuf)
{
    char access_time[20];

    if(typeflag != FTW_F) return 0;

    if(check_mode(sb, mode)) {
        strftime(access_time, 20, "%Y-%m-%d %H:%M:%S", localtime(&sb->st_atime));
        printf("File name: \t%s\n", fpath);
        printf("File size: \t%d bytes\n", (int)sb->st_size);
        printf("Last access: \t%s\n", access_time);
        printf("------------------------\n");            
    }

    return 0;
}

void list_dir(const char *name)
{
    int flags = 0;
    nftw(name, handle_file_entry, 20, flags);
}


int main(int argc, char** argv)
{
    char *target_directory;

    if(argc != 3) {
        printf("Invalid number of arguments\n");
        exit(1);
    }

    target_directory = strdup(argv[1]);
    mode = strdup(argv[2]);
 
    if(is_mode_valid(mode)) {
        list_dir(target_directory);
    } else{
        printf("Invalid mode argument\n");
    }

    free(target_directory);
    free(mode);
    return 0;
}