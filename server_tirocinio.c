#include <sys/socket.h>
#include <netinet/in.h>
#include "common.h"  //mio file per avere informazioni in comune
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>  
#include <semaphore.h>
#include <pthread.h>
#include <errno.h>

int verifica_posti(char* lista_posti, char* cont_file);
void manda_mess(int client_desc , char mess[]);
void* connection_handler(void* arg /*client_desc*/);
int calcola_pos(char* posto);

//enum per distinguere in che stato del menu ci troviamo
enum stati_menu { menu , tratte , postidisp , codicelim , codiceprenot};

//funzione per calcolare posizione nell'array dei posti
int calcola_pos(char* posto){
    
    char lettera = posto[0];
    char* prova = &posto[1];
    int riga = atoi( prova ); 
    int val_lett;
    /*
    mod_str[135]='x'; 
    posizione iniziale della matrice 135
    prossima riga  +10
    formula -> valore lettera + 135 +10*(livello - 1)
    */
    switch(lettera){
        case 'A':
            val_lett=0;
            break;
        case 'B':
            val_lett=1;
            break;
        case 'C':
            val_lett=2;
            break;
        case 'D':
            val_lett=4;
            break;
        case 'E':
            val_lett=5;
            break;
        case 'F':
            val_lett=6;
            break;
        default:
            break;
    }

    int pos = val_lett + 135 + 10 * (riga - 1);
    return pos;
}

//funzione verifica che i posti siano liberi    DA FINIRE
int verifica_posti(char* lista_posti, char* cont_file){

    const char splitter[] = ",";
    char *token;
    token = strtok(lista_posti , splitter);
    int num_posto;

    while(token != NULL){
        printf("stringa tokenizzata %s\n",token);
        num_posto = calcola_pos(token);
        if(cont_file[num_posto] == 'x'){
            return 0;
        }

        token = strtok(NULL , splitter);
    }

    return 1;
}

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

