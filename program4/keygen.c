/*
* * Assignment4 - OTP: keygen.c
* * CS 344 - 2019 Fall 
* * Taekyoung Kim - Due date: Dec. 6, 2019
* * kimtaeky@oregonstate.edu
* * Description: this program works for OTP client server program.
* * It generates key file that has alphabet characters and space.
* * Also, they are generated randomly using rand() function as many as the given number.
*/

#include <stdlib.h> // malloc(), free(), exit(), execvp(),
#include <stdio.h>  //fprintf(), printf(), stderr, perror()
#include <time.h>  // time() 


// Generates keys
char* generate(int n) {
  
  // Allocate memory for key
  char* key = (char *) malloc(sizeof(char) * n);

  int i;
  for (i = 0; i < n; i++) {
    
	if((rand() % 10) == 8){
	//Originally, tried % 27 with total alphabets but it gave a little weird key files.	 
		 key[i] = ' ';
	}
	else {
	 // Save the characters into array of char.
	 // 26 is total alphabet and 65 => ascii 'A'
		key[i] = (rand() % (26) + 65);
	}
	 
  }
  key[i] = '\0'; //null terminator

  return key;
}

// https://www.tutorialspoint.com/cprogramming/c_command_line_arguments.htm
int main(int argc, char* argv[]) {
	
   //to avoid same number generating.
    srand(time(NULL));
  
  //if there is not enough arguments, give error message
  if (argc < 2) {
     fprintf(stderr,"Needs a key length.\n");
    return 1;
  }

  // Get the length of the key to generate from user input, which is string/characters.
  // https://www.tutorialspoint.com/c_standard_library/c_function_atoi.htm
  int keyLeng = atoi(argv[1])  ;

  // Generate key 
  char* enkey = generate(keyLeng);

  printf("%s\n", enkey); //add newline

  // Free key
  //free(enkey);

  return 0;
}
