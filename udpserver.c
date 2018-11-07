/* udp_server.c */
/* Programmed by Adarsh Sethi */
/* Sept. 13, 2018 */

#include <ctype.h>          /* for toupper */
#include <stdio.h>          /* for standard I/O functions */
#include <stdlib.h>         /* for exit */
#include <string.h>         /* for memset */
#include <sys/socket.h>     /* for socket, sendto, and recvfrom */
#include <netinet/in.h>     /* for sockaddr_in */
#include <unistd.h>         /* for close */
#include <stdlib.h>         /* for atof */
#include <time.h>           /* for seeding srand */

#define STRING_SIZE 1024

/* SERV_UDP_PORT is the port number on which the server listens for
 *   incoming messages from clients. You should change this to a different
 *   number to prevent conflicts with others in the class. 
 */

#define SERV_UDP_PORT 20987

struct clientData {
    /* Values from the client */
    int count;          /* Number of data characters in the packet (0-80) */
    int seqNum;		/* 0 or 1, in accordance with the stop and wait protocol*/
    char data[81];       /* Characters from the document sent from the client */
}dataRecv;

struct response {
    /* Response from the server */
    int ACK;            /* ACK to return for the data packet (0 or 1) */
}ack;

int SimulateLoss(float packLoss){
    float testVal = (rand()%101)/100; //Generate a random number from 0-100, and divide by 100 to get a float from 0-1
    if(testVal < packLoss){
        return 0; //The packet is lost
    }else{
        return 1; //The packet is recieved successfully.
    }
}

int SimulateACKLoss(float ackLoss){
    float testVal = (rand()%101)/100; //Generate a random number from 0-100, and divide by 100 to get a float from 0-1
    if(testVal < ackLoss){
        return 0; //The ACK is lost
    }else{
        return 1; //The ACK is sent successfully
    }
}

int main(int argc, char** argv) {
    srand(time(0));
    
    float packLoss;
    float ackLoss;
    
    if(argc != 3){
        printf("Usage is:\nudpserver <Packet Loss Rate> <ACK Loss Rate>\n");
        exit(1);
    }else{
        packLoss = atof(*(argv+1));
        ackLoss = atof(*(argv+2));
    }
    
    int sock_server;  /* Socket on which server listens to clients */
    
    struct sockaddr_in server_addr;  /* Internet address structure that
    stores server address */
    unsigned short server_port;  /* Port number used by server (local port) */
    
    struct sockaddr_in client_addr;  /* Internet address structure that
    stores client address */
    unsigned int client_addr_len;  /* Length of client address structure */
    
    char sentence[STRING_SIZE];  /* receive message */
    char modifiedSentence[STRING_SIZE]; /* send message */
    unsigned int msg_len;  /* length of message */
    int bytes_sent, bytes_recd; /* number of bytes sent or received */
    unsigned int i;  /* temporary loop variable */
    
    /* open a socket */
    
    if ((sock_server = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        perror("Server: can't open datagram socket\n");
        exit(1);
    }
    
    /* initialize server address information */
    
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl (INADDR_ANY);  /* This allows choice of
    any host interface, if more than one
    are present */
    server_port = SERV_UDP_PORT; /* Server will listen on this port */
    server_addr.sin_port = htons(server_port);
    
    /* bind the socket to the local server port */
    
    if (bind(sock_server, (struct sockaddr *) &server_addr,
        sizeof (server_addr)) < 0) {
        perror("Server: can't bind to local address\n");
    close(sock_server);
    exit(1);
        }
        
        /* wait for incoming messages in an indefinite loop */
        
        printf("Waiting for incoming messages on port %hu\n\n", 
               server_port);
        
        client_addr_len = sizeof (client_addr);
        
        while(1) {
            
            bytes_recd = recvfrom(sock_server, &dataRecv, STRING_SIZE, 0,
                                  (struct sockaddr *) &client_addr, &client_addr_len);
            printf("Received Sentence is: %s\n     with length %d\n\n",
                   sentence, bytes_recd);
            
            /* prepare the message to send */
            
            msg_len = bytes_recd;
            for (i=0; i<msg_len; i++)
                modifiedSentence[i] = toupper (sentence[i]);
            
            /* send message */
            
            bytes_sent = sendto(sock_server, modifiedSentence, msg_len, 0,
                                (struct sockaddr*) &client_addr, client_addr_len);
        }
}
