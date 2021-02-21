#include "update.h"

/**
 *          Poll function to wait for incomming data,
 *          both over UDP and TCP.
 *          Using poll to listen for data on each socket.
 *
 *          The data is saved in a buffer.
 *
 *          During each timeout a alive message is sent to
 *          the tracker.
 *
 * @author  dv18mln dv18mfg
 *
 * @param pfds      Poll file descriptors referring to sockets.
 * @param buffer    Buffer to store the received data.
 * @param size      Size of the buffer
 * @param serv_addr Address of tracker.
 * @return          The pfds index which received data.
 *
 * @{
 */
int update(struct pollfd pfds[4],
           uint8_t *buffer,
           int size,
           struct sockaddr_in serv_addr)
{

    //SEND NET_ALIVE_PDU
    struct NET_ALIVE_PDU alive;
    alive.type = NET_ALIVE;
    int recv_size;

    int res = sendto(pfds[0].fd, &alive, sizeof(alive), 0,
                     (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    if (res == -1)
    {
        perror("Could not send NET_ALIVE_PDU");
    }

    for (size_t i = 0; i < 4; i++)
    {
        pfds[i].events = POLLIN;
    }

    while (1)
    {
        fprintf(stdout, "[Q6]\n");
        struct sockaddr from;
        socklen_t addrlen = sizeof(from);
        int r;
        sleep(0.2);
        if(pfds[1].revents & POLLHUP && pfds[3].revents & POLLHUP){
          poll(pfds, 4, 50);
          r = poll(pfds, 1, 5000);
        }
        else{
          r = poll(pfds, 4, 5000);
        }
        if (r == -1)
        {
            //CTRL+C
            return -1;
        }
        else if (r == 0)
        {
            fprintf(stdout, "Timeout\n");
        }
        else
        {
            //UDP tracker

            if (pfds[0].revents & POLLIN)
            {
                recvfrom(pfds[0].fd,
                           buffer,
                           sizeof(uint8_t) * 1,
                           MSG_PEEK,
                           &from,
                           &addrlen);

                recv_size = msg_size(pfds[0], buffer[0]);

                int res = recvfrom(pfds[0].fd,
                                   buffer,
                                   sizeof(uint8_t) * recv_size,
                                   0,
                                   &from,
                                   &addrlen);
                return 0;
            }
            //TCP successor
            if (pfds[1].revents & POLLIN)
            {
                recv(pfds[1].fd,
                             buffer,
                             sizeof(uint8_t) * 1,
                             MSG_PEEK);

                recv_size = msg_size(pfds[1], buffer[0]);

                recv(pfds[1].fd,
                               buffer,
                               sizeof(uint8_t) * recv_size,
                               0);
                return 1;
            }

            //TCP accepting connections
            if (pfds[2].revents & POLLIN)
            {
                fprintf(stdout, "Poll accepting connections");
                return 2;
            }
            //TCP predecessor
            if (pfds[3].revents & POLLIN)
            {
                recv(pfds[3].fd,
                           buffer,
                           sizeof(uint8_t) * 1,
                           MSG_PEEK);

                recv_size = msg_size(pfds[3], buffer[0]);

                recv(pfds[3].fd,
                               buffer,
                               sizeof(uint8_t) * recv_size,
                               0);
                return 3;
            }
        }

        //SEND NET_ALIVE_PDU
        int res = sendto(pfds[0].fd, &alive, sizeof(alive), 0,
                         (struct sockaddr *)&serv_addr, sizeof(serv_addr));
        if (res == -1)
        {
            perror("Could not send NET_ALIVE_PDU");
        }
    }
}

int msg_size(struct pollfd pfds, int pdu){

      uint8_t buffer[100];

  switch(pdu){
    case NET_JOIN:
      return 14;
      break;
    case NET_NEW_RANGE:
      return 3;
      break;
    case NET_LEAVING:
      return 7;
      break;
    case NET_CLOSE_CONNECTION:
      return 1;
      break;
    case VAL_INSERT:
    recv(pfds.fd,
               buffer,
               sizeof(uint8_t) * (3 + SSN_LENGTH),
               MSG_PEEK);
    recv(pfds.fd,
              buffer,
              sizeof(uint8_t) * (4 + SSN_LENGTH + buffer[1 + SSN_LENGTH]),
              MSG_PEEK);
    return 3 + SSN_LENGTH + buffer[1 + SSN_LENGTH] + buffer[buffer[1 + SSN_LENGTH] + 2 + SSN_LENGTH];
      break;
    case VAL_LOOKUP:

      recv(pfds.fd,
                 buffer,
                 sizeof(uint8_t) * (2 + SSN_LENGTH),
                 MSG_PEEK);
      recv(pfds.fd,
                buffer,
                sizeof(uint8_t) * (3 + SSN_LENGTH + buffer[1 + SSN_LENGTH]),
                MSG_PEEK);

      return 3 + SSN_LENGTH + buffer[1 + SSN_LENGTH] + buffer[buffer[1 + SSN_LENGTH] + 2 + SSN_LENGTH];
      break;
    case VAL_REMOVE:
      return 1 + SSN_LENGTH;
      break;
    default:
      return pdu;
    break;
  }
}
