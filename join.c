#include "join.h"

/**
 *          Allow a node to connect to the another node. This is the
 *          essential part in creating a network. New nodes that
 *          connect to the network will be inserted as a successor,
 *          the nodes that already exists in the network will become
 *          the new nodes successor.
 *
 *
 * @author  dv18mln dv18mfg
 *
 * @{
 */

/**
 *            Inserts a new node into the network.
 *
 *            The joining node is inserted as a successor to the
 *            node with the "highest" index-range. To find the
 *            node with the largest size, the pdu is sent to all
 *            nodes and updated when a node with a larger size is found.
 *            This pdu is sent until it comes back to the nodes with
 *            matching address.
 *
 *            When the joined node have a successor, the successor is
 *            disconnected and the joining node is set "inbetween" the
 *            joined node and its previous successor.
 *
 *            If the joined node has no successor or predecessor
 *            the new node can be inserted directly as a successor.
 *
 *            When the node his connected, half the index range is
 *            transferred to the new node.
 *
 * @param pfds    Poll file descriptors referring to sockets.
 * @param Buffer  Containing the received PDU
 * @param size    The size of the buffer
 * @param Table   Hashtable storing all entites
 * @param my_addr The address to the node.
 * @param succ_addr Address to the successor.
 * @param range_start The start of the index.
 * @param range_end The end of the index.
 */

