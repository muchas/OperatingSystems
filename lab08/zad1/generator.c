#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define RECORD_SIZE 1024
#define BUFFER_SIZE (RECORD_SIZE - sizeof(int))


typedef struct RecordData {
    int id;
    char text[BUFFER_SIZE];
} record_data_t;


typedef union Record {
    char bytes[RECORD_SIZE];
    record_data_t data;
} record_t;


int main(int argc, char* argv[])
{
    char buffer[BUFFER_SIZE];
    int id, i;
    record_t record;
    FILE *file;

    if(argc != 2) {
        printf("Invalid number of arguments. Usage: <filename>\n");
        exit(EXIT_FAILURE);
    }

    file = fopen(argv[1], "w+");

    if(file == NULL) {
        perror("file open error");
        exit(EXIT_FAILURE);
    }

    id = 1;

    while(fgets(buffer, BUFFER_SIZE, stdin)) {
        record.data.id = id;
        strncpy(record.data.text, buffer, BUFFER_SIZE);
        fwrite(record.bytes, RECORD_SIZE, 1, file);
        printf("Inserted record with ID: %d\n", id);
        id += 1;
    }

    fclose(file);
}