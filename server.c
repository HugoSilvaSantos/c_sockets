/*
Socket Server
To compile: $ gcc -o server server.c
TO run: ./server <port>
Example: ./server 30000
*/
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFFSIZE 255

//------------------------------------------------------------------------------
//Copy Function

int cp(const char *to, const char *from)
{
	int fd_to, fd_from;
	char buf[4096];
	ssize_t nread;
	int saved_errno;

	fd_from = open(from, O_RDONLY);
	if (fd_from < 0)
		return -1;

	fd_to = open(to, O_WRONLY | O_CREAT | O_EXCL, 0666);
	if (fd_to < 0)
		goto out_error;

	while (nread = read(fd_from, buf, sizeof buf), nread > 0)
	{
		char *out_ptr = buf;
		ssize_t nwritten;

		do {			
			nwritten = write(fd_to, out_ptr, nread);

			if (nwritten >= 0)
			{
				nread -= nwritten;
				out_ptr += nwritten;
			}
			else if (errno != EINTR)
			{
				goto out_error;
			}
		} while (nread > 0);
	}

	if (nread == 0)
	{
		if (close(fd_to) < 0)
		{
			fd_to = -1;
			goto out_error;
		}
		close(fd_from);

		/* Success! */
		return 0;
	}

	out_error:
	saved_errno = errno;

	close(fd_from);
	if (fd_to >= 0)
			close(fd_to);

	errno = saved_errno;
	return -1;
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
// Difference between two files

int diff(char* fname1, char* fname2)
{	
	FILE *fp1, *fp2;
	int ch1, ch2;

	//Open and read the two files
	fp1 = fopen(fname1, "a+");
	fp2 = fopen(fname2, "a+");

	if (fp1 == NULL) {
		printf("Cannot open %s for reading ", fname1);
		exit(1);
	} else if (fp2 == NULL) {
		printf("Cannot open %s for reading ", fname2);
		exit(1);
	} else {		

		ch1 = getc(fp1);
		ch2 = getc(fp2);

		while ((ch1 != EOF) && (ch2 != EOF) && (ch1 == ch2)) {
			ch1 = getc(fp1);
			ch2 = getc(fp2);
		}
		fclose(fp1);
		fclose(fp2);

		//Compares the two files
		if (ch1 == ch2)
		{
			// printf("Files are identical\n");
			return(0);
		}
		else if (ch1 != ch2)
		{
			// printf("Files are Not identical\n");
			return(1);      
		} 
	}
}

//------------------------------------------------------------------------------

void Err(char *mess) {
	perror(mess);
	exit(1);
}

//------------------------------------------------------------------------------
//Main 
int main(int argc, char *argv[]) {	

	int sock;
	struct sockaddr_in echoserver;
	struct sockaddr_in echoclient;
	char buffer[BUFFSIZE] = {0}; // buffer to filename
	char buffer2[BUFFSIZE] = {0}; // buffer to file contents
	unsigned int echolen, clientlen, serverlen;
	int received = 0;
	size_t nread;
	FILE *file; //*fp file pointer
//------------------------------------------------------------------------------
	
	if (argc != 2) {
		fprintf(stderr, "usage: %s <port>\n", argv[0]);
		exit(1);
		}
//------------------------------------------------------------------------------
// Create the UDP socket

	if ((sock = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
		Err("Failed to create socket");
	}
	/* Construct the server sockaddr_in structure */
	memset(&echoserver, 0, sizeof(echoserver));		// Clear struct
	echoserver.sin_family = AF_INET;
	echoserver.sin_addr.s_addr = htonl(INADDR_ANY);	// Any IP address
	echoserver.sin_port = htons(atoi(argv[1]));		// Server port
	
	/* Bind the socket */
	serverlen = sizeof(echoserver);
	if (bind(sock, (struct sockaddr *) &echoserver, serverlen) < 0) {
		Err("Failed to bind server socket");
	}

	clientlen = sizeof(echoclient);
//------------------------------------------------------------------------------
	while (1) {

		/* Receive a message from client */
		if ((received = recvfrom(sock, buffer, BUFFSIZE, 0, (struct sockaddr *) &echoclient, &clientlen)) < 0) {
			Err("Failed to receive message");
		}
		printf("Client connected: %s\n", inet_ntoa(echoclient.sin_addr));
				
//------------------------------------------------------------------------------
// Reads the contents of the file, and send to the client

		file = fopen(buffer, "r");
		if (file != NULL)
		{
			fprintf(stdout, "e diferente\n");
			printf("Read: %s\n", buffer);
			while ((nread = fread(buffer2, 1, sizeof (buffer2), file)) >0)
				fwrite(buffer2, 1, nread, stdout);
				printf("\n");
				fclose(file);
		}
		if (sendto(sock, buffer2, received, 0, (struct sockaddr  *) &echoclient, sizeof(echoclient)) != received) {
			Err("Mismatch in number of echo'd bytes");
		}
		memset(buffer2,0,BUFFSIZE);
//------------------------------------------------------------------------------
// Creation of a temp file to monitorize the differeces

		while(1){

			// printf("Initiating the comparison\n");
			// cp(const char *to, const char *from)
			// creates a temporary file
			cp("temp.txt", buffer);
			// printf("Copy done...\n");

			// verification for diferences on the files
			while(diff("temp.txt", buffer) == 0)
			{
			// If the files are identical compares again, util changes
			diff("temp.txt", buffer);
			// printf("Comparison Done...\n");
			sleep(5);
			}
			// When differences the contents are sended to the client

			file = fopen(buffer, "r");
			if (file==NULL)
			{
				printf("Impossible to read: %s\n", buffer);
			}
			else
			{	
				printf("Reading: %s\n", buffer);
				while ((nread = fread(buffer2, 1, sizeof (buffer2), file)) >0)					
					fwrite(buffer2, 1, nread, stdout);
					printf("\n");
					fclose(file);
			}
			//Delete the temporary file
			remove("temp.txt");
			// sleep(5);
//------------------------------------------------------------------------------
// Send the message back to client
			
			if (sendto(sock, buffer2, received, 0, (struct sockaddr  *) &echoclient, sizeof(echoclient)) != received) {
				Err("Mismatch in number of echo'd bytes");
			}
			memset(buffer2,0,BUFFSIZE); //del buffer data
		}
	}
}
