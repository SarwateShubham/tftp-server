#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <inttypes.h>
#include <signal.h>
#include <time.h>


#include <unistd.h>
#include <sys/wait.h>


#define MAX_BUFFER 100
int count_clients=0;
/*
 * Main function
 */
void sigchild_handler(int s)
{
	while(waitpid(-1, NULL, WNOHANG) > 0);
	count_clients--;
	printf("Client disconnected!!");
}

// unsigned short int getRandPort(){
//     srand(time(NULL));
//     unsigned short int range = rand()%2 + 2;
//     srand(time(NULL));
//     //unsigned short int port = (rand()%1000) + (range*10000);
    
//     //unsigned short int port = rand()%16382 + 49152;
//     unsigned short int port = rand()%100 + 5000;
//     return port;
// }


FILE *find_file(char *buffer_message)
{
	FILE *fp ;
	char *mode = "rb";
	fp = fopen ( buffer_message+2, mode);
	//printf("Rechead here\n" );
	return fp;
}

int generate_socket(char* PORT,int count,struct addrinfo **i,int first){
	//char* PORT = (char*)argv[1]; 
	int socket_fd, accepted_fd; 
	struct addrinfo info_server, *result;
	struct sockaddr_storage client_addr; 
	socklen_t sin_size;
	int flag=1;
	int get_addr_res;
	char pport[5] = "";
	if (first != 1)
        sprintf(pport,"%d", 0);  // random port
    else
      	sprintf(pport,"%d", atoi(PORT));
	
	

	//Basic Set-up code.
	memset(&info_server, 0, sizeof info_server);
	info_server.ai_family = AF_UNSPEC;			// Compatible with both IPv4 and IPv6
	info_server.ai_socktype = SOCK_DGRAM;
	info_server.ai_flags = AI_PASSIVE; 

	//if ((get_addr_res = getaddrinfo(NULL, (const char *)pport, &info_server, &result)) != 0) {
	if ((get_addr_res = getaddrinfo(NULL, (const char *)pport, &info_server, &result)) != 0){
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(get_addr_res));
		return 1;
	}

	for( *i = result; *i != NULL; *i = (*i)->ai_next ) {
		if ((socket_fd = socket((*i)->ai_family, (*i)->ai_socktype,
				(*i)->ai_protocol)) == -1) {
			perror("server: socket");
			continue;
		}

		if (bind(socket_fd, (*i)->ai_addr, (*i)->ai_addrlen) == -1) {
			close(socket_fd);
			perror("server: bind");
			continue;
		}

		break;
	}

	freeaddrinfo(result);
	//printf("Bound socket: %d, Port : %d",socket_fd, Port);
	fflush(stdout);
	return socket_fd;
	//Set-up code--------------
}

void send_fnf_error(int socket_fd,struct addrinfo *i,struct sockaddr_storage client_addr)
{
	char *send_error;
	uint16_t error_opcode,error_code;
	uint8_t gap;
	//struct sockaddr_storage client_addr;
	int numbytes;
	gap = htons(0);
	send_error = malloc(200);
	char error_message[14] = "file not found";
	if (errno == ENOENT){
		printf("File not found\n");
		error_opcode = htons(5);
		error_code = htons(1);
		memcpy(send_error, (const char *)&error_opcode, sizeof(uint16_t));
		printf("Error after this\n");
		memcpy(send_error+sizeof(uint16_t), (const char *)&error_code, sizeof(uint16_t));
		memcpy(send_error+(2*sizeof(uint16_t)), (const char *)error_message, sizeof(error_message));
		memcpy(send_error+(2*sizeof(uint16_t))+sizeof(error_message), (const char *)&gap, sizeof(uint8_t));
		// /printf("Error after this\n");
		if ((numbytes = sendto(socket_fd, send_error, 19, 0, (struct sockaddr *)&client_addr, i->ai_addrlen)) == -1)
			perror("Error in sending fnf");
			else
			printf("successfully sent error packet\n");
		}
}


