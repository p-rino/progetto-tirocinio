#include <sys/socket.h>
#include <netinet/in.h>
#include "common.h"  //mio file per avere informazioni in comune
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>  

//enum per distinguere in che stato del menu ci troviamo
enum stati_menu { menu , tratte , postidisp , codicelim };

//input = descrittore del client e messaggio da inviare
void manda_mess(int client_desc , char mess[]){
    int ret;
    size_t msg_l = strlen(mess);

    int bytes_sent = 0;
    while ( (ret = send( client_desc, mess + bytes_sent, msg_l, 0 )) < 0 ){
        bytes_sent += ret;
        //gestisci errori
    }
}

void connection_handler(int client_desc){
    int ret;
    int comm; //comandi per navigazione menu
    char sen[400]; //usato per inviare messaggi
    enum stati_menu stato = menu; //stato iniziale = menu

    printf("server connesso a client: %d\n",client_desc);

    int byte_ric, sent_bytes; //variabili per send e recv

    char quit_comm[]="quit";      //comando quit e lunghezza
    size_t lun_q = strlen(quit_comm);

    //PER ORA stringhe da inviare al client
    char comando1[] = "1) FCO -> JFK\n2) JFK -> FCO\n3) CDG -> FCO\n4) FCO -> CDG\n5) CDG -> JFK\nInvia il numero della tratta interessata o 6 per tornare indietro";
    char comando2[] = "Inserisci il codice della prenotazione oppure 'q' per tornare indietro";
    char comando3[] = "Carattere errato, riprova per favore";
    char comando4[] = "Premi 1 per accedere alla lista delle tratte\nPremi 2 per cancellare una prenotazione.";
    char comando5[] = "Per selezionare un posto inviali nella forma seguente 'A1,C6,E2'\nAltrimenti invia q per tornare indietro\n\nABC DEF\n*** *** 1\n*** *** 2\n*** *** 3\n*** *** 4\n*** *** 5\n*** *** 6\n*** *** 7\n*** *** 8\n";
    char send_buf[] = "Sei connesso al server!\nPremi 1 per accedere alla lista delle tratte\nPremi 2 per cancellare una prenotazione.";
    char comandoq[] = "q";

    size_t msg_len = strlen(send_buf); //mandiamo il messaggio di benvenuto
 
    while ( (ret = send( client_desc, send_buf, msg_len, 0 )) < 0 ){
        if(1){
            //error management 
        }
    }

    //riceviamo e mandiamo fino a che non riceviamo comando quit
    while (1){

        
        int bytes_red = 0;
        char buf_ric[256]; //buffer per messaggi ricevuti
        size_t len_buf = sizeof(buf_ric); 
        memset(buf_ric , 0 , len_buf ); //resettiamo il buffer

        //usiamo un while per assicurarci che siano stati mandati tutti i byte
        while( (byte_ric = recv(client_desc , buf_ric + bytes_red , len_buf - 1, 0)) < 0){
            //gestisci errori
            bytes_red += byte_ric;
        }

        printf("ho ricevuto: %s\n",buf_ric);

        //condizione per chiudere la comunicazione, per ora solo 1 messaggio
        if( !memcmp(quit_comm, buf_ric , lun_q)){
            printf("comunicazione finita");
            break;
        }

        size_t lunghezza = bytes_red - 1;
        buf_ric[lunghezza]='\0';

        //resettiamo sen
        size_t sen_len = sizeof(sen);
        memset(sen , 0 , sen_len);

        //switch su stati
        switch (stato){
            case menu:
                comm = atoi(buf_ric);

                if(comm==1){
                    memcpy(sen , comando1 , strlen(comando1));
                    manda_mess(client_desc , sen);
                    stato = tratte;
                }
                else if(comm==2){
                    memcpy(sen, comando2 , strlen(comando2)); 
                    manda_mess(client_desc , sen);
                    stato = codicelim;
                }
                else{
                    memcpy(sen, comando3 , strlen(comando3));
                    manda_mess(client_desc , sen);    
                }
                break;
        

            case tratte:
                comm = atoi(buf_ric);
                
                if(comm == 1){
                    memcpy(sen , comando5 , strlen(comando5));
                    manda_mess(client_desc , sen);
                    stato = postidisp;
                }
                else if (comm == 2){

                }
                else if (comm == 3){

                }
                else if (comm == 4){

                }
                else if (comm == 5){

                }
                else if (comm == 6){
                    memcpy(sen, comando4 , strlen(comando4));
                    manda_mess(client_desc , sen);
                    stato = menu;
                }
                else{
                    memcpy(sen, comando3 , strlen(comando3));
                    manda_mess(client_desc , sen);    
                }
                break;


            case postidisp:
                
                if( !memcmp( comandoq , buf_ric , sizeof(comandoq)) ){
                    memcpy(sen, comando1 , strlen(comando1));
                    manda_mess(client_desc , sen);
                    stato = tratte;
                }
                else{
                    memcpy(sen, comando3 , strlen(comando3));
                    manda_mess(client_desc , sen);    
                }
                break;
            
            case codicelim:
                
                if(!memcmp( comandoq , buf_ric , sizeof(comandoq))){
                    memcpy(sen, comando4 , strlen(comando4));
                    manda_mess(client_desc , sen);
                    stato = menu;
                }
                else{
                    memcpy(sen, comando3 , strlen(comando3));
                    manda_mess(client_desc , sen);    
                }
                break;
            
            
            default:
                break;
        }
    }
    return;
}

int main(int argc, char* argv[]){

    //variabile per controllare eventuali errori
    int ret;

    //definiamo i descrittori, le struct per indirizzi, la size della struct
    int socket_desc,client_desc;
    struct sockaddr_in client_addr = {0};   // alcuni valori devono essere settati a 0
    struct sockaddr_in server_addr = {0};
    int sockaddr_len = sizeof(struct sockaddr_in); 

    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc<0){
        //error management 
    }

    server_addr.sin_addr.s_addr = INADDR_ANY; //per ricevere connessioni da qualunque interfaccia
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT); 

    //colleghiamo socket ad address
    ret = bind( socket_desc ,  (struct sockaddr*) &server_addr , sockaddr_len); 
    if(ret < 0){
        //error management
    }

    //iniziamo ad "ascoltare"
    ret = listen( socket_desc , CONNESSIONI_MAX);
    if( ret < 0 ){
        //handle error
    }

    while(1){ //cambia questa parte per multi thread
        
        //accettiamo connessioni
        client_desc = accept(socket_desc , (struct sockaddr*) &client_addr , (socklen_t*)&sockaddr_len );
        if (client_desc < 0){
            //handle error
        }

        connection_handler(client_desc);
    }
}