#include <stdio.h>
#include <string.h>
#include <libssh/libssh.h>
#include <libssh/sftp.h>
//#include <libssh.h>
//#include <sftp.h>
//to move to new file
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#define BUFFER_SIZE 1000
#define DEFAULT_PATH "START_FOLDER"

int push_single(char local[], char remote[], char fname[], ssh_session sshses,sftp_session sftpses);
int pull_single(char local[], char remote[], char fname[], ssh_session sshses,sftp_session sftpses);
int do_command(char command[], ssh_session myssh);
int pull_all_files(char pathl[], char pathr[], ssh_session sshses, sftp_session sftpses);
int list_remote_stuff(char path[], ssh_session sshses, sftp_session sftpses);
int change_remote_directory(char remote[], char dirname[], ssh_session sshses, sftp_session sftpses);
int push_all_files(char pathl[],char pathr[],ssh_session sshses, sftp_session sftpses);
int list_local_stuff(char pathl[]);

char* help = "Commands:\nexit\t quit\nhelp\t this help message\ndispl\t display local path (if not home)\ndispr\t display remote path (if not home)\ncdl\t change  local path\ncdr\t change remote path\npushs\t push single file\npulls\t pull single file\nrun\t execute frequent command\nlsr\t list remote stuff\nlsl\t list stuff local\npulla\t pull all regular files\npusha\t push all regular files\n";

char* welcome = "Welcome to SSHProject.  'help' for help.\n";

char* prompt = "\n> ";

