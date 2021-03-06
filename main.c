#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "menuloop.c"

int auth_ssh(char name[],char pass[],ssh_session* new_ses);

int verify_knownhost(ssh_session session);


int main(){
  int rc;
  char name[100];//name@remote.domain
  char pass[100];//password

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

int verify_knownhost(ssh_session session)
{
  char *hexa;
  int state;
  char buf[10];
  unsigned char *hash = NULL;
  size_t hlen;
  ssh_key srv_pubkey;
  int rc;

  state=ssh_is_server_known(session);

  rc = ssh_get_publickey(session, &srv_pubkey);
  if (rc < 0) {
      return -1;
  }

  rc = ssh_get_publickey_hash(srv_pubkey,
                              SSH_PUBLICKEY_HASH_SHA1,
                              &hash,
                              &hlen);
  ssh_key_free(srv_pubkey);
  if (rc < 0) {
      return -1;
  }
  
  switch(state){
    case SSH_SERVER_KNOWN_OK:
      break; /* ok */
    case SSH_SERVER_KNOWN_CHANGED:
      fprintf(stderr,"Host key for server changed : server's one is now :\n");
      ssh_print_hexa("Public key hash",hash, hlen);
      ssh_clean_pubkey_hash(&hash);
      fprintf(stderr,"For security reason, connection will be stopped\n");
      return -1;
    case SSH_SERVER_FOUND_OTHER:
      fprintf(stderr,"The host key for this server was not found but an other type of key exists.\n");
      fprintf(stderr,"An attacker might change the default server key to confuse your client"
          "into thinking the key does not exist\n"
          "We advise you to rerun the client with -d or -r for more safety.\n");
      return -1;
    case SSH_SERVER_FILE_NOT_FOUND:
      fprintf(stderr,"Could not find known host file. If you accept the host key here,\n");
      fprintf(stderr,"the file will be automatically created.\n");
      /* fallback to SSH_SERVER_NOT_KNOWN behavior */
    case SSH_SERVER_NOT_KNOWN:
      hexa = ssh_get_hexa(hash, hlen);
      fprintf(stderr,"The server is unknown. Do you trust the host key?\n");
      fprintf(stderr, "Public key hash: %s\n>", hexa);
      ssh_string_free_char(hexa);
      if (fgets(buf, sizeof(buf), stdin) == NULL) {
	    ssh_clean_pubkey_hash(&hash);
        return -1;
      }
      //if(strncasecmp(buf,"yes",3)!=0){
      if (!strcmp(buf, "y")) {
        ssh_clean_pubkey_hash(&hash);
        return -1;
      }
      fprintf(stderr,"This new key will be written on disk for further usage. do you agree? y/n\n>");
      if (fgets(buf, sizeof(buf), stdin) == NULL) {
	    ssh_clean_pubkey_hash(&hash);
        return -1;
      }
      //if(strncasecmp(buf,"yes",3)==0){
      if (strcmp(buf, "y")) {
        if (ssh_write_knownhost(session) < 0) {
          ssh_clean_pubkey_hash(&hash);
          fprintf(stderr, "error %s\n", strerror(errno));
          return -1;
        }
      }

      break;
    case SSH_SERVER_ERROR:
      ssh_clean_pubkey_hash(&hash);
      fprintf(stderr,"%s",ssh_get_error(session));
      return -1;
  }
  ssh_clean_pubkey_hash(&hash);
  return 0;
}
