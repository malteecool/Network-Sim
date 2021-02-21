#ifndef SHUTDOWN_H
#define SHUTDOWN_H

/**
 * @brief   Functions to handle the leaving procedure of a node.
 *          For more information visit the source-file.
 *
 * @see     shutdown.c
 * @author  dv18mln, dv18mfg
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <poll.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>
#include "pdu.h"
#include "join.h"
#include "hashtable.h"

#define MAX_SOCK 4

/**
 * @brief   Handle the shutdown request.
 */

int shutdown_request(struct pollfd pfds[MAX_SOCK], struct h_table *table,
                     struct sockaddr_in succ_addr,
                     uint8_t *range_start, uint8_t *range_end);
/**
 * @brief   Send a Update size request to node.
 */

int update_size(struct pollfd pfds[MAX_SOCK], int to_socket, uint8_t range_start,
                uint8_t range_end);
/**
 * @brief   Transfer each entity to a new node.
 */

int transfer_data(struct pollfd pfds, struct h_table *table, uint8_t range_start,
                  uint8_t range_end);
/**
 * @brief   Close each socket file descriptor.
 */

void close_socket(struct pollfd pfds[MAX_SOCK]);
/**
 * @brief   Update a nodes range.
 */

void new_range(struct pollfd pfds,
               struct h_table **table,
               uint8_t *buffer,
               int size,
               uint8_t *min_range,
               uint8_t *max_range);

/**
 * @brief   Handle the reconnection of a successor node.
 */
int node_leaving(struct pollfd pfds[MAX_SOCK], struct sockaddr_in *succ_addr, uint8_t *buffer);

/**
 * @brief   Accept new a predecessor node.
 */
int close_connection(struct pollfd pfds[MAX_SOCK], uint8_t *buffer);

#endif
