#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

int lock_test(int fd, int type, off_t offset, int whence, off_t len)
{
    struct flock lock;
    lock.l_type = type;
    lock.l_start = offset;
    lock.l_whence = whence;
    lock.l_len = len;

    if (fcntl(fd, F_GETLK, &lock) < 0) {
        printf("fcntl error");
        return 0;
    }
    
    if (lock.l_type == F_UNLCK)
        return 0; 

    return(lock.l_pid); 
}

int lock_reg(int fd, int cmd, int type, off_t offset, int whence, off_t len)
{
    struct flock lock;

    lock.l_type = type;
    lock.l_start = offset;
    lock.l_whence = whence;
    lock.l_len = len;

    return fcntl(fd, cmd, &lock);
}

void list_locks(int fd)
{
    size_t length;
    struct flock lock;
    int i;

    lock.l_start = 0;
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_len = 1;

    length = lseek(fd, 0, SEEK_END) + 1;

    for(i=1; i<length; i+=1) {
        if(fcntl(fd, F_GETLK, &lock) < 0) {
            printf("fctnl GET LOCK error\n");
        }

        if(lock.l_type != F_UNLCK) {
            printf("PID: \t%d\n", lock.l_pid);
            printf("Byte: \t%d\n", (int) lock.l_start);
            printf("Type: \t%s\n", lock.l_type == F_WRLCK ? "write" : "read");
            printf("-----------------------\n");
        }

        lock.l_start = i;
        lock.l_type = F_WRLCK;
    }
}

void set_write_lock(int fd, off_t offset)
{
    if(lock_reg(fd, F_SETLK, F_WRLCK, offset, SEEK_SET, 1) < 0) {
        printf("write_lock error\n");
    } else {
        printf("Byte %lld locked succesfully\n", (long long) offset);
    }
}

void set_read_lock(int fd, off_t offset)
{
    if(lock_reg(fd, F_SETLK, F_RDLCK, offset, SEEK_SET, 1) < 0) {
        printf("read_lock error\n");
    } else {
        printf("Byte %lld locked succesfully\n", (long long) offset);
    }
}

void unlock(int fd, off_t offset)
{
    if(lock_reg(fd, F_SETLK, F_UNLCK, offset, SEEK_SET, 1) < 0) {
        printf("unlock error\n");
    } else {
        printf("Byte %lld unlocked succesfully\n", (long long) offset);
    }
}

void print_character(int fd, off_t offset)
{
    char character;
    int read_bytes;

    if(lock_test(fd, F_RDLCK, offset, SEEK_SET, 1) != 0) {
        printf("Cannot read. Character is locked.\n");
        return;
    }


    lseek(fd, offset, SEEK_SET);
    read_bytes = read(fd, &character, 1);
    if(read_bytes == 1) {
        printf("Read character: %c\n", character);
    } else {
        printf("Character could not be read\n");
    }
}

void write_character(int fd, off_t offset, char character)
{
    int written_bytes;

    if(lock_test(fd, F_WRLCK, offset, SEEK_SET, 1) != 0) {
        printf("Cannot write. Character is locked.\n");
        return;
    }

    lseek(fd, offset, SEEK_SET);
    written_bytes = write(fd, &character, 1);
    printf("written bytes: %d and character: %c \n", written_bytes, character);
    fsync(fd);
    if(written_bytes == 1) {
        printf("Character written succesfully\n");
    } else {
        printf("Character could not be written\n");
    }
}


void run_interactive_mode(int fd)
{
    long long offset;
    char cmd, character;

    while(1) {
        printf("Available commands: w (set write lock), r (set read lock), u (unlock), l (list locks), c (write character), p (print character), q (quit)\n");
        scanf(" %c", &cmd);

        switch(cmd) {
            case 'w':
                printf("Byte number: ");
                scanf("%lld", &offset);
                set_write_lock(fd, offset);
                break;
            case 'r':
                printf("Byte number: ");
                scanf("%lld", &offset);
                set_read_lock(fd, offset);
                break;
            case 'u':
                printf("Byte number: ");
                scanf("%lld", &offset);
                unlock(fd, offset);
                break;
            case 'l':
                list_locks(fd);
                break;
            case 'c':
                printf("Byte number: ");
                scanf("%lld", &offset); 
                printf("Character: ");
                scanf(" %c", &character);
                write_character(fd, offset, character);               
                break;
            case 'p':
                printf("Byte number: ");
                scanf("%lld", &offset);
                print_character(fd, offset);
                break;
            case 'q':
                printf("Bye!\n");
                return;
            default:
                printf("Invalid command\n");
        }
    }
}


int main(int argc, char **argv)
{
    int fd;
    char *filename;

    if(argc != 2) {
        printf("Invalid number of arguments\n");
        exit(EXIT_FAILURE);
    }

    filename = strdup(argv[1]);
    fd = open(filename, O_RDWR | O_CREAT, 0666);

    if(fd == -1) {
        printf("Invalid filename\n");
    } else {
        run_interactive_mode(fd);
    }

    free(filename);
    close(fd);

    return 0;
}