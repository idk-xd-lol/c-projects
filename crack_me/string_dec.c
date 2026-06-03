#include <stdio.h>

int main()
{
  unsigned long long enc;
  unsigned long long w = 0;
  unsigned long long c = 0; 
  char code[256];
  char decoded[256];

  int byte = 0;
  int byte_len = 0;

  
  printf("Write encoded hex:");
  if(scanf("%llx", &enc) != 1) return 1;
  printf("Write code:");
  if(scanf("%255s", code) != 1) return 1;

  int code_len = sizeof(code);
  for (int i = 0; i < code_len && code[i] != '\0'; i++)
  {
    c = (c<<8)|code[i];
  }
  w = enc ^ c;

  unsigned long long temp = w;
  while (temp > 0)
  {
    ++byte_len;
    temp >>= 8;
  }
  
  int k = 0;
  for (int i = byte_len-1; i >= 0; i--)
  {
    byte = (w>>(8*i)) & 0xFF;
    decoded[k++] = (char)byte;
  }
  decoded[k+1] = '\0';

  printf("%s\n", decoded);

}

