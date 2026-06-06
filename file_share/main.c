#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT        8080
#define BUFFER_SIZE 4096

int main(int argc, char **argv)
{
  if (argc != 2)
  {
    printf("USAGE: ./share <path-to-file>");
    return 1;
  }
  char* file = argv[1];
  FILE *fp = fopen(file, "rb");
  if (!fp)
  {
    perror("No such file");
    return 1;
  }
  char* filename = strrchr(file, '/');
  filename = filename ? filename + 1 : file;

  int server_fd, client_fd;
  struct sockaddr_in address;
  int addrlen = sizeof(address);
  char buffer[BUFFER_SIZE] = {0};
  fseek(fp, 0, SEEK_END);
  int filesize = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    perror("Socket failed");
    exit(EXIT_FAILURE);
  }

  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(PORT);

  if(bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
  {
    perror("Bind failed");
    close(server_fd);
    exit(EXIT_FAILURE);
  }

  if (listen(server_fd, 10) < 0)
  {
    perror("Listen failed");
    close(server_fd);
    exit(EXIT_FAILURE);
  }

  printf("Sharing:%s\nlink: http://10.218.112.89:%d\n",filename, PORT);

  while(1) {
    client_fd = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
    if (client_fd < 0)
    {
      perror("Accept failed");
      continue;
    }
    memset(buffer, 0, BUFFER_SIZE);
    read(client_fd, buffer, BUFFER_SIZE);
    printf("Data from user:\n%s\n", buffer);

    char http_response[4096];
    snprintf(http_response, sizeof(http_response),
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: application/octet-stream\r\n"
    "Content-Disposition: attachment; filename=%s\r\n"
    "Content-Length:%d\r\n"
    "\r\n",
    filename, filesize); 
    printf("%s\n", http_response);
    fseek(fp, 0, SEEK_SET);
    send(client_fd, http_response, strlen(http_response), 0);
    int bytes_read = 0;
    while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, fp)) > 0)
    {
      send(client_fd, buffer, bytes_read, 0);
    }

    close (client_fd);
  }
  close(server_fd);
  fclose(fp);
  return 0;
}
