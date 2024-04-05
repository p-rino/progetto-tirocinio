#include "common.h"
#include <arpa/inet.h>  
#include <netinet/in.h> 
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>


int main(int argc, char* argv[]){
    int ret;

    int socket_desc;
    struct sockaddr_in server_addr = {0};

    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if( socket_desc < 0 ){
        //handle error 
    }

    server_addr.sin_addr.s_addr = inet_addr(SERVER_ADDRESS);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);


    ret = connect(socket_desc , (struct sockaddr*) &server_addr , sizeof(struct sockaddr_in));
    if(ret < 0){
        //handleerror
    }
    char quit_comm[]="quit";
    size_t lun_q = strlen(quit_comm);
    

    while(1){    
        
        int recv_bytes;

        //ricevi e stampa messaggio
        char recv_buf[1000];
        size_t buf_len = sizeof(recv_buf);

        memset(recv_buf , 0 , buf_len );
        
        int bytes_recv = 0;
        while ( (recv_bytes = recv(socket_desc, recv_buf + bytes_recv , buf_len - 1 , 0)) < 0 ) {
            bytes_recv += recv_bytes;
        }
        size_t lunghezza = bytes_recv -1 ;
        //recv_buf[lunghezza]='\0';
        printf("---------------------------------------------------------------\n\n");
        printf("%s",recv_buf);
        printf("\n");
        printf("\n");
        printf("---------------------------------------------------------------\n");

        
        //manda risposta
        char input[20];
        memset(input , 0 , strlen(input));
        printf("\n");
        printf("inserisci un input: ");
        scanf( "%s" , input);
        //gets(input);
        //printf("input arrivato: %s\n",input);
        size_t in_len = strlen(input);

        if( !memcmp(input , quit_comm, lun_q) ){
            //manda prima il segnale al server
            break;
        }
     
        int bytes_sent = 0 ;
        while ( (ret = send(socket_desc, input + bytes_sent, in_len, 0)) < 0) {
        bytes_sent += ret ;
        }
    
        printf("\n");
    }

    ret = close(socket_desc);
    if(ret < 0){
        //handle error
    }

}





