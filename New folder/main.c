//#include "SFTPencaps.c"
//#include "SSHencaps.c"
#include "conntest.c"
#include "menuloop.c"

int main(){
  char name[100];//name@remote.domain
  char pass[100];//password
  char command[100];//commonly used command
  char local[200];//path other than current working dir
  char remote[200];//path other than current working dir

  printf("Enter name@remote.domain\n> ");
  scanf("%s",name);
  printf("Enter password\n> ");
  scanf("%s",pass);


  //test connection
  printf("Attempting to connect");
  if( 0 > testconnection() )
    { printf("failed");
      return 1;
    }
  else
    printf("succeeded\n");

  //get common command
  printf("Enter commonly used command\n> ");
  scanf("%s",command);

  if( 0 > menuloop() )
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
