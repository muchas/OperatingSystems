#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/times.h>
typedef struct dirent dirent_t;


const char *get_filename_ext(const char *filename) {
    const char *dot = strrchr(filename, '.'); // first occurrence of dot
    if(!dot || dot == filename) return "";
    return dot + 1;
}


int collect_children_file_count()
{
    int status, file_count;

    file_count = 0;

    while(wait(&status) > 0) {
        // increase count if child terminated normally (return, exit or _exit)
        if(WIFEXITED(status) && WEXITSTATUS(status) != -1) {
            file_count += WEXITSTATUS(status);
        }
    }

    return file_count;
}


void execute_child_process(char* path, char* argv[])
{
    pid_t process_id;

    setenv("PATH_TO_BROWSE", path, 1);

    process_id = fork(); // child process extends: session id, current workspace, bitmask, system resource limits...

    if(process_id == 0) {
        // filename is looked up in PATH env var
        execvp(argv[0], argv); // (const char* file, const char* arg, ...)
        fprintf(stderr, "exec error\n");
        _exit(0);
    } else if(process_id < 0) {
        fprintf(stderr, "fork error\n");
    }
}


void run(char *path, char* argv[], int display, int wait, char* ext)
{
    int file_count, children_file_count;
    char *entry_path;
    DIR *dir;
    dirent_t *entry;

    file_count = 0;
    dir = opendir(path);

    if(dir == NULL) {
        perror("Invalid directory name\n");
        exit(-1);
    }

    while(entry = readdir(dir)) {
        if(!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")) continue;

        if(entry->d_type == DT_DIR) {
            entry_path = malloc(strlen(path) + strlen(entry->d_name) + 2);
            sprintf(entry_path, "%s/%s", path, entry->d_name);

            execute_child_process(entry_path, argv);

            free(entry_path);
        } else if(entry->d_type == DT_REG) {
            if(ext == NULL || strcmp(get_filename_ext(entry->d_name), ext) == 0) {
                file_count += 1;
            }
        }
    }

    if(wait) sleep(15);

    children_file_count = collect_children_file_count();

    if(display) {
        printf("Directory: %s \nFile count: %d\nChildren file count: %d\n-----------\n", path, file_count, children_file_count);
    }

    closedir(dir);
    exit(file_count + children_file_count);
}


int main(int argc, char *argv[])
{
    char *dir, *ext;
    int display, wait, opt;

    display = 0;
    wait = 0;

    dir = getenv("PATH_TO_BROWSE");
    ext = getenv("EXT_TO_BROWSE");

    while ((opt = getopt(argc, argv, "vw")) != -1) {
        switch (opt) {
            case 'v': display = 1; break;
            case 'w': wait = 1; break;
        }
    }

    if(dir == NULL || strlen(dir) == 0) {
        dir = ".";
    }

    run(dir, argv, display, wait, ext);

    return 0;
}