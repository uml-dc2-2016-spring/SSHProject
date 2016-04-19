#include <stdio.h>
#include <string.h>
#include <libssh/libssh.h>
#include <libssh/sftp.h>
//#include <libssh.h>
//#include <sftp.h>
//to move to new file
#include <sys/stat.h>
#include <fcntl.h>
#define BUFFER_SIZE 1000
int push_single(char local[],char remote[],char fname[],ssh_session sshses,sftp_session sftpses);

int push_single(char local[],char remote[],char fname[],ssh_session sshses,sftp_session sftpses){
  sftp_file rfile;
  int access_type = O_WRONLY | O_CREAT | O_TRUNC;
  int rc, nwrite, nread;
  rc = 0;
  char buffer[BUFFER_SIZE];
  //open local file
  FILE* lfile = fopen(fname,"rb");
  if( lfile == NULL){
    perror("local file cannot open");
    return -1;}
  //open remote file
  rfile = sftp_open(sftpses,fname,access_type,-1);
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

int pull_single(char local[],char remote[],char fname[],ssh_session sshses,sftp_session sftpses);

int pull_single(char local[],char remote[],char fname[],ssh_session sshses,sftp_session sftpses){
  sftp_file rfile;//remote file
  int access_type = O_RDONLY;
  int rc, nwrite, nread;//return code, #byte written, #byte read
  rc = 0;
  char buffer[BUFFER_SIZE];
  //open remote file
  rfile = sftp_open(sftpses,fname,access_type,0);
  if (rfile == NULL)
    {
    fprintf(stderr, "Can't open file for writing: %s\n",
            ssh_get_error(sshses));
    return SSH_ERROR;
  }
  //open local file
  FILE* lfile = fopen(fname,"wb");
  if( lfile == NULL){
    perror("local file cannot open");
    return -1;}
  //loop it
  do{
    nread = sftp_read(rfile,buffer,sizeof(buffer));
    nwrite = fwrite( buffer, 1 , nread, lfile );
    //nwrite = fwrite( buffer, sizeof(char),BUFFER_SIZE,lfile);
  }while( (nread == nwrite) && (nwrite == BUFFER_SIZE) );
  if( nread != nwrite)
    fprintf(stderr, "Can't write data to local file %s\n",
	    ssh_get_error(sshses));
  rc = sftp_close(rfile);
  fclose(lfile);
  if( rc != SSH_OK )
    fprintf(stderr, "Can't close the remote file %s\n",
	    ssh_get_error(sshses));
  return rc;
}

int do_command(char command[],ssh_session myssh);
int do_command(char command[],ssh_session myssh){
  ssh_channel channel;
  int rc;
  char buffer[256];
  int nbytes;
  channel = ssh_channel_new(myssh);
  if (channel == NULL)
    return SSH_ERROR;
  rc = ssh_channel_open_session(channel);
  if (rc != SSH_OK)
  {
    ssh_channel_free(channel);
    return rc;
  }
  rc = ssh_channel_request_exec(channel, command);
  if (rc != SSH_OK)
  {
    ssh_channel_close(channel);
    ssh_channel_free(channel);
    return rc;
  }
  nbytes = ssh_channel_read(channel, buffer, sizeof(buffer), 0);
  while (nbytes > 0)
  {
    if (write(1, buffer, nbytes) != (unsigned int) nbytes)
    {
      ssh_channel_close(channel);
      ssh_channel_free(channel);
      return SSH_ERROR;
    }
    nbytes = ssh_channel_read(channel, buffer, sizeof(buffer), 0);
  }
    
  if (nbytes < 0)
    {
    ssh_channel_close(channel);
    ssh_channel_free(channel);
    return SSH_ERROR;
  }
  ssh_channel_send_eof(channel);
  ssh_channel_close(channel);
  ssh_channel_free(channel);
  return SSH_OK;
}

int list_remote_stuff(char path[], ssh_session sshses, sftp_session sftpses);

int list_remote_stuff(char path[], ssh_session sshses, sftp_session sftpses){
  sftp_dir dir;
  sftp_attributes attributes;
  int rc;
  //dir = sftp_opendir(sftpses, path);
  dir = sftp_opendir(sftpses, ".");
  if (!dir)
  {
    fprintf(stderr, "Directory not opened: %s\n",
            ssh_get_error(sshses));
    return SSH_ERROR;
  }
  printf("Name                       Size Perms    Owner\tGroup\n");
  while ((attributes = sftp_readdir(sftpses, dir)) != NULL)
  {
    printf("%-20s %10llu %.8o %s(%d)\t%s(%d)\n",
     attributes->name,
     (long long unsigned int) attributes->size,
     attributes->permissions,
     attributes->owner,
     attributes->uid,
     attributes->group,
     attributes->gid);
     sftp_attributes_free(attributes);
  }
  if (!sftp_dir_eof(dir))
  {
    fprintf(stderr, "Can't list directory: %s\n",
            ssh_get_error(sshses));
    sftp_closedir(dir);
    return SSH_ERROR;
  }
  rc = sftp_closedir(dir);
  if (rc != SSH_OK)
  {
    fprintf(stderr, "Can't close directory: %s\n",
            ssh_get_error(sshses));
    return rc;
  }
  return 0;
}

char* help = "Commands:\nexit\t quit\nhelp\t this help message\ndispl\t display local path (if not home)\ndispr\t display remote path (if not home)\ncdl\t change  local path\ncdr\t change remote path\npushs\t push single file\npulls\t pull single file\nrun\t execute frequent command\nlsr\t list remote stuff";

char* welcome = "Welcome to SSHProject.  'help' for help.\n";

char* prompt = "\n> ";
int menuloop(char name[100], char pass[100],ssh_session myssh,sftp_session mysftp){
  //int menuloop(char name[100], char pass[100]){
  char comm[100];
  char fname[100];
  char local[200]={"\0"};
  char remote[200]={"\0"};
  char command[100];

  local = ".";
  remote = ".";
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
    case 2: // display local path
      printf("%s",local);
      break;
    case 3: //display remote path
      printf("%s",remote);
      break;
    case 4: //change local path
      printf("Old local: %s",local);
      printf("\n Enter new local %s",prompt);
      scanf("%s",local);
      printf("New local: %s",local);
      break;
    case 5: //change remote path
      printf("Old remote: %s",remote);
      printf("\n Enter new remote %s",prompt);
      scanf("%s",remote);
      printf("New remote: %s",remote);
      break;
    case 6: //push single
      printf("Enter file name %s",prompt);
      scanf("%s",fname);
      if( SSH_OK != push_single(local, remote, fname,myssh,mysftp) )
	printf("Push error\n");
      else
	printf("Push success");
      break;
    case 7: //pull single
      printf("Enter file name %s",prompt);
      scanf("%s",fname);
      if( SSH_OK != pull_single(local, remote, fname,myssh, mysftp) )
	printf("Pull error\n");
      else
	printf("Pull success\n");
      break;
    case 8: // run command on remote
      if( SSH_OK != do_command(command,myssh) )
	printf("run error\n");
      else
	printf("Run success");
      break;
    case 9:
      list_remote_stuff(remote,myssh,mysftp);
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
#define NUMWORDS 10
char* words[NUMWORDS] = {"exit","help","displ","dispr","cdl","cdr","pushs","pulls","run","lsr"};

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

