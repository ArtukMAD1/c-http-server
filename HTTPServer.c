/*
 *  HTTPServer
 *
 *  Created by Aydin Tarik Artukmac 
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>

#define PORT 8080
#define LINE_MAX 4096 /* maximum number of bytes per line defined*/
#define MAX_METHOD 4
#define MAX_PATH 255



int main(void)
{
    int sockFD, clientConnection, bytes, c, counter;
    char buf[LINE_MAX], method[MAX_METHOD], path[MAX_PATH], filename[256];
    char *method_error = "Error: Unknown Method\r\n";
    char *file_open_error = "Error: Document Not Found\r\n";
    char *forbidden_error = "Error: Forbidden\r\n";
    char character;
    FILE *client_file;


    struct sockaddr_in server;
    struct sockaddr_in client;

    socklen_t client_addr_len = sizeof(client);


    sockFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);		
    if(sockFD < 0){
        fprintf(stderr, "socket creation failed\n");
        exit(EXIT_FAILURE);
    }

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);      
    server.sin_addr.s_addr = INADDR_ANY;

    /*Bind socket*/
    if(bind(sockFD, (struct sockaddr*)&server, sizeof(server)) < 0){
        fprintf(stderr, "bind function has returned non-zero exit value\n");
        exit(EXIT_FAILURE);
    }

    if(listen(sockFD, 5) < 0){
        fprintf(stderr, "listen function has returned non-zero exit value\n");
        exit(EXIT_FAILURE);
    }

    while(1){
        client_addr_len = sizeof(client);
        clientConnection = accept(sockFD, (struct sockaddr*)&client, &client_addr_len);
        if(clientConnection < 0){
            fprintf(stderr, "Accept failed -- %d %d", clientConnection, errno);
            continue;
        }
        
        bytes = read(clientConnection, buf, sizeof(buf) - 1);
        if(bytes < 0){
            fprintf(stderr, "read function has returned less than zero exit value\n");
            close(clientConnection);
            continue;
        }
        if(bytes == 0){
            fprintf(stderr, "client close connection, continue listen\n");
            close(clientConnection);
            continue;
        }



        buf[bytes] = '\0'; 
        if(sscanf(buf, "%3s %254s", method, path) != 2){
            fprintf(stderr, "sscanf value is not 2\n");
            close(clientConnection);
            continue;
        }

        if(strcmp(method, "GET") != 0){
            fprintf(stderr, "Unknown method. Usage: GET /textfile.txt\n");
            write(clientConnection, method_error, strlen(method_error));	/*Compraing if the first message 
				of the user was GET, if not I end connection and restart */ 
            close(clientConnection);
            continue;
        }

        if(path[0] == '/'){
            strcpy(filename, path + 1);
        } else {
            strcpy(filename, path);
        }

        /* Reject paths that could escape the serving folder */
        if (filename[0] == '/' || strstr(filename, "..") != NULL) {
            fprintf(stderr, "rejected unsafe path: %s\n", path);
            write(clientConnection, forbidden_error, strlen(forbidden_error));
            close(clientConnection); // reject unsafe connection trying to reach outside folders
            continue;
        }

        client_file = fopen(filename, "r");
        if(client_file == NULL){
            fprintf(stderr, "requested file cannot be opened or does not exist\n");
            write(clientConnection, file_open_error, strlen(file_open_error));
            close(clientConnection);
            continue;
        }
        counter = 0;
        while((c = fgetc(client_file)) != EOF){	/*Using fgetc to print char by char
			so I can check each character for \n or max line amount of 80 */ 
            character = (char)c;
            if(character == '\n'){
                write(clientConnection, "\r\n", 2);
                counter = 0;
                continue;
            }
            write(clientConnection, &character, 1);
            counter++;
            if(counter == 80){
                write(clientConnection, "\r\n", 2);
                counter = 0;
            }

        }
        if(counter > 0){
            write(clientConnection, "\r\n", 2);
        }
        printf("File contents sent to user succesfully!\n");

        fclose(client_file);
        close(clientConnection);
    }
    
    close(sockFD);
    exit(EXIT_SUCCESS);
}
