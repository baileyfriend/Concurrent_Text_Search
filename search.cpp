/*
    Bailey Freund
    2/14/18
    CIS 452 10 
    Program 2 - Concurrent Text Search
*/

#include <iostream> 
#include <errno.h> 
#include <unistd.h> 
#include <stdlib.h> 
#include <string.h>
#include <fstream>
#include <boost/filesystem.hpp>
#include <sys/types.h>
#include <dirent.h>

using namespace std;
string FILENAME = "/Users/baileyfreund/Desktop/OS/prog2/sherlock.txt";
int search (string word, string filename, pid_t p_pid); 

// arguments :  arg is an untyped pointer 
// returns :   a pointer to whatever was passed in to arg 
void read_directory(const string&, vector<string>& );

int READ = 1;
int WRITE = 0;

int main() 
{ 
    // put all files in directory into a vector
    vector<string> files;
    read_directory(".", files);
    copy(files.begin(), files.end(),
         ostream_iterator<string>(cout, "\n"));

    bool done = false;
    bool fork_flag = true;
    bool i_am_the_parent = false;
    int num_processes = 0;

    pid_t pid, c_pid, p_pid;
    int status;
    string my_file;

    // Create down pipe
    int down_pipe[2];
    // Create up pipe
    int up_pipe[2];

    while(!done && fork_flag) {

        num_processes = 0;
       
        p_pid = getpid(); // Get parent pid
        cout << "PARENT: My pid is " << p_pid << " \n";


        // Get user input
        string word = "";
        cout << "What word would you like to search for: ";
        cin >> word;
        cout << "You entered: " << word << "\n";
        
        for(int i = 0; i < files.size(); i++){ //loop through files
            if(fork_flag) { // prevent children from re-entering the loop
                num_processes++;
                cout << "looking at file " << files[i] << "\n";
                if(files[i].find(".txt") != string::npos){ // only search text files
                    my_file = files[i];
                    if (pipe(down_pipe) < 0) {
                        perror ("plumbing problem");
                        exit (1);
                    }
                    
                    
                    if (pipe(up_pipe) < 0) {
                        perror ("plumbing problem");
                        exit (1);
                    }
                    
                    if(fork_flag){
                        fork_flag = false; // set to false - will cause child not to fork again
                        if ((pid = fork()) < 0) {
                            perror("fork failure");
                            exit(1);
                        }
                    }

                    if(pid > 0){ // parent
                        fork_flag = true;  // reset fork flag so that parent can fork again
                    }

                } // end if files find
            } // end if fork flag
        } // end for

        if (pid == 0) { // CHILD
            cout << pid << " entered child process logic \n";
            close(down_pipe[WRITE]); // child will only read from down pipe
            close(up_pipe[READ]); // child will only write to up pipe

            if (dup2( up_pipe[WRITE], STDOUT_FILENO  ) < 0) {
                cout << "DUP2 FAILED child";
                exit(1);
            }
            
            search(word, my_file, p_pid);

        } else { // PARENT
            cout << pid << " entered parent process logic \n";
            i_am_the_parent = true;
            close(down_pipe[READ]); // down pipe is for writing from parent to child
            close(up_pipe[WRITE]); // down pipe is for writing from parent to child
            cout << "DUP2ING UPPIPE READ TO STDOUT \n";
            if (dup2(STDOUT_FILENO, up_pipe[READ] ) < 0) { // this seems to be failing
                cout << "DUP2 FAILED parent";
                exit(1);
            }
            
            for(int i = 0; i <= num_processes; i++){ // wait for all of the child processes
                c_pid = wait(&status);
                cout << "waited for child " << c_pid << " which returned status " << status << "\n";
            }

            
        }

        cout << " done = " << done << " and fork flag = " << fork_flag << "\n";
    } // end while loop
        
}

int search (string word, string filename, pid_t p_pid) 
{ 
    
    int num = 0;
    ifstream search_file (filename); // http://www.cplusplus.com/doc/tutorial/files/
    if (search_file.is_open())
    {
        string line;
        while ( getline (search_file,line) )
        {
            if( line.find(word) != string::npos){
                // cout << line << "\n";
                num++;
            } 
        }
        search_file.close();
    }

    int c_pid = getpid();
    cout << "CHILD: My pid is " << c_pid << ", parent's pid is " << p_pid << ", found word '" << word << "' " << num << " times " << "in file " << filename << "\n"; 
    exit(0);
    //return num;
}

void read_directory(const string& name, vector<string>& v) // http://www.martinbroadhurst.com/list-the-files-in-a-directory-in-c.html
{
    DIR* dirp = opendir(name.c_str());
    struct dirent * dp;
    while ((dp = readdir(dirp)) != NULL) {
        v.push_back(dp->d_name);
    }
    closedir(dirp);
}