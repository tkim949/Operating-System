/*
* * Assignment4 - OTP: opt_dec_d.c
* * CS 344 - 2019 Fall 
* * Taekyoung Kim - Due date: Dec. 6, 2019
* * kimtaeky@oregonstate.edu
* * Description: this program works 
* * ............
*/

#include <stdio.h> //fprintf(), printf(), stderr, perror()
#include <stdlib.h> // malloc(), free(), exit()
#include <unistd.h> //chdir(), fork(), exec(), pid_t
#include <string.h> // strcmp(), strtok()
#include <sys/types.h> //waitpid()
#include <sys/wait.h> //waitpid()
#include <sys/socket.h> //sa_family_t
#include <netinet/in.h> //in_addr_t, sa_family_t, in_port_t
//#include <signal.h> //SIGINT
//#include <netdb.h> //h_addrtype, h_length,struct hostent*

//I reused this struct to reap children pids from my smallsh.c
/****
struct PidBGs
{
    int countPidBG;
    pid_t pidNumBG[5];
};***/
struct childBG {
	
	int *pids; //process id number
	int count; //the number of children
	
};

//From server.c in lecture, Error function used for reporting issues.
void error(const char *msg) { perror(msg);  exit(1);} 

//https://www.studytonight.com/c/pointers-to-structure-in-c.php
//https://www.includehelp.com/c/pointer-to-structure.aspx
struct childBG* initCBG();

void  freeCBG(struct childBG*);
void  pushChild(struct childBG*, int);
void  removeChild(struct childBG*, int);
void  waitChild(struct childBG*);

struct childBG* initCBG() {
	//for 5 children and one more
	int* array = malloc(sizeof(int) * 6); //This is for the process id number
	struct childBG* childST = malloc(sizeof(struct childBG));
	childST->pids = array;
	
	childST->count = 0;
	
	return childST;
}

void  freeCBG(struct childBG* child) {
	
	free(child->pids);
	free(child);
	return;
	//https://www.geeksforgeeks.org/return-void-functions-c/
	//https://stackoverflow.com/questions/2249108/can-i-return-in-void-function
}

void pushChild(struct childBG* child, int childID){
	//we only can have up to 5 child processes
	if(child->count < 5) {
		child->pids[child->count] = childID;
		child->count++;
	}
	return;
}

void waitChild(struct childBG* child) {
    
	int total = child->count;
	int status;
	if (total == 0) {
		return;
	}
	int i; 
    for (i = total - 1; i >= 0; i--) {
		// Call wait for all children
		pid_t childPID = waitpid(child->pids[i], &status, WNOHANG); //without waiting
		if (childPID == 0) {
            continue;  //if there is a child, alive
        }
		else {
			  // But if there is already terminated child, call remove function
			  removeChild(child, child->pids[i]);
		}
    }
}

void removeChild(struct childBG* child, int pidN) {
	
    if (child->count == 0) { //if there is no child
        return;
    }

    // Find the index that has the process id that is given
    int i;
    int found = 0;
    for (i = 0; i < child->count; i++) {
        if (child->pids[i] == pidN) {
            found = 1;
            break;
        }
    }

    // Remove the child with that given pid 
	//To do this, we need to shift it
    if (found == 1) {
        child->pids[5] = child->pids[i];
        int j;
        for (j = i; j < child->count; j++) {
            child->pids[j] = child->pids[j + 1];
        }
        child->count--;
    }
}



/******
struct PidBGs pidBackG;
//signal made some troubles....
//void killBGPids(int sig)
void killBGPids()
{   
      pid_t childPid;
	int childStatus;
    int i;
    for(i = 0;i < pidBackG.countPidBG;i++){
		childPid = waitpid(pidBackG.pidNumBG[i],&childStatus, WNOHANG); //without waiting
		if(childPid == 0) {  //It means child process id is still running!
                 kill( pidBackG.pidNumBG[i], SIGINT); 
		         // interrupt all child pids. 
		}
    }
}

void waitChild()
{
	
	int currentNum = pidBackG.countPidBG;
	int i, status;
	
	if(currentNum == 0) return; //there is no child process
	
	for(i = 0; i <currentNum; i++){
		
		pid_t childPID = waitpid(pidBackG.pidNumBG[i],&status, WNOHANG);
		if(childPID == 0) { 
		continue;
		}
		else {
			removeBackGPid(childPID);
		}
		
	}
		
}

*******************/

void decryptText(char* ciphertxt, char* key, int fileLeng)
//char* decryptText(char* ciphertxt, char* key, int fileLeng)
{
  
    int i;
   
   for (i=0; i < fileLeng; i++){
	   
	   
	   int chaT = (int) ciphertxt[i];
       int chaK = (int) key[i];

       //back the chracter into integer between 0 and 26.
      chaT = (chaT == 32) ? 26 : (chaT - 65);
      chaK = (chaK == 32) ? 26 : (chaK - 65);
 
       //subtract chaK into chaT and get the modulo
      chaT -= chaK;
      chaT = (chaT+27) % 27; //divide 27 because it has space as well as alphabets.
	                         // before it, it add 27 for the case of negative and all.

       //convert back to ascii number for character
      chaT += 65;
       // if the number is 91, it needs to change space, which ascii num is 32. 
       if(chaT == 91) chaT = 32;

       ciphertxt[i] = (char)chaT; 
	   
  }
  
}

