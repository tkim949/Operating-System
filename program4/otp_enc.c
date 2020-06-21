/*
* * Assignment4 - OTP: otp_enc.c
* * CS 344 - 2019 Fall 
* * Taekyoung Kim - Due date: Dec. 6, 2019
* * kimtaeky@oregonstate.edu
* * Description: this program works OTP client and server
* * And this file read plaintext file and key file and send it to the server, which is otp_enc_d
*/

#include <stdio.h> //fprintf(), printf(), stderr, perror()
#include <stdlib.h> // malloc(), free(), exit()
#include <unistd.h> //chdir(), fork(), exec(), pid_t ssize_t
#include <string.h> // strcmp(), strtok()
#include <sys/types.h> //waitpid()
#include <sys/socket.h> //sa_family_t
#include <netinet/in.h> //in_addr_t, sa_family_t, in_port_t
#include <netdb.h> //h_addrtype, h_length,struct hostent*
#include <fcntl.h> //O_RDONLY

//From client.c in lecture, Error function used for reporting issues.
void error(const char *msg) { perror(msg);  exit(1);} 

int main(int argc, char *argv[])
{
	 //check if it has proper number of arguments.
    if (argc < 4) { fprintf(stderr, "USAGE: %s plaintext key prt\n", argv[0]); exit(1);}
	
	//char buffer[256];
	char resFromServer;
	int fileLength; 
	int keyLength;
	
	/***********************************************
	** For the plaintext file, which is argv[1]
	**For the key file, which is argv[2]
	** and save the file information and use it later
	**********************************************/
	
	//File information: plaintext & key
	int textFileDC = open(argv[1], O_RDONLY);
	if(textFileDC < 0) { fprintf(stderr, "ERROR: opening plaintext file"); exit(1);	}
	int keyFileDC = open(argv[2], O_RDONLY);
	if(keyFileDC < 0) { fprintf(stderr, "ERROR: opening key file"); exit(1);	}
	
	
	/****************************************************************
	** Get the file length
	** https://stackoverflow.com/questions/238603/how-can-i-get-a-files-size-in-c


	****************************************************************/
	struct stat textF, keyF;
	if(stat(argv[1], &textF) < 0) { fprintf(stderr, "ERROR: using struct stat for plaintext file."); exit(1); }
	if(stat(argv[2], &keyF) < 0) { fprintf(stderr, "ERROR: using struct stat for key file."); exit(1); }
	//check if key file size is enough!
	if(keyF.st_size < textF.st_size) { fprintf(stderr, "Error: key '%s' is too short\n", argv[2]); exit(1); }
	else{
	     fileLength = textF.st_size - 1; //the null character https://linux.die.net/man/2/stat
		 keyLength = keyF.st_size - 1;
	  }
	    //strlen() give the length without the null terminator, but st_size gives the length without terminating it.
	
	char* plaintextB	= malloc(fileLength * sizeof(char*));
	char* keyfileB = malloc(keyLength * sizeof(char*));
	
		
	if(read(textFileDC, plaintextB, fileLength)<0){fprintf(stderr, "Error: couldn't read plain Text file"); exit(1);}
	if(read(keyFileDC, keyfileB, keyLength)<0){fprintf(stderr, "Error: couldn't read key Text file"); exit(1);}	
	
	//checking if the text has invalid characters.
	int i;
	for (i = 0; i < fileLength; i++) {
		
		if((plaintextB[i] < 65 || plaintextB[i] > 90) && plaintextB[i] != 32) {	
			fprintf(stderr, "otp_enc error: input contains bad characters\n");
			//free(plaintextB);
			exit(1);
		}
	}
	
	close(textFileDC);
	close(keyFileDC);
	 
	//variables for socket (From the lecture and client.c file)
    int socketFD, portNumber; // charsWritten, charsRead;
    struct sockaddr_in serverAddress;
    struct hostent*  serverHostInfo;
    //https://jameshfisher.com/2017/02/22/ssize_t/	
	ssize_t charsWritten, charsRead;
	
	     
    /*********************************************************
	 **Set up the server address struct: from the lecture and client.c
   	***********************************************************/
	memset((char*)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
    portNumber = atoi(argv[3]); // Get the port number, convert to an integer from a string.
    serverAddress.sin_family = AF_INET; //Create a network-capable socket
    serverAddress.sin_port = htons(portNumber); //Store the port number
    serverHostInfo = gethostbyname("localhost"); //Convert the machine name into a special form of address.
    
    if(serverHostInfo == NULL) { fprintf(stderr, "otp_enc:ERROR, no such host\n"); exit(1);}
    memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length); //Copy in the address
    
	//https://www.geeksforgeeks.org/explicitly-assigning-port-number-client-socket/
	
     
     // Set up the socket
     socketFD = socket(AF_INET, SOCK_STREAM, 0);
     if (socketFD < 0) error("otp_enc: ERROR opening socket.");
     //Connect to opt_enc_d
     if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0)
            error("otp_enc: ERROR connecting.");
     //Send checking message(buffer) to otp_enc_d
	 
	 /***********************************************
	  First sending: checking the connection and length!
	  It sends 'ENC' to the server.
	  And get the response, which is Y or N
	 *************************************************/
	 //char clientENC = 'E';
	 char buffer[256];
	 //fileLength = checkPlaintext -1;
	 memset(buffer, '\0', sizeof(buffer));
	 sprintf(buffer, "ENC%d", fileLength);
	//https://www.geeksforgeeks.org/sprintf-in-c/ sprintf store output on char buffer
    
	charsWritten = send(socketFD, buffer, strlen(buffer), 0); //write buffer, "ENC" to otp_enc_d
     if(charsWritten < 0) error("otp_enc: ERROR writing to socket");
     if(charsWritten < strlen(buffer)) printf("otp_enc: WARNING: Not all data written to socket!\n");

	 
     // charsWritten = send(socketFD, &clientENC, sizeof(char), 0); 
     //  if(charsWritten < 0) error("otp_enc: ERROR writing to socket");
     
     //Get return message from otp_enc_d
     //memset(buffer, '\0', sizeof(buffer)); //clear out the buffer again for reuse
     charsRead = recv(socketFD, &resFromServer, sizeof(char), 0);
	 //charsRead = recv(socketFD, buffer, strlen(buffer), 0);
     if (charsRead < 0) error("otp_enc: ERROR reading from socket");
	 
	 if(resFromServer != 'Y'){
		 close(socketFD);
		 fprintf(stderr, "Error: could not connect otp_enc_d on port %s\n", argv[3]);
         return  2;
		 
	 }
	
	 plaintextB[fileLength] = '\0'; //From smallsh.c
	 keyfileB[keyLength] = '\0'; //remove \n 
	 
	 //sending a file length info.
	 //charsWritten = send(socketFD, &fileLeng, sizeof(fileLeng), 0);
     //if(charsWritten < 0) error("otp_enc: ERROR writing file length to socket");
	 /************************************************************************
     ** Sending text: plaintext
	 ** After checking connection, now send the plaintext message to otp_enc_d
	 ** using a while loop: before using it, error occurred with plaintext4.
	 ** https://codereview.stackexchange.com/questions/43914/client-server-implementation-in-c-sending-data-files
	 *************************************************************************/
	 
	 int currentBf = 0;
	 int remainBf = fileLength;
	 
	  while (currentBf < fileLength) {
		 
		  charsWritten = send(socketFD, plaintextB + currentBf, remainBf, 0);
		   if(charsWritten < 0) {
			   fprintf(stderr, "otp_enc: ERROR writing plaintext file to socket");
			   exit(1);			   
			   
		   }
		 currentBf += charsWritten;
		 remainBf -= charsWritten;
		 
	 }
	  
	 /***************************************************************************
      ** Sending text: keyText
	  ** Now send the key message to otp_enc_d	  
	  ****************************************************************************/
     currentBf = 0;
	 remainBf = fileLength;
	 
	 while (currentBf < fileLength) {
		 
		  charsWritten = send(socketFD, keyfileB + currentBf, remainBf, 0);
		   if(charsWritten < 0) {
			   fprintf(stderr, "otp_enc: ERROR writing key file to socket");
			   exit(1);			   
			   
		   }
		 currentBf += charsWritten;
		 remainBf -= charsWritten;
		 
	 }
	  /*************************************************************
	 ** Receive the text from otp_enc_d
	 ** it also uses while loop
	 ***************************************************************/
	 currentBf = 0;
	 char textFromSV[fileLength+1];
	 memset(textFromSV, '\0', sizeof(textFromSV));
	 
	 remainBf = fileLength;
	 
	 while (currentBf < fileLength) {
		 
		 charsRead = recv(socketFD, textFromSV + currentBf, remainBf, 0); // Read the client's message from the socket
		 if (charsRead < 0) {
			fprintf(stderr, "otp_enc: ERROR reading text from socket");
			exit(1);
		}
		currentBf += charsRead;
		remainBf -= charsRead;
		 
	 }
	 
	 close(socketFD);
	 

	 //output the text file that is encrypted one.
	 fprintf(stdout, "%s\n", textFromSV);
	 fflush(stdout);

	 free(plaintextB);
	 free(keyfileB);
	 
     return 0;

}