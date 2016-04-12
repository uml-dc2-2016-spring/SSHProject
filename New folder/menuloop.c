#include <stdio.h>
#include <string.h>

char* prompt = "\n> ";
int menuloop(char* name, char* pass, char* *command){
  char comm[100];
  char fname[100];
  char local[200]={"\0"};
  char remote[200]={"\0")};
  printf("%s %s"welcome, prompt);


  //0 exit
  //-1 error
  //1 help
  //2 print local
  //3 print remote
  //4 change local
  //5 change remote
  scanf(%s, comm);

  while( 0 < parse(comm) ){
    switch( parse(comm) ) {
    case 0:
      return 0;
      break;
    case -1:
      printf("Not recognized %s",prompt);
      break;
    case 1: //display help
      printf("%s",help);
      break;
    case 2: // display local
      printf("%s",local);
      break;
    case 3: //display remote
      printf("%s",remote);
      break;
    case 4: //change local
      printf("Old local: %s",local);
      printf("\n Enter new local %s",prompt);
      scanf("%s",local);
      printf("New local: %s",local);
      break;
    case 5: //change remote
      printf("Old remote: %s",remote);
      printf("\n Enter new remote %s",prompt);
      scanf("%s",remote);
      printf("New remote: %s",remote);
      break;
    case 6: //push single
      printf("Enter file name %s",prompt);
      scanf("%s",fname);
      push_single(local, remote, fname);
      break;
    case 7: //pull single
      printf("Enter file name %s",prompt);
      scanf("%s",fname);
      pull_single(local, remote, fname);
      break;
    case 8: // run command
      do_command(*command);
      break;
    case default:
      printf("oops");
      return -1;
    }
    
    printf("%s",prompt);
    scanf("%s",comm);
  }
  return 0;
}

//for parsing user input
#define NUMWORDS 9
char* words[NUMWORDS] = {"exit","help","displ","dispr","cdl","cdr","pushs","pulls","run"}

int parse(char* input){
  int index = 0;
  //map use input to command number
  do{
    //return index of first case of match of input and words
    if( 0 == strcmp( input ,  words[index]) )
      return index;
    index ++;
  }
  //if input not found in words, return -1
  return -1;
}

char* help = "Commands:\n
exit\t quit\n
help\t this help message\n
displ\t display local path (if not home)\n
dispr\t display remote path (if not home)\n
cdl\t change  local path\n
cdr\t change remote path\n
pushs\t push single file\n
pulls\t pull single file\n
run\t execute frequent command\n";

char* welcome = "Welcome to SSHProject.  'help' for help.\n";
