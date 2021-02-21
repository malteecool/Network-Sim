#ifndef UPDATE_H
#define UPDATE_H

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

/**
 * @brief   Functionallity to handle data received on socket.
 *          For more info visit the source-file.
 */
int update(struct pollfd pfds[4],
           uint8_t *buffer,
           int size,
           struct sockaddr_in serv_addr);

int msg_size(struct pollfd pfds, int pdu);

#endif