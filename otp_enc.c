
/* CS344- Assignment 4 - Week 9/
 * encoding Program. Takes in 3 command line arguments (normal text, key, socket address for otp_enc_d).
 * Checks to make sure that the key and text are the same length, then sends a combined string of both files
 * to otp_enc_d for encoding. Outputs the encoded message upon receiving it.
 */

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

int compare_char_count(FILE *f1, FILE *f2);//compare the character count of two files and return '0' if they are the same
int char_count; //holds number of characters that need to be encoded
void insert_file_array(FILE *f1, FILE *f2, char *arr);//inserts the file's text into an array, files are separated by the digit '8'
int valid_char(char input);//return zero if the character is a letter, space, or the number 8
int main(int argc, char *argv[])
{
    int i,j,k,index;//for looping
    FILE *text_file = fopen(argv[1],"r");//holds file to get initial text from
    if (text_file == NULL)
    {
        printf("Could not find %s",argv[1]);
        perror("Error: Bad file input");
        exit(1);
    }
    FILE *key_file = fopen(argv[2],"r"); //holds file to get key from

    if((compare_char_count(text_file,key_file)) == 1) //make sure that the text to encrypt and the key match in length
    {
        printf("\nThe target file size is larger than the key length\n");
        exit(1);
    }
    //reset file stream to the start of the files
    fclose(text_file);
    fclose(key_file);
    text_file = fopen(argv[1],"r");
    key_file = fopen(argv[2],"r");

    char *output; //dynamic char array to hold input text and key content
    output =  malloc(sizeof (char*) * ((char_count * 2)) + 4 );//set dynamic array, leave space for '\0' as well as an '!' to mark the end of output when streaming
    insert_file_array(text_file, key_file, output);//copy all file content into char array
    fclose(text_file);
    fclose(key_file);
    int output_size = (char_count * 2) + 4; //size of data output to otp_enc_d.c

    int port_num;//holds the port number
    port_num = atoi(argv[3]);//retrive port number from command line arguments
    int socket_fd; //file descriptor for socket
    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("Socket Err client");
        exit(1);
    }

    struct sockaddr_in encode;//set up socket connection to otp_enc_d
    encode.sin_family = AF_INET;
    encode.sin_port = htons(port_num);
    encode.sin_addr.s_addr = INADDR_ANY;

    if (connect(socket_fd,(struct sockaddr *) &encode, sizeof(encode)) == -1)//connect to otp_enc_d
    {
        perror("Connect Err Client: opt_enc_d.c");
        exit(1);
    }
    int text;
    text = write(socket_fd,output,output_size);//send string to otp_enc_d with both plain text and key characters contained within

    free(output);
    char *receive;
    receive =  malloc(sizeof (char*) * char_count + 5 );
    text = read(socket_fd,receive,char_count+2);//receive back encoded message from otp_enc_d
    printf("%s", receive);
    close(text);
    free(receive);
    return 0;
}


int compare_char_count(FILE *f1, FILE *f2)
{
    int count1,count2;//holds char counts
    int comparison;//holds results of comparison. 0'0' means that they are equal
    int i; //for looping
    int c; //used to extract chars,
    char text_end_check;//checks for end of text in file
    count1 = 0;//start count at zero for the first file
    i = 0;
    while((c = fgetc(f1)) != EOF)//count the number of non NULL or endline chars. Some syntax borrowed from  syntax from http://stackoverflow.com/questions/4823177/reading-a-file-character-by-character-in-c
    {
        text_end_check = (char)c;
        if ((text_end_check != '\0') && (text_end_check != '\n'))
        {
            i++;
        }
    }
    count1 = i; //put char count of first file into count1
    i = 0;
    count2 = 0;//start count at zero for the second file
    while((c = fgetc(f2)) != EOF)//count the number of non NULL or endline chars. Some syntax borrowed from  syntax from http://stackoverflow.com/questions/4823177/reading-a-file-character-by-character-in-c
    {
        text_end_check = (char)c;
        if ((text_end_check != '\0') && (text_end_check != '\n'))
        {
            i++;
        }
    }
    count2 = i; //put char count of second file into count2
    comparison = 1;
    if (count1 <= count2) //if the counts are the same, flag the comparison as '0'
    {
        comparison = 0;
        char_count = count1;//set the char count to the file size
    }
    return comparison;
}

void insert_file_array(FILE *f1, FILE *f2, char *arr)
{
    int i,j; //for looping
    int c; //used to extract chars,
    char text_end_check;//checks for end of text in file
    i = 0;
    while((c = fgetc(f1)) != EOF)//extract all non null or endline chars from file into array
    {
        text_end_check = (char)c;
        if ((text_end_check != '\0') && (text_end_check != '\n'))
        {
            if (valid_char(text_end_check) == 1)
            {
                perror("\nInvalid Character in input file");
                exit(1);
            }
            arr[i] = text_end_check;
            i++;
        }
    }
    arr[i] = '8'; //mark the delimiting point of the text and key with an 8
    i++;
    j = 0;
    while(((c = fgetc(f2)) != EOF) && j < char_count)//extract all non null or endline chars from file into array
    {
        text_end_check = (char)c;
        if ((text_end_check != '\0') && (text_end_check != '\n'))
        {
            arr[i] = text_end_check;
            i++;
            j++;
        }
    }
    arr[i] = '!'; //mark the end of the text
    arr[i+1] = '\0';
    return;
}

int valid_char(char input)
{
    int i;
    int valid = 1;
    char alpha[28] = {'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z',' ','8'};
    for (i = 0; i < 28; i++)
    {
        if(alpha[i] == input)
        {
            valid = 0;
        }
    }
    return valid;
}
