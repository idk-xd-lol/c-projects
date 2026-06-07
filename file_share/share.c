#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <signal.h>

#define PORT        8080 //port
#define BUFFER_SIZE 4096

void handle_sigint(int sig)
{
    printf("\n Stopping...\n");
    exit(sig);
}

int main(int argc, char **argv)
{

  //read arguments
  if (argc != 2)
  {
    printf("USAGE: ./share <path-to-file>");
    exit(EXIT_FAILURE);
  }
  char *file = argv[1]; //set file to first argument

  //open file
  FILE *fp = fopen(file, "rb");
  if (!fp)
  {
    perror("No such file");
    exit(EXIT_FAILURE);
  }

  //get file length
  fseek(fp, 0, SEEK_END);
  long filesize = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  //varibles
  char* filename = strrchr(file, '/');       //filename from path
  filename = filename ? filename + 1 : file; //remove '/' or return filepath if there is no '/'
  int server_fd, client_fd;                  //sockets
  struct sockaddr_in address;                //address struct
  int addrlen = sizeof(address);             //get len of address struct
  char buffer[BUFFER_SIZE] = {0};

  //creating socket
  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    perror("Socket failed");
    fclose(fp);
    exit(EXIT_FAILURE);
  }

  //address struct fill
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(PORT);

  //binding socket
  int opt = 1;
  setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
  if(bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
  {
    perror("Bind failed");
    fclose(fp);
    close(server_fd);
    exit(EXIT_FAILURE);
  }

  //listen
  if (listen(server_fd, 10) < 0)
  {
    perror("Listen failed");
    fclose(fp);
    close(server_fd);
    exit(EXIT_FAILURE);
  }

  //get ip
  struct ifaddrs *ifaddr, *ifa;
  int family;
  char ip[INET6_ADDRSTRLEN];

  if (getifaddrs(&ifaddr) == -1)
  {
    perror("getifaddrs");
    fclose(fp);
    close(server_fd);
    exit(EXIT_FAILURE);
  }

  //print link
  for(ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
  {
    if (ifa->ifa_addr == NULL)
      continue;

    family = ifa->ifa_addr->sa_family;

    if(family == AF_INET)
    {
      inet_ntop(AF_INET, &((struct sockaddr_in *)ifa->ifa_addr)->sin_addr, ip, sizeof(ip));
      printf("link for %s: http://%s:%d\n",ifa->ifa_name, ip, PORT);
    }
  }
  printf("\n");

  signal(SIGINT, handle_sigint);

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
    "Content-Disposition: attachment; filename=\"%s\"\r\n"
    "Content-Length:%ld\r\n"
    "\r\n",
    filename, filesize);
    send(client_fd, http_response, strlen(http_response), 0);
    int bytes_read = 0;
    fseek(fp, 0, SEEK_SET);
    while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, fp)) > 0)
    {
      send(client_fd, buffer, bytes_read, 0);
    }

    close (client_fd);
  }

  //close all
  close(server_fd);
  fclose(fp);
  freeifaddrs(ifaddr);
  return 0;
}