int main(int argc, char *argv[])
{
	//initPidBgs(); //initialize the struct for child process
	struct childBG* childPS = initCBG();
	
    int listenSocketFD, establishedConnectionFD, portNumber;
    socklen_t sizeOfClientInfo;
	ssize_t  charsWritten, charsRead;
	
	//char serverDEC = 'D';
	char clientSignal;
	char serverRes; 
	char buffer[256];
	int fileLen;
    struct sockaddr_in serverAddress, clientAddress;

    if (argc < 2) { fprintf(stderr,"USAGE: %s port\n", argv[0]); exit(1); } // Check usage & args

    // Set up the address struct for this process (the server)
 	memset((char *)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
 	portNumber = atoi(argv[1]); // Get the port number, convert to an integer from a string
 	serverAddress.sin_family = AF_INET; // Create a network-capable socket
 	serverAddress.sin_port = htons(portNumber); // Store the port number
 	serverAddress.sin_addr.s_addr = INADDR_ANY; // Any address is allowed for connection to this process.
        
    // Set up the socket
 	listenSocketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
 	if (listenSocketFD < 0) error("ERROR opening socket");

 	// Enable the socket to begin listening
 	if (bind(listenSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to port
 		error("ERROR on binding");
 	listen(listenSocketFD, 5); // Flip the socket on - it can now receive up to 5 connections
    
    
   while(1) // 1 means true
   {     //waitChild(); //call waitChild function
           waitChild(childPS);
      	// Accept a connection, blocking if one is not available until one connects
      sizeOfClientInfo = sizeof(clientAddress); // Get the size of the address for the client that will connect	
      establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); // Accept
      if (establishedConnectionFD < 0) error("ERROR on accept");

      //create a child process
      pid_t childID = fork();
	  
      if(childID < 0) {
        
               fprintf(stderr, "ERROR: in forking!");
			   exit(1);
	   }
        if (childID == 0){ //childID == 0
               
              memset(buffer, '\0', 256);
			  /**********************************************************************
			  ** First Receive for 'DEC': checking the connection and get the length. 
			  ** And sending response: S or F
			  ***********************************************************************/
			  charsRead = recv(establishedConnectionFD, buffer, 255, 0); // Read the client's message from the socket
              //charsRead = recv(establishedConnectionFD, &clientSignal, sizeof(char), 0); // Read the client's message from the socket
             if (charsRead < 0) error("ERROR reading from socket");
           
		   if(strncmp(buffer, "DEC", 3) != 0) { //first 3 characters of the buffer.
			//if(clientSignal != serverDEC) { 
			     serverRes = 'F';
				 //fprintf(stderr, "the connection is not right");
				 charsRead = send(establishedConnectionFD, &serverRes, sizeof(char), 0);
				  if (charsRead < 0) error("ERROR writing N message to socket.");
				  close(establishedConnectionFD);
				  return 1;

            }
			else {
				serverRes = 'S';
			
			   fileLen = atoi(&buffer[3]); //get file length
            //fileLeng = atoi(&buffer[7]); //fifth index,[4], is the number of file length.

			charsRead = send(establishedConnectionFD, &serverRes, sizeof(char), 0); // Send success back
			 if (charsRead < 0) error("ERROR writing to socket");
			  
			}			 
			 
			//getting the file length
			//charsRead = recv(establishedConnectionFD, &fileLen, sizeof(fileLen), 0);
			//if (charsRead < 0) error("ERROR reading fileLeng from socket.");

			 char* textFromClient = malloc(sizeof(char) * (fileLen+1));
            memset(	textFromClient, '\0', fileLen+1);
			/******************************************************************\
			** Text receive: cipherext
			** After the connection success,it receives text files using while loop
			** Without while loop, it cannot get a big file.
			 ** it keeps checking the current file and the remaining file.
			********************************************************************/

			int currentBf = 0;
			int remainBf = fileLen;

			// Keep track of if data transfer is incomplete
			while (currentBf < fileLen) {
				charsRead = recv(establishedConnectionFD, textFromClient + currentBf, remainBf, 0);
				if (charsRead < 0) {
					fprintf(stderr, "otp_dec_d: ERROR reading from socket");
					exit(1);
				}
				currentBf += charsRead;
				remainBf -= charsRead;
			}
             /***************************************************
			 ** Receive: key message
			 ** It now receives the key file with while loop as well.
			*****************************************************/
			char* keyText = malloc(sizeof(char) * (fileLen +1));
			 memset(keyText, '\0', fileLen +1);
			 
			currentBf = 0;
			remainBf = fileLen;

			// Keep track of if data transfer is incomplete
			while (currentBf < fileLen) {
				charsRead = recv(establishedConnectionFD, keyText + currentBf, remainBf, 0);
				if (charsRead < 0) {
					fprintf(stderr, "otp_dec_d: ERROR reading from socket");
					exit(1);
				}
				currentBf += charsRead;
				remainBf -= charsRead;
			}   
			
			decryptText(textFromClient, keyText, fileLen);
			/****************************************************
			 ** Send: decrypted text
			*****************************************************/
			currentBf = 0;
            remainBf = fileLen;

           
			while (currentBf < fileLen) {
				charsWritten = send(establishedConnectionFD, textFromClient + currentBf, remainBf, 0);
				if (charsWritten < 0) {
					fprintf(stderr, "otp_dec_d: ERROR writing to socket");
					exit(1);
				}
				currentBf += charsWritten;
				remainBf -= charsWritten;
			}


			close(establishedConnectionFD);
            close(listenSocketFD);
            return 0;		                     
			  
		  }
		  
      else{ 
		    //parent
		     
		     int childStatus;
			 //after checking if any child is alive, it stores the child info.
			 pid_t  processID = waitpid(childID, &childStatus, WNOHANG);
			 
			 if(processID == 0) {
				  //pushBackGPid(childID);
				  pushChild(childPS, (int)childID);
			 }
		    close(establishedConnectionFD);
			
	  } 

   }
   freeCBG(childPS);
   //killBGPids(); instead of this, using waitChild() above.
   close(listenSocketFD);
   return  0;
}