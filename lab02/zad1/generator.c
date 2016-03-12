#include <stdlib.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>


void validate_input(FILE* handler, unsigned record_length, unsigned record_count)
{
    if(handler == NULL) {
        printf("Invalid filename\n");
        exit(1);
    }
    if(record_count > INT_MAX) {
        printf("Record count should be lower than MAX_INT value");
        exit(1);
    } else if(record_count == 0) {
        printf("Record count should be greater than 0");
        exit(1);
    }
    if(record_length > INT_MAX) {
        printf("Record length should be lower than MAX_INT value");
        exit(1);
    } else if(record_length == 0) {
        printf("Record length should be greater than 0");
        exit(1);
    }
}

char generate_byte()
{
    return (char)(rand() % 256);
}

void generate_record(unsigned length, char* buffer)
{
    unsigned i;
    for(i=0; i<length; i++) {
        buffer[i] = generate_byte();
    }
}

void generate_records_to_file(FILE* pointer, unsigned record_count, unsigned record_length)
{
    char* buffer;
    unsigned i;

    buffer = (char*)(malloc(sizeof(char)*record_count));

    for(i=0; i<record_count; i++) {
        generate_record(record_length, buffer);
        fwrite(buffer, 1, record_length, pointer);
    }

    free(buffer);
}


int main(int argc, char** argv)
{
    char* target_filename;
    unsigned long record_length, record_count;
    FILE* file_handler;

    if(argc != 4) {
        printf("Invalid number of arguments\n");
        exit(1);
    }

    srand(time(NULL));

    target_filename = strdup(argv[1]);
    record_length = atol(argv[2]);
    record_count = atol(argv[3]);

    file_handler = fopen(target_filename, "w");

    free(target_filename);

    validate_input(file_handler, record_length, record_count);
    generate_records_to_file(file_handler, record_count, record_length);

    printf("Records generated succesfully\n");

    fclose(file_handler);
    return 0;
}
