#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <mpi.h>
#include <vector>
#include <pthread.h>

#define MASTER 0
#define NrOfCore 4

using namespace std;

string file;
int nrOfTags;


int getNrFromType(string type) {
    if (!type.compare("horror")) {
        return 1;
    } else if (!type.compare("comedy")){
        return 2;
    } else if (!type.compare("fantasy")) {
        return 3;
    } else if (!type.compare("science-fiction"))  {
        return 4;
    }

    return 0;
} 

string getTypeFromNr(int nr) {
    if (nr == 1) {
        return "horror";
    } else if (nr == 2){
        return "comedy";
    } else if (nr == 3) {
        return "fantasy";
    } else if (nr == 4){
        return "science-fiction";
    }
    return "";
}

bool isAlphabetCharacter(char a) {
    return ((a <= 122) && (a >= 97));
}

// reverse sequence from text
void reverse_String(string& text, long i, long n){

  if ( n <= i)
    return;

  swap(text[i], text[n]);
  reverse_String(text, i+1, n-1);
}

void get_args(int argc, char **argv) {

    if(argc < 2) {
        printf("Numar insuficient de parametri: ./program file \n");
        exit(1);
    }
    
    file = argv[1];
}

void *thread_function(void *arg) {   
    int thread_id = *(int *)arg;

    string myText;
    string text;

    int tag = 1;

    // open input file
    ifstream MyReadFile(file);
    string type;
    getline (MyReadFile, type);


    while (getline (MyReadFile, myText)) {
        // make paragraphs (every thread make for its type)
        if (getNrFromType(type) == thread_id + 1) {
            if (myText == "") {
                text = text + myText + "\n"; 
            } else {
                text = text + myText + " \n"; 
            }
        } 

        // if we read paragraph we send it to worker 
        if (!myText.compare("")) {
            if (getNrFromType(type) == thread_id + 1) {
                MPI_Send(text.c_str(), text.size() + 1, 
                    MPI_CHAR, thread_id + 1, tag, MPI_COMM_WORLD);
            }

            text.clear();  
            tag++; 
            getline (MyReadFile, type);
            if (type == "") {
                tag--;
            }
        }
    }

    if (getNrFromType(type) == thread_id + 1) {
        MPI_Send(&text[0], text.length()+1, MPI_CHAR, getNrFromType(type), tag, MPI_COMM_WORLD);
    }

    // Last send with tag = 0 to stop receiveing in workers
    MPI_Send(&text[0], text.length()+1, MPI_CHAR, thread_id + 1, 0, MPI_COMM_WORLD);

    
    text.clear();

    nrOfTags = tag;
    MyReadFile.close();
    pthread_exit(NULL);
}



int main(int argc, char *argv[]) {
    int id;
    int ierr;
    int p;
    int source;
    MPI_Status status;
    int tag;

    int thread_id[NrOfCore];
    pthread_t tid[NrOfCore];

    //  Initialize MPI.

    int provided;
    get_args(argc, argv);
    ierr = MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);

    if (ierr != 0) {
        cout << "Fatal Error\n";
        exit ( 1 );
    }

    //  Get the number of processes.

    ierr = MPI_Comm_size(MPI_COMM_WORLD, &p);

    //  Determine this process's rank.

    ierr = MPI_Comm_rank(MPI_COMM_WORLD, &id);

    if (id == MASTER) {

        // Start threads
        for (int i = 0; i < NrOfCore; i++) {
            thread_id[i] = i;
            pthread_create(&tid[i], NULL, thread_function, &thread_id[i]);
        }

        for (int i = 0; i < NrOfCore; i++) {
            pthread_join(tid[i], NULL);
        }
        
        // Make name of output file
        file.resize(file.size() - 3);

        // Create and open file (rw)
        ofstream MyFile(file+"out");

        string recv_text;

        // Receive paragraphs from workers and put in output file
        for (int i = 1; i <= nrOfTags; i++) {
            // get length of received text
            MPI_Probe(MPI_ANY_SOURCE, i, MPI_COMM_WORLD, &status);
            int count;
            MPI_Get_count(&status, MPI_CHAR, &count);

            char *recv_text = new char[count];
            MPI_Recv(recv_text, count, MPI_CHAR, MPI_ANY_SOURCE, i, MPI_COMM_WORLD, &status);
            
            MyFile << getTypeFromNr(status.MPI_SOURCE)+"\n";
            MyFile << recv_text;
            
            delete [] recv_text;
        }

        MyFile.close();

    } else {


        vector<pair<string, int>> v;

        while (1) { 
            // get length of received text
            int count;
            MPI_Probe(0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            MPI_Get_count(&status, MPI_CHAR, &count);
            char *recv_text = new char[count];

            MPI_Recv(recv_text, count, MPI_CHAR, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            
            

            string send_text(recv_text, count);
            delete [] recv_text;
            // Every worker builds paragraphs for its type
            // When we received text with tag 0 we stop recieved paragraphs
            // and start send it to Master
            if (status.MPI_TAG == 0) {
                for (int i = 0; i < v.size(); i++) {
                    MPI_Send(v[i].first.c_str(), v[i].first.size()+1, 
                                MPI_CHAR, 0, v[i].second, MPI_COMM_WORLD);
                }
                break;
            } else if (id == 1) {
                string consonants = "bcdgfhjklmnpqrstvwxzy";
                for (long i = 0; i < send_text.length(); i++) {
                    if (consonants.find(send_text[i]) != std::string::npos) { 
                        send_text.insert(i+1, 1, send_text[i]); 
                        i++;
                    } else if (consonants.find(send_text[i] + 32) != std::string::npos) {
                        send_text.insert(i+1, 1, send_text[i] + 32); 
                        i++;
                    }
                }
            } else if (id == 2) {
                long j = 1;
                for (long i = 0; i < send_text.length(); i++) {

                    if (send_text[i] == ' ') {
                        j = 1;
                        i++;
                    }

                    if (send_text[i] == '\n') {
                        j = 1;
                        i++;
                    }

                    if (j % 2 == 0 && isAlphabetCharacter(send_text[i])) { 
                        send_text[i] = send_text[i] - 32;
                    }
                    j++;
                }
            } else if (id == 3) {
                if (isAlphabetCharacter(send_text[0])) {
                    send_text[0] = send_text[0] - 32;   
                }

                for (int i = 1; i < send_text.length(); i++) {
                    if (isAlphabetCharacter(send_text[i]) && 
                        (send_text[i - 1] == ' ' || send_text[i - 1] == '\n')) {
                        send_text[i] = send_text[i] - 32;
                    }
                }
            } else if (id == 4) {
                long nrOfWords = 0;
                long nrOfChar = 0;
                for (long i = 0; i < send_text.length(); i++) {
                    if (send_text[i] == '\n') {
                        nrOfWords = 0;
                    }

                    if (send_text[i] == ' ') {
                        nrOfWords++;
                        if (nrOfWords % 7 == 0){
                            reverse_String(send_text, i - nrOfChar, i-1);
                        } 
                        nrOfChar = 0;
                    } else {
                        nrOfChar++;
                    }
                }
            }
            // Store paragraphs and its tag in vector
            v.push_back(make_pair(send_text, status.MPI_TAG));
        }
    }


    MPI_Finalize();
    
    
    return 0;
}