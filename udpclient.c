/* udp_client.c */ 
/* Programmed by Adarsh Sethi */
/* Sept. 13, 2018 */

#include <stdio.h>          /* for standard I/O functions */
#include <stdlib.h>         /* for exit */
#include <string.h>         /* for memset, memcpy, and strlen */
#include <netdb.h>          /* for struct hostent and gethostbyname */
#include <sys/socket.h>     /* for socket, sendto, and recvfrom */
#include <netinet/in.h>     /* for sockaddr_in */
#include <unistd.h>         /* for close */
#include <math.h>	    /* for exponents, specifically pow() */

#define STRING_SIZE 1024

struct clientData {
    /* Values from the client */
    short count;          /* Number of data characters in the packet (0-80) */
    short seqNum;		/* 0 or 1, in accordance with the stop and wait protocol*/
    char data[80];       /* Characters from the document sent from the client */
}dataSend;

struct response {
    /* Response from the server */
    short ACK;            /* ACK to return for the data packet (0 or 1) */
}ack;

void ClearData(char *data){ //Pass by reference so the char array is modified directly
	memset(data, ' ', 80*sizeof(char));
}

int sock_client;  /* Socket used by client */ 

struct sockaddr_in client_addr;  /* Internet address structure that
                                        stores client address */
unsigned short client_port;  /* Port number used by client (local port) */

struct sockaddr_in server_addr;  /* Internet address structure that
                                        stores server address */
struct hostent * server_hp;      /* Structure to store server's IP
                                        address */
char server_hostname[STRING_SIZE]; /* Server's hostname */
unsigned short server_port;  /* Port number used by server (remote port) */

char sentence[STRING_SIZE];  /* send message */
char modifiedSentence[STRING_SIZE]; /* receive message */
unsigned int msg_len;  /* length of message */
int bytes_sent, bytes_recd; /* number of bytes sent or received */

struct timeval tv; /* Struct used to set the timeout value */

int main(int argc, char* argv[]) {
   int timeoutExp;
   dataSend.seqNum = 0; //Initialize the sequence number to 0

   if(argc == 2){
   	timeoutExp = atof(argv[1]);
	if(timeoutExp > 10){
		printf("Timeout too large, select a number between 0 and 10, inclusive");
		exit(0);
	}
   }else{
	printf("Usage:\n./udpclient <timeout exponent>\n");
	exit(0);
   }

   tv.tv_usec = pow(10.0,timeoutExp); //Set the timeout value in microseconds
   tv.tv_sec = tv.tv_usec * 1/(double)pow(10.0,6.0); //Set the timeout value in seconds


  
   /* open a socket */

   if ((sock_client = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
      perror("Client: can't open datagram socket\n");
      exit(1);
   }

   setsockopt(sock_client, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv)); //Sets the timeout value for the socket defined

   /* Note: there is no need to initialize local client address information
            unless you want to specify a specific local port.
            The local address initialization and binding is done automatically
            when the sendto function is called later, if the socket has not
            already been bound. 
            The code below illustrates how to initialize and bind to a
            specific local port, if that is desired. */

   /* initialize client address information */

   client_port = 0;   /* This allows choice of any available local port */

   /* Uncomment the lines below if you want to specify a particular 
             local port: */
   /*
   printf("Enter port number for client: ");
   scanf("%hu", &client_port);
   */

   /* clear client address structure and initialize with client address */
   memset(&client_addr, 0, sizeof(client_addr));
   client_addr.sin_family = AF_INET;
   client_addr.sin_addr.s_addr = htonl(INADDR_ANY); /* This allows choice of
                                        any host interface, if more than one 
                                        are present */
   client_addr.sin_port = htons(client_port);

   /* bind the socket to the local client port */

   if (bind(sock_client, (struct sockaddr *) &client_addr,
                                    sizeof (client_addr)) < 0) {
      perror("Client: can't bind to local address\n");
      close(sock_client);
      exit(1);
   }

   /* end of local address initialization and binding */

   /* initialize server address information */

   printf("Enter hostname of server: ");
   scanf("%s", server_hostname);
   if ((server_hp = gethostbyname(server_hostname)) == NULL) {
      perror("Client: invalid server hostname\n");
      close(sock_client);
      exit(1);
   }
   printf("Enter port number for server: ");
   scanf("%hu", &server_port);

   /* Clear server address structure and initialize with server address */
   memset(&server_addr, 0, sizeof(server_addr));
   server_addr.sin_family = AF_INET;
   memcpy((char *)&server_addr.sin_addr, server_hp->h_addr,
                                    server_hp->h_length);
   server_addr.sin_port = htons(server_port);

   /* user interface */


   FILE *fp;
   char str[81];
   fp = fopen("test1.txt", "r");

   /* opening file for reading */
   fp = fopen("test1.txt", "r");
   if (fp == NULL) {
	   perror("Error opening file");
	   return(-1);
   }

   while (!feof(fp)) {
	   ClearData(dataSend.data); //Clear the data field of the struct
	   dataSend.count = 0;
	   if (fgets(str, 81, fp) != NULL) {
		   /* removing null character */
		   for (int i = 0; str[i] != '\0'; i++) {
			   dataSend.data[i] = str[i];
			   dataSend.count++;
			   if(str[i] == '\n'){
			   	break;
			   }
		   }
	   }
	   msg_len = sizeof(dataSend);
	   dataSend.count = htons(dataSend.count); //Convert the count variable to newtwork form
	   dataSend.seqNum = htons(dataSend.seqNum); //Convert the sequence number to network form
	   bytes_sent = sendto(sock_client, &dataSend, msg_len, 0, (struct sockaddr *) &server_addr, sizeof (server_addr));

   }
   
   fclose(fp);




/*
   printf("Please input a sentence:\n");
   scanf("%s", sentence);
   msg_len = strlen(sentence) + 1;
*/

   /* send message */
  
  
   /* get response from server */
  
   printf("Waiting for response from server...\n");
   bytes_recd = recvfrom(sock_client, modifiedSentence, STRING_SIZE, 0,
                (struct sockaddr *) 0, (int *) 0);
   printf("\nThe response from server is:\n");
   printf("%s\n\n", modifiedSentence);

   /* close the socket */

   close (sock_client);
}
