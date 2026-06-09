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

struct mime_type {
    char *ext;
    char *mime;
};


int main(int argc, char **argv)
{
  //read arguments
  if (argc < 2 && argc > 256)
  {
    printf("USAGE: ./share <path-to-file> ...");
    exit(EXIT_FAILURE);
  }
  char **files = argv+1;

  int i = 0;

  //open file
  FILE *fp = fopen(files[i], "rb");
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
  char filenames[sizeof(files)][256];

  for (int k = 0; k < sizeof(files); k++)
  {
    strncpy(filenames[k], strrchr(files[k], '/'), 255);     //filename from path
    filenames[k] ? strncpy(filenames[k], filenames[k] +1, 255) : strncpy(filenames[k], files[k], 255);
  }
  int server_fd, client_fd;                  //sockets
  struct sockaddr_in address;                //address struct
  int addrlen = sizeof(address);             //get len of address struct
  char client_buffer[BUFFER_SIZE] = {0};

  char exts[sizeof(files)][8];
  char mimes[sizeof(files)][64];
  char cont_disp[256];
  snprintf(cont_disp, 256, "Content-Disposition: attachment; filename=\"%s\"\r\n", filenames[i]);

  //mime types 
  static struct mime_type mime_types[] = {
    {".html", "text/html"},
    {".txt",  "text/plain"},
    {".png",  "image/png"},
    {".jpg",  "image/jpeg"},
    {".jpeg", "image/jpeg"},
    {".gif",  "image/gif"},
  };

  for (int e = 0; e < sizeof(files); e++)
  {
    for (size_t k = 0; k < sizeof(mime_types)/sizeof(mime_types[0]); k++)
    {
      if (strcmp(exts[e], mime_types[k].ext) == 0)
      {
          strncpy(mimes[k], mime_types[k].mime, 255);
          cont_disp[0] = '\0';
      }
    }
  }
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
  freeifaddrs(ifaddr);

  signal(SIGINT, handle_sigint);

  while(1) {

    client_fd = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
    if (client_fd < 0)
    {
      perror("Accept failed");
      continue;
    }
    memset(client_buffer, 0, BUFFER_SIZE);
    read(client_fd, client_buffer, BUFFER_SIZE);
    printf("Data from user:\n%s\n", client_buffer);

    char html[4096];
    snprintf(html, 4096, "<html><body><a href=\"/%s\">%s</a></body></html>\n", filenames[i], filenames[i]);
    char home_http[4096];
    snprintf(home_http, sizeof(home_http),
        "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n"
        "Content-Length:%ld\r\n\r\n", strlen(html));

    char file_http[4096];
    snprintf(file_http, 4096, 
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: %s\r\n"
        "%s"
        "Content-Length:%ld\r\n\r\n", mimes[i], cont_disp, filesize);
    char getfile[1024];
    snprintf(getfile, 1024, "GET /%s HTTP/1.1", filenames[i]);

    int bytes_read;
    char buffer[4096];

    if (strncmp(client_buffer, getfile, strlen(getfile)) != 0)
    {
      send(client_fd, home_http, strlen(home_http), 0);
      send(client_fd, html, strlen(html), 0);
    }
    else {
      fseek(fp, 0, SEEK_SET);
      send(client_fd, file_http, strlen(file_http), 0);
      while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, fp)) > 0)
      {
        send(client_fd, buffer, bytes_read, 0);
      }
    }
    close (client_fd);
  }

  //close all
  close(server_fd);
  fclose(fp);
  return 0;
}
