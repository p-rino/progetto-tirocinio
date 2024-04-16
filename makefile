all: server_tirocinio client

server_tirocinio: server_tirocinio.c common.h
	gcc -o server_tirocinio server_tirocinio.c common.h

client: client.c common.h
	gcc -o client client.c common.h