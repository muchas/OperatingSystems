#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
typedef struct dirent dirent_t;
typedef struct stat  stat_t;


int check_mode(stat_t *file_stat, char *mode_pattern)
{
    int i, bits[9] = {S_IRUSR, S_IWUSR, S_IXUSR, S_IRGRP, S_IWGRP, S_IXGRP,
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


void list_dir(const char *name, char* mode_pattern)
{
    DIR *dir;
    dirent_t *ent;
    char *entry_path, access_time[20];
    stat_t entry_stat;

    dir = opendir(name);

    if(dir == NULL) {
        printf("Invalid directory name\n");
        return;
    }

    while(ent = readdir(dir)) {
        if(!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, "..")) continue;

        entry_path = malloc(strlen(name) + strlen(ent->d_name) + 2);
        sprintf(entry_path, "%s/%s", name, ent->d_name);
        stat(entry_path, &entry_stat);

        if(S_ISREG(entry_stat.st_mode) && check_mode(&entry_stat, mode_pattern)) {
            strftime(access_time, 20, "%Y-%m-%d %H:%M:%S", localtime(&entry_stat.st_atime));
            printf("File name: \t%s\n", entry_path);
            printf("File size: \t%d bytes\n", (int)entry_stat.st_size);
            printf("Last access: \t%s\n", access_time);
            printf("------------------------\n");            
        }

        if(S_ISDIR(entry_stat.st_mode)) {
            list_dir(entry_path, mode_pattern);
        }

        free(entry_path);
    }

    closedir(dir);
}


int main(int argc, char** argv)
{
    char *target_directory, *mode;
    DIR* handler;

    if(argc != 3) {
        printf("Invalid number of arguments\n");
        exit(1);
    }

    target_directory = strdup(argv[1]);
    mode = strdup(argv[2]);
 
    if(is_mode_valid(mode)) {
        list_dir(target_directory, mode);
    } else{
        printf("Invalid mode argument\n");
    }

    free(target_directory);
    free(mode);
    return 0;
}