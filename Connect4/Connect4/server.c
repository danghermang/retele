#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>

#define PORT 5888
#define GALBEN -1
#define LIBER 0
#define ROSU 1

/* codul de eroare returnat de anumite apeluri */
extern int errno;

typedef struct thData{
	int idThread;
	int player_yellow;
  int player_red;
  int board[6][7];
}thData;

struct thData array_of_matches[100];
int id = 0, signal_close = 0;

static void *prepare_match(void *); /* functia executata de fiecare thread ce realizeaza comunicarea cu clientii */
void start_game(void *);
void mutare(int board[6][7], char rasp[25]);
void afisare_board(int board[6][7]);
void initial_board(int board[6][7]);

void handle_quit (int quit){
  printf("am ajuns aici : id = %i\n", id);
  int i;
  char mesaj_shutdown[60];
  bzero(mesaj_shutdown, 64);
  strcpy(mesaj_shutdown, "Deconectare: Serverul s-a inchis!\n");
  if(id > 0){
    for(i = 1; i <= id; i++){
      printf("cli 1: %i; cli 2: %i\n", array_of_matches[i].player_yellow, array_of_matches[i].player_red);
      if(write(array_of_matches[i].player_red, mesaj_shutdown, strlen(mesaj_shutdown) + 1) <= 0) {
         perror ("Eroare la write() catre client.");
      }
      if(write(array_of_matches[i].player_yellow, mesaj_shutdown, strlen(mesaj_shutdown) + 1) <= 0) {
         perror ("Eroare la write() catre client.");
      }
      shutdown(array_of_matches[i].player_yellow, 2);
      shutdown(array_of_matches[i].player_red, 2);
    }
    signal_close = 1;
  }
}

pthread_mutex_t block;
int client_redirectat = 0;

