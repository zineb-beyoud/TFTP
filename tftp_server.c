//-i localhost GET test.txt
#include <stdio.h>
#include <stdlib.h>
#include "tftp_server.h"
#include <winsock2.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "string.h"

//different clients has to do with while 1 peut etre .... 7it une fois je termine avec une cmd line I loose connection while 1 ib9a m bindi

#define taille 512
#define codeData 3
#define RQR 1
#define RQW 2
#define ACK 4
#define bloc 1
#define ERROR 5

int serv;         // Identifiant du socket serveur
int iResult;      // Résultat des fonctions de la bibliothèque
char Packet[taille];   // Tampon pour stocker les données reçues
char Packet2[taille];   // Tampon pour stocker les données reçues du 2 eme client
int len;          // Taille de la zone mémoire disponible pour l'adresse du client
int bytesRead;    // Longueur des données contenues dans le datagramme reçu
int bytesSent;
int lastpac;

char filename[100];
int flag=0;
struct sockaddr_in servAddr;
struct sockaddr_storage clientAddr;
//struct sockaddr_storage clientAddr_2;
//struct sockaddr_storage clientAddr_3;
int n=7;
char paquet[512];
char paquets[][512];
char ack[4];


enum tftp_err tftp_server_init(void){

    WSADATA wsaData;
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult !=0) {
    printf("WSAStartup failed: %d\n",iResult);
    return TFTPSERV_ERR;
    }

    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servAddr.sin_port = htons(69);
    serv = socket(AF_INET,SOCK_DGRAM, IPPROTO_UDP);

    iResult = bind(serv,(struct sockaddr *)&servAddr, sizeof(servAddr));
    if (iResult != 0) {
    printf("bind failed: %d\n",iResult);
    return TFTPSERV_ERR;
    }
    else
        return TFTPSERV_OK;;

}

void tftp_server_run(void ){

    len = sizeof(struct sockaddr_in);
    ZeroMemory(&Packet,sizeof(Packet));

    printf("\n TFTP SERVER...\n");

    RQT();

    /*if((RQT()==0) & (validateRQ()==0)){                         //si le packet n'est pas bien recu et si
        printf("Cannot unpack Packet\n");
    }
    else printf("Packet received ! \n");*/

    if((RQT()==1) & (validateRQ()== TFTPSERV_OK))                  //pas d'erreur en reception de paquet et Request Validée
        printf("\n \t \t ***Packet received ! ****\n");
    else printf("Cannot unpack Packet\n");

    printf("\n \t \t *** Sending response with contents of the file****\n");

    if(TFTP_Packet(filename)!=1){
        printf("\n NO DATA FOUND \n");
    }
    TFTP_Ack();
}
/** waiting for Request function **/

int RQT(void){

    int i;
    iResult = bind(serv,(struct sockaddr *)&servAddr, sizeof(servAddr));                     //Associates the local address with a socket.
    /*  if (iResult != 0) {
    printf("bind failed: %d\n",iResult);
    return 0;
    }
    */
    len = sizeof(struct sockaddr_in);
    ZeroMemory(&Packet,sizeof(Packet));
    printf("\n \t \t*** Waiting for datagram...***\n");
    bytesRead = recvfrom(serv,Packet,taille, 0, (struct sockaddr *) &clientAddr, &len);       //Server receives a datagram(Packet) and stores the source address(aka Client @).
   /* for(i=0;i<taille;i++){
    printf("%x", Packet[i]);}*/
    if (bytesRead == SOCKET_ERROR)
    {
        iResult = WSAGetLastError();
        printf("Error: %d\n",iResult);
        return 0;
    }
    return 1;

}

//creation du paquet acquitement

int TFTP_Ack(void){
    len = sizeof(struct sockaddr_in);
    ZeroMemory(&Packet,sizeof(Packet));
    printf("\n \t \t ****Waiting for acknowledgement...***\n");
    bytesRead = recvfrom(serv,Packet, taille,0,(struct sockaddr*)&clientAddr,&len);

     if (bytesRead == SOCKET_ERROR)
    {
        iResult = WSAGetLastError();
        printf("Error: %d\n",iResult);
        return 0;
    }
    if(validateRQ() != ACK)return 0; //////******************** to modify

    return 1;
}


