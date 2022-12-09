#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <fcntl.h> // for open
#include <unistd.h> // for close
#include<pthread.h>

void * client_parallel(void *arg)
{
  int socket_desc;
  struct sockaddr_in server_addr;
  // Create socket:
  socket_desc = socket(AF_INET, SOCK_STREAM, 0);

  if(socket_desc < 0){
    printf("Unable to create socket\n");
    pthread_exit(NULL);
  }

  printf("Socket created successfully\n");

  // Set port and IP the same as server-side:
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(7799);
  server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

  // Send connection request to server:
  if(connect(socket_desc, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
    printf("Unable to connect\n");
    pthread_exit(NULL);
  }
  printf("Connected with server successfully\n");
    char * message = malloc(sizeof(char)*25);;
    char random_letter = 'A' + (random() % 26);
    sprintf(message, "MKF,sample%c.txt,%s", random_letter, "abcd");
    printf("Sending %s\n", message);
   if(send(socket_desc , message , 25 , 0) < 0)
    {
            printf("Send failed\n");
    }
    char response[10];
    memset(response, '\0', sizeof(response));

    //Read the message from the server into the buffer
    if(recv(socket_desc, response, 10, 0) < 0)
    {
       printf("Receive failed\n");
    }
    //Print the received message
    printf("Data received: %s\n",response);
    close(socket_desc);
    pthread_exit(NULL);
}

int main(){
  int i = 0;
  pthread_t tid[11];
  while(i< 10)
  {
    if( pthread_create(&tid[i], NULL, client_parallel, NULL) != 0 )
           printf("Failed to create thread\n");
    i++;
  }
  sleep(5);
  i = 0;
  while(i< 10)
  {
     pthread_join(tid[i++],NULL);
  }
  return 0;
}