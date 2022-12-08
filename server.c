/*
 * server.c -- TCP Socket Server
 * 
 * adapted from: 
 *   https://www.educative.io/answers/how-to-implement-tcp-sockets-in-c
 */

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>

char * get_request = "GET";
char * inf_request = "INF";
char root_volume1[] = "/Volumes/FS1/";

int send_response(int client_sock, char* server_message) {
    if (send(client_sock, server_message, strlen(server_message), 0) < 0){
    printf("Can't send\n");
    return -1;
  }
  return 0;
}

char * build_file_info_msg(struct stat stats, char* filedata) {
    strcpy(filedata, "File permissions: ");
    if (stats.st_mode & R_OK)
        strcat(filedata,"r ");
    if (stats.st_mode & W_OK)
        strcat(filedata,"w ");
    if (stats.st_mode & X_OK)
        strcat(filedata,"x ");
    char *size;
    sprintf(size, "File size: %lld ", stats.st_size);
    strcat(filedata, size);

    struct tm dt;
    dt = *(gmtime(&stats.st_ctime));
    char * created_date = malloc(sizeof(char) * 50);
    sprintf(created_date, "Created on: %d-%d-%d %d:%d:%d ", dt.tm_mday, dt.tm_mon, dt.tm_year + 1900,
                                              dt.tm_hour, dt.tm_min, dt.tm_sec);
    strcat(filedata, created_date);

    dt = *(gmtime(&stats.st_mtime));
    char *modified_date = malloc(sizeof(char) * 50);
    sprintf(modified_date, "Modified on: %d-%d-%d %d:%d:%d ", dt.tm_mday, dt.tm_mon, dt.tm_year + 1900,
                                              dt.tm_hour, dt.tm_min, dt.tm_sec);
    strcat(filedata, modified_date);
    return filedata;

}

char * get_file_info(char * filename) {
    struct stat stats;
    char * full_path1 = malloc(strlen(root_volume1) + strlen(filename) + 1);
    strcpy(full_path1, root_volume1);
    strcat(full_path1, filename);
    if (stat(full_path1, &stats) == 0)
    {
        char data[500];
        char * response = build_file_info_msg(stats, data);
        printf("File info retrieved: %s\n", response);
        return response;
    }
    else
    {
        printf("Unable to get file properties.\n");
        return NULL;
    }
}

char * get_file_data(char * filename, int size) {
    FILE* ptr;
    char * server_message = malloc(sizeof(char) * size);
    char * full_path1 = malloc(strlen(root_volume1) + strlen(filename) + 1);
    strcpy(full_path1, root_volume1);
    strcat(full_path1, filename);
    ptr = fopen(full_path1, "r");
    if (NULL == ptr) {
        printf("file can't be opened \n");
    }
    else {
        fgets(server_message, size, ptr);
    }
    fclose(ptr);
    return server_message;
}

int main(void)
{
  int socket_desc, client_sock;
  socklen_t client_size;
  struct sockaddr_in server_addr, client_addr;
  char client_message[4098];

  memset(client_message, '\0', sizeof(client_message));
  
  // Create socket:
  socket_desc = socket(AF_INET, SOCK_STREAM, 0);
  
  if(socket_desc < 0){
    printf("Error while creating socket\n");
    return -1;
  }
  printf("Socket created successfully\n");
  
  // Set port and IP:
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(2000);
  server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
  
  // Bind to the set port and IP:
  if(bind(socket_desc, (struct sockaddr*)&server_addr, sizeof(server_addr))<0){
    printf("Couldn't bind to the port\n");
    return -1;
  }
  printf("Done with binding\n");
  
  // Listen for clients:
  if(listen(socket_desc, 1) < 0){
    printf("Error while listening\n");
    return -1;
  }
  printf("\nListening for incoming connections.....\n");
  
  // Accept an incoming connection:
  client_size = sizeof(client_addr);
  client_sock = accept(socket_desc, (struct sockaddr*)&client_addr, &client_size);
  
  if (client_sock < 0){
    printf("Can't accept\n");
    return -1;
  }
  printf("Client connected at IP: %s and port: %i\n", 
         inet_ntoa(client_addr.sin_addr), 
         ntohs(client_addr.sin_port));
  
  // Receive client's message:
  if (recv(client_sock, client_message, 
           sizeof(client_message), 0) < 0){
    printf("Couldn't receive\n");
    return -1;
  }
  printf("Msg received from client\n");
  char * data = strdup(client_message);
  char * request_type = strtok(data, ",");
  if (strcmp(request_type, get_request) == 0) {
    char * filename = strtok(NULL, ",");
    int size = atoi(strtok(NULL, ",")) + 1;
    printf("Attempting to read file data\n");
    printf("Filename: %s\n", filename);
    char * server_message = get_file_data(filename, size);
    send_response(client_sock, server_message);
  }
  else if (strcmp(request_type, inf_request) == 0) {
    printf("Attempting to read file info\n");
    char * filename = strtok(NULL, ",");
    printf("Filename: %s\n", filename);
    char * server_message = get_file_info(filename);
    send_response(client_sock, server_message);
  }

  // Closing the socket:
  close(client_sock);
  close(socket_desc);

  return 0;
}
