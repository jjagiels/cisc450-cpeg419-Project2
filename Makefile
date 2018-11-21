CC= /usr/bin/gcc
all:	udpclient udpserver

udpclient: udpclient.c;
	${CC} -g -Wall udpclient.c -o udpclient -lm

udpserver: udpserver.c;
	${CC} -g -Wall udpserver.c -o udpserver

clean:
	rm udpclient udpserver
