#include <stdio.h>
#include <string.h>
#include <libssh/libssh.h>
#include <libssh/sftp.h>

//to move to new file
#include <sys/stat.h>
#include <fcntl.h>
#define BUFFER_SIZE 1000
int push_single(char local[],char remote[],char fname[],ssh_session sshses,sftp_session ses);

int push_single(char local[],char remote[],char fname[],ssh_session sshses,sftp_session sftpses){
  sftp_file rfile;
  int access_type = O_WRONLY | O_CREAT | O_TRUNC;
  int rc, nwrite, nread;
  rc = 0;
  char buffer[BUFFER_SIZE];
  //open local file
  FILE* lfile = fopen(fname,"r");
  if( lfile == NULL){
    perror("local file cannot open");
    return -1;}
  //open remote file
  rfile = sftp_open(sftpses,fname,access_type,S_IRWXU);
  if (rfile == NULL)
  {
    fprintf(stderr, "Can't open file for writing: %s\n",
            ssh_get_error(sshses));
    return SSH_ERROR;
  }

  //loop it
  do{
    nread = fread( buffer, sizeof(char),BUFFER_SIZE,lfile);
    nwrite = sftp_write(rfile,buffer,nread);
  }while( (nread == nwrite) && (nread == BUFFER_SIZE) );
  if( nread != nwrite)
    fprintf(stderr, "Can't write data to remote file %s\n",
	    ssh_get_error(sshses));
  rc = sftp_close(rfile);
  fclose(lfile);
  if( rc != SSH_OK )
    fprintf(stderr, "Can't close the written file %s\n",
	    ssh_get_error(sshses));
  return rc;
}

int pull_single(char local[],char remote[],char fname[],ssh_session sshses,sftp_session ftpses);

int pull_single(char local[],char remote[],char fname[],ssh_session sshses,sftp_session ses){
  return 1;}
int do_command(char command[],ssh_session myssh);
int do_command(char command[],ssh_session myssh){
  return 1;}

char* help = "Commands:\nexit\t quit\nhelp\t this help message\ndispl\t display local path (if not home)\ndispr\t display remote path (if not home)\ncdl\t change  local path\ncdr\t change remote path\npushs\t push single file\npulls\t pull single file\nrun\t execute frequent command\n";

char* welcome = "Welcome to SSHProject.  'help' for help.\n";

char* prompt = "\n> ";
int menuloop(char name[100], char pass[100],ssh_session myssh,sftp_session mysftp){
  //int menuloop(char name[100], char pass[100]){
  char comm[100];
  char fname[100];
  char local[200]={"\0"};
  char remote[200]={"\0"};
  char command[100];

  printf("Enter commonly used command\n> ");
  scanf("%s",command);

  printf("%s %s",welcome, prompt);


  //0 exit
  //-1 error
  //1 help
  //2 print local
  //3 print remote
  //4 change local
  //5 change remote
  scanf("%s", comm);

  while( 0 != parse(comm) ){
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
      push_single(local, remote, fname,myssh,mysftp);
      break;
    case 7: //pull single
      printf("Enter file name %s",prompt);
      scanf("%s",fname);
      pull_single(local, remote, fname,myssh, mysftp);
      break;
    case 8: // run command
      do_command(command,myssh);
      break;
    default:
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
char* words[NUMWORDS] = {"exit","help","displ","dispr","cdl","cdr","pushs","pulls","run"};

int parse(char* input){
  int index = 0;
  //map use input to command number
  do{
    //return index of first case of match of input and words
    if( 0 == strcmp( input ,  words[index]) )
      return index;
    index ++;
  }while(index < NUMWORDS);
  //if input not found in words, return -1
  return -1;
}