int main ()
{
  struct sockaddr_in server;
  struct sockaddr_in from;
  int sd, x, y;
  pthread_t th[100];
  int nr_clienti_acceptati = 0;
  int client_impar, client_par;

  if (pthread_mutex_init(&block, NULL) != 0){
      printf("Eroare la initializarea mutexului\n");
      return 1;
  }

// initializare tabla de sah
  int board[6][7]= {0,0,0,0,0,0,0
  					0,0,0,0,0,0,0
  					0,0,0,0,0,0,0
  					0,0,0,0,0,0,0
  					0,0,0,0,0,0,0
  					0,0,0,0,0,0,0};

  /* crearea unui socket */
  if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
      perror ("[server]Eroare la socket().\n");
      return errno;
    }
  /* utilizarea optiunii SO_REUSEADDR */
  int on=1;
  setsockopt(sd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));

  /* pregatirea structurilor de date */
  bzero (&server, sizeof (server));
  bzero (&from, sizeof (from));

  /* umplem structura folosita de server */
  /* stabilirea familiei de socket-uri */
    server.sin_family = AF_INET;
  /* acceptam orice adresa */
    server.sin_addr.s_addr = htonl (INADDR_ANY);
  /* utilizam un port utilizator */
    server.sin_port = htons (PORT);

  /* atasam socketul */
  if (bind (sd, (struct sockaddr *) &server, sizeof (struct sockaddr)) == -1)
    {
      perror ("[server]Eroare la bind().\n");
      return errno;
    }

  /* punem serverul sa asculte daca vin clienti sa se conecteze */
  if (listen (sd, 2) == -1){
    perror ("[server]Eroare la listen().\n");
    return errno;
  }

  /* servim in mod iterativ clientii */
  printf ("[server]Asteptam la portul %d...\n",PORT);
  fflush (stdout);
  signal(SIGINT, handle_quit);
  while (1){
      if(signal_close == 1){
        break;
      }
      int client;
      char mesaj_asteptare[60], mesaj_pentru_partajare[100];
      thData * td; //parametru functia executata de thread
      int length = sizeof (from);

      /* acceptam un client (stare blocanta pina la realizarea conexiunii) */
      if ( (client = accept (sd, (struct sockaddr *) &from, &length)) < 0){
	       perror ("[server]Eroare la accept().\n");
	       continue;
	    }
      nr_clienti_acceptati++;
      if(nr_clienti_acceptati % 2 != 0){
          client_impar = client;
          bzero(mesaj_asteptare, 64);
          strcpy(mesaj_asteptare, "Esti conectat! Asteapta inca un client pentru a incepe partida.\n");
          if(write(client_impar, mesaj_asteptare, strlen(mesaj_asteptare) + 1) <= 0) {
             perror ("Eroare la write() catre client.");
             printf("Eroare la write() catre clientul %i.\n", client_impar);
          }
      } else if(nr_clienti_acceptati % 2 == 0){
          client_par = client;
          bzero(mesaj_pentru_partajare, 100);
          strcpy(mesaj_pentru_partajare, "Esti conectat! A inceput partida!!!\n Joci cu piesele negre.\n");
          if(write(client_par, mesaj_pentru_partajare, strlen(mesaj_pentru_partajare) + 1) <= 0) {
             perror ("Eroare la write() catre client.");
             printf("Eroare la write() catre clientul %i.\n", client_par);
           }
          bzero(mesaj_pentru_partajare, 100);
          strcpy(mesaj_pentru_partajare, " S-a gasit inca un jucator. A inceput partida!!!\n Joci cu piesele albe.\n");
          if(write(client_impar, mesaj_pentru_partajare, strlen(mesaj_pentru_partajare) + 1) <= 0) {
             perror ("Eroare la write() catre client.");
             printf("Eroare la write() catre clientul %i.\n", client_impar);
          }

          int idThread; //id-ul threadului
          int player_yellow; //descriptorul intors de accept pentru jucatorul impar
          int player_red; //descriptorul intors de accept pentru jucatorul par

          td = (struct thData*)malloc(sizeof(struct thData) * 2);
          td->idThread = id++;
          td->player_yellow = client_impar;
          td->player_red = client_par;
          array_of_matches[id].player_yellow = td->player_yellow;
          array_of_matches[id].player_red = td->player_red;
          //atasare tabla de sah initiala din structura threadului

          for(x = 1; x <= 8; x++){
            for(y = 1; y <= 8; y++){
              if(board[x][y] < 10){
              }
              td->board[x][y] = board[x][y];
            }
          }
          client_redirectat = 0;
          pthread_create(&th[id], NULL, &prepare_match, td);
      }
	 }
   close(sd);
};
static void *prepare_match(void * arg){
		struct thData tdL;
		tdL= *((struct thData*)arg);
		printf ("[thread]- %d - Asteptam mesajul...\n", tdL.idThread);
		fflush (stdout);
    start_game((struct thData*)arg);
		return(NULL);
};

