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

int sock_client;  /* Socket used by client */ 

struct sockaddr_in client_addr;  /* Internet address structure that
stores client address */
unsigned short client_port;  /* Port number used by client (local port) */

struct sockaddr_in server_addr;  /* Internet address structure that stores server address */
struct hostent * server_hp;      /* Structure to store server's IP address */
char server_hostname[STRING_SIZE]; /* Server's hostname */
unsigned short server_port;  /* Port number used by server (remote port) */

char sentence[STRING_SIZE];  /* send message */
char modifiedSentence[STRING_SIZE]; /* receive message */
unsigned int msg_len;  /* length of message */
int bytes_sent, bytes_recd; /* number of bytes sent or received */
int ack_len; /* length of each ACK */

struct timeval tv; /* Struct used to set the timeout value */
int expectedACK = 0; /* Counter used to keep track of the ACK expected from the server */
int i; /* Temporary Loop Variable */

/* Variables to track telemetry */
int initialPackets = 0; //Number of data packets transmitted (initial transmission only)
int totalBytesTransmitted = 0; //Total number of data bytes transmitted (sum of the count fields of all transmitted packets when transmitted for the first time)
int retransmittedPackets = 0; //Total number of retransmissions
int totalPackets = 0; //Total number of data packets transmitted (initial transmissions plus retransmissions)
int ACKsReceived = 0; //Number of ACKs received
int numTimeouts = 0; //How many times the timeout expired

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
    memset(data, ' ', 80*sizeof(char)); //Clear the data field to ensure that no data is copied accidentally
}

int SendAndReceive(){ //Send data to the server, wait for an ACK (until timeout reached), and handle any retransmissions. This function will only return once a suitable ACK has been returned
    initialPackets++; //Increment the original transmission counter
    totalBytesTransmitted+=dataSend.count; //Add the count of bytes in this packet to the running total
    msg_len = sizeof(dataSend);
    dataSend.seqNum = expectedACK; //Set the sequence number of the data packet to be the same value as the ACK expected from the server
    printf("Packet %i transmitted with %i data bytes\n", dataSend.seqNum, dataSend.count);
    dataSend.count = htons(dataSend.count); //Convert the count variable to newtwork form
    dataSend.seqNum = htons(dataSend.seqNum); //Convert the sequence number to network form
    int bytes_sent_temp; //Initialize a temp value for bytes_sent
    int bytes_recd_temp; //Initialize a temp value for bytes_recd

    do{ //Loop The transmission and ACK waiting until bytes_recd is greater than 0 (Meaning that a valid ACK was received) and the received ACK is the expected ACK (Meaning that the server received an in-order packet)
        bytes_sent_temp = sendto(sock_client, &dataSend, msg_len, 0, (struct sockaddr *) &server_addr, sizeof (server_addr)); //Send the data packet
        bytes_recd_temp = recvfrom(sock_client, &ack, ack_len, 0, (struct sockaddr *) 0, (int *) 0); //Wait for an ACK
        if(bytes_recd == -1){ //The recvfrom function returned an error (most likely from a timeout)
            //Increment the timeout counter and the retransmit counter
	    numTimeouts++;
	    retransmittedPackets++;
	    printf("Packet %i retransmitted with %i data bytes\n",ntohs(dataSend.seqNum),ntohs(dataSend.count)); //The seqNum and count fields have been converted to host form, so temporarilly convert back for printing purposes
            
        }
        else{ //The recvfrom function did not return an error (received an ACK)
            ack.ACK = ntohs(ack.ACK); //Convert the received ACK from network to host form
            if(ack.ACK != expectedACK){ //If the ACK is not the expected ACK...
                //Increment the ACK counter and the retransmit counter
		ACKsReceived++;
		retransmittedPackets++;
		printf("ACK %i received\n",ack.ACK);
		printf("This is a duplicate ACK, so packet %i retransmitted with %i data bytes\n",ntohs(dataSend.seqNum),ntohs(dataSend.count)); //The seqNum and count fields have been converted to network form, so temporarilly convert back for printing purposes
            }else{
                //Increment the successful ACKs received counter
		ACKsReceived++;
		printf("ACK %i received\n",ack.ACK);
                return bytes_recd_temp;
            }
        }
    }while((bytes_recd_temp < 0) || (ack.ACK != expectedACK)); //if bytes_recd is -1 or the ACK is not the expected ACK, loop again (retransmit)
    return -1; //If this is returned, something went terribly wrong
}


