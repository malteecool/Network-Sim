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
 * @brief   Handle the insertion of a new node.
 *          For more info visit the source-file.
 * @author  dv18mfg
 * @see     join.c
 * 
 * @{
 */

#define VAL_SHUTDOWN 15


void joinNewNode(struct pollfd pfds[4],
                 uint8_t *buffer,
                 int size,
                 struct h_table *table,
                 struct sockaddr_in my_addr,
                 struct sockaddr_in *succ_addr,
                 uint8_t *range_start,
                 uint8_t *range_end);

void sendJoinResponse(struct pollfd pfds,
                      struct NET_JOIN_PDU joinPDU,
                      uint32_t next_address,
                      uint16_t next_port,
                      uint8_t range_start,
                      uint8_t range_end);

void table_transfer(struct pollfd pfds, struct h_table *table, uint8_t *min, uint8_t *max);

unsigned char *serialize(unsigned char *buffer, uint32_t ip, uint16_t port);

/**@}*/