void* connection_handler(void* arg /*client_desc*/){
    char* open_file;
    int ret;
    int comm; //comandi per navigazione menu
    char sen[400]; //usato per inviare messaggi
    enum stati_menu stato = menu; //stato iniziale = menu
    int* client_desc_pt = (int*)arg;
    int client_desc = *client_desc_pt;

    printf("server connesso a client: %d\n",client_desc);

    int byte_ric, sent_bytes; //variabili per send e recv

    //comando per finire comunicazione da common.h
    char* quit_comm = QUIT_COMM;
    size_t lun_q = strlen(quit_comm);

    FILE *f;

    //PER ORA stringhe da inviare al client
    char comando1[] = "1) FCO -> JFK\n2) JFK -> FCO\n3) CDG -> FCO\n4) FCO -> CDG\n5) AMS -> BER\n6) BER -> AMS\n7) LHR -> ZRH\n8) ZRH -> LHR\n9) VIE -> BCN\n10) BCN -> VIE\n11) torna indietro\nInvia il numero della tratta interessata o 11 per tornare indietro";
    char comando2[] = "Inserisci il codice della prenotazione oppure 'q' per tornare indietro";
    char comando3[] = "input errato, riprova per favore";
    char comando4[] = "Premi 1 per accedere alla lista delle tratte\nPremi 2 per cancellare una prenotazione\nEsci: premi quit per uscire";
    char comando5[] = "Prenotazione effettuata! \nPremi 1 per tornare al menu iniziale";
    char comando6[] = "Uno di queti posti è occupato, per favore seleziona posti liberi";
    char send_buf[] = "Sei connesso al server!\nPremi 1 per accedere alla lista delle tratte\nPremi 2 per cancellare una prenotazione\nEsci: premi quit per uscire";
    char comandoq[] = "q";
    //char occupato[] = "x";

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
            if( byte_ric == -1 && errno == EINTR) continue;
            if (ret == -1){
                perror("errore lettura nel client.\n");
                exit(EXIT_FAILURE);
            } 
            bytes_red += byte_ric;
        }

        printf("Dal client %d ho ricevuto: %s\n",client_desc , buf_ric);

        //condizione per chiudere la comunicazione, per ora solo 1 messaggio
        if( !memcmp(quit_comm, buf_ric , lun_q)){
            printf("comunicazione finita con client: %d\n",client_desc);
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
                //comm = atoi(buf_ric);

                if(atoi(buf_ric)==1){
                    memcpy(sen , comando1 , strlen(comando1));
                    manda_mess(client_desc , sen);
                    stato = tratte;
                }
                else if(atoi(buf_ric)==2){
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
                    
                    //apro il file contenente dati della tratta 1
                    if((f = fopen( "posti_uno.txt" , "r"))==NULL){
                        //errore
                    }

                    //calcolo lunghezza file
                    fseek(f , 0 , SEEK_END);
                    long fsize = ftell(f);
                    fseek(f,0,SEEK_SET);

                    //leggiamo tutto e mettiamolo su send_str
                    char *send_str = malloc(fsize + 1);
                    fread(send_str , fsize , 1 , f);
                    fclose(f);
                    
                    send_str[fsize] = 0;
                    
                    printf("%s",send_str);
                    memcpy(sen , send_str , strlen(send_str));
                    manda_mess(client_desc , sen);
                    open_file = "posti_uno.txt";
                    stato = postidisp;
                }
                else if (comm == 2){
                    //apro il file contenente dati della tratta 1
                    if((f = fopen( "posti_due.txt" , "r"))==NULL){
                        //errore
                    }

                    //calcolo lunghezza file
                    fseek(f , 0 , SEEK_END);
                    long fsize = ftell(f);
                    fseek(f,0,SEEK_SET);

                    //leggiamo tutto e mettiamolo su send_str
                    char *send_str = malloc(fsize + 1);
                    fread(send_str , fsize , 1 , f);
                    fclose(f);
                    
                    send_str[fsize] = 0;
                    
                    printf("%s",send_str);
                    memcpy(sen , send_str , strlen(send_str));
                    manda_mess(client_desc , sen);
                    open_file = "posti_due.txt";
                    stato = postidisp;
                }
                else if (comm == 3){
                    //apro il file contenente dati della tratta 1
                    if((f = fopen( "posti_tre.txt" , "r"))==NULL){
                        //errore
                    }

                    //calcolo lunghezza file
                    fseek(f , 0 , SEEK_END);
                    long fsize = ftell(f);
                    fseek(f,0,SEEK_SET);

                    //leggiamo tutto e mettiamolo su send_str
                    char *send_str = malloc(fsize + 1);
                    fread(send_str , fsize , 1 , f);
                    fclose(f);
                    
                    send_str[fsize] = 0;
                    
                    printf("%s",send_str);
                    memcpy(sen , send_str , strlen(send_str));
                    manda_mess(client_desc , sen);
                    open_file = "posti_tre.txt";
                    stato = postidisp;
                }
                else if (comm == 4){
                   //apro il file contenente dati della tratta 1
                    if((f = fopen( "posti_quattro.txt" , "r"))==NULL){
                        //errore
                    }

                    //calcolo lunghezza file
                    fseek(f , 0 , SEEK_END);
                    long fsize = ftell(f);
                    fseek(f,0,SEEK_SET);

                    //leggiamo tutto e mettiamolo su send_str
                    char *send_str = malloc(fsize + 1);
                    fread(send_str , fsize , 1 , f);
                    fclose(f);
                    
                    send_str[fsize] = 0;
                   
                    printf("%s",send_str);
                    memcpy(sen , send_str , strlen(send_str));
                    manda_mess(client_desc , sen);
                    open_file = "posti_quattro.txt";
                    stato = postidisp;
                }
                else if (comm == 5){
                   //apro il file contenente dati della tratta 1
                    if((f = fopen( "posti_cinque.txt" , "r"))==NULL){
                        //errore
                    }

                    //calcolo lunghezza file
                    fseek(f , 0 , SEEK_END);
                    long fsize = ftell(f);
                    fseek(f,0,SEEK_SET);

                    //leggiamo tutto e mettiamolo su send_str
                    char *send_str = malloc(fsize + 1);
                    fread(send_str , fsize , 1 , f);
                    fclose(f);
                    
                    send_str[fsize] = 0;
                    
                    printf("%s",send_str);
                    memcpy(sen , send_str , strlen(send_str));
                    manda_mess(client_desc , sen);
                    open_file = "posti_cinque.txt";
                    stato = postidisp;
                }
                else if (comm == 6){
                    //apro il file contenente dati della tratta 1
                    if((f = fopen( "posti_sei.txt" , "r"))==NULL){
                        //errore
                    }

                    //calcolo lunghezza file
                    fseek(f , 0 , SEEK_END);
                    long fsize = ftell(f);
                    fseek(f,0,SEEK_SET);

                    //leggiamo tutto e mettiamolo su send_str
                    char *send_str = malloc(fsize + 1);
                    fread(send_str , fsize , 1 , f);
                    fclose(f);
                    
                    send_str[fsize] = 0;
                    
                    printf("%s",send_str);
                    memcpy(sen , send_str , strlen(send_str));
                    manda_mess(client_desc , sen);
                    open_file = "posti_sei.txt";
                    stato = postidisp;
                }       
                else if (comm == 7){
                    //apro il file contenente dati della tratta 1
                    if((f = fopen( "posti_sette.txt" , "r"))==NULL){
                        //errore
                    }

                    //calcolo lunghezza file
                    fseek(f , 0 , SEEK_END);
                    long fsize = ftell(f);
                    fseek(f,0,SEEK_SET);

                    //leggiamo tutto e mettiamolo su send_str
                    char *send_str = malloc(fsize + 1);
                    fread(send_str , fsize , 1 , f);
                    fclose(f);
                    
                    send_str[fsize] = 0;
                    
                    printf("%s",send_str);
                    memcpy(sen , send_str , strlen(send_str));
                    manda_mess(client_desc , sen);
                    open_file = "posti_sette.txt";
                    stato = postidisp;
                }
                else if (comm == 8){
                    //apro il file contenente dati della tratta 1
                    if((f = fopen( "posti_otto.txt" , "r"))==NULL){
                        //errore
                    }

                    //calcolo lunghezza file
                    fseek(f , 0 , SEEK_END);
                    long fsize = ftell(f);
                    fseek(f,0,SEEK_SET);

                    //leggiamo tutto e mettiamolo su send_str
                    char *send_str = malloc(fsize + 1);
                    fread(send_str , fsize , 1 , f);
                    fclose(f);
                    
                    send_str[fsize] = 0;
                    
                    printf("%s",send_str);
                    memcpy(sen , send_str , strlen(send_str));
                    manda_mess(client_desc , sen);
                    open_file = "posti_otto.txt";
                    stato = postidisp;
                }
                else if (comm == 9){
                    //apro il file contenente dati della tratta 1
                    if((f = fopen( "posti_nove.txt" , "r"))==NULL){
                        //errore
                    }

                    //calcolo lunghezza file
                    fseek(f , 0 , SEEK_END);
                    long fsize = ftell(f);
                    fseek(f,0,SEEK_SET);

                    //leggiamo tutto e mettiamolo su send_str
                    char *send_str = malloc(fsize + 1);
                    fread(send_str , fsize , 1 , f);
                    fclose(f);
                    
                    send_str[fsize] = 0;
                    
                    printf("%s",send_str);
                    memcpy(sen , send_str , strlen(send_str));
                    manda_mess(client_desc , sen);
                    open_file = "posti_nove.txt";
                    stato = postidisp;
                }
                else if (comm == 10){
                    //apro il file contenente dati della tratta 1
                    if((f = fopen( "posti_dieci.txt" , "r"))==NULL){
                        //errore
                    }

                    //calcolo lunghezza file
                    fseek(f , 0 , SEEK_END);
                    long fsize = ftell(f);
                    fseek(f,0,SEEK_SET);

                    //leggiamo tutto e mettiamolo su send_str
                    char *send_str = malloc(fsize + 1);
                    fread(send_str , fsize , 1 , f);
                    fclose(f);
                    
                    send_str[fsize] = 0;
                    
                    printf("%s",send_str);
                    memcpy(sen , send_str , strlen(send_str));
                    manda_mess(client_desc , sen);
                    open_file = "posti_dieci.txt";
                    stato = postidisp;
                }        
                else if (comm == 11){
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
                else if(1==1){  //capire che condizione mettere (probabilmente controlla prima l'input e usa quello)

                     if((f = fopen( open_file , "r+"))==NULL){
                        //errore
                    }
                    //calcolo lunghezza file
                    fseek(f , 0 , SEEK_END);
                    long fsize = ftell(f);
                    fseek(f,0,SEEK_SET); 
                    //leggiamo tutto e mettiamolo su mod_str
                    char *mod_str = malloc(fsize + 1);
                    fread(mod_str , fsize , 1 , f);
                    char copia_buf[400];
                    memcpy(copia_buf , buf_ric , sizeof(buf_ric));
                    int ver = verifica_posti( copia_buf , mod_str );
                    
                  /*int pos = calcola_pos(buf_ric);//val_lett + 135 + 10 * (riga - 1);
                    printf("valore posizione: %d\n",pos);*/
                    printf("debug: verifica: %d\n",ver);
                    if( ver==0/*mod_str[pos] == 'x'*/){
                        memcpy(sen, comando6 , strlen(comando6));
                        manda_mess(client_desc , sen);
                    }
                    else{
                        
                        const char splitter[] = ",";
                        char *token;
                        printf("ricevuto: %s\n",buf_ric);
                        token = strtok(buf_ric , splitter);
                        int num_posto;

                        while(token != NULL){   
                            printf("posto da prenotare: %s\n",token);
                            num_posto = calcola_pos(token);
                            mod_str[num_posto]='x';

                            token = strtok(NULL , splitter);
                        }
                        printf("%s",mod_str);
                        //scriviamo su file 
                        fseek(f , 0 , SEEK_SET);
                        fprintf(f , "%s" ,mod_str);

                        memcpy(sen, comando5 , strlen(comando5));
                        manda_mess(client_desc , sen);
                        stato = codiceprenot;
                    }
                    fclose(f);
                
                }
                else{
                    memcpy(sen, comando3 , strlen(comando3));
                    manda_mess(client_desc , sen);    
                }
                break;

            case codiceprenot:
                if(atoi(buf_ric) == 1){
                    memcpy(sen, comando4 , strlen(comando4));
                    manda_mess(client_desc , sen);
                    stato = menu;
                }
                else{
                    memcpy(sen, comando5 , strlen(comando5));
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
    
}

int main(int argc, char* argv[]){

    //variabile per controllare eventuali errori
    int ret;

    //definiamo i descrittori, le struct per indirizzi, la size della struct
    int socket_desc,client_desc;
  
    // alcuni valori devono essere settati a 0
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
        

        //setup per i thread 
        pthread_t thread;

        //allochiamo a ogni ciclo una nuova struttura 
        struct sockaddr_in* client_addr = calloc(1, sizeof(struct sockaddr_in));

        //accettiamo connessioni
        client_desc = accept(socket_desc , (struct sockaddr*) &client_addr , (socklen_t*)&sockaddr_len );
        if (client_desc < 0){
            //handle error
        }

        //per passarlo a pthread_create
        int* client_desc_pt = calloc( 1 , sizeof(int) );
        *client_desc_pt = client_desc;


        ret = pthread_create(&thread , NULL , connection_handler , client_desc_pt);
        if(ret){
            //gestisci errore
        }

        ret = pthread_detach(thread);
        if(ret){
            //gestisci errore 
        }

        //connection_handler(client_desc_pt);
    }

    return 0;
}