#include "libssh/libssh.h"
#include <stdlib.h>
#include <stdio.h> 

extern char name[100];
extern char pass[100];

int testconnection (){
  ssh_session my_ssh_session;
  int rc;
  // Open session and set options
  my_ssh_session = ssh_new();
  if (my_ssh_session == NULL)
    return -1;
  ssh_options_set(my_ssh_session, SSH_OPTIONS_HOST, name);
  // Connect to server
  rc = ssh_connect(my_ssh_session);
  if (rc != SSH_OK)
  {
    fprintf(stderr, "Error connecting to %s: %s\n",
	    name,
            ssh_get_error(my_ssh_session));
    ssh_free(my_ssh_session);
    return -1;
  }
  // Verify the server's identity
  // For the source code of verify_knowhost(), check previous example
  if (verify_knownhost(my_ssh_session) < 0)
  {
    ssh_disconnect(my_ssh_session);
    ssh_free(my_ssh_session);
    return -1;
  }
  // Authenticate ourselves
  rc = ssh_userauth_password(my_ssh_session, NULL, pass);
  if (rc != SSH_AUTH_SUCCESS)
  {
    fprintf(stderr, "Error authenticating with password: %s\n",
            ssh_get_error(my_ssh_session));
    ssh_disconnect(my_ssh_session);
    ssh_free(my_ssh_session);
    return -1;
  }
  ssh_disconnect(my_ssh_session);
  ssh_free(my_ssh_session);
  return 0;
}
