CC = gcc	

all : tcpechoserver_p

tcpechoserver_p : tcpechoserver_p.c
	$(CC) tcpechoserver_p.c `mysql_config --cflags --libs` -o tcpechoserver_p

clear :
	rm *.o


