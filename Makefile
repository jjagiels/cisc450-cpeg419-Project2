CC= /usr/bin/gcc
all:	udpclient udpserver

udpclient: udpclient.c;
	${CC} -g udpclient.c -o udpclient -lm -std=gnu99

udpserver: udpserver.c;
	${CC} -g udpserver.c -o udpserver -std=gnu99

clean:
	rm udpclient udpserver output.txt
