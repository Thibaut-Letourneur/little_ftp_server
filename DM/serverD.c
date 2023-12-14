#include <arpa/inet.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>//Pour umask()
#include <unistd.h>//Pour STDIN_FILENO etc...

#define SIZE 1024

void readFileSizeFromSocket(int socket, long int* fileSize)
{
    long int totalReadBytes = 0;

    /*
    * On va lire dans la socket la taille du fichier envoyée par le client
    octet par octet et l'écrire octet par octet dans le pointeur fileSize
    */
    while(totalReadBytes < sizeof(long int))//< sizeof(long int) car on sait que la taille du fichier envoyée
    //par le client est codée sur un long int donc on va lire des octets dans la socket jusqu'à avoir lu
    //autant d'octets que compose un long int
    {
        unsigned char byteBuffer = 0;//Buffer qui va contenir l'octet lu dans la socket

        if(recv(socket, &byteBuffer, 1, 0) == -1)//On lit 1 octet
            exit(1);

        *((unsigned char*)fileSize + totalReadBytes++) = byteBuffer;//On écrit dans l'octet numéro
        //totalReadBytes du pointeur fileSize l'octet qu'on vient de lire dans la socket
    }
}

void write_file(int sockfd)
{
    int n;
    FILE *fp;
    char filename[128]; sprintf(filename, "file2-%d.txt", getpid());//On ajoute à "file2.txt" le PID du fils
    //Permet de faciliter le repérage des fichiers créés lorsque plusieurs clients sont créés en même temps
    char buffer[SIZE];

    fp = fopen(filename, "w");
    if(fp==NULL)
        exit(1);

    long int fileSize = 0;
    readFileSizeFromSocket(sockfd, &fileSize);//On récupère la taille du fichier
    //qui a été envoyée par le client

    printf("SERVEUR: Taille reçue: %ld\n", fileSize);

    long int totalReadBytes = 0;

    while(totalReadBytes < fileSize)//Tant qu'on a pas lu autant d'octet que la taille du fichier
    {
        n = recv(sockfd, buffer, SIZE, 0);

        if(n<=0)
            break;

        totalReadBytes += n - 1;//On ajoute au nombre total d'octets lu le
        //nombre d'octet que contient le buffer après lecture dans la socket
        //-1 pour ne pas compter le zéro terminal à la fin de la chaîne qui
        //a été rajouté par 'fgets' côté client

        fprintf(fp, "%s", buffer);
        bzero(buffer, SIZE);
    }
    printf("SERVEUR: Tous les octets reçus\n");

    //On a fini de lire le fichier, on envoi un accusé de réception au client
    char accuseReception[] = "RECU";
    send(sockfd, accuseReception, sizeof(accuseReception) - 1, 0);
    printf("SERVEUR: Accusé envoyé\n");

    fclose(fp);
    return;
}

void make_daemon()
{
    int returned = fork();

    if(returned < 0)//Erreur de fork
        exit(EXIT_FAILURE);
    if(returned > 0)
        exit(0);//On termine le père. A pour effet de faire penser au terminal que la commande
        // ./server est terminée

    umask(0);//Tous les fichiers qui seront créés par le serveur daemon seront accessibles
    //en lecture, écriture et exécution

    returned = setsid();//Déconnecte le processus du terminal qui l'a lancé
    if(returned == -1)//Erreur pendant le setsid
        exit(EXIT_FAILURE);

    //La pratique courante est de changer le répertoire de travail du processus daemon pour éviter qu'il ne bloque
    //un répertoire du système (ce répertoire ne pourrait, par exemple, ni être déplacé ni supprimé).
    //Notre processus étant un serveur FTP, il est plus pratique pour nos tests de transfert de fichiers de ne pas
    //déplacer le processus.
    /*
        returned = chdir("/");
        if(returned == -1)//Erreur pendant le changement de répertoire
            exit(EXIT_FAILURE);
    */

    ///On ferme stdin, stdout et stderr car le daemon n'en aura pas besoin. Cela évitera d'éventuelles erreurs futures.
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
}

int main ()
{
    //make_daemon();

    char *ip = "127.0.0.1";
    int port = htons(50000);
    int e;

    int sockfd, new_sock;
    struct sockaddr_in server_addr, new_addr;
    socklen_t addr_size;
    char buffer[SIZE];

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd<0)
        exit(1);

     server_addr.sin_family = AF_INET;
     server_addr.sin_port = port;
     server_addr.sin_addr.s_addr = inet_addr(ip);

     e = bind(sockfd,(struct sockaddr*)&server_addr, sizeof(server_addr));
     if(e<0)
         exit(1);

     e = listen(sockfd, 10);
     if(e!=0)
         exit(1);
     addr_size = sizeof(new_addr);

     signal(SIGCHLD, SIG_IGN);//On ignore le SIG_CHILD pour ne pas avoir de defunct
     while(1)
     {
        new_sock = accept(sockfd,(struct sockaddr*)&new_addr, &addr_size);

        int pid = fork();//Fork pour créer un processus de travail en parallèle
        if(pid < 0)
            exit(EXIT_FAILURE);

        if(pid == 0)//fils
        {
            close(sockfd);//On ferme la socket d'écoute du serveur car le fils n'en a pas besoin

            write_file(new_sock);

            close(new_sock);//Fermeture de la socket de travail. Le travail est terminé

            exit(0);
        }

        close(new_sock);//Fermeture de la socket de travail, le serveur en a pas besoin
     }
}
