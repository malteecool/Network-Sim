#include "shutdown.h"

/**
 * @defgroup        shutdown Shutdown
 * @brief           Functions to handle the leaving procedure of a node.
 *
 *                  When a node leaves the network, its index-range
 *                  and the stored data is sent forwarded to another
 *                  node in the network.
 *                  As a standard procedure, the leaving nodes range is
 *                  sent to the predecessor. If no predecessor exists,
 *                  the range is instead sent to the successor.
 *                  When the node has left, the remaning nodes should
 *                  "reconnect" with eachother.
 *                  Example: The leaving nodes predecessor should connect
 *                  to the leaving nodes successor, and the "ring of nodes"
 *                  remain intact.
 *
 * @author          dv18mln dv18mfg
 * @see             node.h
 * @{
 */

/**
 * @brief           Handle the shutdown request.
 *
 *                  Called in case if Sig-Int. The leaving nodes
 *                  index-range is sent to the predecessor node, which
 *                  updates its size. When a update response is received
 *                  in the leaving node, its data can be transferred to the
 *                  predecessor node. A net_leaving pdu is sent to the
 *                  predecessor node containing the leaving nodes successor.
 *
 *                  If no predecessor exists, the index-range is sent
 *                  to the successor instead, along with a close-connection
 *                  PDU.
 *
 *                  If a node receives SIGINT and is alone in the network,
 *                  it is terminated immediately.
 *
 *                  Algortithm:
 *                      If predecessor exists
 *                          send request to update its size.
 *                          transfer data to precessor.
 *                          Send Net leaving.
 *                          If Successor exist
 *                              Send close connection.
 *                      Else if only successor exists
 *                          send request to update its size.
 *                          transfer data to successor.
 *                          send close connection.
 *                      Else if alone in the network
 *                          exit.
 *
 * @param pfds      The poll file descriptors referring to the created
 *                  sockets.
 * @param table     Hashtable to store the data.
 * @param range_start   Beginning of the nodes index.
 * @param range_end     End of the nodes index.
 *
 */
int shutdown_request(struct pollfd pfds[MAX_SOCK], struct h_table *table,
                     struct sockaddr_in succ_addr,
                     uint8_t *range_start, uint8_t *range_end)
{
    fprintf(stdout, "[Q10]\n");
    if (!(pfds[3].revents & POLLHUP))
    {

        int res = update_size(pfds, 3, *range_start, *range_end);
        if (res == -1)
        {
            perror("predecessor:Could not update");
        }
        if (res == NET_NEW_RANGE_RESPONSE)
        {
            transfer_data(pfds[3], table, *range_start, *range_end);

            uint8_t net_leaving_buffer[7];
            net_leaving_buffer[0] = NET_LEAVING;

            serialize(net_leaving_buffer, succ_addr.sin_addr.s_addr, succ_addr.sin_port);
            pfds[3].events = POLLOUT;
            poll(&(pfds[3]),1,0);
            fprintf(stdout,"Sending NET_LEAVING %d\n", net_leaving_buffer[0]);
            res = send(pfds[3].fd, &net_leaving_buffer, sizeof(net_leaving_buffer), 0);
            if (res == -1)
            {
                perror("Could not send net_leaving");

                close_socket(pfds);
                exit(0);
            }

        }

        if (!(pfds[1].revents & POLLHUP))
        {

            struct NET_CLOSE_CONNECTION_PDU close_connection;
            close_connection.type = NET_CLOSE_CONNECTION;
            fprintf(stdout,"Sending NET_CLOSE_CONNECTION\n");
            pfds[1].events = POLLOUT;
            poll(&(pfds[1]),1,0);
            int res = send(pfds[1].fd, &close_connection, sizeof(close_connection), 0);
            if (res == -1)
            {
                perror("Could not send close_connection");
                close_socket(pfds);
                exit(0);
            }
        }
    }
    else if (!(pfds[1].revents & POLLHUP))
    {

        int res = update_size(pfds, 1, *range_start, *range_end);
        if (res == -1)
        {
            perror("Could not update");
        }
        transfer_data(pfds[3], table, *range_start, *range_end);

        struct NET_CLOSE_CONNECTION_PDU close_connection;
        close_connection.type = NET_CLOSE_CONNECTION;
        pfds[1].events = POLLOUT;
        poll(&(pfds[1]),1,0);
        fprintf(stdout,"Sending NET_CLOSE_CONNECTION\n");
        res = send(pfds[1].fd, &close_connection, sizeof(close_connection), 0);
        if (res == -1)
        {
            perror("Could not send close_connection");
            exit(0);
        }
    }
    else
    {
        close_socket(pfds);
        fprintf(stdout, "I am the last node in the network, goodbye!\n");
        return 1;
    }
    close_socket(pfds);
    fprintf(stdout, "Leaving procedure successfull, goodbye!\n");
    return 1;
}