void start_game(void *arg)
{
 	int x, y;
	struct thData tdL;
 	char winner[15], looser[15], reconnect_message[100];
	tdL= *((struct thData*)arg);
  	int client_read;
  	int count_moves = 0;
  	int verif_rege_alb = 0, verif_rege_negru = 0;
  	for( ; ; ){
	    char rasp_client_impar[25], rasp_client_par[25];
	    int ok = 1;
	    //clientul impar muta
	    client_read = read(tdL.player_yellow, rasp_client_impar, 26);
	  	if (client_read < 0){
	  	  printf("[Thread %d]\n",tdL.idThread);
	  	  perror ("Eroare la read() de la client.\n");
	    } else if(client_read == 0){
	      shutdown(tdL.player_yellow, 2);
	        strcpy(winner, "You won!!!");
	        if(write (tdL.player_red, winner, 15) <= 0){
	           printf("[Thread %d] ",tdL.idThread);
	           perror ("[Thread]Eroare la write() catre client.\n");
	        }
	        shutdown(tdL.player_red, 2);
	      printf("Partida %i terminata!\n", tdL.idThread);
	      break;
	    }

	    mutare(&tdL.board, rasp_client_impar);
	    
	}

    if(verif_rege_alb != 1 || verif_rege_negru != 1){
      strcpy(winner, "You won!!!");
      if(write (tdL.player_yellow, winner, 15) <= 0){
         printf("[Thread %d] ",tdL.idThread);
         perror ("[Thread]Eroare la write() catre client.\n");
      }
      shutdown(tdL.player_yellow, 2);
      strcpy(looser, "You lost!!!");
      if(write (tdL.player_red, looser, 15) <= 0){
         printf("[Thread %d] ",tdL.idThread);
         perror ("[Thread]Eroare la write() catre client.\n");
      }
      shutdown(tdL.player_red, 2);
      printf("Partida %i terminata!\n", tdL.idThread);
      break;
    } else{
      verif_rege_alb = 0;
      verif_rege_negru = 0;
    }
    // count_moves++;
    if (write (tdL.player_red, rasp_client_impar, 26) <= 0){
    	 printf("[Thread %d] ",tdL.idThread);
    	 perror ("[Thread]Eroare la write() catre client.\n");
    }

    // clientul par muta
    client_read = read (tdL.player_red, rasp_client_par, 26);
    if (client_read < 0){
      printf("[Thread %d]\n",tdL.idThread);
      perror ("Eroare la read() de la client.\n");
    } else if(client_read == 0){
        shutdown(tdL.player_red, 2);
      // if(count_moves <= 6){
      //   strcpy(reconnect_message, "Adversarul a abandonat partida. Vei fi redirectat catre un alt joc!\n");

      //   pthread_mutex_lock(&block);
      //     client_redirectat = tdL.player_yellow;
      //     if(write (tdL.player_yellow, reconnect_message, 100) <= 0){
      //        printf("[Thread %d] ",tdL.idThread);
      //        perror ("[Thread]Eroare la write() catre client.\n");
      //     }
      //   pthread_mutex_unlock(&block);

      // } else{
        strcpy(winner, "You won!!!");
        if(write (tdL.player_yellow, winner, 15) <= 0){
           printf("[Thread %d] ",tdL.idThread);
           perror ("[Thread]Eroare la write() catre client.\n");
        }
      shutdown(tdL.player_yellow, 2);
      printf("Partida %i terminata!\n", tdL.idThread);
      break;
    } 

    mutare(&tdL.board, rasp_client_par);

    for(x = 1; x <= 8; x++){
      for(y = 1; y <= 8; y++){
        if(tdL.board[x][y] == REGE_ALB){
          verif_rege_alb = 1;
        } else if(tdL.board[x][y] == REGE_NEGRU){
          verif_rege_negru = 1;
        }
      }
    }

    if(verif_rege_alb != 1 || verif_rege_negru != 1){
      strcpy(winner, "You won!!!");
      if(write (tdL.player_red, winner, 15) <= 0){
         printf("[Thread %d] ",tdL.idThread);
         perror ("[Thread]Eroare la write() catre client.\n");
      }
     shutdown(tdL.player_red, 2); 
      strcpy(looser, "You lost!!!");
      if(write (tdL.player_yellow, looser, 15) <= 0){
         printf("[Thread %d] ",tdL.idThread);
         perror ("[Thread]Eroare la write() catre client.\n");
      }
      shutdown(tdL.player_yellow, 2);
      printf("Partida %i terminata!\n", tdL.idThread);
      break;
    } else{
      verif_rege_alb = 0;
      verif_rege_negru = 0;
    }

    if (write (tdL.player_yellow, rasp_client_par, 26) <= 0){
       printf("[Thread %d] ",tdL.idThread);
       perror ("[Thread]Eroare la write() catre client.\n");
    }
    // count_moves++;
  }
}

void afisare_board(int board[6][7]){
  int x, y;
  for(x = 1; x <= 8; x++){
    for(y = 1; y <= 8; y++){
      if(board[x][y] < 10){
        printf(" ");
      }
      printf("%i ", board[x][y]);
    }
    printf("\n");
  }
  printf("\n--TABLA SAH--\n");
}

void initial_board(int board[6][7]){
  
}
