/* CS344- Assignment 4 - Week 9/
 * Decoding Program. Takes in 3 command line arguments (coded text, key, socket address for otp_dec_d).
 * Checks to make sure that the key and text are the same length, then sends a combined string of both files
 * to otp_dec_d for decoding. Outputs the decoded message upon receiving it.
 */

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <time.h>


int main(int argc, char *argv[] )
{
    srand(time(0));
    int i; //for looping
    int keylength;//length of key
    int select;//random selection of key character
    keylength =  atoi(argv[1]);//retrive key length from command line arguments
    char alpha[28] = {'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z',' '};
    char *output; //dynamic char array to hold encryption key
    output =  malloc(sizeof (char*) * (keylength + 2));//set dynamic array to hold key string
    for (i = 0; i < keylength; i++)//create a string of random character of length 'keylength' to use as a code key
    {
        select = rand() % 27;//select a random character
        output[i] = alpha[select];
        //TESTING: printf("\n the count is %i, the selection index %i is %c", i, select, alpha[select]);
    }
    //TESTING: printf("\nThe current value of i is %i",i);
    output[i] = '\n';//add string terminating character
    i++;
    output[i] = '\0';//add string terminating character
    printf("%s",output);
    free(output);
    return 0;
}