/**
 * @brief           Send a Update size request to node.
 *
 *                  The update size request is sent to the node
 *                  which shall receive the leaving nodes hashtable.
 *                  The new range is the leaving nodes index-range.
 *
 * @param pfds      The poll file descriptors referring to the created
 *                  sockets.
 * @param to_socket     The socket to sent the request.
 * @param range_start   Beginning of the nodes index.
 * @param range_end     End of the nodes index.
 */

int update_size(struct pollfd pfds[MAX_SOCK], int to_socket, uint8_t range_start,
                uint8_t range_end)
{

    fprintf(stdout, "[Q11]\n");
    struct NET_NEW_RANGE_PDU new_range;
    new_range.range_start = range_start;
    new_range.range_end = range_end;

    uint8_t new_range_buffer[3];
    new_range_buffer[0] = NET_NEW_RANGE;
    new_range_buffer[1] = range_start;
    new_range_buffer[2] = range_end;

    fprintf(stdout, "Sending new range request...\n");
    pfds->events = POLLOUT;
    poll(pfds,1,0);
    int res = send(pfds[to_socket].fd, &new_range_buffer, sizeof(new_range_buffer), 0);
    if (res == -1)
    {
        perror("Could not send NET_NEW_RANGE_PDU");
        close_socket(pfds);
        exit(0);
    }

    fprintf(stdout, "Waiting for response... ");

    uint8_t response_buffer[1];
    if(poll(&(pfds[to_socket]), 1, 5000) > 0){
      if (recv(pfds[to_socket].fd, &response_buffer, sizeof(response_buffer), 0) == -1)
      {
          perror("Could not receive NEW_RANGE_RESPONSE");
          exit(0);
      }
    }
    fprintf(stdout, "Got reponse: %d\n", response_buffer[0]);
    return response_buffer[0];
}
/**
 * @brief           Close each socket file descriptor.
 *
 * @param pfds      The poll file descriptors referring to the created
 *                  sockets.
 */
void close_socket(struct pollfd pfds[MAX_SOCK])
{
    for (int i = 0; i < MAX_SOCK; i++)
    {
        shutdown(pfds[i].fd,SHUT_RDWR);
        close(pfds[i].fd);
    }
}

/**
 * @brief           Transfer each entity to a new node.
 *
 *                  Transfer each entity in the leaving node
 *                  to the receving node. The size needs to be
 *                  updated in beforehand. The entities stored in
 *                  the hashtable are converted to a buffer before
 *                  beging sent over TCP.
 *
 * @param pfds      The poll file descriptors referring to the created
 *                  sockets.
 * @param table
 * @param range_start   Beginning of the nodes index.
 * @param range_end     End of the nodes index.
 */