int validateRQ(void ){

char mode[20];
int i=2;
int modeIndex;
/** traitement du operation code*/
int opCode = Packet[1];
printf("op code is %d\n",opCode);

    switch(opCode){

    /** case RQR: On recupere le filename qui se situe dans le 3eme octet du paquet*/
    /**   2 bytes     string    1 byte     string   1 byte
            ------------------------------------------------
           | Opcode |  Filename  |   0  |    Mode    |   0  |
            ------------------------------------------------
    **/
    case RQR:{                                 //puisque le client cherche à lire on récupère le nom du fichier pour lui transmettre ceci apres

        while(Packet[i] != 0 ){
        filename[i-2]=Packet[i];
        i++;
        }

        /**Just testing**/
     /*   int j;
        printf("\n****filename*****\n");
        for(j=0;j<100;j++)
            printf("%x\t",filename[j]);             // print du nom du file pour débogage
        printf("\n");
        /**test end**/

        filename[i-2]='\0';
        i++;
        //printf("\n filename is : %s\n", filename);


        // mode index is i+1
        modeIndex=i;
        while(Packet[i] != 0 ){
            mode[i - modeIndex] = Packet[i];
            i++;
        }
        //we convert the mode to a string
        mode[i-modeIndex]='\0';

    //    if(strcmp(mode,"octet")==0){
    //        printf("\n Octet mode \n");
    //    }
    //    else return TFTPSERV_ERR;                         //*************************erreur liée au type return 9
    //
    //
    //    /** testing the file : calling the testfile function **/
    //    test_file(filename);
    //    //return RQR;
    //    return TFTPSERV_OK;                               // in case everything goes right with the RQR the function returns OK
    //    break;

        //------****************---------
        //First of all we check if the file exist or not ?
        if( access( filename, F_OK ) == 0 ) {
        // file exists
            printf("\n File exists\n");
        } else {
        // file doesn't exist
            printf("\n File doesn't exists Error : TFTPSERV_NO_SUCH_FILE \n");
            exit(0);
            return TFTPSERV_NO_SUCH_FILE;
            break;
        }
        //2nd we check the mode
        if(strcmp(mode,"octet")!=0){
            printf("\n Erreur Not Octet mode Error : TFTPSERV_ILLEGAL_OPERATION \n");
            exit(0);
            return TFTPSERV_ILLEGAL_OPERATION;            // if it's not the Octet Mode it's considered as illegal operation ERR = 4
            break;
        }
        else{
        test_file(filename);
        //return RQR;
        return TFTPSERV_OK;                               // in case everything goes right with the RQR the function returns OK
        break;
        }
        
        }

    /**Trame DATA**/

    case(codeData):{
        /** longueur du paquet**/
        int length=0;
        for(i=0;i<512;i++){
           if(Packet[i]!=0){
            length++;
           }
           else break;
        }
        /**
                       2 bytes     2 bytes      n bytes
                        ----------------------------------
                      | Opcode |   Block #  |   Data     |
                       ----------------------------------
        **/
        int i=4;
        char data[100];
        while(Packet[i]!=0){

        data[i-4]=Packet[i];
        i++;}
        /*  for(i=0;i<length+4;i++){
        printf("\n recieved data %x", data[i]);
            }*/
        //return codeData;                                           //codeData defined as 3
        return TFTPSERV_OK;
        break;}

    /**Trame ACK**/

    /** 2 bytes     2 bytes
        ---------------------
      | Opcode |   Block #  |
       ---------------------
    **/
    case ACK :{
//To DO !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        /*if(acquittement reçu ne correspond pas à celui attendu){
           printf("ACK as not Excpected ");
           return TFTPSERV_ERR;                                     // if ack as not excpected the funct° returns ERR = 10
           break;}
        else {*/
            printf("\n \t \t !!!!!!!!!ACK Received!!!!!!!!\n");
            //return ACK;                                                 // ACK defined as 4
            return TFTPSERV_OK;                                     // returns 10 if everything is OK
            break;
            //}
            }

    case RQW :{
        printf("\n Error : TFTPSERV_ILLEGAL_OPERATION \n");
        exit(0);
        return TFTPSERV_ILLEGAL_OPERATION;                      //the server can't handle writing operations ERR = 4
        break;}

    case ERROR :{
        printf("\n Error : TFTPSERV_ERR \n");
        exit(0);
        return TFTPSERV_ERR;                                    //the client sends an error msg and the funct° returns ERR = 10
        break;}

    default :{
        printf("\n TFTPSERV_UNEXPECTED_PACKET \n");
        exit(0);
        return TFTPSERV_UNEXPECTED_PACKET;                      //the server receives an unexpected packet ERR = 8
        break;}


}

//return 1;
}

