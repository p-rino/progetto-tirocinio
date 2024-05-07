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

#define MAX(a,b) ((a) > (b) ? (a) : (b))

//PER ORA stringhe da inviare al client
const char comando1[] = "1) FCO -> JFK\n2) JFK -> FCO\n3) CDG -> FCO\n4) FCO -> CDG\n5) AMS -> BER\n6) BER -> AMS\n7) LHR -> ZRH\n8) ZRH -> LHR\n9) VIE -> BCN\n10) BCN -> VIE\n11) torna indietro\nInvia il numero della tratta interessata o 11 per tornare indietro";
const char comando2[] = "Inserisci il codice della prenotazione oppure 'q' per tornare indietro";
const char comando3[] = "input errato, riprova per favore";
const char comando4[] = "Premi 1 per accedere alla lista delle tratte\nPremi 2 per cancellare una prenotazione\nEsci: premi quit per uscire";
char comando5[] = "Prenotazione effettuata!\nIl tuo codice prenotazione è: ";
const char comando5bis[] = "\nPremi 1 per tornare indietro";
const char comando6[] = "Uno di queti posti è occupato, per favore seleziona posti liberi";
const char comando7[] = "Prenotazione cancellata!\nPremi 1 per tornare al menù principale";
const char comando8[] = "Codice prenotazione non trovato!\nRiprova oppure premi q per tornare indietro.";
const char send_buf[] = "Sei connesso al server!\nPremi 1 per accedere alla lista delle tratte\nPremi 2 per cancellare una prenotazione\nEsci: premi quit per uscire";
const char comandoq[] = "q";

int verifica_posti(char* lista_posti, char* cont_file);
void manda_mess(int client_desc , char mess[]);
void* connection_handler(void* arg);
int calcola_pos(char* posto);
int get_num_prenotazione(char num);
void aum_cod_pren(char num);
void eliminaprenotati(char prenotazione[]);
//enum per distinguere in che stato del menu ci troviamo
enum stati_menu { menu , tratte , postidisp , codicelim , codiceprenot , codiceliminato};

//funzione per aprire file e modificare i posti prenotati
void eliminaprenotati(char prenotazione[]){
    char file = prenotazione[0];
    char posto[2];
    FILE* h;
    char buf[30];
    //printf("carattere: %c",file);
    switch(file){
        case '1':
            if((h = fopen( "posti_uno.txt" , "r+"))==NULL){
                //errore
            }
            break;

        case '2':
            if((h = fopen( "posti_due.txt" , "r+"))==NULL){
                //errore
            }
            break;

        case '3':
            if((h = fopen( "posti_tre.txt" , "r+"))==NULL){
                //errore
            }
            break;

        case '4':
            if((h = fopen( "posti_quattro.txt" , "r+"))==NULL){
                //errore
            }
            break;

        case '5':
            if((h = fopen( "posti_cinque.txt" , "r+"))==NULL){
                //errore
            }
            break;

        case '6':
            if((h = fopen( "posti_sei.txt" , "r+"))==NULL){
                //errore
            }
            break;

        case '7':
            if((h = fopen( "posti_sette.txt" , "r+"))==NULL){
                //errore
            }
            break;

        case '8':
            if((h = fopen( "posti_otto.txt" , "r+"))==NULL){
                //errore
            }
            break;

        case '9':
            if((h = fopen( "posti_nove.txt" , "r+"))==NULL){
                //errore
            }
            break;

        case 'A':
            if((h = fopen( "posti_dieci.txt" , "r+"))==NULL){
                //errore
            }
            break;
    }
    
    //calcolo lunghezza file
    fseek(h , 0 , SEEK_END);
    long fsize = ftell(h);
    fseek(h,0,SEEK_SET);

    //leggiamo tutto e mettiamolo su tomod_str
    char *tomod_str = malloc(fsize + 1);
    fread(tomod_str , fsize , 1 , h);
    //printf("stringa totale:\n %s",tomod_str);
    int numero_posto;
    int i=0;

    //arriviamo ai posti
    while (prenotazione[i]!=':'){
        i++;
    }
    i++;
    //printf("numero i: %d\nlen prenotazione %ld\n",i,strlen(prenotazione));
    //rendiamo i posti disponibili
    int j=i;
    for ( i ; i<strlen(prenotazione)-j+1 ; i+=2){
        posto[0]=prenotazione[i];
        posto[1]=prenotazione[i+1];
        if(prenotazione[i+2]==','){i++;}
        //printf("posto analizzato: %s\n",posto);
        //printf("la i è a: %d , mentre lenpren è: %ld\n",i,strlen(prenotazione));
        numero_posto = calcola_pos(posto);
        tomod_str[numero_posto]='*';
    }

    //printf("stringa totale:\n %s",tomod_str);
    fseek(h , 0 , SEEK_SET);
    fprintf(h , "%s" ,tomod_str);
    fclose(h);
}
 
