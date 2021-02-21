#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <inttypes.h>

#define SSN_LENGTH 12

struct h_table;

/**
 * @defgroup        hashtable Hashtable
 * @brief           Accually just a table
 *
 *                  It is a table mascerading as a hashtable.
 *
 * @author          dv18mln dv18mfg
 */


/**
 *@brief Creates a new table
 *
 *@param size How many keys that can be applied to the table
 *@return the new table
 */


struct h_table *htable_create(int size);


/**
 *@brief Inserts a person
 *
 *@param size ssn person number
 *@param name_length the size of name
 *@param name the persons name
 *@param email_length the size of email
 *@param email the persons email
 *@param key the key to the table
 *@param table the table
 */

int htable_insert(uint8_t ssn[SSN_LENGTH],
            uint8_t name_length,
            uint8_t *name,
            uint8_t email_length,
            uint8_t *email,
            uint8_t key,
            struct h_table *table);

/**
 *@brief Looks up a ssn to get name and email
 *
 *name_length, name, email_length and email
 *are all allocated memory and must be freed
 *
 *
 *@param size ssn person number
 *@param table the table
 */

int htable_lookup(uint8_t ssn[SSN_LENGTH],
           uint8_t *name_length,
           uint8_t **name,
           uint8_t *email_length,
           uint8_t **email,
           uint8_t key,
           struct h_table *table);


           /**
            *@brief Looks up a ssn to get name and email
            *
            *name_length, name, email_length and email
            *are all allocated memory and must be freed
            *
            *
            *@param size ssn person number
            *@param name_length the size of name
            *@param name the persons name
            *@param email_length the size of email
            *@param email the persons email
            *@param key the key to the table
            *@param table the table
            */
int htable_remove(uint8_t ssn[SSN_LENGTH],
           uint8_t key,
           struct h_table *table);


//need to free values, two in name and email
int htable_empty_bucket(uint8_t (**ssn)[12],
                uint8_t **name_length,
                uint8_t ***name,
                uint8_t **email_length,
                uint8_t ***email,
                uint8_t key,
                struct h_table *table);


int htable_destroy(struct h_table *table);

int *htable_merge(struct h_table **low_table, struct h_table *high_table);


struct h_table *htable_split(struct h_table **table, int split);

int table_size(struct h_table *table);

#endif