int main(int argc, char* argv[]) {
    int timeoutExp;
    ack_len = sizeof(ack); //set the ack length to the size of the ack struct
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
    
    int temp_usec = pow(10.0,timeoutExp); //This is the timeout value in microseconds only
    tv.tv_sec = temp_usec * 1/(double)pow(10.0,6.0); //Set the timeout value in seconds
    tv.tv_usec = temp_usec%1000000; //Sets the remainder of microseconds to the microsecond field.
    
    
    /* open a socket */
    
    if ((sock_client = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        perror("Client: can't open datagram socket\n");
        exit(1);
    }
    
    setsockopt(sock_client, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv)); //Sets the timeout value for the socket defined
    
    /* Note: there is no need to initialize local client address information
     *            unless you want to specify a specific local port.
     *            The local address initialization and binding is done automatically
     *            when the sendto function is called later, if the socket has not
     *            already been bound. 
     *            The code below illustrates how to initialize and bind to a
     *            specific local port, if that is desired. */
    
    /* initialize client address information */
    
    client_port = 0;   /* This allows choice of any available local port */
    
    /* Uncomment the lines below if you want to specify a particular 
     *            local port: */
    /*
     * printf("Enter port number for client: ");
     * scanf("%hu", &client_port);
     */
    
    /* clear client address structure and initialize with client address */
    memset(&client_addr, 0, sizeof(client_addr));
    client_addr.sin_family = AF_INET;
    client_addr.sin_addr.s_addr = htonl(INADDR_ANY); /* This allows choice of any host interface, if more than one are present */
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
        memcpy((char *)&server_addr.sin_addr, server_hp->h_addr, server_hp->h_length);
        server_addr.sin_port = htons(server_port);
        
        /* user interface */
        
        
        FILE *fp;
        char str[81]; //Buffer is size 81, since fgets appends a null terminator after a newline
        
        /* opening file for reading */
        fp = fopen("test1.txt", "r");
        if (fp == NULL) {
            perror("Error opening file");
            return(-1);
        }
        
        while (fgets(str, 80, fp) != NULL) {
            ClearData(dataSend.data); //Clear the data field of the struct
            dataSend.count = 0; //Set the count to 0

                /* removing null character */
		i = 0;
		while(str[i] != '\0'){
                    dataSend.data[i] = str[i];
                    dataSend.count++;
		    i++;
                }
          
            bytes_recd = SendAndReceive(); //This function always waits for a valid ACK
            expectedACK = 1 - expectedACK; //Once SendAndReceive has returned, toggle the expected ACK
        } //Once this while loop has finished, the EOF has been reached, and the client should transmit the EOF packet (count = 0)
       
	dataSend.count = 0;
	ClearData(dataSend.data);
	dataSend.seqNum = expectedACK;
	dataSend.count = htons(dataSend.count);
	dataSend.seqNum = htons(dataSend.seqNum);
	sendto(sock_client, &dataSend, msg_len, 0, (struct sockaddr *) &server_addr, sizeof (server_addr)); //Send the EOF data packet

	printf("End of File Reached, EOF Packet sent with Sequence number %i, printing statistics:\n\nNumber of data packets transmitted(initial transmission only): %i\nTotal number of data bytes transmitted: %i\nTotal number of retransmissions: %i\nTotal number of data packets transmitted: %i\nNumber of ACKs received: %i\nHow many times the timeout expired: %i\n", ntohs(dataSend.seqNum), initialPackets, totalBytesTransmitted, retransmittedPackets, totalPackets, ACKsReceived, numTimeouts);

        fclose(fp); //close the output file       
        
        /* close the socket */
        
        close (sock_client);
}
