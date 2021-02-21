#include "node.h"

/**
 *          Program to represent a node. Nodes should be able to
 *          connect to eachother and form a network. The nodes in
 *          the network can pass inforamtion to/from its successor
 *          and predecessor. The nodes should store their own
 *          hashtable with data passed over UDP from a client.
 *          The initialization of address is done though a tracker.
 *
 * @author  dv18mln dv18mfg
 *
 * @{
 */

volatile sig_atomic_t connected;
volatile sig_atomic_t term_signal;

/**
 *        Main handles foremost communication with the tracker.
 *        The function handles the init of addresses to a node by
 *        communicating with a tracker. The tracker returns the
 *        address of the sending node. The tracker is also used
 *        by a joining node to request its predecessors address.
 */
int main(int argc, char const *argv[]){


  TEST t = { 0,NULL,NULL};
  printf("%d", t.number);
  return 0;

  struct h_table *table;
  struct pollfd *pfds = malloc(MAX_SOCK * sizeof(struct pollfd));
  struct NET_GET_NODE_RESPONSE_PDU net_response;
  struct sockaddr_in serv_addr, my_addr, succ_addr;
  term_signal = 0;
  connected = 0;
  int type = -1;
  uint8_t range_start = 0;
  uint8_t range_end = 0;
  uint8_t *buffer;
  int size = 300;
  struct in_addr temp;

  if (argc < 3){
    fprintf(stderr, "usage: %s <tracker_address> <tracker_port>\n", argv[0]);
    exit(0);
  }
  else{
    fprintf(stdout, "using: %s : %s\n", argv[1], argv[2]);
  }

  serv_addr = Q1(pfds, argv[1], argv[2]);

  signal(SIGINT, sighandler);

  temp = Q2(pfds);

  //port is given in network byte order.
  socklen_t addr_size = sizeof(my_addr);

  if(getsockname(pfds[0].fd, (struct sockaddr *)&my_addr, &addr_size) < 0){
    perror("Getsockname");
  }

  fprintf(stdout, "My Port: %d\n", ntohs(my_addr.sin_port));

  my_addr.sin_addr = temp;

  my_addr.sin_port = 0;

  if (bind(pfds[2].fd, (struct sockaddr *)&my_addr, sizeof(my_addr)) == -1){
  perror("Bind");
  exit(0);
  }


  if (listen(pfds[2].fd, 10) == -1){
  perror("Listen");
  exit(0);
  }


  net_response = Q3(serv_addr, pfds);

  if (net_response.address == 0 && net_response.port == 0)
  {
    fprintf(stdout, "[Q4] I am the first node to join the network!\n");
    range_start = 0;
    range_end = 255;
    table = htable_create(range_end - range_start);
    connected = 1;
  }
  else{
    fprintf(stdout, "[Q7] Not the first to join the network.\n");
    Q7(pfds, net_response, my_addr, range_start, range_end);
    connected = 1;
    pfds[3].events = POLLIN;
    poll(&(pfds[3]), 1, 0);
    uint8_t net_join_response[10];
    recv(pfds[3].fd,
                   net_join_response,
                   sizeof(uint8_t) * 10,
                   0);
    Q8(pfds, net_join_response, &table, &range_start, &range_end, &succ_addr);
  }

  while(!term_signal){

    buffer = malloc(sizeof(uint8_t) * size);
    int fd = update(pfds, buffer, size, serv_addr);
    type = buffer[0];

    switch(type){
      case VAL_INSERT:
        insert_val(pfds, table, buffer, size, range_start, range_end);
        break;
      case VAL_LOOKUP:
        lookup_val(pfds, table, buffer, size, range_start, range_end);
        break;
      case VAL_REMOVE:
        remove_val(pfds, table, buffer, size, range_start, range_end);
        break;
      case NET_JOIN:
        fprintf(stdout, "Net join request\n");
        joinNewNode(pfds, buffer, size, table, my_addr, &succ_addr, &range_start, &range_end);
        break;
      case NET_NEW_RANGE:
        fprintf(stdout, "New range, pfds: %d\n", fd);
        new_range(pfds[fd], &table, buffer, size, &range_start, &range_end);
        break;
      case NET_LEAVING:
        fprintf(stdout, "Net leaving\n");
        node_leaving(pfds, &succ_addr, buffer);
        break;
      case NET_CLOSE_CONNECTION:
        fprintf(stdout, "Net close connection\n");
        close_connection(pfds, buffer);
        break;
      default:
        fprintf(stdout, "Type: %d\nSocket: %d\n", type , fd);
        break;
    }
    if(term_signal == 1){
      shutdown_request(pfds, table, succ_addr, &range_start, &range_end);
    }
  free(buffer);
  }

  close_socket(pfds);
  printf("done\n");
  return 0;
}

