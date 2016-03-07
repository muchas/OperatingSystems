#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <sys/times.h>
#include <dlfcn.h>
#include <unistd.h>
#include "contacts.h"


typedef struct tms tms_t;


char* _random_string_choice(char** array, unsigned size)
{
  unsigned i = rand() % size;
  return array[i];
}

char* generate_first_name(char** first_names, unsigned size)
{
  return _random_string_choice(first_names, size);
}

char* generate_last_name(char** last_names, unsigned size)
{
  return _random_string_choice(last_names, size);
}

char* generate_email()
{
  int i;
  char* email = malloc(sizeof(char)*20);
  strcpy(email, "aaaaaaaaa@gmail.com");

  for(i=0;i<9;i++){
    email[i] = rand()%('z'-'a')+'a';
  }
  return email;
}

char* generate_phone()
{
  int i;
  char* phone;

  phone = malloc(sizeof(char)*13);
  strcpy(phone, "+48xxxxxxxxx");

  for(i=3; i<12; i++) {
    phone[i] = rand() % 10 + '0';
  }
  phone[12]='\0';

  return phone;
}

date_t* generate_date(date_t* (*create_date)(unsigned, unsigned, unsigned))
{
  return create_date(rand() % 31, (rand() % 13) + 1, 1970 + (rand() % 30));
}

address_t* generate_address(address_t* (*create_address)(char*, char*, char*, unsigned))
{
  return create_address("Polska", "Krakow", "Kawiory", rand() % 60); 
}

contact_t* generate_contact(char** first_names, unsigned first_names_size, char** last_names, unsigned last_name_size,
                            contact_t* (*create_contact)(char*, char*, char*, char*, date_t*, address_t*),
                            address_t* (*create_address)(char*, char*, char*, unsigned),
                            date_t* (*create_date)(unsigned, unsigned, unsigned))
{
    contact_t* contact;
    char *email, *phone;

    email = generate_email();
    phone = generate_phone();

    contact = create_contact(generate_first_name(first_names, first_names_size),
                          generate_last_name(last_names, last_name_size), 
                          email,
                          phone, 
                          generate_date(create_date), 
                          generate_address(create_address));

    free(email);
    free(phone);
    return contact;
}


void print_times(clock_t real, tms_t *tms_start, tms_t *tms_end)
{
  static long clk = 0;
  if(clk == 0) {
    clk = sysconf(_SC_CLK_TCK);
  }
  printf("Real time: %7.3fs\n", real / (double) CLOCKS_PER_SEC);
  printf("User time: %7.3fs\n", (tms_end->tms_utime - tms_start->tms_utime) / (double) clk );
  printf("System time: %7.3fs\n", (tms_end->tms_stime - tms_start->tms_stime) / (double) clk );
  printf("---------------------------\n");
}

void run_benchmark(clock_t* clock_start, clock_t* clock_interval, clock_t* clock_previous_interval,
                   tms_t* tms_start, tms_t* tms_interval, tms_t* tms_previous_interval)
{
  if(clock_previous_interval != NULL && tms_previous_interval != NULL) {
    printf("Compared with previous checkpoint:\n");
    print_times((*clock_interval) - (*clock_previous_interval), tms_previous_interval, tms_interval);
    *clock_previous_interval = *clock_interval;
    *tms_previous_interval = *tms_interval;
  }

  printf("Compared with first checkpoint:\n");
  print_times((*clock_interval) - (*clock_start), tms_start, tms_interval);
}


