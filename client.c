/*
 * client.c -- TCP Socket Client
 * 
 * adapted from: 
 *   https://www.educative.io/answers/how-to-implement-tcp-sockets-in-c
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>

int get_user_input_integer() {
    int result;
    char *line = NULL;
    size_t len = 0;
    while (1) {
       if (getline(&line, &len, stdin) == -1) {
           /* eof or error, do whatever is sensible in your case */
           exit(EXIT_FAILURE);
       }
       if (sscanf(line, "%d", &result) != 1) {
           printf("Invalid choice, please try again: ");
           continue;
       }
       break;
    }
    return result;
}

char * build_file_read_message(int size) {
    char path[50];
    printf("Enter full path\n");
    scanf("%49[^\n]%*c", path);
    char* message_to_server;
    char* m_size = malloc(sizeof(char)*(int)log10(size));
    sprintf(m_size, "%d", size);
    message_to_server = malloc(3+1+strlen(path)+1+strlen(m_size)+1);
    strcpy(message_to_server, "GET,");
    strcat(message_to_server, path);
    strcat(message_to_server, ",");
    strcat(message_to_server, m_size);
    return message_to_server;
}

char * build_file_info_message() {
    char path[50];
    printf("Enter full path\n");
    scanf("%49[^\n]%*c", path);
    char* message_to_server;
    message_to_server = malloc(3+1+strlen(path)+1);
    strcpy(message_to_server, "INF,");
    strcat(message_to_server, path);
    return message_to_server;
}

char * build_create_folder_message() {
    char path[50];
    printf("Enter folder name\n");
    scanf("%49[^\n]%*c", path);
    char* message_to_server;
    message_to_server = malloc(3+1+strlen(path)+1);
    strcpy(message_to_server, "MKD,");
    strcat(message_to_server, path);
    return message_to_server;
}

char * build_create_file_message() {
    char path[50];
    printf("Enter file path\n");
    scanf("%49[^\n]%*c", path);
    printf("Enter file size\n");
    int size = get_user_input_integer();
    char data[size];
    printf("Enter data\n");
    scanf("%s", data);
    char* message_to_server;
    message_to_server = malloc(3+1+strlen(path)+1+strlen(data)+1);
    strcpy(message_to_server, "MKF,");
    strcat(message_to_server, path);
    strcat(message_to_server, ",");
    strcat(message_to_server, data);
    return message_to_server;
}

char * build_delete_file_message() {
    char path[50];
    printf("Enter file path\n");
    scanf("%49[^\n]%*c", path);
    char* message_to_server;
    message_to_server = malloc(3+1+strlen(path)+1);
    strcpy(message_to_server, "DEL,");
    strcat(message_to_server, path);
    return message_to_server;
}

int send_message_to_server(char * message, int socket_desc) {
    // Send the message to server:
    if(send(socket_desc, message, strlen(message), 0) < 0){
        printf("Unable to send message\n");
        return -1;
    }
    return 0;
}

int receive_response(int socket_desc, char * response, int size) {
    if(recv(socket_desc, response, size, 0) < 0){
        printf("Error while receiving server's msg\n");
        return -1;
    }
    else {
        printf("Received data: %s\n", response);
        return 0;
    }
}

int main(void)
{
  int socket_desc;
  struct sockaddr_in server_addr;
  // Create socket:
  socket_desc = socket(AF_INET, SOCK_STREAM, 0);
  
  if(socket_desc < 0){
    printf("Unable to create socket\n");
    return -1;
  }
  
  printf("Socket created successfully\n");
  
  // Set port and IP the same as server-side:
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(7799);
  server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
  
  // Send connection request to server:
  if(connect(socket_desc, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
    printf("Unable to connect\n");
    return -1;
  }
  printf("Connected with server successfully\n");
  
  int keepRunning = 1;
    char dataScanned;
    while (keepRunning == 1) {
        printf("Enter choice\n 1:Read file \n 2:Get file info\n 3:Create folder\n 4:Create file\n 5:Delete file\n");
        int choice = get_user_input_integer();
        if (choice == 1) {
            printf("Enter size of file to read\n");
            int size = get_user_input_integer();
            char * message = build_file_read_message(size);
            send_message_to_server(message, socket_desc);
            char response1[size];
            memset(response1, '\0', sizeof(response1));
            receive_response(socket_desc, response1, size);
        }
        else if (choice == 2) {
            char * message = build_file_info_message();
            send_message_to_server(message, socket_desc);
            char response2[500];
            memset(response2, '\0', sizeof(response2));
            receive_response(socket_desc, response2, 500);
        }
        else if (choice == 3) {
            char * message = build_create_folder_message();
            send_message_to_server(message, socket_desc);
            char response3[5];
            memset(response3, '\0', sizeof(response3));
            receive_response(socket_desc, response3, 5);
        }
        else if (choice == 4) {
            char * message = build_create_file_message();
            send_message_to_server(message, socket_desc);
            char response4[5];
            memset(response4, '\0', sizeof(response4));
            receive_response(socket_desc, response4, 5);
        }
        else if (choice == 5) {
            char * message = build_delete_file_message();
            send_message_to_server(message, socket_desc);
            char response5[5];
            memset(response5, '\0', sizeof(response5));
            receive_response(socket_desc, response5, 5);
        }
        else {
            printf("Invalid choice\n");
        }
        keepRunning = 0;
    }
    // Close the socket:
    close(socket_desc);
    return 0;
}