/**
 * Sends STUN_LOOKUP to the tracker.
 *
 *  @param pfds Poll filedescriptor to handle the socket.
 *  @param ip_address Ip address to tracker.
 *  @param port Port to tracker
 *  @return sockaddr struct to hold tracker information.
 */
struct sockaddr_in Q1(struct pollfd pfds[MAX_SOCK],
                      const char *ip_address,
                      const char *port) {

  struct sockaddr_in serv_addr;
  int sockets[MAX_SOCK];

  /*
   * 0: UDP sending/recieving to/from tracker
   * 1: TCP sending/recieving to/from successor
   * 2: TCP acceping new connections
   * 3: TCP sending/recieving to/from predecessor
   */
  sockets[0] = socket(AF_INET, SOCK_DGRAM, 0);
  sockets[1] = socket(AF_INET, SOCK_STREAM, 0);
  sockets[2] = socket(AF_INET, SOCK_STREAM, 0);
  sockets[3] = socket(AF_INET, SOCK_STREAM, 0);
  for (size_t i = 0; i < MAX_SOCK; i++) {
    if (sockets[i] == -1){
        perror("socket error");
        exit(1);
    }
  }

  serv_addr.sin_family = AF_INET;
  //converts the unsigned integer host byte order to network byte order.
  serv_addr.sin_port = htons(atoi(port));

  for (size_t i = 0; i < MAX_SOCK; i++){
    pfds[i].fd = sockets[i];
    int time = 10;
    socklen_t len = sizeof(time);
    setsockopt(pfds[i].fd, SOL_SOCKET, SO_LINGER, &time, len);
  }
  /********Needed for poll to work on windows, always needed*********/
  pfds[0].events = POLLIN;
  pfds[1].events = 0;
  pfds[2].events = 0;
  pfds[3].events = 0;



  int p = poll(pfds, 1, TIMEOUT);

  if(pfds[0].revents == 0){

    fprintf(stdout, "Node started, sending STUN_LOOKUP ");
    fprintf(stdout, "to tracker: %s : %s\n", ip_address, port);

    struct STUN_LOOKUP_PDU lookup;
    lookup.type = 200;

    int res = sendto(pfds[0].fd, &lookup, sizeof(lookup), 0,
                    (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    if(res == -1){
        perror("Error sending data");
        exit(0);
    }
  }

  return serv_addr;
}

/**
 * Receive and handle the response from tracker.
 * The response contains the nodes Ip address.
 *
 * @param pfds Poll fd to handle the socket.
 * @return in_addr struct to hold the nodes ip address and information.
 */
struct in_addr Q2(struct pollfd pfds[MAX_SOCK]){

  struct STUN_RESPONSE_PDU response;
  struct sockaddr from;

  char buffer[5];
  socklen_t addrlen = sizeof(from);
  int res = recvfrom(pfds[0].fd, buffer, sizeof(buffer), 0, &from, &addrlen);
  if(res == -1){
    perror("receive from");
  }
  response.type = buffer[0];
  response.address = buffer[1] | buffer[2] << 8 |
                     buffer[3] << 16 | buffer[4] << 24;

  if(res == -1){
      perror("Could not get response");
      exit(0);
  }

  struct in_addr ip_addr;
  ip_addr.s_addr = response.address;
  fprintf(stdout, "Got STUN_RESPONSE, my address is: %s\n", inet_ntoa(ip_addr));

  return ip_addr;
}

/**
 * Send and receive infromation of other nodes from the tracker.
 * The function sends a NET_GET_NODE_PDU to the tracker to get
 * the first node of the tracker.
 *
 * If the tracker response is empty the calling node is the first
 * to join the network.
 *
 * TODO: Handle the information of other nodes in the network.
 *
 * @param serv_addr struct containing tracker ip and information.
 * @param pfds Poll fd to handle the socket.
 * @return 0 if node is the first to join, 1 if other nodes exist.
 */
struct NET_GET_NODE_RESPONSE_PDU Q3(struct sockaddr_in serv_addr,
                                    struct pollfd pfds[MAX_SOCK]){

  struct sockaddr from;
  struct NET_GET_NODE_RESPONSE_PDU get_Node_Response;

  int p = poll(pfds, 0, TIMEOUT);
  if(p == -1){
    perror("Could not use poll.");
    exit(-1);
  }

  if (pfds[0].revents == 0){

    struct NET_GET_NODE_PDU getNode;
    getNode.type = 1;
    pfds[0].events = POLLOUT;
    poll(&(pfds[0]),1,0);
    int res = sendto(pfds[0].fd, &getNode, sizeof(getNode), 0,
                    (struct sockaddr *)&serv_addr, sizeof(serv_addr));

    if (res == -1){
      perror("Error sending NET_GET_NODE_PDU");
      exit(0);
    }

    uint8_t buffer[7];
    socklen_t addrlen = sizeof(from);
    pfds[0].events = POLLIN;
    poll(&(pfds[0]),1,0);
    res = recvfrom(pfds[0].fd, buffer, sizeof(buffer), 0, &from, &addrlen);
    if (res == -1){
      perror("Error recv NET_GET_NODE_RESPONSE_PDU");
      exit(0);
    }

    get_Node_Response.type = buffer[0];
    get_Node_Response.address = buffer[1] | buffer[2] << 8 |
                                buffer[3] << 16 | buffer[4] << 24;
    //get_Node_Response.address = ntohl(get_Node_Response.address);
    //sometimes invalid port.
    get_Node_Response.port = buffer[5] | buffer[6] << 8;
    //get_Node_Response.port = ntohs(get_Node_Response.port);

  }
  else{
    //error message about unopen socket?
    get_Node_Response.address = 0;
    get_Node_Response.port = 0;
  }

  return get_Node_Response;
}

/**
 * Function used by the prospect node when joining the network.
 * Sends the join-request to the first node in the network.
 * The address to the first node is given in net_response.
 * The first node gets the address and port of the prospect
 * node to return a NET_JOIN_RESPONSE_PDU.
 *
 * @param pfds Poll fd to handle the socket.
 * @param net_response Get node response with ip of first node.
 * @param my_addr The address of the prospect node.
 */
void Q7(struct pollfd pfds[MAX_SOCK], struct NET_GET_NODE_RESPONSE_PDU net_response,
   struct sockaddr_in my_addr, int range_start, int range_end)
{

  //send net join to the node in the response
  struct NET_JOIN_PDU join;
  struct sockaddr_in cli;

  cli.sin_family = AF_INET;
  cli.sin_port = net_response.port;
  cli.sin_addr.s_addr = net_response.address;

  fprintf(stdout, "Sending to node: %s:%d\n", inet_ntoa(cli.sin_addr), net_response.port);
  //fprintf(stdout, "htons Sending to node: %s:%d\n", inet_ntoa(cli.sin_addr), htons(net_response.port));

  socklen_t len = sizeof(my_addr);
  getsockname(pfds[2].fd, (struct sockaddr *)&my_addr, &len);
  fprintf(stdout, "my address: %s:%d", inet_ntoa(my_addr.sin_addr),my_addr.sin_port);

  join.type = 3;

  join.max_span = range_end-range_start;

  join.src_address = my_addr.sin_addr.s_addr;

  //already in network byte order.
  join.src_port = my_addr.sin_port;

  join.max_address = 0;
  join.max_port = 0;

  uint8_t buffer[14], *ptr;
  memset(buffer, '\0', sizeof(buffer));
  buffer[0] = join.type;

  ptr = serialize(buffer, join.src_address, join.src_port);

  pfds[0].events = POLLOUT;
  poll(&(pfds[0]),1,0);
  int res = sendto(pfds[0].fd, buffer, sizeof(buffer), 0, (struct sockaddr *)&cli, sizeof(cli));
  if (res == -1){
    perror("Could not send net_join");
    exit(0);
  }

  fprintf(stdout, "\nTrying to accept... ");
  fflush(stdout);
  socklen_t size = sizeof(cli);

  close(pfds[3].fd);

  pfds[3].fd = accept(pfds[2].fd, (struct sockaddr *)&cli, &size);
  if (pfds[3].fd < 0){
    perror("Accept");
    fprintf(stdout, "%d\n", pfds[3].fd);
    exit(0);
  }
  int time = 10;
  socklen_t len_t = sizeof(time);
  setsockopt(pfds[3].fd, SOL_SOCKET, SO_LINGER, &time, len_t);
  pfds[3].events = POLLIN;

  /*pfds[3].fd = accept(pfds[2].fd, NULL, NULL);
  if (pfds[3].fd < 0){
    perror("Accept");
    fprintf(stdout, "%d\n", pfds[3].fd);
    exit(0);
  }*/
  fprintf(stdout, "Done!\n");
}

/**
 *              Received the response from a joined node.
 *              The reponse contains the new successor
 *              of the prospect node, along with the
 *              size to initialize the table.
 *
 * @param pfds        Poll fd to handle the socket.
 * @param buffer      Buffer to hold the received data.
 * @param table       Hashtable to initialize.
 * @param range_start Current starting index of node.
 * @param range_end   Current ending index of node.
 * @param succ_addr   Address to the current successor.
 */
void Q8(struct pollfd pfds[MAX_SOCK], uint8_t buffer[9], struct h_table **table,
        uint8_t *range_start, uint8_t *range_end, struct sockaddr_in *succ_addr)
{

  fprintf(stdout, "[Q8]\n");

  struct sockaddr from;
  socklen_t addrlen = sizeof(from);

  struct NET_JOIN_RESPONSE_PDU response;

  if (buffer[0] == NET_JOIN_RESPONSE)
  {
    //Successor node.
    response.next_address = buffer[1] | buffer[2] << 8 |
                            buffer[3] << 16 | buffer[4] << 24;
    //already in network byte order
    response.next_port = buffer[5] | buffer[6] << 8;

    response.range_start = buffer[7];
    response.range_end = buffer[8];

    *range_start = response.range_start;
    *range_end = response.range_end;
    //Init hashtable with responded size.
    (*table) = htable_create(response.range_end - response.range_start);

    succ_addr->sin_family = AF_INET;
    succ_addr->sin_addr.s_addr = response.next_address;
    succ_addr->sin_port = response.next_port;

    fprintf(stdout,"successor address %s %d\n", inet_ntoa(succ_addr->sin_addr), succ_addr->sin_port);

    if (connect(pfds[1].fd, (struct sockaddr *)succ_addr, sizeof(*succ_addr)) < 0)
    {
        perror("Connect");
        exit(0);
    }
    fprintf(stdout, "Connected\n");
  }
}

void sighandler(int sig)
{
  fprintf(stdout, "\nGot shutdown request...\n");
  term_signal = 1;
  if (connected == 0){
    fprintf(stdout, "Not yet connected, goodbye!\n");
    exit(0);
  }
}
