/*
Socket client
To compile: $ gcc -o client client.c
TO run: ./client <server_ip> <name> <extension> <port>
Example: ./client 127.0.0.1 file .txt 30000
*/

#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFFSIZE 255

//------------------------------------------------------------------------------
// Tiny error handler

void Err(char *mess) {  		
	perror(mess);
	exit(1);
}
//------------------------------------------------------------------------------
// String concat

char* concat(char *s1, char *s2)
{
	char *result = malloc(strlen(s1)+strlen(s2)+1);//+1 for the zero-terminator
	strcpy(result, s1);
	strcat(result, s2);
	return result;
}

//------------------------------------------------------------------------------
//Main

int main (int argc, char *argv[]) {

	int sock; // socket handler
	struct sockaddr_in echoserver;
	struct sockaddr_in echoclient;
	char buffer[BUFFSIZE] = {0};        // Buff to hold returned string
	unsigned int echolen, clientlen;    // string's size holders
	int received = 0;
	
	if (argc != 5) {
		fprintf(stderr, "usage: %s <server_ip> <name> <extension> <port>\n", argv[0]);
		exit(1);
	}

//------------------------------------------------------------------------------
// Creating the UDP socket

	if ((sock = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
		Err("Failed to create socket");
	}
	
	/* Construct the server sockaddr_in structure */
	memset(&echoserver, 0, sizeof(echoserver));	// Clear struct
	echoserver.sin_family = AF_INET;
	echoserver.sin_addr.s_addr = inet_addr(argv[1]); // IP address
	echoserver.sin_port = htons(atoi(argv[4])); // Server port
	

	echolen = 255; 
	char* s = concat(argv[2], argv[3]); //concat filename and file extension

//------------------------------------------------------------------------------
	while(1){
		memset(buffer,0,BUFFSIZE);
		/*Send message to the server*/
		if (sendto(sock, s, echolen, 0, (struct sockaddr *) &echoserver, sizeof(echoserver)) != echolen) {
			Err("Mismatch in number of sent bytes");
		}
		else {
			fprintf(stdout, "Message sent \n"); 
			fprintf(stdout, "----------------------------\n");
		}
//------------------------------------------------------------------------------
// Receive back from the server

		fprintf(stdout, "Received from server: \n");
		clientlen = sizeof(echoclient);

		if ((received = recvfrom(sock, buffer, BUFFSIZE, 0, (struct sockaddr *) &echoclient, &clientlen)) != echolen) {
			Err("Mismatch in number of received bytes");
		}
		/* Check that client and server are using the same socket */
		if (echoserver.sin_addr.s_addr != echoclient.sin_addr.s_addr) {
			Err("Received a packet from an unexpected server");
		}
		// fflush(buffer);
		buffer[received] = '\0'; // Assure null terminated string

		fprintf(stdout, "%s", buffer); // buffer printed on the client
		fprintf(stdout, "%s", "\n");
		fprintf(stdout, "----------------------------\n");
	}
}