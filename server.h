#include "game.h"

void construct_connection();
int handle_msg(struct client_data* client, struct client_data* ss, int sockfd);
int handle_new_client(struct client_data* client, struct client_data* ss, int sockfd);
void update_clients(struct client_data* ss, int sockfd);
static void update_clients_handler(int signum);
int find_free_slot(struct client_data* ss);
void clr_connections(struct client_data* ss);
void print_clnt_msg(struct client_msg* msg);
void print_clnt_data(struct client_data* data);




