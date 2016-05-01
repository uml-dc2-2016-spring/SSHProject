# Project Goal:

Students here develop their homework on the CS server.  Currently this is awkward.  Using an editor directly on Mercury (putty > Vim) is subject to internet interruptions.  Using an editor on your laptop requires WinSCP to move your files, and putty to compile them.  This is clunky and clutters your workspace.

I want to make the process easier.  This project is a SSH Client which allows you to move your file, compile, and run it all in the same window.  You can navigate both local and remote filesystems at the same time.

This service is not limited to the CS server.  You can login to any machine running SSH services you have an account for.

# Project Design

![](https://github.com/uml-dc2-2016-spring/SSHProject/blob/master/project-flow.png?raw=true)

We split the project into three parts:  Managing the overall connection, interpreting user commands, and acting on that connection.  The latter two somehow ended up sharing the same file.
There were two options for file transfer, SCP and SFTP.  
The ssh_session is used for authentication, requesting new channels, and reading errors.  The ssh_session is used to create the sftp_session (sftp channel).  The sftp channel is used for remote directory listing and file reading and writing.  We expected the sftp to be used multiple times per program run, so we opted to create one sftp channel and leave it open for the duration of the run.  No sense in repeatedly opening and closing channels.

Menuloop.c:

- Parse(char* input, ssh_session mysess)

This function does two things.  First it checks whether the session is still connected.  If not, returns a value 0, the exit case.
Second it iterates through the array of recognized commands and returns the index.  The index is used in a switch statement in menuloop.  -1 when the input is not recognized.

- Push_single(char[] local, char remote[], char filename[], ssh_session sshses, sftp_session sftpses)

This function is called when we want to push a single file.  Local is the local path, remote is the remote path.  Filename is used both to find the source file and name the remote file to create or overwrite.  The file is opened locally, and read into the sftp channel.  Failure to read or write will print an error message and return an error code.

- Push_all_files(char pathl[], char pathr[], ssh_session sshses, sftp_session sftpses)

This function opens the local folder indicated by pathl and iterates through each item.  If the item begins with '.', it is skipped.
If the item is not a regular files it is skipped.  All regular files are pushed by a call to push_single.  Once the iteration is finished, the number of transferred files and the number of skipped items is printed.  Then the folder is closed.  We chose to close the folder after listing the number of transfered and skipped items, not before, so that even if there is an error in closing the folder, we still know whether there was a transfer.

- list_remote_stuff(char path[], ssh_session sshses, sftp_session sftpses)

This function is passed the current remote path. It uses the function sftp_opendir to open the current remote directory and uses sftp_readdir to go through and print out information about each item in the directory. 


- list_local_stuff(char pathl[])

       This function uses only the current local path and thus does not need the ssh or sftp sessions. It uses opendir to open the local directory and readdir to go through and print out information about each item in the directory. It was necessary to use the dirent struct, provided by dirent.h, for this function.


- change_remote_directory(char remote[], char dirname[], ssh_session sshses, sftp_session sftpses)

       The purpose of this function is to change the current directory on the remote server. Remote is the current remote path, passed by refference, and dirname should be the name of a directory in the current remote directory. When the directory entered is "..", the function strrchr is used to find the last slash character in the current path and replace it with '\0', effectively removing the current directory from the path. Otherwise, it appends the entered directory to the end of the current remote path. 

# File Structure

Our files:

- main.c - Contains connection setup.  #includes menuloop.c

- menuloop.c - Contains the menu loop and all the functions called by said loop.

- readme.md - synopsis and weekly updates

Other files:

- ssh.dll - required for execution

- libeay32.dll

- libssh/headers - needed for compiling
