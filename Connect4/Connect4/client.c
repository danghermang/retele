/* cliTCPIt.c - Exemplu de client TCP
   Trimite un nume la server; primeste de la server "Hello nume".
*/
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

/* codul de eroare returnat de anumite apeluri */
extern int errno;

/* portul de conectare la server*/
int port;
int tabla[6][7]
int main (int argc, char *argv[])
{
  int sd;			// descriptorul de socket
  struct sockaddr_in server;	// structura folosita pentru conectare 
  char msg[64];		// mesajul trimis
  char msg2[100];
  int first_client = 0;

  /* exista toate argumentele in linia de comanda? */
  if (argc != 3)
    {
      printf ("Sintaxa: %s <adresa_server> <port>\n", argv[0]);
      return -1;
    }

  /* stabilim portul */
  port = atoi (argv[2]);

  /* cream socketul */
  if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
      perror ("Eroare la socket().\n");
      return errno;
    }

  /* umplem structura folosita pentru realizarea conexiunii cu serverul */
  /* familia socket-ului */
  server.sin_family = AF_INET;
  /* adresa IP a serverului */
  server.sin_addr.s_addr = inet_addr(argv[1]);
  /* portul de conectare */
  server.sin_port = htons (port);
  
  /* ne conectam la server */
  if (connect (sd, (struct sockaddr *) &server,sizeof (struct sockaddr)) == -1)
    {
      perror ("[client]Eroare la connect().\n");
      return errno;
    }

  /* citirea mesajului de inceput*/
  if (read (sd, msg, 100) < 0){
    perror ("[client]Eroare la read() de la server.\n");
    return errno;
  }
  printf("%s\n", msg);
  if(strstr(msg, "Asteapta") != NULL){
    bzero (msg, 100);
    fflush (stdout);
    first_client = 1;
    if (read (sd, msg, 100) < 0){
      perror ("[client]Eroare la read() de la server.\n");
      return errno;
    } 
    printf("%s\n", msg);
  }
  // efectuarea mutarilor
  for( ; ; ){
      char *mutare_client_impar, *mutare_client_par, raspuns_pentru_client_impar[100], raspuns_pentru_client_par[100];
      size_t buff_size = 25;
      fflush (stdout);
      if(first_client == 1){

            mutare_client_impar = (char *)malloc(buff_size * sizeof(char));
            fflush(stdin);
            getline(&mutare_client_impar, &buff_size, stdin);
            if (write (sd, mutare_client_impar, 26) <= 0){
              perror ("[client]Eroare la write() spre server.\n");
              return errno;
            }

          if (read (sd, raspuns_pentru_client_impar, 100) < 0){
              perror ("[client]Eroare la read() de la server.\n");
              return errno;
          } else {
            printf("Mesaj primit: %s\n", raspuns_pentru_client_impar);
          }
        if(strstr(raspuns_pentru_client_impar, "won") != NULL || strstr(raspuns_pentru_client_impar, "lost") != NULL){
          break;
        }
      } else{
        if (read (sd, raspuns_pentru_client_par, 100) < 0){
          perror ("[client]Eroare la read() de la server.\n");
          return errno;
        } else{
          printf("Mesaj primit: %s\n", raspuns_pentru_client_par);
        }
        if(strstr(raspuns_pentru_client_par, "won") != NULL || strstr(raspuns_pentru_client_par, "lost") != NULL){
          break;
        } else{
          mutare_client_par = (char *)malloc(buff_size * sizeof(char));
          fflush(stdin);
          getline(&mutare_client_par, &buff_size, stdin);
          if (write (sd, mutare_client_par, 26) <= 0){
            perror ("[client]Eroare la write() spre server.\n");
            return errno;
          }
        }
      }
  }
  close (sd);
}
