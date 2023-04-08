#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include<string.h>

int checkerror(char input[]);
void Reduce(char *str, char program_n[], char err[], char outputfile[]);
void handler(int sig);

char *reducestring = NULL;

//main function recives 4 arguments.
int main(int argc, char *argv[]){
  signal(SIGINT, handler);
  char inputfile[256] = "";
  char err[256] = "";
  char outputfile[256] = "";
  char program_n[256] = "";

  //recive arguments using getopt
  int param_opt;
  while(-1 !=( param_opt = getopt( argc, argv, "i:m:o:"))){
    switch(param_opt) {
			case 'i':
				printf("Option : i, Argument : %s\n",optarg);
        strcpy(inputfile,optarg);
				break;

			case 'm':
				printf("Option : m, Argument : %s\n",optarg);
        strcpy(err,optarg);
				break;

			case 'o':
				printf("Option : o, Argument : %s\n",optarg);
        strcpy(outputfile,optarg);
				break;

			case '?':
				printf("Option Error!! No Option : %c\n",optopt);
				break;
		}
  }
  for  (int i = optind;  i  <  argc;  i++){
    strcpy(program_n,argv[i]);
  }

  //checking options
  printf("%s\n%s\n%s\n%s\n", inputfile,err,outputfile,program_n);

  //read inputfile files and make string
  FILE *fp;
  char *str, *temp;
  long size;

  fp = fopen(inputfile, "r");
  if (fp == NULL) {
      printf("Error: Could not open file\n");
      return 1;
  }

  fseek(fp, 0L, SEEK_END);
  size = ftell(fp);

  str = (char *) malloc(size + 1);
  if (str == NULL) {
      printf("Error: Out of memory\n");
      return 1;
  }

  fseek(fp, 0L, SEEK_SET);
  fread(str, size, 1, fp);
  str[size] = '\0';
  fclose(fp);

  temp = strchr(str, '\n');
  while (temp != NULL) {
      *temp = ' ';
      temp = strchr(temp, '\n');
  }
  printf("%s\n", str);
  reducestring = (char *)malloc(strlen(str));

  //Doing Delta Debugging
  Reduce(str, program_n, err, outputfile);
  free(str);
  printf("finally: %s\n", reducestring);
  free(reducestring);
}

//Recurcively Reduce the inputfile finding the error parts.
void Reduce(char *str, char program_n[], char err[], char outputfile[]){
  printf("inpustring: %s\n",str);
  int input_size = strlen(str);
  int mid_size = input_size-1;
  int index_t, hplustsize, tailsize, headsize;

  while(mid_size>0){
    hplustsize = input_size-mid_size;
    //check head and tail
    for(int i=0;i<=(hplustsize);i++){
      headsize = i;
      tailsize = hplustsize-i;
      index_t = mid_size+i;
      char *head, *tail, *hplust;
      //There is no head
      if(headsize==0){
        tail = (char *) malloc(sizeof(char)*tailsize+1);
        strncpy(tail, str + (index_t), tailsize);
        printf("tail: %s\n",tail);
        if(checkerror(tail) != 0){
          memmove(str + index_t, str + index_t + tailsize, strlen(str) - index_t - tailsize + 1);
          printf("reducedstr: %s\n",str);
          Reduce(tail, program_n, err, outputfile);
          Reduce(str, program_n, err, outputfile);
          free(tail);
          return;
        }
        free(tail);
      }
      //There is no tail
      else if(tailsize==0){
        tail = NULL;
        head = (char *) malloc(sizeof(char)*headsize+1);
        strncpy(head, str + 0, headsize);
        printf("head: %s\n",head);
        if(checkerror(head) != 0){
          memmove(str, str + headsize, strlen(str) - headsize + 1);
          printf("reducedstr: %s\n",str);
          Reduce(str, program_n, err, outputfile);
          Reduce(head, program_n, err, outputfile);
          free(head);
          return;
        }
        free(head);
      }
      //hplust = head+tail
      else{
        tail = (char *) malloc(sizeof(char)*tailsize+1);
        strncpy(tail, str + (index_t), tailsize);
        head = (char *) malloc(sizeof(char)*headsize+1);
        strncpy(head, str + 0, headsize);
        hplust = (char *) malloc(sizeof(char)*hplustsize+1);
        hplust = strcat(head, tail);
        printf("hplust: %s\n",hplust);
        if(checkerror(hplust) != 0){
          memmove(str + index_t, str + index_t + tailsize, strlen(str) - index_t - tailsize + 1);
          memmove(str, str + headsize, strlen(str) - headsize + 1);
          printf("reducedstr: %s\n",str);
          Reduce(str,program_n, err, outputfile);
          Reduce(hplust, program_n, err, outputfile);
          free(hplust);
          free(head);
          free(tail);
          return;
        }
        if(head != NULL)
          free(head);
        if(tail != NULL)
          free(tail);
        //if(hplust != NULL)
          //free(hplust);
      }
    }
    //check mid
    for(int i=0;i<=hplustsize;i++){
      char *mid;
      mid = (char *) malloc(sizeof(char)*mid_size+1);
      strncpy(mid, str + i, mid_size);
      printf("mid: %s\n",mid);
      if(checkerror(mid) != 0){
        memmove(str + i, str + i + mid_size, strlen(str) - i - mid_size + 1);
        printf("reducedstr: %s\n",str);
        Reduce(str,program_n, err, outputfile);
        Reduce(mid, program_n, err, outputfile);
        free(mid);
        return;
      }
      free(mid);
    }
    mid_size = mid_size-1;
  }
  if(checkerror(str) != 0){
    printf("add to reducestring: %s\n",str);
    strcat(reducestring,str);
    printf("ooooooooo: %s\n",reducestring);
  }
}

//interrupt handler
void handler(int sig){
  if(sig == SIGINT){
    printf("Do you want to quit?");
    if(getchar() == 'y')
      printf("%s",reducestring);
      exit(0);
  }
}


//테스트용 함수
int checkerror(char input[]){
  //printf("check: %s\n",input);
  if(strstr(input, "o") || strstr(input, "l")){
    return 1;
  }
  return 0;
}