//funzione per aumentare nel file il codice
void aum_cod_pren(char num){
    int tratta;
    char num_letto[10];
    char da_sovrascrivere[100]="";
    char snum[10];
    int i=1;
    //apriamo il file e leggiamo la riga giusta
    FILE *t;
    if((t = fopen( "numeri_prenotazione.txt" , "r+"))==NULL){
        //errore
    }
    
    if(num=='A'){tratta=10;}
    else{tratta=num - '0';}


    while(fgets(num_letto, 10, t)){
        //leggiamo le righe fino a che non troviamo la riga giusta
        
        if(i==tratta){
            //aumenta numero tratta
            int num_da_scrivere = atoi(num_letto);
            num_da_scrivere++;
            sprintf(snum ,"%d\n" , num_da_scrivere );
            strcat(da_sovrascrivere , snum);
        }
        else{
            strcat(da_sovrascrivere , num_letto);
        }
        i++;
    }
    //printf("num codici\n%s" , da_sovrascrivere);
    fseek(t , 0 , SEEK_SET);
    fprintf(t , "%s" ,da_sovrascrivere);
    fclose(t);
}

//funzione per prendere da file il prossimo numero prenotazione
int get_num_prenotazione(char num){
    int tratta;
    char num_letto[10];
    int i=1;
    //apriamo il file e leggiamo la riga giusta
    FILE *f;
    if((f = fopen( "numeri_prenotazione.txt" , "r+"))==NULL){
        //errore
    }
    
    if(num=='A'){
        tratta=10;
    }
    else{
        tratta=num - '0';
    }

    while(fgets(num_letto, 10, f)){
        //leggiamo le righe fino a che non troviamo la riga giusta
        
        if(i==tratta){
            return atoi(num_letto);
        }
        i++;
    }
}

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
        //printf("stringa tokenizzata %s\n",token);
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

