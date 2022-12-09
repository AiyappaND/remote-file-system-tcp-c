#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include<pthread.h>
#include <time.h>

char * get_request = "GET";
char * inf_request = "INF";
char * mkd_request = "MKD";
char * mkf_request = "MKF";
char * del_request = "DEL";
char root_volume1[] = "/Volumes/FS1/";
char root_volume2[] = "/Volumes/FS2/";
char client_message[4098];
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
int keepRunning = 1;

int check_file_exists(char * filepath) {
    return (access(filepath, F_OK));
}

void duplicate_file(char * source, char * destination) {
    FILE *fp = NULL;
    char command[150];
    sprintf(command, "cp %s %s", source, destination);
    if ((fp=popen(command,"r")) != NULL) {
    pclose(fp);
    }
}

void mirror_existing_data(char * filepath1, char * filepath2) {
    if (check_file_exists(filepath1) == 0) {
        if (check_file_exists(filepath2) != 0) {
                duplicate_file(filepath1, filepath2);
        }
    }
    else {
        if (check_file_exists(filepath2) == 0) {
           duplicate_file(filepath2, filepath1);
        }
    }
}

int send_response(int client_sock, char* server_message) {
    int send_result = send(client_sock, server_message, strlen(server_message), 0);
    printf("Send result %d\n", send_result);
    if (send_result < 0){
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
    char *size = malloc(sizeof(char) * 100);
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

char * create_directory(char * dirname) {
    char * full_path1 = malloc(strlen(root_volume1) + strlen(dirname) + 1);
    char * full_path2 = malloc(strlen(root_volume2) + strlen(dirname) + 1);
    strcpy(full_path1, root_volume1);
    strcat(full_path1, dirname);

    strcpy(full_path2, root_volume2);
    strcat(full_path2, dirname);

    int result1 = mkdir(full_path1, 0777);
    int result2 = mkdir(full_path2, 0777);
    char * resultstr = malloc(sizeof(char) * 5);

    if (result1 == 0 || result2 == 0) {
        resultstr = "PASS";
    }
    else {
        resultstr = "FAIL";
    }
    return resultstr;
}

char * create_file(char * filename, char * filedata) {
    FILE *fp1;
    FILE * fp2;
    char * full_path1 = malloc(strlen(root_volume1) + strlen(filename) + 1);
    strcpy(full_path1, root_volume1);
    strcat(full_path1, filename);

    char * full_path2 = malloc(strlen(root_volume2) + strlen(filename) + 1);
    strcpy(full_path2, root_volume2);
    strcat(full_path2, filename);

    int result = 0;
    fp1 = fopen(full_path1,"w");
    fp2 = fopen(full_path2,"w");

    if(fp1 == NULL && fp2 == NULL) {
          result = -1;
    }
   if (result != -1) {
       fprintf(fp1,"%s",filedata);
       fprintf(fp2,"%s",filedata);
       if(fclose(fp1) != 0 && fclose(fp2) != 0) {
          result = -1;
    }
   }

    char * resultstr = malloc(sizeof(char) * 5);
    if (result == 0) {
        resultstr = "PASS";
    }
    else {
        resultstr = "FAIL";
    }
    return resultstr;
}

char * delete_file(char * filename) {
    char * full_path1 = malloc(strlen(root_volume1) + strlen(filename) + 1);
    strcpy(full_path1, root_volume1);
    strcat(full_path1, filename);

    char * full_path2 = malloc(strlen(root_volume2) + strlen(filename) + 1);
    strcpy(full_path2, root_volume2);
    strcat(full_path2, filename);

    int result1 = remove(full_path1);
    int result2 = remove(full_path2);

    char * resultstr = malloc(sizeof(char) * 5);

    if (result1 == 0 || result2 == 0) {
        resultstr = "PASS";
    }
    else {
        resultstr = "FAIL";
    }
    return resultstr;
}

char * get_file_info(char * filename) {
    struct stat stats;
    char * full_path1 = malloc(strlen(root_volume1) + strlen(filename) + 1);
    strcpy(full_path1, root_volume1);
    strcat(full_path1, filename);

    char * full_path2 = malloc(strlen(root_volume2) + strlen(filename) + 1);
    strcpy(full_path2, root_volume2);
    strcat(full_path2, filename);

    mirror_existing_data(full_path1, full_path2);
    char * filepath = malloc(sizeof(char) * 150);

    if (check_file_exists(full_path1)) {
        filepath = full_path1;
    }
    else {
        filepath = full_path2;
    }

    if (stat(filepath, &stats) == 0)
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

    char * filepath = malloc(sizeof(char) * 150);

    char * full_path2 = malloc(strlen(root_volume2) + strlen(filename) + 1);
    strcpy(full_path2, root_volume2);
    strcat(full_path2, filename);

    mirror_existing_data(full_path1, full_path2);

    if (check_file_exists(full_path1)) {
        filepath = full_path1;
    }
    else {
        filepath = full_path2;
    }

    ptr = fopen(filepath, "r");
    if (NULL == ptr) {
        printf("file can't be opened \n");
    }
    else {
        fgets(server_message, size, ptr);
    }
    fclose(ptr);
    return server_message;
}

void * socketThread(void *arg)
{
  int newSocket = *((int *)arg);
  // Receive client's message:
  if (recv(newSocket, client_message,
           sizeof(client_message), 0) < 0){
    printf("Couldn't receive\n");
    }

  pthread_mutex_lock(&lock);
  printf("Msg received from client\n");
  char * data = strdup(client_message);
  memset(client_message, '\0', sizeof(client_message));
  char * request_type = strtok(data, ",");
  if (strcmp(request_type, get_request) == 0) {
    char * filename = strtok(NULL, ",");
    int size = atoi(strtok(NULL, ","));
    printf("Attempting to read file data\n");
    printf("Filename: %s\n", filename);
    char * server_message = get_file_data(filename, size);
    send_response(newSocket, server_message);
  }
  else if (strcmp(request_type, inf_request) == 0) {
    printf("Attempting to read file info\n");
    char * filename = strtok(NULL, ",");
    printf("Filename: %s\n", filename);
    char * server_message = get_file_info(filename);
    send_response(newSocket, server_message);
  }
  else if (strcmp(request_type, mkd_request) == 0) {
    printf("Attempting to create directory\n");
    char * dirname = strtok(NULL, ",");
    printf("Directory name: %s\n", dirname);
    char * server_message = create_directory(dirname);
    send_response(newSocket, server_message);
  }
  else if (strcmp(request_type, mkf_request) == 0) {
    printf("Attempting to create file\n");
    char * filename = strtok(NULL, ",");
    char * filedata = strtok(NULL, ",");
    printf("File name: %s\n", filename);
    printf("File data: %s\n", filedata);
    char * server_message = create_file(filename, filedata);
    send_response(newSocket, server_message);
  }
  else if (strcmp(request_type, del_request) == 0) {
    printf("Attempting to delete file\n");
    char * filename = strtok(NULL, ",");
    printf("File name: %s\n", filename);
    char * server_message = delete_file(filename);
    send_response(newSocket, server_message);
  }
  else if (strcmp(request_type, "EXIT") == 0) {
    keepRunning = 0;
  }
  pthread_mutex_unlock(&lock);
  printf("Exit socketThread \n");
  pthread_exit(NULL);
}


int main(){
  int socket_desc, client_sock;
  socklen_t client_size;
  struct sockaddr_in server_addr, client_addr;

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
  server_addr.sin_port = htons(7799);
  server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

  // Bind to the set port and IP:
  if(bind(socket_desc, (struct sockaddr*)&server_addr, sizeof(server_addr))<0){
    printf("Couldn't bind to the port\n");
    return -1;
  }
  printf("Done with binding\n");

  // Listen for clients:
  if(listen(socket_desc, 20) < 0){
    printf("Error while listening\n");
    return -1;
  }
  printf("\nListening for incoming connections.....\n");
  pthread_t tid[30];
  int i = 0;
  while(keepRunning)
    {
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

        if( pthread_create(&tid[i++], NULL, socketThread, &client_sock) != 0 )
           printf("Failed to create thread\n");

        if( i >= 20)
        {
          i = 0;
          while(i < 20)
          {
            pthread_join(tid[i++],NULL);
          }
          i = 0;
        }
    }
  // Closing the socket:
  close(client_sock);
  close(socket_desc);
  return 0;
}