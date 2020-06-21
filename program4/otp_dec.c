/*
* * Assignment4 - OTP: otp_dec.c
* * CS 344 - 2019 Fall 
* * Taekyoung Kim - Due date: Dec. 6, 2019
* * kimtaeky@oregonstate.edu
* * Description: this program works OTP client and server.
* * This file read cipher-text file, which has encrypted file info and
* * it sends it to the server, which is otp_dec_d along with key file.
*/

#include <stdio.h> //fprintf(), printf(), stderr, perror()
#include <stdlib.h> // malloc(), free(), exit()
#include <unistd.h> //chdir(), fork(), exec(), pid_t
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
	** For the ciphertext file, which is argv[1]
	** For the key file, which is argv[2]
	** and save the file information and use it later
	*************************************************/
	
	//File information: plaintext & key
	int textFileDC = open(argv[1], O_RDONLY);
	if(textFileDC < 0) { fprintf(stderr, "ERROR: [opt_dec]opening ciphertext file"); exit(1);	}
	int keyFileDC = open(argv[2], O_RDONLY);
	if(keyFileDC < 0) { fprintf(stderr, "ERROR: [opt_dec]opening key file"); exit(1);	}
	
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
		
	char* ciphertextB = malloc(fileLength * sizeof(char*));
	char* keyfileB = malloc(keyLength * sizeof(char*));
		
	if(read(textFileDC, ciphertextB, fileLength) <0) {fprintf(stderr, "Error: couldn't read cipher Text file"); exit(1);}
	if(read(keyFileDC, keyfileB, keyLength) <0) {fprintf(stderr, "Error: couldn't read key Text file"); exit(1);};	
    
	//checking if the text has invalid characters.
	int i;
	for (i = 0; i < fileLength; i++) {
		
		if((ciphertextB[i] < 65 || ciphertextB[i] > 90) && ciphertextB[i] != 32) {	
			fprintf(stderr, "otp_dec error: input contains bad characters\n");
			fflush(stdout);
			exit(1);
		}
	}
	
	 close(textFileDC);
	 close(keyFileDC);
	 
		
    //variables for socket
    int socketFD, portNumber; 
    struct sockaddr_in serverAddress;
    struct hostent*  serverHostInfo;
    ssize_t  charsWritten, charsRead; 
	//https://jameshfisher.com/2017/02/22/ssize_t/	
	
	
    /*********************************************************
	 **Set up the server address struct: from the lecture and client.c
   	***********************************************************/
    memset((char*)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
    portNumber = atoi(argv[3]); // Get the port number, convert to an integer from a string.
    serverAddress.sin_family = AF_INET; //Create a network-capable socket
    serverAddress.sin_port = htons(portNumber); //Store the port number
    serverHostInfo = gethostbyname("localhost"); //Convert the machine name into a special form of address.
    
    if(serverHostInfo == NULL) { fprintf(stderr, "otp_dec:ERROR, no such host\n"); exit(1);}
    memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length); //Copy in the address
    
	//https://www.geeksforgeeks.org/explicitly-assigning-port-number-client-socket/
	

     // Set up the socket
     socketFD = socket(AF_INET, SOCK_STREAM, 0);
     if (socketFD < 0) error("otp_dec: ERROR opening socket.");

     //Connect to opt_enc_d
     if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
	 error("otp_dec: ERROR connecting."); }

      /***********************************************
	  First sending: checking the connection and length!
	  It sends 'DEC' to the server.
	  And get the response, which is S or F.
	 *************************************************/
	 
	 char buffer[256];
	 
	 memset(buffer, '\0', sizeof(buffer));
	 sprintf(buffer, "DEC%d", fileLength); 
	
	charsWritten = send(socketFD, buffer, strlen(buffer), 0); //write buffer, "ENC" to otp_enc_d
     if(charsWritten < 0) error("otp_enc: ERROR writing to socket");
     if(charsWritten < strlen(buffer)) printf("otp_enc: WARNING: Not all data written to socket!\n");
	 
	 /**
	char clientDEC = 'D';		 
     //Send checking message(buffer) to otp_enc_d
     charsWritten = send(socketFD, &clientDEC, sizeof(char), 0); //write buffer, "Done" to otp_enc_d
     if(charsWritten < 0) error("otp_dec: ERROR writing to socket");
     

    //Get return message from otp_enc_d
    //memset(buffer, '\0', sizeof(buffer)); //clear out the buffer again for reuse
     **/
	 
     charsRead = recv(socketFD, &resFromServer, sizeof(char), 0);
	 //charsRead = recv(socketFD, buffer, 39, 0);
     if (charsRead < 0) error("otp_dec: ERROR reading from socket");
	 
	 
	 if(resFromServer != 'S'){
		 close(socketFD);
		 fprintf(stderr, "Error: could not connect otp_dec_d on port %s\n", argv[3]);
         return  2;
		 
	 }
	 
	 //send the file length!
	// charsWritten = send(socketFD, &(fileLeng-1), sizeof(fileLeng), 0);
    // if(charsWritten < 0) error("otp_dec: ERROR writing file length to socket");
	 
	 ciphertextB[fileLength] = '\0'; //From smallsh.c
	 keyfileB[keyLength] = '\0'; //remove newline character,\n 
     /************************************************************************
     ** Sending text: ciphertext
	 ** After checking connection, now send the plaintext message to otp_dec_d
	 ** using a while loop: before using it, error occurred with plaintext4.
      ** It keeps checking the current and remaining file and determines to send.
	  ** https://codereview.stackexchange.com/questions/43914/client-server-implementation-in-c-sending-data-files
	  *************************************************************************/
	 
	  int currentBf = 0;
	  int remainBf = fileLength;
	 
	 while (currentBf < fileLength) {	 
		  charsWritten = send(socketFD, ciphertextB + currentBf, remainBf, 0);
		   if(charsWritten < 0) {
			   fprintf(stderr, "otp_dec: ERROR writing ciphertext file to socket");
			   exit(1);			   
			   
		   }
		 currentBf += charsWritten;
		 remainBf -= charsWritten;
		 
	 }
	 
	  /***************************************************************************
      ** Third sending: keyText
	  ** Now send the key message to otp_enc_d	  
	  ****************************************************************************/
	  
	  currentBf = 0;
	  remainBf = fileLength;
	 
	  while (currentBf < fileLength) {	 
		  charsWritten = send(socketFD, keyfileB + currentBf, remainBf, 0);
		   if(charsWritten < 0) {
			   fprintf(stderr, "otp_dec: ERROR writing key file to socket");
			   exit(1);			   
			   
		   }
		 currentBf += charsWritten;
		 remainBf -= charsWritten;
		 
	 }
      /***************************************************************************
	 **Now receive the decrypted text.
     ** from otp_dec_d
     ****************************************************************************/
	 
	 // Receive the text from otp_dec_d
	 currentBf = 0;
	 remainBf = fileLength;
	
	 char textFromSV[fileLength + 1];
	 memset(textFromSV, '\0', sizeof(textFromSV));
	 

	 while (currentBf < fileLength) { 
		 
		 charsRead = recv(socketFD, textFromSV + currentBf, remainBf, 0); // Read the client's message from the socket
		 if (charsRead < 0) {
			fprintf(stderr, "otp_dec: ERROR reading text from socket");
			exit(1);
		}
		currentBf += charsRead;
		remainBf -= charsRead;
		 
	 }
	  close(socketFD);
	  
	 // Output the decrypted text onto stdout
	 fprintf(stdout, "%s\n", textFromSV);
	 fflush(stdout);
	 free(ciphertextB);
	 free(keyfileB);
	
    
     return 0;

}