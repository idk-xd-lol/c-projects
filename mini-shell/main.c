#include <stdio.h>
#include <string.h>

#include "functions.h"

int getcommand(char *buffer);

int main()
{
  char buffer[256];
  int len = sizeof(commands)/sizeof(commands[0]);
  int err = 1;
  while (1)
  {
    getcommand(buffer);
    for (int i = 0; i < len; i++)
    {
      if (!strcmp(buffer, commands[i]))
      {
        err = 0;
        break; 
      }
    }
    if(err == 1)
    {
      printf("Command doesn't exist\n");
      continue;
    }
    else
    {
      printf("Command exist\n");
    }

    err = 1;
  }
  return 0;
}

int getcommand(char buffer[])
{
  int c;
  int i = 0;
  while((c = getchar()) != EOF && c != '\n')
  {
    buffer[i] = c;
    i++;
  }
  buffer[i] = '\0';
  return 0;
}

