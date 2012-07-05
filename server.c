#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <stdio.h>
#include <errno.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <signal.h>

#include "server.h"

#define MAX_CLIENTS 200
#define MAX_MSGSIZE 2000
#define TIC_RATE 500000

/*PLAN: maintain shared 'server state', shared memory
structure that contains all info that, when updated, is rebroadcast to every client.

1.server maintains udp info for every 
client connected to game. 
2.Select()'s from among the list. reads new state 
3. create 'changed state' struct. aggregate changes 
over some time period, (1/10 second say) then broadcast to clients all changes in that time period.
*/

int next_fid;
struct client_data srv_state[MAX_CLIENTS];	
struct client_data dirty_state[MAX_CLIENTS];
int openfd;

int main(int argc, char* argv[]){

	//printf("argc:%d arg1:%s\n", argc, argv[1]);
	clr_connections(srv_state);
	clr_connections(dirty_state);

	int cont =1;
	next_fid =0;
	
	struct sigaction act;
	act.sa_handler = (update_clients_handler);
	if(sigaction(SIGALRM, &act, NULL)){
		printf("error setting alarm handler\n");
	}
	struct timeval tv;
	//kill(getpid(), SIGVTALRM);
	tv.tv_sec=0;
	tv.tv_usec = TIC_RATE;
	const struct itimerval it_update = {tv, tv};
	if(setitimer(ITIMER_REAL, &it_update, NULL)) perror("setitimer failure");

	openfd = socket(AF_INET, SOCK_DGRAM, 0);

	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(HOST_PORT);
	bind(openfd, (struct sockaddr*)&server_addr, sizeof(struct sockaddr_in));

	while(cont){
		struct client_data new_client;
		int socklen = sizeof(struct sockaddr_in);
		if(recvfrom(openfd, &new_client, sizeof(struct client_data), 0, (struct sockaddr*)&new_client.destaddr, &socklen)!=0){
			printf("recieved msg. client_id=%d, address=%s\n", new_client.client_id, inet_ntoa(new_client.destaddr.sin_addr));
			if(new_client.client_id>-1){
				if(handle_msg(&new_client, dirty_state, openfd)){
					printf("error handle_msg()\n");
					return -1;
					}
			}
			else{
				if(handle_new_client(&new_client, dirty_state, openfd)){
					printf("error handle_msg()\n");
					return -1;
				}
			}
		}
	}
	return 0;
}
int handle_msg(struct client_data* msg, struct client_data* ss, int sockfd){
	printf("in handle_msg. Got message from client_id=%d\n", msg->client_id);
	print_clnt_data(msg);	
	memcpy(&ss[msg->client_id], msg, sizeof(struct client_data));
	return 0;
}

int handle_new_client(struct client_data* msg, struct client_data* ss, int sockfd){
	if(next_fid>MAX_CLIENTS){
		printf("next client_id(%d > max_clients(%d)\n", next_fid, MAX_CLIENTS);
		return -1;
	}
	printf("recieved new client request. addr=%s\n", inet_ntoa(msg->destaddr.sin_addr));
	msg->client_id = next_fid;
	memcpy(&ss[next_fid], msg, sizeof(struct client_data));	
	printf("assigned client id=%d ", ss[next_fid].client_id);
	next_fid = find_free_slot(ss);
	printf("and set next_fid=%d\n", next_fid);
	return 0;
}

static void update_clients_handler(int signum){
	printf("updating clients\n");
	int i;
	for(i=0; i<MAX_CLIENTS; i++){
		if(dirty_state[i].client_id!=-1){
			printf("updating client(i=%d) client_id=%d\n", i, dirty_state[i].client_id);
			sendto(openfd, dirty_state, 200*sizeof(struct client_data), 0, (struct sockaddr*)&dirty_state[i].destaddr, sizeof(struct sockaddr_in));	
		}
	}
	printf("done updating clients\n");
}
int find_free_slot(struct client_data* ss){
	int i;
	for(i=0; i<MAX_CLIENTS; i++){
		if(ss[i].client_id==-1){
			next_fid = i;
			return next_fid;
		}
	}
	printf("error no more clients allowed\n");
	return -1;
}
void clr_connections(struct client_data* ss){
	int i;
	//memset(ss, '0', 200*sizeof(struct client_data));
	for(i=0; i<MAX_CLIENTS; i++){
		ss[i].client_id=-1;
	}
}

void print_clnt_data(struct client_data* msg){
	printf("printing client_data\n");
	printf("client_data:\n\tclient_id:%d\n\tdestaddr(%s)\n\t", msg->client_id, inet_ntoa(msg->destaddr.sin_addr));
	print_clnt_msg(&msg->msg);
}
void print_clnt_msg(struct client_msg* msg){
	printf("client_msg:\n\t\t x:%g, y:%g, ts.tv_sec:%d\n", msg->x, msg->y, (int)msg->ts.tv_sec);
}
