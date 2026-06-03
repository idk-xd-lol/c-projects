#include <stdio.h>

int main()
{
  unsigned char buffer[256];
  char code[256];
  printf("Write word: ");
  if (scanf("%255s", buffer) != 1) return 1;
  printf("Write code: ");
  if (scanf("%255s", code) != 1) return 1;
  int word_len = sizeof(buffer);
  int code_len = sizeof(code);
  
  unsigned long long w = 0;
  unsigned long long c = 0;
  for (int i = 0; i < word_len && buffer[i] != '\0'; i++)
  {
    w = (w<<8)|buffer[i];
  }
  for (int i = 0; i < code_len && code[i] != '\0'; i++)
  {
    c = (c<<8)|code[i];
  }
  unsigned long long encrypted = w ^ c;
  printf("Word: %llu\n", w);

  printf("%llx\n", encrypted);
  
  return 0;
}