/**traitement du cas ou le fichier est de moins de 512 octets**/

int test_file(char filename[80]){
   FILE *fp;

   int length;
    //printf("\n filename is %s \n",filename);
    fp= fopen(filename,"r");
    if(fp==NULL){
        printf("\n impossible d'ouvrir le fichier\n");
    }
   else {
      fseek(fp,0,SEEK_END);
      length=ftell(fp);
      printf("\n the file's length is %1dB\n",length);
      fclose(fp);
   }

    return length;
}

/**Reponse a la requete **/

int TFTP_Packet(char* filename){
    int i,j,leng;
    int length;
    char * buffer = 0;
    FILE * f = fopen (filename, "rb");

    length=test_file(filename);
    buffer = malloc (length);
    /**fichier inferieur a 512*/
    if (length<=512){
            //on recopie le contenu du fichier dans le buffer
    fread (buffer, 1, length, f);
    fclose (f);
    paquet[0]=0;
    paquet[1]= codeData;
    paquet[2]=0;
    paquet[3]=bloc;

    for(i=4;i<length+4;i++){
            paquet[i]=buffer[i-4];
    }
    }
    /** fichier superieur a 512*/

    else if(length>512){


    //verifier si la taille peut etre divisee par 512, si c'est un float il faut prevoir un packet de plus pour stocker le reste des donnees inferieures a 512
    if (length%512==0) n=length/512;
    else{n=length/512 +1;
        lastpac=length%512;
    }
     // n packets seront envoyes

     printf("\n n is: %d \n",n);
     for(i=0;i<n;i++){
            paquets[i][0]=0;
            paquets[i][1]= codeData;
            paquets[i][2]=0;
            paquets[i][3]=bloc;

        for(j=4;j<512+4;j++){
            fread (buffer, 1, length, f);
            fclose(f);
            //if(i==n-1 & ){}
            paquets[i][j]=buffer[j-4+i*512];

        }
       /** examples  (2eme paquet) i=1 : paquets[1][4]=buffer[4-4+1*512] => paquets[1][4]=buffer[512]
                    (3eme paquet)i=2 : paquets[2][5]=buffer[5-4+2*512]=> paquets[1][4]=buffer[1025]*/
     }}

if (length<=512){
    printf("\n one paquet\n");
    for(i=0;i<length+4;i++){
    printf("%x", paquet[i]);}
}
else {
    printf("\n multiple paquets\n");
     for(i=0;i<n-1;i++){
           printf("\n paquet number %d\n",i+1);
        for(j=0;j<512+4;j++){
            printf("%x", paquets[i][j]);

        }
        printf("\n");
        }
     if(i==n-1){
             printf("\n paquet number %d\n",i+1);
              printf("\n length of the last packet is: %d\n",lastpac);
             for(j=0;j<lastpac+4;j++){
            printf("%x", paquets[i][j]);

        }
        printf("\n");
        }

        }


    /**Send response to client*/
   if (length<=512){ bytesSent= sendto(serv,paquet,length+4,0,(struct sockaddr *) &clientAddr, len);}
   else {
        for(i=0;i<n-1;i++){

            bytesSent= sendto(serv,paquets[i],516,0,(struct sockaddr *) &clientAddr, len);}
        if(i==n-1){

            bytesSent=sendto(serv,paquets[i],lastpac+4,0,(struct sockaddr *) &clientAddr, len);
        }

        }


    if (bytesSent == SOCKET_ERROR)
    {
        iResult = WSAGetLastError();
        printf("\n Error: %d\n",iResult);
        return 0;
    }
    //printf("\n Number of bytes sent is %d \n",bytesSent);

    return 1;
}
/** envoi d'un fichier superieur a 512 octets*/
