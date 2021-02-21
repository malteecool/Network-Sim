#ifndef VAL_H
#define VAL_H

/**
 * @brief   Allows functionality to receive PDU's over UDP.
 *          For more information visit the source-file.
 * 
 * @see     val.c
 * @author  dv18mln, dv18mfg
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <poll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>
#include "pdu.h"
#include "hashtable.h"
#include "hash.h"

/**
 * @brief   Looks up a entity in the hashtable.
 */
void node_lookup(struct pollfd pfds[4],
                 struct VAL_LOOKUP_PDU request,
                 uint8_t *ssn,
                 struct h_table *table,
                 uint8_t range_start);
/**
 * @brief   Removes a entity from the hashtable.
 */
void remove_val(struct pollfd pfds[4], struct h_table *table,
                uint8_t *buffer, int size, uint8_t range_start,
                uint8_t range_end);

/**
 * @brief   Handle the indexing of the look up of nodes.
 */
void lookup_val(struct pollfd pfds[4], struct h_table *table,
                uint8_t *buffer, int size, uint8_t range_start,
                uint8_t range_end);

/**
 * @brief   Insert values into the nodes hashtable.
 */
void insert_val(struct pollfd pfds[4], struct h_table *table,
                uint8_t *buffer, int size, uint8_t range_start,
                uint8_t range_end);

#endif