#include <sys/socket.h>
#include <stdlib.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <netinet/in.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <curses.h>

#include "client.h"

#define SRV_ADDR "127.0.0.1"

int gID;
struct client_data myState;

int main(int argc, char* argv[]){

	//printw("argc:%d arg1:%s\n", argc, argv[1]);
	initscr();
	cbreak();
	keypad(stdscr, 1);
	noecho();

	int openfd = socket(AF_INET, SOCK_DGRAM, 0);

	//initialize server address
	struct sockaddr_in server_addr;
	printw("host addr declared\n");
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(SRV_ADDR);
	printw("SRV_ADDR assigned to host addr:%s\n",
		inet_ntoa(server_addr.sin_addr));
	server_addr.sin_port = htons(HOST_PORT);
	printw("local socket created, and addr inited\n");


	//initialize client_data and msg
	myState.client_id=-1;
	myState.msg.x=1;
	myState.msg.y=1;
	gettimeofday(&myState.msg.ts, NULL);	
	
	sendto(openfd, &myState, sizeof(struct client_data), 0, (struct sockaddr*)&server_addr, sizeof(struct sockaddr_in));
	printw("message sent\n");
	refresh();
	
	//TODO:recieve response from server with new state, id, etc
	struct client_data result;
	int socklen = sizeof(struct sockaddr_in);
	recvfrom(openfd, &result, sizeof(struct client_data), 0, (struct sockaddr*)&server_addr, &socklen);
	myState.client_id = gID;
	
	int i;
	for(;;){
		//TODO: actual game function stuff. 
		refresh();
		memset(&result, '0', sizeof(struct client_data));
		recvfrom(openfd, &result, sizeof(struct client_data), MSG_DONTWAIT, (struct sockaddr*)&server_addr, &socklen);
		struct client_msg* new_pos = &result.msg;
		new_pos->x =i+1;
		new_pos->y =i+1;
		gettimeofday(&new_pos->ts, NULL);
		printw("time of day set to %ld\n", new_pos->ts);
		sendto(openfd, &result, sizeof(struct client_data), 0, (struct sockaddr*)&server_addr, sizeof(struct sockaddr_in));

	}
	close(openfd);
	endwin();
}