int main(int argc, char *argv[]) {
  
  
  socklen_t addr_len; 
  uint16_t opcode,blockno;  
  int count = 0;
  struct sigaction sig_action;

  char *message_pointer, *send_error;  
  char buffer_message[MAX_BUFFER];  
  struct sockaddr_storage client_address;  

  struct addrinfo *i;  
  int socket_fd, new_socket_fd;  
  int numbytes;  
  int timeout_count = 0;

  //Sign Action handlers
	sig_action.sa_handler = sigchild_handler; // reap all dead processes
	sigemptyset(&sig_action.sa_mask);
	sig_action.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sig_action, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}


  if (argc != 3) {
    printf("usage: <serverIP> <port>\n");
    exit(1);
  }
  socket_fd = generate_socket(argv[2],count,&i,1);

  while(1) {
    printf("Hmm Waiting for new clients Hmm\n");
    addr_len = sizeof client_address;
    if ((numbytes = recvfrom(socket_fd, buffer_message, MAX_BUFFER-1 , 0, (struct sockaddr *)&client_address, &addr_len)) == -1) {
      perror("fail to receive 1st packet");
      exit(2);
    }
    if ( *(buffer_message+1) == 1) { //RRQ request
      count = count + 1;
      FILE *fp ;
      fp = find_file(buffer_message);
     
      if ( fp == NULL ) {
       
        if (errno == ENOENT) {
        	send_fnf_error(socket_fd,i,client_address);
          //exit(2);
        }
      } else {
        
        
        if (fork() == 0) {  

          new_socket_fd = generate_socket(argv[2],count,&i,0);

          opcode =htons(3);
          message_pointer = malloc(516);
          memcpy(message_pointer, (const char *)&opcode, sizeof(uint16_t));
          int x,y;
          int data = 0;
          y = 0;
          long file_size;
          fseek(fp, 0L, SEEK_END);  
          file_size = ftell(fp);  
          rewind(fp);  
          clearerr(fp);  
          printf("file size is : %ld\n", file_size);
          
          while( !feof(fp) && !ferror(fp)) {
            unsigned char message_container[512] = "";
            y=(y+1)%65536;
            for( x=0; x<=511; x++) {
              if (( data = fgetc(fp) ) == EOF) {
                break;
              } else {
                message_container[x] =(unsigned char) data;
              }
            }
            //if(y>65536)
            	//y = y%65536;
            blockno = htons(y); 
            memcpy(message_pointer+sizeof(uint16_t), (const char *)&blockno, sizeof(uint16_t));
            memcpy(message_pointer+(2*sizeof(uint16_t)), (const unsigned char *)message_container, sizeof(message_container));
            if ((numbytes = sendto(new_socket_fd, message_pointer, x+4, 0, (struct sockaddr *)&client_address, i->ai_addrlen)) == -1) {
              perror("Message Send error");
              exit(4);
            }
            printf("Sent %d bytes\n", numbytes);
            printf(" Waiting for ACK message\n");
            
            while(1) {
              struct timeval tv;  
              fd_set readfds;
              tv.tv_sec = 10;// Timeout of 10 seconds.
              tv.tv_usec = 1000000;
              FD_ZERO(&readfds);  
              FD_SET(new_socket_fd, &readfds);  
              select(new_socket_fd+1, &readfds, NULL, NULL, &tv);  
              if (FD_ISSET(new_socket_fd, &readfds)) {  
                if ((numbytes = recvfrom(new_socket_fd, buffer_message, MAX_BUFFER-1 , 0, (struct sockaddr *)&client_address, &addr_len)) == -1) {
                  perror("recvfrom");
                  exit(5);
                }
                uint16_t block_number;  
                memcpy(&block_number,buffer_message+2, sizeof(uint16_t));

                printf("The Opcode %d\n has block number : %d\nj: %d\n", *(buffer_message+1), ntohs(block_number), y);
                
                if( *(buffer_message+1) == 4 && ntohs(block_number)==y) {
                  break;
                } else {
                  
                  if ((numbytes = sendto(new_socket_fd, message_pointer, x+4, 0, (struct sockaddr *)&client_address, i->ai_addrlen)) == -1) {
                    perror("Error retr");
                    exit(6);
                  }
                }
              } else {  
                printf("Timed out retransmitting!!!\n");
                timeout_count++;
                if(timeout_count == 10){
                	//break;
                	printf("Client exceeded number of retransmissions!! Disconnecting client!!\n");
                	exit(7);
                	
                }
                if ((numbytes = sendto(new_socket_fd, message_pointer, x+4, 0, (struct sockaddr *)&client_address, i->ai_addrlen)) == -1) {
                  perror("Error retr");
                  exit(8);
                }
              }
            } 
          }
          close(new_socket_fd);  
          if(feof(fp)){
            printf("EOF\n");
          }
          if(ferror(fp))
            printf("Read Error\n");
        }
        fclose(fp);
      }  
    }

     
  }  
return 0;
}