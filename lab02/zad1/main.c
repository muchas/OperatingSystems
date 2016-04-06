#include <stdlib.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/times.h>

typedef struct tms tms_t;

typedef struct RecordArray {
    FILE* handler;
    int file_descriptor;
    size_t record_size;
} record_array_t;


int validate_input(FILE* handler, size_t record_size)
{
    if(handler == NULL) {
        printf("Invalid filename\n");
        return -1;
    }
    if(record_size > INT_MAX) {
        printf("Record length should be lower than MAX_INT value");
        return -1;
    } else if(record_size == 0) {
        printf("Record length should be greater than 0");
        return -1;
    }
    return 1;
}

int compare_records(unsigned char* first, unsigned char* second)
{
    unsigned a, b;
    a = (unsigned) first[0];
    b = (unsigned) second[0];
    if(a > b) return 1;
    else if(a < b) return -1;
    return 0;
}

int get_record(char* buffer, record_array_t *records, unsigned index)
{
    size_t read_records;
    fseek(records->handler, records->record_size*index, SEEK_SET);
    read_records = fread(buffer, records->record_size, 1, records->handler);

    if(read_records != 1) return -1;
    return 1;
}

int system_get_record(char* buffer, record_array_t *records, unsigned index)
{
    size_t read_records;
    lseek(records->file_descriptor, records->record_size*index, SEEK_SET);
    read_records = read(records->file_descriptor, buffer, records->record_size);
    if(read_records != records->record_size) return -1;
    return 1;
}

void insert_record(record_array_t *records, unsigned index, char *record) 
{
    size_t written_records;
    fseek(records->handler, records->record_size*index, SEEK_SET);
    written_records = fwrite(record, records->record_size, 1, records->handler);
}

void system_insert_record(record_array_t *records, unsigned index, char *record) 
{
    size_t written_records;
    lseek(records->file_descriptor, records->record_size*index, SEEK_SET);
    written_records = write(records->file_descriptor, record, records->record_size);
}

void sort_records_array(record_array_t *records, int (*get_record)(char*, record_array_t*, unsigned),
                        void (*insert_record)(record_array_t*, unsigned, char*))
{
    int i, j;
    unsigned char *current, *compared;

    current = malloc(sizeof(unsigned char)*records->record_size);
    compared = malloc(sizeof(unsigned char)*records->record_size);

    i = 1;
    while(get_record(current, records, i) > 0) {
        j = i-1;
        get_record(compared, records, j);
        while(j >= 0 && compare_records(current, compared) < 0) {
            insert_record(records, j, current);
            insert_record(records, j+1, compared);
            j = j-1;
            get_record(compared, records, j);
        }
        i += 1;
    }

    free(current);
    free(compared);
}

void print_records(record_array_t* records)
{
    unsigned char* record;
    record = malloc(sizeof(unsigned char)*records->record_size);
    while(fread(record, records->record_size, 1, records->handler) > 0) {
        printf("%u ", (unsigned)record[0]);
    }
}

void system_print_records(record_array_t* records)
{
    unsigned char* record;
    record = malloc(sizeof(unsigned char)*records->record_size);
    while(read(records->file_descriptor, record, records->record_size) > 0) {
        printf("%u ", (unsigned)record[0]);
    }
}

void print_times(tms_t *start, tms_t *end)
{
  static long clk = 0;
  if(clk == 0) {
    clk = sysconf(_SC_CLK_TCK);
  }
  printf("User time: %7.3fs\n", (end->tms_utime - start->tms_utime) / (double) clk );
  printf("System time: %7.3fs\n", (end->tms_stime - start->tms_stime) / (double) clk );
}


void run_with_library_functions(char* target_filename, size_t record_size)
{
    FILE *file_handler;
    record_array_t records;
    tms_t tms_start, tms_end;

    file_handler = fopen(target_filename, "r+");

    if(validate_input(file_handler, record_size) < 0) return;

    records.handler = file_handler;
    records.record_size = record_size;

    // print_records(&records);
    printf("\n------------------\n");
    rewind(records.handler);

    times(&tms_start);
    sort_records_array(&records, get_record, insert_record);
    times(&tms_end);
    print_times(&tms_start, &tms_end);

    rewind(records.handler);
    // print_records(&records);

    fclose(file_handler);
}

void run_with_system_functions(char* target_filename, size_t record_size)
{
    int file_descriptor;
    record_array_t records;
    tms_t tms_start, tms_end;

    file_descriptor = open(target_filename, O_RDWR);

    if(file_descriptor == -1) {
        printf("Invalid filename.\n");
        return;
    }

    records.file_descriptor = file_descriptor;
    records.record_size = record_size;

    // system_print_records(&records);
    printf("\n------------------\n");
    lseek(records.file_descriptor, 0, SEEK_SET);

    times(&tms_start);
    sort_records_array(&records, system_get_record, system_insert_record);
    times(&tms_end);
    print_times(&tms_start, &tms_end);

    lseek(records.file_descriptor, 0, SEEK_SET);
    // system_print_records(&records);

    close(file_descriptor);
}


int main(int argc, char** argv)
{
    char *target_filename, *mode;
    size_t record_size;

    if(argc != 4) {
        printf("Invalid number of arguments\n");
        exit(1);
    }

    target_filename = strdup(argv[1]);
    record_size = (size_t)atol(argv[2]);
    mode = strdup(argv[3]);

    if(strcmp(mode, "lib") == 0) {
        run_with_library_functions(target_filename, record_size);
    } else if(strcmp(mode, "sys") == 0) {
        run_with_system_functions(target_filename, record_size);
    } else {
        printf("Invalid mode argument. Available: lib, sys\n");
    }        

    free(target_filename);
    free(mode);

    return 0;
}
