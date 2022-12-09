all: server client parallel_client

server: server.c
	gcc server.c -o server

client: client.c
	gcc client.c -o client

parallel_client: parallel_client.c
	gcc parallel_client.c -o parallel_client
