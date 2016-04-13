#include <stdlib.h>
#include <stdio.h>
//#include "SFTPencaps.c"
//#include "SSHencaps.c"
//#include "conntest.c"
#include "menuloop.c"

int auth_ssh(char name[],char pass[],ssh_session* new_ses);

int verify_knownhost(ssh_session session);


int main(){
  int rc;
  char name[100];//name@remote.domain
  char pass[100];//password
  char command[100];//commonly used command
  //char local[200];//path other than current working dir
  //char remote[200];//path other than current working dir

  // Open session and set options
  ssh_session myssh;
  myssh = ssh_new();
  if (myssh == NULL)
    exit(-1);
  printf("Enter name@remote.domain\n> ");
  scanf("%s", name);
  ssh_options_set(myssh, SSH_OPTIONS_HOST, name);

  // Connect to server
  rc = ssh_connect(myssh);
  if (rc != SSH_OK)
  {
    fprintf(stderr, "Error connecting to %s: %s\n", name, ssh_get_error(myssh));
    ssh_free(myssh);
    exit(-1);
  }
  
  // Verify the server's identity
  if (verify_knownhost(myssh) < 0)
  {
    ssh_disconnect(myssh);
    ssh_free(myssh);
    exit(-1);
  }
  
  // Authenticate ourselves
  //password = getpass("Password: ");
  printf("Enter password\n> ");
  scanf("%s", pass);
  rc = ssh_userauth_password(myssh, NULL, pass);
  if (rc != SSH_AUTH_SUCCESS)
  {
    fprintf(stderr, "Error authenticating with password: %s\n",
            ssh_get_error(myssh));
    ssh_disconnect(myssh);
    ssh_free(myssh);
    exit(-1);
  }
  
  sftp_session mysftp;
  mysftp = sftp_new(myssh);
  if (mysftp == NULL)
  {
    fprintf(stderr, "Error allocating sftp session: %s\n",
            ssh_get_error(myssh));
    return SSH_ERROR;
  }
  rc = sftp_init(mysftp);
  if (rc != SSH_OK)
  {
    fprintf(stderr, "Error initializing sftp session: %s.\n", (char*) sftp_get_error(mysftp));
    sftp_free(mysftp);
    return rc;
  }
  
  if( 0 > menuloop(name, pass, myssh, mysftp)) {
      sftp_free(mysftp);
      ssh_disconnect(myssh);
      ssh_free(myssh);
      
      printf("\nerror\n");
      return 1;
  } else {
      sftp_free(mysftp);
      ssh_disconnect(myssh);
      ssh_free(myssh);
    
      printf("\nBye\n");
      return 0;
  }
      sftp_free(mysftp);
      ssh_disconnect(myssh);
      ssh_free(myssh);
  return 0;
}


int auth_ssh(char name[],char pass[],ssh_session* new_ses){
  int rc = 0;
  /*
  *new_ses = ssh_new();
  ssh_options_set(*new_ses, SSH_OPTIONS_HOST, name);
  rc = ssh_userauth_password(*new_ses, NULL, pass);
  
  if (rc != SSH_AUTH_SUCCESS) {
    fprintf(stderr, "Error authenticating with password: %s\n",
            ssh_get_error(*new_ses));
    ssh_disconnect(*new_ses);
    ssh_free(&new_ses);
    exit(-1);
  }
  */
  return rc;
}

int verify_knownhost(ssh_session session)
{
  int state, hlen;
  unsigned char *hash = NULL;
  char *hexa;
  char buf[10];
  state = ssh_is_server_known(session);
  hlen = ssh_get_pubkey_hash(session, &hash);
  if (hlen < 0)
    return -1;
  switch (state)
  {
    case SSH_SERVER_KNOWN_OK:
      break; /* ok */
    case SSH_SERVER_KNOWN_CHANGED:
      fprintf(stderr, "Host key for server changed: it is now:\n");
      ssh_print_hexa("Public key hash", hash, hlen);
      fprintf(stderr, "For security reasons, connection will be stopped\n");
      free(hash);
      return -1;
    case SSH_SERVER_FOUND_OTHER:
      fprintf(stderr, "The host key for this server was not found but an other"
        "type of key exists.\n");
      fprintf(stderr, "An attacker might change the default server key to"
        "confuse your client into thinking the key does not exist\n");
      free(hash);
      return -1;
    case SSH_SERVER_FILE_NOT_FOUND:
      fprintf(stderr, "Could not find known host file.\n");
      fprintf(stderr, "If you accept the host key here, the file will be"
       "automatically created.\n");
      /* fallback to SSH_SERVER_NOT_KNOWN behavior */
    case SSH_SERVER_NOT_KNOWN:
      hexa = ssh_get_hexa(hash, hlen);
      fprintf(stderr,"The server is unknown. Do you trust the host key?\n");
      fprintf(stderr, "Public key hash: %s\n", hexa);
      free(hexa);
      if (fgets(buf, sizeof(buf), stdin) == NULL)
      {
        free(hash);
        return -1;
      }
      //if (strncasecmp(buf, "yes", 3) != 0)
      if (!
      strcmp(buf, "y"))
      {
        free(hash);
        return -1;
      }
      if (ssh_write_knownhost(session) < 0)
      {
        fprintf(stderr, "Error %s\n", strerror(errno));
        free(hash);
        return -1;
      }
      break;
    case SSH_SERVER_ERROR:
      fprintf(stderr, "Error %s", ssh_get_error(session));
      free(hash);
      return -1;
  }
  free(hash);
  return 0;
}
