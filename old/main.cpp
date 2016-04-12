//Constants:
//number of args needed at command line (for non default)
#DEFINE NEEDED_ARGC 5

int main( int argc, char* argv[]){

  string domain;
  string account;
  string password;
  //connection current;
  string local_path;
  string remote_path;
  bool conn_open = false;
  bool shell_mode = false;
  bool transaction_lock = false;
  bool overwrite_local = true;
  bool overwrite_remote = true;

  if( argc < 3){
    //get values from settings file
    FILE* fopen("settings", 'r');
    //fread?

  }else{
    //get values from command line args
    if( argc != NEEDED_ARGC ){
      printf("Command line args provided: %d, needed: %d\n",argc,NEEDED_ARGC);
      return 1;
    }
    domain = argv[2];
    account = argv[3];
    password = argv[4];
    //local path?
    //remote path?
  }

  //test connection section
  //make connection
  //get connection
  //test
  //end connection
  //close connection


  //Menu loop:
  cout << main_help;//write this in seperate file to allow localization
  string command;

  do{
    cout << prompt;
    cin >> command;
    command = lowercase(command);




  }while( stringcompare( command, quit);
