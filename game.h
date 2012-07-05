struct client_msg {
	struct timeval ts;
	float x;
	float y;
};

struct client_data{
	int client_id;
	struct sockaddr_in destaddr;
	struct client_msg msg;
};

#define HOST_PORT 5000