void joinNewNode(struct pollfd pfds[4],
                 uint8_t *buffer,
                 int size,
                 struct h_table *table,
                 struct sockaddr_in my_addr,
                 struct sockaddr_in *succ_addr,
                 uint8_t *range_start,
                 uint8_t *range_end)
{

    struct NET_JOIN_PDU joinPDU;
    struct sockaddr_in connection_addr;

    socklen_t len = sizeof(connection_addr);
    getsockname(pfds[2].fd, (struct sockaddr *)&connection_addr, &len);

    joinPDU.type = buffer[0];

    joinPDU.src_address = buffer[1] | buffer[2] << 8 |
                          buffer[3] << 16 | buffer[4] << 24;

    joinPDU.src_port = buffer[5] | buffer[6] << 8;

    joinPDU.max_span = buffer[7];

    joinPDU.max_address = buffer[8] | buffer[9] << 8 |
                          buffer[10] << 16 | buffer[11] << 24;

    joinPDU.max_port = buffer[12] | buffer[13] << 8;

    struct sockaddr_in serv_addr;

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = joinPDU.src_port;
    serv_addr.sin_addr.s_addr = joinPDU.src_address;

    fprintf(stdout, "adress to connect to: %s:%d\n", inet_ntoa(serv_addr.sin_addr), serv_addr.sin_port);

  if(pfds[1].revents & POLLHUP){//Q5
    fprintf(stdout,"[Q5]\n");

    succ_addr->sin_family = AF_INET;
    succ_addr->sin_addr.s_addr = serv_addr.sin_addr.s_addr;
    succ_addr->sin_port = serv_addr.sin_port;

    if (connect(pfds[1].fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("Connect");
        exit(0);
    }
    /**********************/
    pfds[1].events = POLLIN;
    /**********************/

    len = sizeof(serv_addr);

    getsockname(pfds[2].fd, (struct sockaddr *)&my_addr, &len);

    fprintf(stdout,"Sending adress: %s:%d\n", inet_ntoa(my_addr.sin_addr), my_addr.sin_port);
    fflush(stdout);

    sendJoinResponse(pfds[1],
                    joinPDU,
                    my_addr.sin_addr.s_addr,
                    my_addr.sin_port,
                    *range_end / 2 + 1,
                    *range_end);

    table_transfer(pfds[1], table, range_start, range_end);

    shutdown(pfds[3].fd,SHUT_RDWR);
    close(pfds[3].fd);

    pfds[3].fd = accept(pfds[2].fd, NULL, NULL);
    int time = 10;
    socklen_t len = sizeof(time);
    setsockopt(pfds[3].fd, SOL_SOCKET, SO_LINGER, &time, len);

  }
  else if (joinPDU.max_span <= 0 || joinPDU.max_span != (*range_end - *range_start) || joinPDU.max_port != connection_addr.sin_port)
  { //14
    fprintf(stdout,"[Q14]\n");
    uint8_t buffer2[14];
      //update max field if the node have a bigger span
      if (joinPDU.max_span < (*range_end - *range_start))
      {
          buffer2[7] = (*range_end - *range_start);

          buffer2[8] = connection_addr.sin_addr.s_addr;
          buffer2[9] = connection_addr.sin_addr.s_addr >> 8;
          buffer2[10] = connection_addr.sin_addr.s_addr >> 16;
          buffer2[11] = connection_addr.sin_addr.s_addr >> 24;
          buffer2[12] = connection_addr.sin_port;
          buffer2[13] = connection_addr.sin_port >> 8;

      }
      else{

        for (size_t i = 7; i < 14; i++) {
          buffer2[i] = buffer[i];
        }
      }
      serialize(buffer2, joinPDU.src_address, joinPDU.src_port);
      buffer2[0] = NET_JOIN;
      //send to succsessor
      fprintf(stdout,"Forwarding Net Join\n");
      pfds[1].events = POLLOUT;
      poll(&(pfds[1]),1,0);
      if(send(pfds[1].fd, &buffer2, sizeof(buffer2), 0) == -1){
          perror("Message:");
      }
  }
  else if (joinPDU.max_span == (*range_end - *range_start) &&
           joinPDU.max_address == connection_addr.sin_addr.s_addr &&
           joinPDU.max_port == connection_addr.sin_port)
  { //q13

      fprintf(stdout,"[Q13]\n");
      struct NET_CLOSE_CONNECTION_PDU close_connection;
      close_connection.type = NET_CLOSE_CONNECTION;
      fprintf(stdout,"Sending close connection\n");

      pfds[1].events = POLLOUT;
      poll(&(pfds[1]),1,0);
      send(pfds[1].fd, &close_connection, sizeof(close_connection), 0);

      shutdown(pfds[1].fd,SHUT_RDWR);
      close(pfds[1].fd);

      pfds[1].fd = socket(AF_INET, SOCK_STREAM, 0);
      int time = 10;
      socklen_t len = sizeof(time);
      setsockopt(pfds[1].fd, SOL_SOCKET, SO_LINGER, &time, len);

      if (connect(pfds[1].fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
      {
          perror("Connect");
          exit(0);
      }

      fprintf(stdout,"successor address %s %d\n", inet_ntoa(succ_addr->sin_addr), succ_addr->sin_port);
      sendJoinResponse(pfds[1],
                       joinPDU,
                       succ_addr->sin_addr.s_addr,
                       succ_addr->sin_port,
                       (*range_end-*range_start)/ 2 + *range_start + 1,
                       *range_end);

      succ_addr->sin_addr.s_addr = serv_addr.sin_addr.s_addr;
      succ_addr->sin_port = serv_addr.sin_port;

      table_transfer(pfds[1], table, range_start, range_end);

  }
}

/**
 *                Responds with the size the joining node should
 *                initialize its table to, along with the address
 *                of the current nodes successor.
 *
 *
 * @param pfds    Poll file descriptors.
 * @param joinPDU struct containing the succ. address and max span.
 * @param next_address  Address of successor.
 * @param next_port     Port of successor.
 * @param range_start   Beginning of index.
 * @param range_end     End of index.
 */
void sendJoinResponse(struct pollfd pfds,
                      struct NET_JOIN_PDU joinPDU,
                      uint32_t next_address,
                      uint16_t next_port,
                      uint8_t range_start,
                      uint8_t range_end)
{

    uint8_t buffer[10], *ptr;

    ptr = serialize(buffer, next_address, next_port);
    buffer[0] = NET_JOIN_RESPONSE;
    buffer[7] = range_start;
    buffer[8] = range_end;
    pfds.events = POLLOUT;
    poll(&pfds,1,0);
    send(pfds.fd, &buffer, sizeof(buffer), 0);
}

/**
 *                    Convert given ip and port to network
 *                    byte order to be passed to nodes.
 *
 * @param buffer      Buffer to store the converted values.
 * @param ip          Ip address to be converted
 * @param next_port   Port to be converted.
 */
unsigned char *serialize(unsigned char *buffer, uint32_t ip, uint16_t port)
{

    buffer[1] = ip;
    buffer[2] = ip >> 8;
    buffer[3] = ip >> 16;
    buffer[4] = ip >> 24;
    buffer[5] = port;
    buffer[6] = port >> 8;

    return buffer + 6;
}
/**
 *                    Half of the current nodes table to a prospect node.
 *                    The table of the current node is halved.
 *
 * @param pfds        Poll file descriptor referring to a socket.
 * @param table       The nodes hashtable.
 * @param min         Tables min range.
 * @param max         Tables max range.
 */
void table_transfer(struct pollfd pfds, struct h_table *table, uint8_t *min, uint8_t *max){

  int amount;
  uint8_t (*ssn)[SSN_LENGTH];
  uint8_t *lname_length;
  uint8_t **lname;
  uint8_t *lemail_length;
  uint8_t **lemail;

  int size = (*max-*min)/2 - 1;

  struct h_table *new_table = htable_split(&table, size + 2);
  *max = (*max-*min)/2 + *min;



  for (size_t i = 0; i < size; i++) {

    amount = htable_empty_bucket(&ssn, &lname_length, &lname, &lemail_length, &lemail, i, new_table);

    for (size_t j = 0; j < amount; j++){

        uint8_t buffer[15 + lname_length[j] + lemail_length[j]];
        buffer[0] = VAL_INSERT;

        for (int m = 0; m < SSN_LENGTH; m++)
        {
            buffer[1 + m] = ssn[j][m];
        }
        buffer[13] = lname_length[j];
        for (int l = 0; l < lname_length[j]; l++)
        {
            buffer[14 + l] = lname[j][l];
        }

        buffer[14 + lname_length[j]] = lemail_length[j];

        for (int k = 0; k < lemail_length[j]; k++)
        {
            buffer[14 + lname_length[j] + 1 + k] = lemail[j][k];
        }
        pfds.events = POLLOUT;
        poll(&pfds,1,0);
        send(pfds.fd, &buffer, sizeof(buffer), 0);

        free(lname[j]);
        free(lemail[j]);
    }

    free(ssn);
    free(lname_length);
    free(lname);
    free(lemail_length);
    free(lemail);


  }

  htable_destroy(new_table);

}

/**@}*/
