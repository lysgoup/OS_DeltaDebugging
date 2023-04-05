#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include<string.h>

void Eliminate(char *str, char ch);

//main function recices 4 arguments.
int main(int argc, char *argv[]){
  char input[256] = "";
  char err[256] = "";
  char outputfile[256] = "";
  char program_n[256] = "";
  int param_opt;
  //recive arguments using getopt
  while(-1 !=( param_opt = getopt( argc, argv, "i:m:o:"))){
    switch(param_opt) {
			case 'i':
				printf("Option : i, Argument : %s\n",optarg);
        strcpy(input,optarg);
        //Eliminate(input," ");
				break;

			case 'm':
				printf("Option : m, Argument : %s\n",optarg);
        strcpy(err,optarg);
       // Eliminate(err," ");
				break;

			case 'o':
				printf("Option : o, Argument : %s\n",optarg);
        strcpy(outputfile,optarg);
        //Eliminate(outputfile," ");
				break;

			case '?':
				printf("Option Error!! No Option : %c\n",optopt);
				break;
		}
  }
  for  (int i  =  optind;  i  <  argc;  i++){
    strcpy(program_n,argv[i]);
  }
  printf("%s\n %s\n %s\n %s\n", input,err,outputfile,program_n);
}

/*
void Eliminate(char *str, char ch)
{
    unsigned len = strlen(str) + 1;
    for (; *str != '\0'; str++,len--)
    {
        if (*str == ch)
        {
            strcpy_s(str,len, str + 1);
            str--;            
        }
    }
}
*/