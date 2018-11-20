CC= /usr/bin/gcc
all:	udpclient udpserver

udpclient: udpclient.c;
	${CC} -g udpclient.c -o udpclient -lm

udpserver: udpserver.c;
	${CC} -g udpserver.c -o udpserver

clean:
	rm udpclient udpserver
