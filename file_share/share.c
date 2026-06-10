#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#define PORT        8080 //port
#define BUFFER_SIZE 4096

struct mime_type {
    char *ext;
    char *mime;
};


int main(int argc, char **argv)
{
  //read arguments
  if (argc < 2 || argc > 256)
  {
    printf("USAGE: ./share <path-to-file> ...");
    exit(EXIT_FAILURE);
  }
  int server_fd, client_fd;                  //sockets
  struct sockaddr_in address;                //address struct
  int addrlen = sizeof(address);             //get len of address struct
  char client_buffer[BUFFER_SIZE] = {0};

  //creating socket
  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    perror("Socket failed");
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
    close(server_fd);
    exit(EXIT_FAILURE);
  }

  //listen
  if (listen(server_fd, 10) < 0)
  {
    perror("Listen failed");
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

  //mime types 
  static struct mime_type mime_types[] = {
    {".html", "text/html"},
    {".txt",  "text/plain"},
    {".png",  "image/png"},
    {".jpg",  "image/jpeg"},
    {".jpeg", "image/jpeg"},
    {".gif",  "image/gif"},
  };

  
  char **files = argv+1;
  int num_file = argc - 1;
  printf("%s %d\n", files[0], num_file);

  FILE **fp = malloc(sizeof(FILE *) * num_file);
  long filesizes[num_file];
  for(int k = 0; k < num_file; k++)
  {
    fp[k] = fopen(files[k], "rb");
    if(!fp[k])
    {
      perror("File doesn't exist");
      exit(EXIT_FAILURE);
    }
    fseek(fp[k], 0, SEEK_END);
    filesizes[k] = ftell(fp[k]);
    fseek(fp[k], 0, SEEK_SET);

  }

  char filenames[num_file][4098];
  for (int k = 0; k < num_file; k++)
  {
    char *name = strrchr(files[k], '/'); 
    if (name)
      strncpy(filenames[k], name + 1, 4096); 
    else 
      strncpy(filenames[k], files[k], 4096);
  }

  char *mimes[num_file];
  char cont_disp[num_file][256];
  
  for (int e = 0; e < num_file; e++)
  {
    mimes[e] = "application/octet-stream";

    snprintf(cont_disp[e], sizeof(cont_disp[e]),
             "Content-Disposition: attachment; filename=\"%s\"\r\n",
             filenames[e]);

    char *ext = strrchr(filenames[e], '.');

    if (!ext)
        continue;

    for (size_t k = 0; k < sizeof(mime_types)/sizeof(mime_types[0]); k++)
    {
        if (strcmp(ext, mime_types[k].ext) == 0)
        {
            mimes[e] = mime_types[k].mime;
            cont_disp[e][0] = '\0';
            break;
        }
    }
}

  size_t offset = 0;
  char html_files[2048];
  html_files[0] = '\0';
  for(int k = 0; k < num_file; k++)
  {
    int written = snprintf(
        html_files + offset,
        2048 - offset,
        "%s<a href=\"/%s\">%s</a>",
        (k > 0) ? "<br>" : "",  // Add newline before all but first
        filenames[k],
        filenames[k]
    );
    
    if (written < 0 || written >= (int)(sizeof(html_files) - offset)) {
        break;
    }
    offset += written;
  }

  char html[4096];
  snprintf(html, 4096, "<html><body>%s</body></html>\n", html_files);

  char file_https[num_file][4096];
  char getfiles[num_file][1024];
  for (int k = 0; k < num_file; k++)
  {
    snprintf(file_https[k], 4096, 
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: %s\r\n"
        "%s"
        "Content-Length:%ld\r\n\r\n", mimes[k], cont_disp[k], filesizes[k]);
    snprintf(getfiles[k], 1024, "GET /%s HTTP/1.1", filenames[k]);
  }

  char home_http[4096];
    snprintf(home_http, sizeof(home_http),
        "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n"
        "Content-Length:%ld\r\n\r\n", strlen(html));

  char *home_get = "GET / HTTP/1.1";
  char nf_error[1024];
  snprintf(nf_error, sizeof(nf_error), 
      "HTTP/1.1 404 Not Found\r\n"
      "Content-Length: 0\r\n\r\n");

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

    int sent = 0;
    int bytes_read;
    char buffer[4096];

    if (strncmp(client_buffer, home_get, strlen(home_get)) == 0)
    {
      send(client_fd, home_http, strlen(home_http), 0);
      send(client_fd, html, strlen(html), 0);
    }
    else {
      for (int k = 0; k < num_file; k++)
      {
        if(strncmp(client_buffer, getfiles[k], strlen(getfiles[k])) == 0)
        {
          fseek(fp[k], 0, SEEK_SET);
          printf("%s\n", file_https[k]);
          send(client_fd, file_https[k], strlen(file_https[k]), 0);
          while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, fp[k])) > 0)
          {
            send(client_fd, buffer, bytes_read, 0);
          }
          sent = 1;
        }
      }
      if(sent == 0)
          send(client_fd, nf_error, strlen(nf_error), 0);
    }
    close (client_fd);
  }

  //close all
  close(server_fd);
  for(int k = 0; k < num_file; k++)
  {
    fclose(fp[k]);
  }
  free(fp);
  return 0;
}