int main(int argc, char** argv)
{

    char* first_names[] = {"Slawomir", "Adam", "Patryk", "Ola", "Anna", "John", "Marian", "Jaroslaw", "Andrzej", "Jan", "Aleksander",
                           "Alicja", "Magdalena", "Antoni", "Innocenty", "Michal", "Maciej", "Tomasz"};
    char* last_names[] = {"Mucha", "Przykladowy", "Swaisse", "Doe", "Zygmunt", "Kowalski", "Kaczynski", "Duda", "Bubak", "Wilk", "Nowak",
                          "Zak", "Grochala", "Warchol", "Macierewicz", "Kwasniewski", "Bezimienny", "Lis", "Olejnik"};
    int i;

    void* handle;

    contact_list_t* list;
    contact_t* generated_contacts[100000];
    contact_t* found;

    tms_t tms_start, tms_interval, tms_previous_interval;
    clock_t clock_start, clock_interval, clock_previous_interval;

    contact_list_t* (*create_list)();
    date_t* (*create_date)(unsigned day, unsigned month, unsigned year);
    void (*add_contact)(contact_list_t* list, contact_t* new_contact);
    void (*sort_list)(contact_list_t* list);
    contact_t* (*find_contact)(contact_list_t* list, char* first_name, char* last_name);
    void (*delete_contact)(contact_list_t* list, contact_t* contact);
    void (*delete_list)(contact_list_t* list);
    contact_t* (*create_contact)(char* first_name, char* last_name, char* email,
                                 char* phone, date_t* birthday, address_t* address);
    address_t* (*create_address)(char* country, char* city, char* street_name,
                                 unsigned street_number);

    handle = dlopen("libcontacts.so", RTLD_LAZY);

    if (!handle) 
    { 
      fprintf(stderr, "dlopen failed: %s\n", dlerror()); 
      exit(EXIT_FAILURE);
    };

    create_list = dlsym(handle, "create_list");
    create_date = dlsym(handle, "create_date");
    add_contact = dlsym(handle, "add_contact");
    sort_list = dlsym(handle, "sort_list");
    find_contact = dlsym(handle, "find_contact");
    delete_contact = dlsym(handle, "delete_contact");
    delete_list = dlsym(handle, "delete_list");
    create_contact = dlsym(handle, "create_contact");
    create_address = dlsym(handle, "create_address");

    clock_start = clock();
    times(&tms_start);
    times(&tms_previous_interval);
    clock_interval = clock();
    times(&tms_interval);
    printf("1. CHECKPOINT - Beginning of the program\n");
    run_benchmark(&clock_start, &clock_interval, &clock_previous_interval, &tms_start, &tms_interval, &tms_previous_interval);

    srand(time(NULL));

    list = (*create_list)();

    for(i=0; i<100000; i+=1) {
        generated_contacts[i] = generate_contact(first_names, 18, last_names, 18, create_contact, create_address, create_date);
    }

    clock_interval = clock();
    times(&tms_interval);
    printf("2. CHECKPOINT - 100000 contacts has been generated\n");
    run_benchmark(&clock_start, &clock_interval, &clock_previous_interval, &tms_start, &tms_interval, &tms_previous_interval);

    for(i=0; i<100000; i+=1) {
        (*add_contact)(list, generated_contacts[i]);
    }

    clock_interval = clock();
    times(&tms_interval);
    printf("3. CHECKPOINT - 100000 contacts has been added to list\n");
    run_benchmark(&clock_start, &clock_interval, &clock_previous_interval, &tms_start, &tms_interval, &tms_previous_interval);

    sort_list(list);

    clock_interval = clock();
    times(&tms_interval);
    printf("4. CHECKPOINT - list has been sorted\n");
    run_benchmark(&clock_start, &clock_interval, &clock_previous_interval, &tms_start, &tms_interval, &tms_previous_interval);


    for(i=0; i<300; i+=1) {
        found = (*find_contact)(list, generated_contacts[i]->first_name, generated_contacts[i]->last_name);
        (*delete_contact)(list, found);
    }

    clock_interval = clock();
    times(&tms_interval);
    printf("5. CHECKPOINT - 300 contacts has been randomly found and deleted\n");
    run_benchmark(&clock_start, &clock_interval, &clock_previous_interval, &tms_start, &tms_interval, &tms_previous_interval);

    (*delete_list)(list);

    clock_interval = clock();
    times(&tms_interval);
    printf("6. CHECKPOINT - list has been removed\n");
    run_benchmark(&clock_start, &clock_interval, &clock_previous_interval, &tms_start, &tms_interval, &tms_previous_interval);

    dlclose(handle);

    return 0;
}