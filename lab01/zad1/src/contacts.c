#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "contacts.h"


char* _strdup_with_null(char* string) {
  if(string == NULL) return NULL;
  return strdup(string);
}


contact_list_t* create_list()
{
    return (contact_list_t*) malloc(sizeof(contact_list_t));
}


contact_t* create_contact(char* first_name, char* last_name, char* email,
                          char* phone, date_t* birthday, address_t* address)
{
    contact_t* new_contact = (contact_t*) malloc(sizeof(contact_t));

    new_contact->first_name = _strdup_with_null(first_name);
    new_contact->last_name = _strdup_with_null(last_name);
    new_contact->phone = _strdup_with_null(phone);
    new_contact->email = _strdup_with_null(email);
    new_contact->birthday = birthday;
    new_contact->address = address;

    return new_contact;
}

date_t* create_date(unsigned day, unsigned month, unsigned year)
{
    date_t* new_date = (date_t*) malloc(sizeof(date_t));

    new_date->day = day;
    new_date->month = month;
    new_date->year = year;

    return new_date;
}

address_t* create_address(char* country, char* city, char* street_name,
                          unsigned street_number)
{
     address_t* new_address = (address_t*) malloc(sizeof(address_t));

     new_address->country = _strdup_with_null(country);
     new_address->city = _strdup_with_null(city);
     new_address->street_name = _strdup_with_null(street_name);
     new_address->street_number = street_number;

     return new_address;
}

void delete_address(address_t* address)
{
  if(address == NULL) return;

  free(address->country);
  free(address->city);
  free(address->street_name);
  free(address);
}


void delete_contact(contact_list_t* list, contact_t* contact)
{
    if(contact == NULL) return;

    if(contact->prev != NULL) {
      contact->prev->next = contact->next;
    } else {
      contact->next->prev = NULL;
      list->head = contact->next;
    }

    if(contact->next != NULL) {
      contact->next->prev = contact->prev;
    } else {
      contact->prev->next = NULL;
      list->tail = contact->prev;
    }

    free(contact->first_name);
    free(contact->last_name);
    free(contact->phone);
    free(contact->email);
    free(contact->birthday);
    delete_address(contact->address);
    free(contact);
}

void delete_list(contact_list_t* list)
{
    contact_t* it;

    if(list == NULL) return;

    it = list->head;

    if(it != NULL) {
        while(it->next != NULL) {
            it = it->next;
            delete_contact(list, it->prev);
        }
        free(it);
    }
}


void add_contact(contact_list_t* list, contact_t* new_contact)
{
   contact_t* tail;

   if(list == NULL || new_contact == NULL) return;

   tail = list->tail;

   if(tail == NULL) {
      list->head = new_contact;
      list->tail = new_contact;
      new_contact->prev = NULL;
      new_contact->next = NULL;
   } else {
      tail->next = new_contact;
      new_contact->prev = tail;
      new_contact->next = NULL;
      list->tail = new_contact;
   }
}


contact_t* find_contact(contact_list_t* list, char* first_name, char* last_name)
{
  contact_t* it;

  if(list == NULL || first_name == NULL || last_name == NULL) return NULL;

  it = list->head;

  while(it != NULL) {
    if(it->first_name == NULL || it->last_name == NULL) { 
      it = it->next;
      continue;
    }

    if(strcmp(it->first_name, first_name) == 0 && strcmp(it->last_name, last_name) == 0) {
      return it;
    }

    it = it->next;
  }

  return it;
}


contact_t* partition(contact_t* head)
{
  contact_t *slow_it, *fast_it;

  if(head == NULL) return NULL;

  slow_it = head;
  fast_it = head;


  while(fast_it != NULL && fast_it->next != NULL) {
    fast_it = fast_it->next->next;
    slow_it = slow_it->next;
  }

  if(slow_it->prev != NULL){
      slow_it->prev->next = NULL;
  }
  slow_it->prev = NULL;
  return slow_it;
}


int compare_contacts(contact_t* first, contact_t* second)
{
  int result;

  if(first == NULL) return -1;
  else if(second == NULL) return 1;

  result = strcmp(first->last_name, second->last_name);

  if(result == 0) {
    return strcmp(first->first_name, second->first_name);
  }

  return result;
}


contact_t* merge(contact_t* first, contact_t* second, contact_t* previous, int (*compare)(contact_t*, contact_t*))
{
  contact_t* result;

  if(first == NULL) {
    second->prev = previous;
    return second;
  } else if(second == NULL) {
    first->prev = previous;
    return first;
  }

  if(compare(first, second) < 0) {
    result = first;
    result->next = merge(result->next, second, result, compare);
  } else {
    result = second;
    result->next = merge(first, result->next, result, compare);
  }
  result->prev = previous;

  return result;
}


contact_t* merge_sort(contact_t* head)
{
  contact_t* middle;

  if(head == NULL) return NULL;
  else if(head->next == NULL) return head;

  middle = partition(head);

  return merge(merge_sort(head), merge_sort(middle), NULL, compare_contacts);
}


void sort_list(contact_list_t* list)
{
  contact_t* it;

  if(list == NULL) return;
  list->head = merge_sort(list->head);

  it = list->head;

  if(it != NULL) {
    while(it->next != NULL) it = it->next;
  }
  list->tail = it;
}


void display_address(address_t* address)
{
  if(address == NULL) return;
  printf("%s %d %s %s", address->street_name, address->street_number, address->city, address->country);
}

void display_date(date_t* date)
{
  if(date == NULL) return;
  printf("%d/%d/%d", date->day, date->month, date->year);
}


void display_list(contact_list_t* list)
{
    contact_t* it;

    if(list == NULL) return;

    it = list->head;

    while(it != NULL) {
        printf("%s %s %s\n", it->first_name, it->last_name, it->email);
        it = it->next;
    }
}

