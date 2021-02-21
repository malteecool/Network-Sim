#ifndef NODE_H
#define NODE_H

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
#include "update.h"
#include "shutdown.h"
#include "val.h"

#define MAX_SOCK 4
#define TIMEOUT 0
#define NETWORK_SIZE 256


typedef struct test
{
        int number;
        int *ptr1;
        int *ptr2;
} TEST;


struct sockaddr_in Q1(struct pollfd pfds[MAX_SOCK],
                      const char *ip_address,
                      const char *port);

struct in_addr Q2(struct pollfd pfds[MAX_SOCK]);

struct NET_GET_NODE_RESPONSE_PDU Q3(struct sockaddr_in serv_addr,
                                    struct pollfd pfds[MAX_SOCK]);

void Q7(struct pollfd pfds[MAX_SOCK], struct NET_GET_NODE_RESPONSE_PDU,
        struct sockaddr_in my_addr, int range_start, int range_end);

void Q8(struct pollfd pfds[MAX_SOCK], uint8_t buffer[9], struct h_table **table,
        uint8_t *range_start, uint8_t *range_end, struct sockaddr_in *succ_addr);

void sighandler(int sig);

#endif