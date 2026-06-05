#include <stdio.h>
#include <inttypes.h>
#include <stdint.h>

int main()
{
  unsigned char buffer[256]; //word to be encrypted
  unsigned char code[256]; //encryption code
  uint64_t w = 0;
  uint64_t c = 0;
  
  printf("Write word: ");
  if (scanf("%255s", buffer) != 1) return 1;
  printf("Write code: ");
  if (scanf("%255s", code) != 1) return 1;

  int word_len = sizeof(buffer);
  int code_len = sizeof(code);
  
  //make uint64 from word
  for (int i = 0; i < word_len && buffer[i] != '\0'; i++)
    w = (w<<8)|buffer[i];
  
  //make uint64 from code
  for (int i = 0; i < code_len && code[i] != '\0'; i++)
    c = (c<<8)|code[i];

  uint64_t encrypted = w ^ c; 

  printf("%" PRIx64 "\n", encrypted);
  return 0;
}
