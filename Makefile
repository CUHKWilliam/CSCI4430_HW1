CC = gcc
LIB = 
all: server client
server: myftpserver.c
	${CC} -o myftpserver myftpserver.c myftp.c ${LIB} -pthread
client: myftpclient.c
	${CC} -o myftpclient myftpclient.c myftp.c ${LIB} -pthread