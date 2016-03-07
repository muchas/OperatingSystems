#include <stdio.h>
#include <string.h>
#include <stdlib.h>


typedef struct Date {
    unsigned day;
    unsigned month;
    unsigned year;
} date_t;


typedef struct Address {
    char* country;
    char* city;
    char* street_name;
    unsigned street_number;
} address_t;


typedef struct Contact {
   char* first_name;
   char* last_name;
   char* email;
   char* phone;

   date_t* birthday;
   address_t* address;

   struct Contact* next;
   struct Contact* prev;
} contact_t;


typedef struct ContactList {
    contact_t* head;
    contact_t* tail;
} contact_list_t;


contact_list_t* create_list();
contact_t* create_contact(char* first_name, char* last_name, char* email,
                          char* phone, date_t* birthday, address_t* address);
date_t* create_date(unsigned day, unsigned month, unsigned year);
address_t* create_address(char* country, char* city, char* street_name,
                          unsigned street_number);
void delete_address(address_t* address);
void delete_contact(contact_list_t* list, contact_t* contact);
void delete_list(contact_list_t* list);
void add_contact(contact_list_t* list, contact_t* new_contact);
contact_t* find_contact(contact_list_t* list, char* first_name, char* last_name);
contact_t* merge_sort(contact_t* head);
void sort_list(contact_list_t* list);
void display_address(address_t* address);
void display_date(date_t* date);
void display_list(contact_list_t* list);

