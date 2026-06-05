#include <stdio.h>

#define CHAR_COLOR  "\033[32m"
#define UNPTR_COLOR "\033[33m"
#define NON_ASCII   "\033[31m"
#define RESET_COLOR "\033[0m"
int main(int argc, char **argv)
{
  if(argc != 2)
  {
    printf("USAGE: ./hex_dump <path_to_file>\n");
    return 1;
  }

  FILE *fp;
  fp = fopen(argv[1], "rb");
  int byte;
  char ascii_line[16];
  int endline = 0;
  int line = 0;
  
  if (!fp)
  {
    printf("File doesn't exist\n");
    return 1;
  }
  printf("%08d:",0); 
  
  while((byte = fgetc(fp)) != EOF)
  {
    if(endline == 16)
    {
      ascii_line[15] = '\0';
      endline = 0;
      line++;
      printf("\033[36m  %s\033[0m\n", ascii_line);
      printf("%08X:", (line * 16));

    }

    if (byte >= 32 && byte <= 127)
    {
      printf("%s %02x%s", CHAR_COLOR, byte, RESET_COLOR);
      ascii_line[endline] =  byte;
    }
    else if(byte < 32) 
    {
      printf("%s %02x%s", UNPTR_COLOR, byte, RESET_COLOR);
      ascii_line[endline] = '.';
    }
    else if(byte > 127) 
    {
      printf("%s %02x%s", NON_ASCII, byte, RESET_COLOR);
      ascii_line[endline] = '.';
    }

    if(endline <= 15)
    {
      ascii_line[endline+1] = '\0';
    }

    endline++;
  }

  while(endline < 16)
  {
    printf("   ");
    ascii_line[endline] = ' ';
    endline++;
  }

  printf("\033[36m  %s\033[0m\n", ascii_line);

  fclose(fp);
  return 0;
}


