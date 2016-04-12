#include <stdlib.h>
#include <stdio.h>
//#include "SFTPencaps.c"
//#include "SSHencaps.c"
//#include "conntest.c"
#include "menuloop.c"
//#include  "libssh/sftp.h"
#include <libssh/libssh.h>
//#include "libssh/legacy.h"

int auth_ssh(char name[],char pass[],ssh_session* new_ses);

int main(){
  char name[100];//name@remote.domain
  char pass[100];//password
  char command[100];//commonly used command
  //char local[200];//path other than current working dir
  //char remote[200];//path other than current working dir

  printf("Enter name@remote.domain\n> ");
  scanf("%s",name);
  printf("Enter password\n> ");
  scanf("%s",pass);

  ssh_session myssh;
  auth_ssh(name,pass,&myssh);
  sftp_session mysftp;

  if( 0 > menuloop(name,pass,myssh, mysftp) )
    {
      printf("\nerror\n");
      return 1;
    }
  else
    {
      printf("\nBye\n");
      return 0;
    }
}


int auth_ssh(char name[],char pass[],ssh_session* new_ses){
  return 1;}
