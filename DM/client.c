#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>

#define SIZE 1024

long int get_file_size(FILE* fp)
{
    int currentPosition = ftell(fp);//On sauvegarde la position à laquelle on est dans le fichier
    //à l'appel de la fonction

    fseek(fp, 0, SEEK_END);//On va à la fin du fichier
    int fileSize = ftell(fp);//On demande la position du curseur dans le fichier, qui va donc
    //nous donner le nombre d'octet du fichier
    fseek(fp, currentPosition, SEEK_SET);//On retourne à la position à laquelle on était
    //au début de la fonction

    return fileSize;
}

void send_file(FILE *fp, int sockfd)
{
    char data[SIZE] = {0};

    long int fileSize = get_file_size(fp);//On récupère la taille du fichiers
    if(send(sockfd, &fileSize, sizeof(fileSize), 0) == -1)//On envoie la taille du fichier au serveur
    {
        perror("[-] Error in sending file size");
        exit(1);
    }

    /*
    * Envoi du fichier par bloc de 1024 octets
    */
    long int totalSent = 0;
    while(totalSent < fileSize)
    {
        fgets(data, SIZE, fp);
        int sent = send(sockfd, data, sizeof(data), 0);
        if(sent == -1)
        {
            perror("[-] Error in sending data");
            exit(1);
        }

        totalSent += sent - 1;//-1 pour ne pas compter le zéro terminal récupéré par fgets

        bzero(data, SIZE);
    }

    printf("CLIENT: Attente de l'accusé de réception du serveur\n");
    recv(sockfd, data, sizeof("RECU") - 1, 0);//On attend l'accuse de réception "RECU" de 4 octets du serveur
    printf("CLIENT: Accusé de réception reçu\n");
}

int main()
{
    char *ip = "127.0.0.1";
    int portServer = htons(50000);
    int e;

    int sockfd;
    struct sockaddr_in server_addr;
    FILE *fp;
    char *filename = "file.txt";
     sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd<0)
    {
        perror("[-]Error in socket");
        exit(1);
    }
     printf("[+] Server socket created. \n");

     server_addr.sin_family = AF_INET;
     server_addr.sin_port = portServer;
     server_addr.sin_addr.s_addr = inet_addr(ip);

     e = connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr));
     if(e == -1)
     {
         perror("[-]Error in Connecting");
         exit(1);
     }
     printf("[+]Connected to server.\n");
     fp = fopen(filename, "r");
     if(fp == NULL)
     {
         perror("[-]Error in reading file.");
         exit(1);
     }
     send_file(fp,sockfd);
     close(sockfd);
     printf("**-----__ Disconnected from the server. __-----**\n");
     return 0;

}
