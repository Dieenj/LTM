#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(int argc, char *argv[]){

	int clientSocket, ret;
	struct sockaddr_in serverAddr;
	char buffer[1024];

	if(argc!=3){
		printf("Wrong number of parameters!\n");
		exit(EXIT_FAILURE);
	}
	clientSocket = socket(AF_INET, SOCK_STREAM, 0);
	if(clientSocket < 0){
		printf("[-]Error in connection.\n");
		exit(1);
	}
	printf("[+]Client Socket is created.\n");

	memset(&serverAddr, '\0', sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(atoi(argv[2]));
	serverAddr.sin_addr.s_addr = inet_addr(argv[1]);

	ret = connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
	if(ret < 0){
		printf("[-]Error in connection.\n");
		exit(1);
	}
	printf("[+]Connected to Server.\n");

	// helper lambda-like functions implemented inline
	char linebuf[2048];
	while(1){
		printf("\nInsert string: ");
		if(!fgets(linebuf, sizeof(linebuf), stdin)) break;
		// if user only pressed Enter -> exit loop
		if(strcmp(linebuf, "\n") == 0) {
			printf("Goodbye.\n");
			break;
		}
		size_t tosend = strlen(linebuf);
		// ensure newline-terminated; fgets keeps the newline
		if(tosend == 0) continue;
		ssize_t s = send(clientSocket, linebuf, tosend, 0);
		if(s <= 0){
			perror("send");
			break;
		}

		// read first response line (digits or 'Error')
		char respbuf[4096];
		size_t resp_len = 0;
		int got_newline = 0;
		while(!got_newline){
			ssize_t r = recv(clientSocket, respbuf + resp_len, 1, 0);
			if(r <= 0){
				perror("recv");
				goto finish;
			}
			if(respbuf[resp_len] == '\n'){
				respbuf[resp_len] = '\0';
				got_newline = 1;
				break;
			}
			resp_len += r;
			if(resp_len >= sizeof(respbuf)-1){
				respbuf[resp_len] = '\0';
				break;
			}
		}
		// print first line
		printf("%s\n", respbuf);

		// If the server sent "Error", it sends a single line. We treat any response equal to "Error" as terminal.
		if(strcmp(respbuf, "Error") == 0){
			continue;
		}

		// read second line (letters)
		resp_len = 0;
		got_newline = 0;
		while(!got_newline){
			ssize_t r = recv(clientSocket, respbuf + resp_len, 1, 0);
			if(r <= 0){
				perror("recv");
				goto finish;
			}
			if(respbuf[resp_len] == '\n'){
				respbuf[resp_len] = '\0';
				got_newline = 1;
				break;
			}
			resp_len += r;
			if(resp_len >= sizeof(respbuf)-1){
				respbuf[resp_len] = '\0';
				break;
			}
		}
		printf("%s\n", respbuf);
	}
finish:
	close(clientSocket);
	return 0;

	return 0;
}
