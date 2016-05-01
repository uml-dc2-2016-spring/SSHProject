# Project Goal:

Students here develop their homework on the CS server.  Currently this is awkward.  Using an editor directly on Mercury (putty > Vim) is subject to internet interruptions.  Using an editor on your laptop requires WinSCP to move your files, and putty to compile them.  This is clunky and clutters your workspace.

I want to make the process easier.  This project is a SSH Client which allows you to move your file, compile, and run it all in the same window.  You can navigate both local and remote filesystems at the same time.

This service is not limited to the CS server.  You can login to any machine running SSH services you have an account for.

# Project Design

insert picture here

Menuloop.c:

- Parse(char* input, ssh_session mysess)
- Pushs

# File Structure

- Our files:

main.c - Contains connection setup.  #includes menuloop.c

menuloop.c - Contains the menu loop and all the functions called by said loop.

readme.md - synopsis and weekly updates

- Other files:

ssh.dll - required for execution

libeay32.dll

libssh/headers - needed for compiling
