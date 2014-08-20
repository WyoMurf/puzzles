#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

int unique_letters(char *word)
{
   int Alph[26] = {0};  /* sets all elements to 0 */
   int i;
   int ret = 0;
   while (word && *word)
   {
      if (isalpha(*word))
      {
         if(islower(*word))
            *word = toupper(*word);

          Alph[*word - 'A']++;
      }
      word++;
   }
   for (i=0; i<26; i++)
       if(Alph[i] > 0)
           ret++;
   return ret;
}

int main(int argc, char **argv)
{
     char *word ;
    
     if (argc > 1)
     {
          word = argv[1];
          printf("'%s' has %d unique letters\n", word, unique_letters(word));
          exit(0);
     }
     else
        printf("Hey, supply a state name as an argument!\n");
     exit(10);
}
