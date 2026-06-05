#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

int main()
{
  uint64_t enc;
  uint64_t w = 0;
  uint64_t c = 0; 

  char code[256];
  char decoded[256];
  
  printf("Write encoded hex: ");
  if(scanf("%" SCNx64, &enc) != 1) return 1;
  printf("Write code: ");
  if(scanf("%255s", code) != 1) return 1;

  size_t code_len = strlen(code);

  for (int i = 0; i < code_len && code[i] != '\0'; i++)
    c = (c<<8)|code[i];

  w = enc ^ c;
  uint64_t temp = w;
  int byte_len = 0;
  while (temp > 0)
  {
    ++byte_len;
    temp >>= 8;
  }
  
  int k = 0;
  for (int i = byte_len-1; i >= 0; i--)
  {
    int byte = (w>>(8*i)) & 0xFF;
    decoded[k++] = (char)byte;
  }
  decoded[k] = '\0';

  printf("%s\n", decoded);
  return 0;
}

