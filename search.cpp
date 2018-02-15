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

    // Create up pipe
    int up_pipe[2];
    
    
    if (pipe(up_pipe) < 0) {
        perror ("plumbing problem");
        exit (1);
    }

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
            //close(up_pipe[READ]); // child will only write to up pipe

            int num = 0;
            ifstream search_file (my_file); // http://www.cplusplus.com/doc/tutorial/files/
            if (search_file.is_open())
            {
                string line;
                while ( getline (search_file,line) )
                {
                    if( line.find(word) != string::npos){ // look for the word in the line
                        num++; // increment the number if you find it
                    } 
                }
                search_file.close();
            }
        
            int c_pid = getpid();
            
            string result = "CHILD: My pid is " + to_string(c_pid) + ", parent's pid is " + to_string(p_pid) + ", found word '" + word + "' " + to_string(num) + " times in file " + my_file + "\n";
            int size = result.length();
            // write(up_pipe[WRITE], &size, sizeof(int)); /*ALTERNATE WAY OF IMPLEMENTING WRITE*/
            perror("CHILD: writing to up_pipe...");
            write(up_pipe[1], result.c_str(), size);
            cout << result;
            perror("CHILD: wrote to up_pipe. Exiting...");
            sleep(2);
            exit(0);


        } else { // PARENT
            i_am_the_parent = true;
            close(up_pipe[WRITE]); // down pipe is for writing from parent to child
            char buf;
            for(int i = 0; i <= num_processes; i++){ // wait for all of the child processes
                
                
                
                    
                perror("About to read from pipe error: ");
                while (read(up_pipe[READ], &buf, 1) > 0){ // loop through the up_pipe
                    //write(STDOUT_FILENO, &buf, 1);
                    cout << buf; //print the character
                    //perror(to_string(buf).c_str());
                }
                c_pid = wait(&status);
                cout << "waited for child " << c_pid << " which returned status " << status << "\n";
                /*ALTERNATE WAY OF IMPLEMENTING READ*/
                // int size;
                // read(up_pipe[READ], &size, sizeof(int));
                // cout << "got size " << size << "\n";
                // char * child_result = new char[size];
                // read(up_pipe[READ], &child_result, size);
                // cout << "PARENT: result from child: " << child_result << "\n";
                
            }

        }
    } // end while loop
        
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