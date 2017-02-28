/* CS344- Assignment 4 - Week 9/
 * Deamon encoding Program. Receives in a string of plain text and a key,
 * splits the string into its component parts, encodes the plain text,
 * and sends the results back to otp_enc
 */

#include <sys/wait.h>
#include <sys/resource.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>

typedef struct dynamic_
{
    char *text;
    int size;
}dynamic;

void resize_arr (dynamic *arr);//resizes dynamic array
int valid_char(char input);//return zero if the character is a letter, space, or the number 8
void split_text(char *chunktext, char *plaintext, char *keytext);//splits the streamed in text into its plain text and key
void encode(char *encrypt, char *plaintext, char *keytext);//creates an encryption of the user message via the key
int get_letter_number(char input);//returns the number of the character (ie A =1, C =3, etc);
void start_fork(int client_text);//main code to process an input stream, is called only after a fork

int main(int argc, char *argv[] )
{
    int client_text;
    int socket_fd; //file descriptor for socket
    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)//set up socket
    {
        perror("Socket Err server");
        exit(1);
    }
    int optval = 1;
    setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int));
    int port_num;//holds the port number
    port_num = atoi(argv[1]);//get port number from command line arguments
    //printf("The port number is %i",port_num);

    struct sockaddr_in server;//set up the socket 'server' on user specified address
    server.sin_family = AF_INET;
    server.sin_port = htons(port_num);
    server.sin_addr.s_addr = INADDR_ANY;
   // memcpy(&server.sin_addr, OSU_ip->h_addr, OSU_ip->h_length);

    if (bind(socket_fd,(struct sockaddr *) &server, sizeof(server)) == -1) //bind the socket
    {
        perror("Bind Err server");
    }

    listen(socket_fd, 5);//listen for input from otp_enc

    pid_t childpid; //for new process

    while (1)
    {

            client_text = accept(socket_fd,NULL,NULL);//accept any input on the socket
            childpid = fork(); //split current process
            //multi-access socket syntax adapted from http://www.linuxhowtos.org/C_C++/socket.htm
            if (childpid ==0)//inside child
            {
                close(socket_fd);//close the socket
                start_fork(client_text);//process the streamed in data
                exit(0);
            }
            else
            {
                close(client_text);
                signal(SIGCHLD,SIG_IGN);//zombie cleanup code from http://www.linuxhowtos.org/C_C++/socket.htm
            }

    }

            //exit(0);
    return 0;
}

void start_fork(int client_text)
{
    int i,j,k; //for looping
    int index;//for resizing dynamic array
    int endpoint = 0;
    int result;
    char receive[11];//gets text from socket
    char check_end;//check for a '!' in the string, signifying that no more data is coming
    char *plaintext;//holds the plain text
    char *key;//holds the key text
    char *encrypt;//holds the encrypted text

    dynamic *text_arr;
    text_arr = malloc (sizeof (dynamic*));
    text_arr->text =  malloc (sizeof (char*) * 20);
    text_arr->size = 20;
    j = 0;
    index = 0;

    while(endpoint == 0)//while the end of the streaming data (marked with '!') has not yet been reached
            {

                result = recv(client_text,receive, 10,0);
                for(i = 0; i < 11; i++)//check to see if the end of received data has been reached by scanning for '!'
                {
                    check_end = receive[i];
                    if (check_end == '!')
                    {
                        endpoint = 1;
                    }
                }

                for(i = 0; i < 11; i++)
                {   if (valid_char(receive[i]) == 0) //copy over valid chars streamed into the program in text_arr
                    {text_arr->text[index] = receive[i];
                        index++;
                        if (index >= text_arr->size)
                        {
                            resize_arr(text_arr);
                        }
                    }
                    else if(receive[i] == '!')//break copying over characters when the end char '!' is reached
                    {
                        index++;
                        if (index >= text_arr->size)
                        {
                            resize_arr(text_arr);
                        }
                        i = 11;
                    }
                    else if(receive[i] == '+')//if the text is already encrypted, shut down the program
                    {
                        perror("\notp_dec cannot find otp_dec_d");
                        encrypt = malloc (sizeof (char*) * 1);
                        encrypt[0] = '\0';
                        result = write(client_text,encrypt, 1);//return null text to otp_dec
                        free(encrypt);
                        exit(1);
                    }
                }

            }
            text_arr->text[index] = '\0';
            int sub_text_arr_size = (text_arr->size / 2) + 2;//size to split plaintext and key text arrays into
            //set up dyanmic arrays to contained streamed in data
            plaintext = malloc (sizeof (char*) * sub_text_arr_size);
            key = malloc (sizeof (char*) * sub_text_arr_size);
            encrypt = malloc (sizeof (char*) * sub_text_arr_size);
            split_text(text_arr->text,plaintext, key);//split the streamed in data into its component parts
            encode(encrypt,plaintext,key);//encode input
            result = write(client_text,encrypt, sub_text_arr_size);//return encoded text to otp_enc
            free(key);
            free(plaintext);
            free(encrypt);
            free(text_arr->text);
            free(text_arr);
            close(result);
    return;
}


void encode(char *encrypt, char *plaintext, char *keytext)
{
    int end;
    char alpha[27] = {'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z',' '};
    int reg_num, key_num, result;//numeric values for text chars
    int i, j;
    i = 0;
    while(plaintext[i] != '\0')
    {
        i++;
    }
    end = i;//set the end point for future loops
    for(i = 0; i < end; i++)
    {

        reg_num = get_letter_number(plaintext[i]) + 1;
        key_num = get_letter_number(keytext[i]) + 1;

        result = reg_num + key_num;

        if (result > 27)
        {
            result = result - 27;
        }
        encrypt[i] = alpha[result -1];
    }
    encrypt[end] = '\n';
    encrypt[end+1] = '\0';
}

void split_text(char *chunktext, char *plaintext, char *keytext)
{
    char selection;//grabs a char at a time from chuncktext
    selection = '!';
    int i,j;
    i = 0;
    j = 0;
    while(selection != '8')//loop until midpoint
    {
        selection = chunktext[i];
        if(selection != '8')
        {
            plaintext[j] = selection;
            j++;
        }
        i++;
    }
    plaintext[j] = '\0';
    j = 0;
    while(selection != '\0')//put remaining text into the key text
    {
        selection = chunktext[i];
        if(selection != '\0')
        {
            keytext[j] = selection;
            j++;
        }
        i++;
    }
    keytext[j] = '\0';
    //free(chunktext);
}

int get_letter_number(char input)
{
    char alpha[27] = {'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z',' '};
    int i;
    i = 0;
    while (input != alpha[i])
    {
        i++;
    }
    return i;
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
void resize_arr (dynamic *arr)
{
    int i;
    char *upper;
    upper = malloc (sizeof (char*) * (arr->size *2));
    for (i = 0; i < arr->size; i++)
    {
        upper[i] = arr->text[i];
    };
    free(arr->text);
    arr->text = upper;
    arr->size = arr->size * 2;
}