//funzione che gestisce connessione/invio
void* connection_handler(void* arg /*client_desc*/){
    char* open_file;
    int ret;
    int comm; //comandi per navigazione menu
    char sen[400]; //usato per inviare messaggi
    enum stati_menu stato = menu; //stato iniziale = menu
    int* client_desc_pt = (int*)arg;
    int client_desc = *client_desc_pt;
    char num_tratta;
    int num_prenotazione;

    printf("server connesso a client: %d\n",client_desc);

    int byte_ric, sent_bytes; //variabili per send e recv

    //comando per finire comunicazione da common.h
    char* quit_comm = QUIT_COMM;
    size_t lun_q = strlen(quit_comm);

    FILE *f;
    FILE *g;

 
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
            if (byte_ric == -1){

                perror("errore lettura nel client.\n");
                exit(EXIT_FAILURE);
            } 
            bytes_red += byte_ric;
        }

        //printf("Dal client %d ho ricevuto: %s\n",client_desc , buf_ric);

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
                    
                    //printf("%s",send_str);
                    num_tratta='1';
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
                    
                    //printf("%s",send_str);
                    num_tratta='2';
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
                    
                    //printf("%s",send_str);
                    num_tratta='3';
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
                   
                    //printf("%s",send_str);
                    num_tratta='4';
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
                    
                    //printf("%s",send_str);
                    num_tratta='5';
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
                    
                    //printf("%s",send_str);
                    num_tratta='6';
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
                    
                    //printf("%s",send_str);
                    num_tratta='7';
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
                    
                    //printf("%s",send_str);
                    num_tratta='8';
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
                    
                    //printf("%s",send_str);
                    num_tratta='9';
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
                    
                    //printf("%s",send_str);
                    num_tratta='A';
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
                    
                    //printf("debug: verifica: %d\n",ver);
                    if( ver==0 ){
                        memcpy(sen, comando6 , strlen(comando6));
                        manda_mess(client_desc , sen);
                    }
                    else{
                        
                        //generiamo codice prenotazione
                        int i=0;
                        char snum_prenotazione[10];
                        aum_cod_pren(num_tratta);
                        num_prenotazione = get_num_prenotazione(num_tratta);
                        char codice_prenotazione[100];
                        codice_prenotazione[i]=num_tratta;
                        i++;
                        sprintf(snum_prenotazione ,"%d" , num_prenotazione );
                        for (int j=0 ; j<strlen(snum_prenotazione) ; j++ ){
                            codice_prenotazione[i] = snum_prenotazione[j];
                            i++;
                        }
                        codice_prenotazione[i]=':';
                        i++;
                        for ( int j=0 ; j < strlen(buf_ric) ; j++ ){
                            codice_prenotazione[i] = buf_ric[j];
                            i++;
                        }
                        codice_prenotazione[i]='\0';
                        printf("codice prenotazione generato: %s\n",codice_prenotazione);
                        
                        //scriviamo codice prenotazione sul file 
                        g=fopen( "codici_prenotazione.txt" , "r+");
                        fseek(g ,  0, SEEK_END);
                        fprintf(g , "%s\n" , codice_prenotazione);
                        fclose(g);

                        //tokenizziamo il buf_ric per prenotaare tutti i posti 
                        const char splitter[] = ",";
                        char *token;
                        //printf("ricevuto: %s\n",buf_ric);
                        token = strtok(buf_ric , splitter);
                        int num_posto;

                        while(token != NULL){   
                            //printf("posto da prenotare: %s\n",token);
                            num_posto = calcola_pos(token);
                            mod_str[num_posto]='x';

                            token = strtok(NULL , splitter);
                        }

                        //printf("%s",mod_str);
                        //scriviamo su file 
                        fseek(f , 0 , SEEK_SET);
                        fprintf(f , "%s" ,mod_str);
                        strcat(comando5,codice_prenotazione);
                        strcat(comando5,comando5bis);
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
                else if(1==1){//dobbiamo verificare se l'input è nella forma che vogliamo
                    int trovato = 0;
                    char letto[40];
                    char nuovo[10000]="";
                    char prova[20]="\n"; 
                    int pro=1;
                    strcat(buf_ric , prova);
                    //apriamo il file e cerchiamolo
                    FILE* p;
                    p=fopen( "codici_prenotazione.txt" , "r+");
                    //printf("codice da cercare: %s\n",buf_ric);

                    while(fgets(letto, 40, p)){
                        pro=1;
                        //printf("codice analizzato: %s\n",letto);
                    
                        int lunghezza_max = MAX( strlen(buf_ric) , strlen(letto) );
                        for (int i = 0 ; i < lunghezza_max-1 ; i++){
                            if(buf_ric[i]!=letto[i]){
                                pro=0;
                            }
                            //printf("comparato b: %c , l: %c \n",buf_ric[i],letto[i]);
                        }
                        
                        if(!pro){
                            strcat(nuovo , letto);
                        }
                        else{
                            trovato=1;
                        }
                    }
                    fclose(p);
                    
                    if(trovato){
                        p=fopen( "codici_prenotazione.txt" , "w+");
                        fseek(p , 0 , SEEK_SET);
                        fprintf(p , "%s" , nuovo);
                        fclose(p);

                        //da un problema nell'inviare la stringa al client
                        eliminaprenotati(buf_ric);

                        //comando7 stranamente è cambiato anche se non viene usato da nessuna altra parte
                        printf("stringa da mandare: %s",comando7);

                        memcpy(sen, comando7 , strlen(comando7));
                        manda_mess(client_desc , sen);    
                        stato=codiceliminato;
                    }
                    else{
                        memcpy(sen, comando8 , strlen(comando8));
                        manda_mess(client_desc , sen);   
                    }
                    
                }
                else{
                    memcpy(sen, comando3 , strlen(comando3));
                    manda_mess(client_desc , sen);   
                }
                break;

            case codiceliminato:
                if(atoi(buf_ric) == 1){
                    memcpy(sen, comando4 , strlen(comando4));
                    manda_mess(client_desc , sen);
                    stato = menu;
                }
                else{
                    memcpy(sen, comando7 , strlen(comando7));
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