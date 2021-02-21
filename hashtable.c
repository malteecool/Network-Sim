#include "hashtable.h"
#include <stdlib.h>
#include <stdio.h>



struct data{

  uint8_t ssn[SSN_LENGTH];
  uint8_t name_length;
  uint8_t *name;
  uint8_t email_length;
  uint8_t *email;

};

struct bucket{

  int current_size;
  int max_size;
  struct data **persons;

};

struct h_table{

  uint8_t size;
  struct bucket **buckets;

};



struct h_table *htable_create(int size){

  struct h_table *table = malloc(sizeof(struct h_table));
  if(table == NULL){
    return NULL;
  }
  table->size = size;
  table->buckets = malloc(sizeof(struct bucket*)*size);

  for (size_t i = 0; i < size; i++) {
    table->buckets[i] = malloc(sizeof(struct bucket));
    table->buckets[i]->current_size = 0;
    table->buckets[i]->max_size = 2;
    table->buckets[i]->persons = malloc(sizeof(struct data*)*2);
    if(table->buckets[i] == NULL){
      return NULL;
    }
  }

  return table;
}

int htable_insert(uint8_t ssn[SSN_LENGTH],
            uint8_t name_length,
            uint8_t *name,
            uint8_t email_length,
            uint8_t *email,
            uint8_t key,
            struct h_table *table){
  if(name_length < 0 && name_length > 255){
    return 1;
  }

  if(email_length < 0 && email_length > 255){
    return 1;
  }

  struct data *new_person = malloc(sizeof(struct data));

  for (size_t i = 0; i < SSN_LENGTH; i++) {
      new_person->ssn[i] = ssn[i];
  }
  new_person->name_length = name_length;
  new_person->name = malloc(sizeof(uint8_t)*name_length);
  for (size_t i = 0; i < name_length; i++) {
    new_person->name[i] = name[i];
  }


  new_person->email_length = email_length;
  new_person->email = malloc(sizeof(uint8_t)*email_length);
  for (size_t i = 0; i < email_length; i++) {
    new_person->email[i] = email[i];
  }

  struct bucket *theBucket = table->buckets[key];

  if(theBucket->current_size < theBucket->max_size){
    theBucket->max_size *= 2;
    theBucket->persons = realloc(theBucket->persons,
                                 sizeof(struct data)*theBucket->max_size);
  }

  theBucket->persons[theBucket->current_size] = new_person;
  theBucket->current_size += 1;


  return 0;
}

int htable_lookup(uint8_t ssn[SSN_LENGTH],
           uint8_t *name_length,
           uint8_t **name,
           uint8_t *email_length,
           uint8_t **email,
           uint8_t key,
           struct h_table *table){

  if(table->size < key && key < 0){
    //error message
    return 1;
  }


  struct data *person = NULL;

  for (size_t i = 0; i <  table->buckets[key]->current_size; i++) {

    int ssn_check = 0;
    for (size_t j = 0; j < SSN_LENGTH; j++) {
      if(ssn[j] == table->buckets[key]->persons[i]->ssn[j]){
        ssn_check++;
      }
      else{
        break;
      }
    }
    if(ssn_check == SSN_LENGTH){
      person = table->buckets[key]->persons[i];
      break;
    }

  }

  if(person == NULL){
    //message about could not find
    return 1;
  }


  *name_length = person->name_length;
  *name = malloc(sizeof(uint8_t)*person->name_length);
  for (size_t i = 0; i < person->name_length; i++) {
    (*name)[i] = person->name[i];
  }

  *email_length = person->email_length;
  *email = malloc(sizeof(uint8_t*)*person->email_length);
  for (size_t i = 0; i < person->email_length; i++) {
    (*email)[i] = person->email[i];
  }

  return 0;
}

int htable_remove(uint8_t ssn[SSN_LENGTH],
           uint8_t key,
           struct h_table *table){

   if(table->size < key && key > 0){
     //error message
     return 1;
   }

   int ssn_found = 0;


   for (size_t i = 0; i <  table->buckets[key]->current_size; i++) {
     int ssn_check = 0;

     for (size_t j = 0; j < SSN_LENGTH; j++) {
       if(ssn[j] == table->buckets[key]->persons[i]->ssn[j]){
         ssn_check++;
       }
       else{
         break;
       }
     }
     if(ssn_found == 1){
       table->buckets[key]->persons[i - 1] = table->buckets[key]->persons[i];

     }
     else if(ssn_check == SSN_LENGTH){
       free(table->buckets[key]->persons[i]->name);
       free(table->buckets[key]->persons[i]->email);
       free(table->buckets[key]->persons[i]);
       table->buckets[key]->current_size -= 1;
     }
   }
   return 0;
}



int htable_empty_bucket(uint8_t (**ssn)[12],
                 uint8_t **name_length,
                 uint8_t ***name,
                 uint8_t **email_length,
                 uint8_t ***email,
                 uint8_t key,
                 struct h_table *table){


   struct bucket *theBucket = table->buckets[key];
   int size = theBucket->current_size;

   *ssn = malloc(sizeof(uint8_t*)*size*SSN_LENGTH);
   *name_length = malloc(sizeof(uint8_t*)*size);
   *name = malloc(sizeof(uint8_t*)*size);
   *email_length = malloc(sizeof(uint8_t*)*size);
   *email = malloc(sizeof(uint8_t*)*size);

   for (size_t i = 0; i < size; i++) {
     for (size_t j = 0; j < SSN_LENGTH; j++) {
       (*ssn)[i][j] = theBucket->persons[i]->ssn[j];
     }
     (*name_length)[i] = theBucket->persons[i]->name_length;
     (*name)[i] = theBucket->persons[i]->name;
     (*email_length)[i] = theBucket->persons[i]->email_length;
     (*email)[i] = theBucket->persons[i]->email;
     free(theBucket->persons[i]);

   }

   theBucket->persons = realloc(theBucket->persons, sizeof(struct data*)*2);
   theBucket->current_size = 0;
   theBucket->max_size = 2;

   return size;
}


int htable_destroy(struct h_table *table){

  for (size_t i = 0; i < table->size; i++) {
    struct bucket *theBucket = table->buckets[i];

    int size = theBucket->current_size;
    for (size_t i = 0; i < size; i++) {
      free(theBucket->persons[i]->name);
      free(theBucket->persons[i]->email);
      free(theBucket->persons[i]);
    }
    free(theBucket->persons);
    free(theBucket);
  }

  free(table->buckets);
  free(table);

  return 0;
}


int *htable_merge(struct h_table **low_table, struct h_table *high_table){

  (*low_table)->buckets = realloc((*low_table)->buckets, sizeof(struct bucket*) *
                                 ((*low_table)->size + high_table->size));
  for (size_t i = 0; i < high_table->size; i++) {
    (*low_table)->buckets[i+(*low_table)->size] = high_table->buckets[i];
  }
  (*low_table)->size = (*low_table)->size + high_table->size;

  free(high_table->buckets);
  free(high_table);

  return 0;
}


struct h_table *htable_split(struct h_table **table, int split){

  struct h_table *new_table = malloc(sizeof(struct h_table));
  if(new_table == NULL){
    return NULL;
  }
  new_table->size = (*table)->size - split;
  new_table->buckets = malloc(sizeof(struct bucket*)*new_table->size);
  for (size_t i = 0; i < new_table->size; i++) {
    new_table->buckets[i] = (*table)->buckets[split+i];
  }
  (*table)->buckets = realloc((*table)->buckets, sizeof(struct bucket*)*split);
  (*table)->size = split - 1;

  return new_table;
}

int table_size(struct h_table *table)
{
  return table->size;
}
