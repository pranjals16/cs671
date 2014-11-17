#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv)
{
  FILE *file;
  file = fopen("result.txt","w");
  char tmp[256]={0x0};
  char *typeline;
  char *scoreline;
  int numline = 1;
  char *s1="<type>";
  char *s2="<score>";
  char *st;
  printf("Enter the text\n");
  gets(st);
  fputs(sentiment(st),file);
  fclose(file);
  file=fopen("results.txt","r");
  while(file != NULL && fgets(tmp, sizeof(tmp),file) !=NULL)
  {
    typeline = strstr(tmp,s1);
    scoreline=strstr(tmp,s2);
    if(typeline) {
      printf("%s", typeline);
    }
    if(scoreline){
      printf("%s", scoreline);
    }             
    numline++;
  }
  if (file != NULL) fclose(file);

  return 0;
}
