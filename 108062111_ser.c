#include <stdio.h>
#include <stdlib.h>
#include <string.h>	// for bzero()
#include <unistd.h>	// for close()
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#define MAXNUM 10
#define MAXLEN 30
#define MAXBUF 500

int main(int argc, char *argv[])
{
	struct sockaddr_in serverAddress, clientAddress;
	int server_addr_length = sizeof(serverAddress);
    int client_addr_length = sizeof(clientAddress);
    int serverSocket, clientSocket;
	int ServerPortNumber;

	if(argc == 2){
		ServerPortNumber = atoi(argv[1]);
	}

   	serverSocket = socket(PF_INET, SOCK_STREAM, 0); // protocol family, TCP/UDP, 0: let system decide
	if(serverSocket < 0){
		fprintf(stderr, "Error creating socket : %s\n", strerror(errno));
		exit(0);
	}

 	bzero(&serverAddress, server_addr_length);
	serverAddress.sin_family = AF_INET; // address family: Internet family, IPv4
  	serverAddress.sin_port = htons(ServerPortNumber); // "Host byte order" to "Network byte order"
	serverAddress.sin_addr.s_addr = INADDR_ANY;

	if(bind(serverSocket,(struct sockaddr *) &serverAddress, server_addr_length) == -1){
		fprintf(stderr, "Error binding : %s\n", strerror(errno));
		close(serverSocket);
		exit(0);
	}

	if(listen(serverSocket, 3) == -1){
		fprintf(stderr, "Error listening : %s\n", strerror(errno));
		close(serverSocket);
		exit(0);
	}

	printf("Waiting...\n");
	if((clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddress, &client_addr_length)) == -1){
		// accept: return new socket descriptor
		// serverSocket is still listening
		printf("accept failed\n");
		close(serverSocket);
		exit(0);
	}
	printf("Client connect successfully\n");

	int i;
	int bytesRecv, bytesSend;
	int messageNumber = 0;
	char messageList[MAXNUM][MAXLEN];
    char send_buf[MAXBUF];
	char recv_buf[MAXBUF];
    char *menu = "\
\n\n--- Menu ---\n\
1. Read all existing massages.\n\
2. Write a new message.\n\
Please enter your operation : \0";

	char *show = "All existing messages :\n\0";
	char *input = "Please enter your message : \0";
	char *isFull = "The message board is full. You cannot enter new message.\0";

	// Send menu to client
	send_buf[0] = '\0';
	strcat(send_buf, menu);
	bytesSend = send(clientSocket, send_buf, sizeof(send_buf), 0);
	if(bytesSend < 0) printf("Error sending packet\n");


  	while(1){
		bytesRecv = recv(clientSocket, recv_buf, sizeof(recv_buf), 0);
		if(bytesRecv < 0) printf("Error receiving packet\n");

		printf("%s\n", recv_buf);

		if(!strncmp(recv_buf, "1", 1)){
			send_buf[0] = '\0';
			strcat(send_buf, show);
			for(i = 0; i < messageNumber; i++){
                strcat(send_buf, messageList[i]);
                strcat(send_buf, "\n\0");
			}
			strcat(send_buf, menu);
			bytesSend = send(clientSocket, send_buf, sizeof(send_buf), 0);
			if(bytesSend < 0) printf("Error sending packet\n");
		}
		else if(!strncmp(recv_buf, "2", 1)){
            if(messageNumber >= MAXNUM){
                send_buf[0] = '\0';
                strcat(send_buf, isFull);
                strcat(send_buf, menu);
                bytesSend = send(clientSocket, send_buf, sizeof(send_buf), 0);
                if(bytesSend < 0) printf("Error sending packet\n");
            }
            else {
                send_buf[0] = '\0';
                strcat(send_buf, input);
                bytesSend = send(clientSocket, send_buf, sizeof(send_buf), 0);
                if(bytesSend < 0) printf("Error sending packet\n");

                bytesRecv = recv(clientSocket, recv_buf, sizeof(recv_buf), 0);
                if(bytesRecv < 0) printf("Error receiving packet\n");
                printf("%s!\n", recv_buf);

                strcpy(messageList[messageNumber++], recv_buf);
                messageList[messageNumber - 1][MAXLEN - 1] = '\0';

                send_buf[0] = '\0';
                strcat(send_buf, menu);
                bytesSend = send(clientSocket, send_buf, sizeof(send_buf), 0);
                if(bytesSend < 0) printf("Error sending packet\n");
            }
		}
		else{
			bytesSend = send(clientSocket, menu, strlen(menu), 0);
			if(bytesSend < 0) printf("Error sending packet\n");
		}


	}

	return 0;
}