int transfer_data(struct pollfd pfds, struct h_table *table, uint8_t range_start,
                  uint8_t range_end)
{


    fprintf(stdout, "[Q18]\n");
    uint8_t(*ssn)[SSN_LENGTH];
    uint8_t *lname_length;
    uint8_t **lname;
    uint8_t *lemail_length;
    uint8_t **lemail;

    for (size_t i = 0; i < table_size(table); i++)
    {
        int amount = htable_empty_bucket(&ssn, &lname_length, &lname, &lemail_length, &lemail, i, table);
        for (size_t j = 0; j < amount; j++)
        {

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
  htable_destroy(table);
}

/**
 * @brief           Update a nodes range.
 *
 *                  When receving a new_range pdu containing
 *                  new a new min and max range, the nodes previous
 *                  index-range is updated accordingly.
 *                  A new table with the size of the received index-
 *                  range is created and marged with the previous
 *                  table. A response is sent to the sending node
 *                  To indicate that the table size has been updated.
 *
 * @param pfds      The poll file descriptors referring to the created
 *                  sockets.
 * @param table     Hashtable to store the data.
 * @param buffer    Received buffer.
 * @param size      Size of the buffer
 * @param range_start   Beginning of the nodes index.
 * @param range_end     End of the nodes index.
 */
void new_range(struct pollfd pfds,
               struct h_table **table,
               uint8_t *buffer,
               int size,
               uint8_t *min_range,
               uint8_t *max_range)
{

    struct NET_NEW_RANGE_PDU new_range;
    new_range.type = buffer[0];
    new_range.range_start = buffer[1];
    new_range.range_end = buffer[2];

    fprintf(stdout, "[Q15]\n");

    struct h_table *new_table = htable_create(new_range.range_end - new_range.range_start);

    if (new_range.range_end > *max_range)
    {
        htable_merge(table, new_table);
        *max_range = new_range.range_end;
    }
    else
    {
        htable_merge(&new_table, *table);
        *table = new_table;
        *min_range = new_range.range_start;
    }

    uint8_t response_buffer[1];
    response_buffer[0] = NET_NEW_RANGE_RESPONSE;
    fprintf(stdout, "sending new range response:...\n");
    pfds.events = POLLOUT;
    poll(&pfds,1,0);
    int res = send(pfds.fd, &response_buffer, sizeof(response_buffer), 0);
    if(res == -1){
        perror("Send");
    }
}
/**
 * @brief           Handle the reconnection of a successor node.
 *
 *                  When a successor is leaving, the remaning node
 *                  has to "reconnect" to a new successor (if one
 *                  exists). This is done by receving a new_leaving
 *                  request from the leaving node, containing its
 *                  successor address. This is used by the receving
 *                  node to, first, disconnect from the leaving node
 *                  and then connect to the address received. If
 *                  no successor to the leaving node exists, the address
 *                  sent in the pdu is set to 0.
 *
 *
 * @param pfds      The poll file descriptors referring to the created
 *                  sockets.
 * @param buffer    Received buffer.
 */
int node_leaving(struct pollfd pfds[MAX_SOCK], struct sockaddr_in *succ_addr, uint8_t *buffer)
{

    fprintf(stdout, "[Q16]\n");
    //close successor
    shutdown(pfds[1].fd,SHUT_RDWR);
    close(pfds[1].fd);

    //connect to new successor
    struct sockaddr_in addr;
    struct sockaddr_in my_addr;
    socklen_t my_addr_len = sizeof(my_addr_len);

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = buffer[1] | buffer[2] << 8 |
                           buffer[3] << 16 | buffer[4] << 24;
    addr.sin_port = buffer[5] | buffer[6] << 8;

    succ_addr->sin_addr.s_addr = addr.sin_addr.s_addr;
    succ_addr->sin_port = addr.sin_port;

    getsockname(pfds[2].fd, (struct sockaddr *)&my_addr, &my_addr_len);
    fprintf(stdout, "Connecting to: %s : %d\n", inet_ntoa(addr.sin_addr), addr.sin_port);
    if (addr.sin_addr.s_addr != 0 &&
        addr.sin_port != 0 &&
        addr.sin_addr.s_addr != my_addr.sin_addr.s_addr &&
        addr.sin_port != my_addr.sin_port)
    {
        pfds[1].fd = socket(AF_INET, SOCK_STREAM, 0);
        int time = 10;
        socklen_t len = sizeof(time);
        setsockopt(pfds[1].fd, SOL_SOCKET, SO_LINGER, &time, len);
        if (connect(pfds[1].fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
        {
            perror("Connect");
            exit(0);
        }
    }
    else
    {
        fprintf(stdout, "Node left but no successor node to connect to.\n");
        pfds[1].fd = socket(AF_INET, SOCK_STREAM, 0);
        int time = 10;
        socklen_t len = sizeof(time);
        setsockopt(pfds[1].fd, SOL_SOCKET, SO_LINGER, &time, len);
    }
    return 0;
}

/**
 * @brief           Accept new a predecessor node.
 *
 *                  When a predecessor is leaving, a new predecessor
 *                  should be accepted. If there is no successor connected, no
 *                  new accept need to be done. If not, a new
 *                  predecessor should be accepted. A new socket is created
 *                  for the file descriptor to be referring to.
 *
 * @param pfds      The poll file descriptors referring to the created
 *                  sockets.
 * @param buffer    Received buffer.
 */
 int close_connection(struct pollfd pfds[MAX_SOCK], uint8_t *buffer)
 {

   fprintf(stdout, "[Q17]\n");
   shutdown(pfds[3].fd,SHUT_RDWR);
   close(pfds[3].fd);
   pfds[2].events = POLLIN;
   if (poll(&(pfds[2]), 1, 50000) > 0)
   {
     pfds[3].fd = accept(pfds[2].fd, NULL, NULL);
     int time = 10;
     socklen_t len = sizeof(time);
     setsockopt(pfds[3].fd, SOL_SOCKET, SO_LINGER, &time, len);
     if (pfds[3].fd < 0)
     {
       perror("Accept");
       exit(0);
     }
     pfds[3].events = POLLIN;

   }
   else
   {
     fprintf(stdout, "No predecessor to accept.\n");
     pfds[3].fd = socket(AF_INET, SOCK_STREAM, 0);
     int time = 10;
     socklen_t len = sizeof(time);
     setsockopt(pfds[3].fd, SOL_SOCKET, SO_LINGER, &time, len);
     pfds[3].events = 0;
   }


   return 0;
 }