int menuloop(char name[100], char pass[100],ssh_session myssh,sftp_session mysftp){
  //int menuloop(char name[100], char pass[100]){
  char comm[100];
  char fname[100];
  char local[200] = ".";//replace with DEFAULT_PATH
  char remote[200] = ".";
  char command[100];
  
  printf("%s %s",welcome, prompt);


  //0 exit
  //-1 error
  //1 help
  //2 print local
  //3 print remote
  //4 change local
  //5 change remote
  scanf("%s", comm);

  while( 0 != parse(comm,myssh) ){
    switch( parse(comm,myssh) ) {
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
      scanf("%s",fname);
      if( 0 == change_local_directory(local, fname) )
	printf("Path Changed succesfully\n");
      printf("New local: %s",local);
      break;
    case 5: //change remote path
      printf("Current remote path: %s", remote);
      printf("\n Enter a directory: %s", prompt);
      scanf("%s",fname);
      if( SSH_OK != change_remote_directory(remote, fname, myssh, mysftp) )
        printf("Path changed successfully\n");
      printf("New remote: %s", remote);
      break;
    case 6: //push single
      printf("Enter file name:\n%s/", local);
      scanf("%s",fname);
      if( SSH_OK != push_single(local, remote, fname,myssh,mysftp) )
	printf("Push error\n");
      else
	printf("Push success");
      break;
    case 7: //pull single
      printf("Enter file name:\n%s/", remote);
      scanf("%s",fname);
      if( SSH_OK != pull_single(local, remote, fname,myssh, mysftp) )
	printf("Pull error\n");
      else
	printf("Pull success\n");
      break;
    case 8: // run command on remote
      printf("Enter commonly used command\n> ");
      scanf(" %[^\n]s",command);
      if( SSH_OK != do_command(command,myssh) )
      	printf("run error\n");
      else
	printf("Run success");
      break;
    case 9:
      list_remote_stuff(remote,myssh,mysftp);
      break;
    case 10: //list stuff local
      list_local_stuff(local);
      break;
    case 11: //pull all files from path remote to path local
      printf("Pulling all files from %s to %s.",remote,local);
      if( 0 == pull_all_files(local,remote,myssh,mysftp))
	printf("Success");
      else
	printf("Failure");
      break;
    case 12: //push all files
      if( 0 == push_all_files(local,remote,myssh,mysftp) )
	printf("Sucess");
      else
	printf("Failure");
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
#define NUMWORDS 13
char* words[NUMWORDS] = {"exit", "help", "displ", "dispr", "cdl", "cdr", "pushs", "pulls", "run", "lsr", "lsl", "pulla", "pusha"};

int parse(char* input,ssh_session myses){
  //make sure our session is still open
  if(! ssh_is_connected(myses) ){
    printf("Session is not connected.  Exiting.");
    return 0;
  }
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


int push_single(char local[],char remote[],char fname[],ssh_session sshses,sftp_session sftpses){
  sftp_file rfile;
  int access_type = O_WRONLY | O_CREAT | O_TRUNC;
  int rc, nwrite, nread;
  rc = 0;
  char buffer[BUFFER_SIZE];
  
  char rfilepath[100] = "";
  char lfilepath[100] = "";
  strcpy(rfilepath, remote);
  strcpy(lfilepath, local);
  char temp[100] = "/";
  strcat(temp, fname);      // temp just adds a / to fname, "/fname"
  
  //open local file
  strcat(lfilepath, temp);
  FILE* lfile = fopen(lfilepath,"rb");
  if( lfile == NULL){
    perror("local file cannot open");
    return -1;}
  //open remote file
  strcat(rfilepath, temp);
  rfile = sftp_open(sftpses,rfilepath,access_type,-1);
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

int pull_single(char local[],char remote[],char fname[],ssh_session sshses,sftp_session sftpses){
  sftp_file rfile;//remote file
  int access_type = O_RDONLY;
  int rc, nwrite, nread;//return code, #byte written, #byte read
  rc = 0;
  char buffer[BUFFER_SIZE];
  
  char rfilepath[100] = "";
  char lfilepath[100] = "";
  strcpy(rfilepath, remote);
  strcpy(lfilepath, local);
  char temp[100] = "/";
  strcat(temp, fname);      // temp just adds a / to fname, "/fname"
  
  //open remote file
  strcat(rfilepath, temp);
  rfile = sftp_open(sftpses, rfilepath, access_type, 0);
  if (rfile == NULL)
    {
    fprintf(stderr, "Can't open file for writing: %s\n",
            ssh_get_error(sshses));
    return SSH_ERROR;
  }
  //open local file
  strcat(lfilepath, temp);
  FILE* lfile = fopen(lfilepath,"wb");
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

int do_command(char command[],ssh_session myssh){
  ssh_channel channel;
  int rc;
  char buffer[256];
  int nbytes;
  //pre-pend this to force remote encoding to be compatible with
  //local terminal encoding
  char encoding[500] = "LC_ALL=C LANG=C ";
  strcat(encoding, command);
  //create channel to put command onto
  channel = ssh_channel_new(myssh);
  if (channel == NULL)
    return SSH_ERROR;
  rc = ssh_channel_open_session(channel);
  if (rc != SSH_OK)
    {
      ssh_channel_free(channel);
      return rc;
    }
  //execute remote command
  rc = ssh_channel_request_exec(channel, encoding);
  if (rc != SSH_OK)
    {
      ssh_channel_close(channel);
      ssh_channel_free(channel);
      return rc;
    }
  //read loop for remote stdout
  nbytes = ssh_channel_read(channel, buffer, sizeof(buffer), 0);
  while( nbytes > 0)
    {
      if (fwrite(buffer, 1, nbytes, stdout) != nbytes)
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
  //read loop for remote stderr
  nbytes = ssh_channel_read(channel, buffer, sizeof(buffer), 1);
  while( nbytes > 0)
    {
      if (fwrite(buffer, 1, nbytes, stdout) != nbytes)
	{
	  ssh_channel_close(channel);
	  ssh_channel_free(channel);
	  return SSH_ERROR;
	}
      nbytes = ssh_channel_read(channel, buffer, sizeof(buffer), 1);
    }
  if (nbytes < 0)
    {
      ssh_channel_close(channel);
      ssh_channel_free(channel);
      return SSH_ERROR;
    }
  //clean up
  ssh_channel_send_eof(channel);
  ssh_channel_close(channel);
  ssh_channel_free(channel);
  return SSH_OK;
}

int pull_all_files(char pathl[],char pathr[], ssh_session sshses, sftp_session sftpses){
  sftp_dir dir;
  sftp_attributes attributes;
  int rc;
  int filecount = 0; // count of files
  int foldercount = 0;// count of folders
  //attempt to open remote directory
  dir = sftp_opendir(sftpses, pathr);
  //dir = sftp_opendir(sftpses, ".");
  if (!dir)
  {
    fprintf(stderr, "Directory not opened: %s\n",
            ssh_get_error(sshses));
    return SSH_ERROR;
  }
  //loop through each entry of the directory
  while ((attributes = sftp_readdir(sftpses, dir)) != NULL)
    {
      //skip stuff that begins with '.' as it is hidden files
      if('.' == attributes->name[0])
	continue;
      //2 is the type correspoinding to folders
      if(2 == attributes->type){
	foldercount ++;
	//do not move folders
	continue;
      }
      //pull the file and increase count for each success
      if(SSH_OK == pull_single(pathl, pathr,attributes->name, sshses, sftpses))
	filecount ++;
    }
  //handle bad things
  if (!sftp_dir_eof(dir))
    {
      fprintf(stderr, "Can't list directory: %s\n",
	      ssh_get_error(sshses));
      sftp_closedir(dir);
      return SSH_ERROR;
    }
  //place report before closing so a failed close still tells what was moved
  printf("Pulled %d files.  Skipped %d folders.",filecount,foldercount);
  rc = sftp_closedir(dir);
  if (rc != SSH_OK)
    {
      fprintf(stderr, "Can't close directory: %s\n",
	      ssh_get_error(sshses));
      return rc;
    }
 
  return 0;
}

int push_all_files(char pathl[], char pathr[], ssh_session sshses, sftp_session sftpses)
{
  DIR* dirr;
  struct dirent* attrib;
  int rc;
  int filecount = 0;
  int foldercount = 0;
  //attempt to opel local directory
  dirr = opendir(pathl);
  if(NULL == dirr){
    perror("Cannot open local directory.");
    return errno;
  }
  //loop through each entry of the directory
  while( (attrib = readdir( dirr)) != NULL)
    {
      //later on see if the "." filter is needed
      if( '.' == attrib->d_name[0] )
	continue;
      //skip all but regular files
      if( attrib->d_type != DT_REG ){
	foldercount ++;
	continue;
      }
      if(SSH_OK == push_single(pathl, pathr, attrib->d_name,sshses, sftpses) )
	filecount ++;
    }
  //place report before closing so a failed close still tells what was moved
  printf("Pushed %d regular files.  Skipped %d others.",filecount,foldercount);
  if( 0 != closedir(dirr) ){
    perror("Cannot close directory.");
    return errno;
  }
  return 0;
}

int list_remote_stuff(char path[], ssh_session sshses, sftp_session sftpses){
  sftp_dir dir;
  sftp_attributes attributes;
  int rc;
  dir = sftp_opendir(sftpses, path);
  //dir = sftp_opendir(sftpses, ".");
  if (!dir)
  {
    fprintf(stderr, "Directory not opened: %s\n",
            ssh_get_error(sshses));
    return SSH_ERROR;
  }
  printf("Name                       Size Perms ModTime Type\n");
  while ((attributes = sftp_readdir(sftpses, dir)) != NULL)
  {
    //skip all things starting with "."
    if('.' == (attributes->name)[0])
      continue;
    printf("%-20s %10llu %.8o %d %d\n",
     attributes->name,
     (long long unsigned int) attributes->size,
	   attributes-> permissions,
	   attributes-> mtime,
	   attributes-> type);
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

int list_local_stuff(char path[]){
  DIR* dir;
  struct dirent* attrib;
  //attempt to open directory
  dir = opendir(path);
  if(!dir){
    perror("Cannot open directory");
    return errno;
  }
  //print title
  //printf("Name                       Size Perms ModTime Type\n");
  printf("Name\t\t\t\tType\n");
  while( (attrib = readdir(dir)) != NULL){
    //later on see if the "." filter is needed.
    //print the entry
    //printf("%-20s #1011u %.80 %d %d\n",
    //    printf("%-20s\t\t%x\n",
    //	   attrib -> d_name,
    //	   attrib -> d_type
	   //use stat???
    //	   );
        printf("%-20s\t\t", attrib->d_name);
        if (attrib -> d_type == DT_REG) {
            printf("file\n");
        } else if (attrib -> d_type == DT_DIR) {
            printf("folder\n");
        } else {
            printf("%x\n", attrib -> d_type);
        }
    }
  if( closedir(dir) != 0 ){
    perror("Cannot close directory.");
    return errno;
  }
  return 0;
}

int change_remote_directory(char remote[], char dirname[], ssh_session sshses, sftp_session sftpses){
  sftp_dir dir;
  int rc;
  char *p;
  //when the new directory is "..", trim the existing path
  if (strcmp("..", dirname) == 0) {
    p = strrchr(remote, (int) '/');
    if (p != NULL) {
        remote[p - remote] = '\0';
    }
  } else {
    char rdirpath[100] = "";
    strcpy(rdirpath, remote);
    char temp[100] = "/";
    strcat(temp, dirname);      // temp just adds a '/' to dirname, "/dirname"
    
    strcat(rdirpath, temp);
    
    // check that directory is valid by opening it
    dir = sftp_opendir(sftpses, rdirpath);
    if (!dir)
    {
      fprintf(stderr, "Directory not opened: %s\n",
              ssh_get_error(sshses));
      return SSH_ERROR;
    }
    rc = sftp_closedir(dir);
    if (rc != SSH_OK)
    {
      fprintf(stderr, "Can't close directory: %s\n",
              ssh_get_error(sshses));
      return rc;
    }
    //when not returned by failures to open directory,
    //write the new path over the old one.
    strcpy(remote, rdirpath);
  }
  return 0;
}
 int change_local_directory(char local[], char dirname[]){
  DIR* dir;
  int rc;
  char *p;
  
  if (strcmp("..", dirname) == 0) {
    p = strrchr(local, (int) '/');
    if (p != NULL) {
        local[p - local] = '\0';
    }
  } else {
    char rdirpath[100] = "";
    strcpy(rdirpath, local);
    char temp[100] = "/";
    strcat(temp, dirname);      // temp just adds a / to dirname, "/dirname"
    
    strcat(rdirpath, temp);
    
    // check that directory is valid by opening it
    dir = opendir( rdirpath);
    if (!dir)
    {
      perror( "Directory not opened:");
      return errno;
    }
    rc = closedir(dir);
    if (rc != 0)
    {
      perror( "Can't close directory:");
      return errno;
    }
    
    strcpy(local, rdirpath);
  }
  return 0;
}
