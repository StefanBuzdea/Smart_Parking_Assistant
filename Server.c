#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <sys/select.h>

#define PORT 2908
#define NR_SENZORI 8

extern int errno;

typedef struct thData
{
  int idThread; // id-ul thread-ului tinut in evidenta de acest program
  int cl;       // descriptorul intors de accept
} thData;

static void *treat(void *); /* functia executata de fiecare thread ce realizeaza comunicarea cu clientii */
void raspunde(void *arg);
// alte functii de response

int soferNou[102], soferNou2[102], locParcare[102];
int senzori[52], datePentruSofer[53];
int SOFER_NOU, ID_SENZOR, PARK_PLIN, PORNIRE_SENZORI;
int id_thread_sofer;
int changedMap[102];
// MAIN

int main()
{

  struct sockaddr_in server; // structura folosita de server
  struct sockaddr_in from;
  int sd; // descriptorul de socket
  int pid;
  pthread_t th[100]; // Identificatorii thread-urilor care se vor crea
  int i = 0;

  // crearea unui socket
  if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
  {
    perror("[server]Eroare la socket().\n");
    return errno;
  }

  // utilizarea optiunii SO_REUSEADDR pentru refolosire adresa socket
  int on = 1;
  setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

  // pregatirea structurilor de date
  bzero(&server, sizeof(server));
  bzero(&from, sizeof(from));

  // umplem structura folosita de server
  // stabilirea familiei de socket-uri
  server.sin_family = AF_INET;
  // acceptam orice adresa
  server.sin_addr.s_addr = htonl(INADDR_ANY);
  // utilizam un port utilizator
  server.sin_port = htons(PORT);

  /// serverul este executat si se pornesc si clientii senzori automat (5)

  // initializam vectorul soferNou, changed map si locParcare
  for (int j = 1; j <= 100; j++)
  {
    soferNou[j] = 0;
    soferNou2[j] = 0;
    changedMap[j] = 0;
    //locParcare[j] = -1;
  }

  // atasam socketul
  if (bind(sd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
  {
    perror("[server]Eroare la bind().\n");
    return errno;
  }

  // punem serverul sa asculte daca vin clienti sa se conecteze
  if (listen(sd, 2) == -1)
  {
    perror("[server]Eroare la listen().\n");
    return errno;
  }

  // servim in mod concurent clientii...folosind thread-uri
  while (1)
  {
    int client;
    thData *td; // parametru functia executata de thread
    int length = sizeof(from);

    printf("[server]Asteptam la portul %d...\n", PORT);
    fflush(stdout);

    // se asteapta veniri ale clientilor


    // // PORNIM SENZORII AUTOMAT
    // if(PORNIRE_SENZORI == 0) {
    //   for(int e = 0; e < NR_SENZORI; e++) {
    //     if(system("./Senzor") == -1) {
    //       printf("Eroare: Senzorul %d nu s-a pornit", e);
    //     }
    //   }
    //   PORNIRE_SENZORI = 1;
    // }
    //daca nu merge facem script si incercam sa-l apelam din cod


    // client= malloc(sizeof(int));
    // acceptam un client (stare blocanta pana la realizarea conexiunii) //SOFER?
    if ((client = accept(sd, (struct sockaddr *)&from, &length)) < 0)
    {
      perror("[server]Eroare la accept().\n");
      continue;
    }

    // s-a realizat conexiunea, se astepta mesajul

    td = (struct thData *)malloc(sizeof(struct thData));
    td->idThread = i++;
    td->cl = client;

    // VERIFICAM DACA S-A CONECTAT UN SOFER NOU
    if (td->idThread >= NR_SENZORI)
    {
      soferNou[td->idThread] = 1;
      soferNou2[td->idThread] = 1;
    }

    pthread_create(&th[i], NULL, &treat, td);
  } // end while

} // END MAIN

// FUNCTII FOLOSITE

static void *treat(void *arg) /* functia executata de fiecare thread ce realizeaza comunicarea cu clientii */
{
  struct thData tdL;
  tdL = *((struct thData *)arg);
  printf("[thread]- %d - Asteptam mesajul...\n", tdL.idThread);
  fflush(stdout);
  pthread_detach(pthread_self());
  raspunde((struct thData *)arg);
  // am terminat cu acest client, inchidem conexiunea
  // printf("S-a inchis thread-ul %d", tdL.idThread);
  // close((intptr_t)arg);
  return (NULL);
}

void raspunde(void *arg)
{
  int i = 0;
  int senzor_status = 0;
  int vreau_parcare = 1;
  struct thData tdL;
  tdL = *((struct thData *)arg); // INITIALIZAREA VARIABILELOR TREBUIE PUSA AICI?
  while (1)
  {
    if (tdL.idThread < NR_SENZORI)
    { // AVEM DE A FACE CU UN SENZOR

      // FACEM READ NEBLOCANT
      int forReadServer;

      fd_set inS;
      FD_ZERO(&inS);
      FD_SET(tdL.cl, &inS);

      struct timeval timeoutS; // interval de timp de asteptare a mesajului de la server
      timeoutS.tv_sec = 1;     // 1 secunda
      timeoutS.tv_usec = 0;    // + 0 milisecunde

      forReadServer = select(tdL.cl + 1, &inS, NULL, NULL, &timeoutS);

      if (forReadServer == -1)
      {
        perror("[server]Eroare la select().\n");
        // return errno;
      }
      else if (forReadServer)
      {
        // citim
        if (FD_ISSET(tdL.cl, &inS))
        {
          if (read(tdL.cl, &senzor_status, sizeof(int)) < 0)
          {
            perror("[server]Eroare la read() de la senzor.\n");
            // return errno;
          }

          // de la senzor primim un 1 care inseamna s-a schimbat statusul senzorului
          // adica senzor[tdl.idthread] = 1 - senzor[tdl.idthread]; si CHANGED MAP = 1;
          if (senzor_status == 1)
          {
            senzori[tdL.idThread] = 1 - senzori[tdL.idThread];
            for (int z = 0; z <= 100; z++)
            {
              changedMap[z] = 1;
            }
            senzor_status = 0;
          }
        }
      }

      // DOAR PENTRU SENZORUL LIBER CU UN ANUMIT ID SE FACE WRITE
      int k = 0;

      for (k = 0; k < NR_SENZORI; k++)
      {
        if (senzori[k] == 0)
        {
          ID_SENZOR = k;
          PARK_PLIN = 0;
          break;
        }
      }

      // printf("S-a ales senzorul %d \n", ID_SENZOR);

      if (k == NR_SENZORI)
      { // INSEAMNA CA PARCAREA E FULL DECI NU SE MAI TRIMITE ALARMA LA SENZOR PANA NU SE DEBLOCHEAZA MACAR UNUL
        PARK_PLIN = 1;
      }

      if (PARK_PLIN == 0) // DACA PARCAREA NU E PLINA, TRIMITEM MESAJ LA SENZOR CA SOFERUL VREA SA PARCHEZE
      {
        if (SOFER_NOU == 0)
        {
          for (k = NR_SENZORI; k <= 100; k++)
          {
            if (soferNou[k] == 1)
            {
              SOFER_NOU = 1;
              id_thread_sofer = k;
              soferNou[k] = 0;
              break;
            }
          }
        }

       
        // if (SOFER_NOU)
        // {
        //   printf("S-a gasit soferul nou %d si threadul este %d \n", id_thread_sofer, tdL.idThread);
        // }

        // ACTIUNE SUPLIMENTARA:
        /* noi stim ca id_thread_sofer este indicele soferului nou venit care vrea sa se parcheze.
        am putea face cumva, daca el parcheaza, sa ii scriem mesajul: ai parcat pe locul ID_SENZOR, iar dupa
        sa ii trimitem in continuare actualizarile hartii
        */

        if (SOFER_NOU && (ID_SENZOR == tdL.idThread))
        {
          sleep(17); // PAUZA IN CARE SOFERUL SE UITA PE HARTA SI ALEGE UNDE SA PARCHEZE

          //memoram locul de parcare al soferului;
          //locParcare[id_thread_sofer] = tdL.idThread;

          // printf("[thread] Avem sofer nou %d care vrea sa se puna pe locul %d \n", k, ID_SENZOR);
          if (write(tdL.cl, &vreau_parcare, sizeof(int)) <= 0)
          {
            printf("[Thread %d] ", tdL.idThread);
            perror("[Thread]Eroare la write() catre client.\n");
          }
          else
          {
            printf("[Thread %d]Alarma de sofer nou a fost transmisa cu succes catre senzor.\n", tdL.idThread);
            SOFER_NOU = 0;
          }
        }
      }
    }
    else
    { // AVEM DE A FACE CU SOFER

      // returnam mesajul soferului daca e nou sau s-a actualizat harta
      if (changedMap[tdL.idThread] || soferNou2[tdL.idThread])
      {                                        
        // //prelucram datele pe care trebuie sa le trimitem la sofer
        // for(int e = 0; e < NR_SENZORI; e++) {
        //   datePentruSofer[e] = senzori[e];
        // }

        // // pe ultima pozitie a vectorului trimis memoram locul pe care s-a pus soferul respectiv 
        // datePentruSofer[NR_SENZORI] = locParcare[tdL.idThread];

        // if(senzori[locParcare[tdL.idThread]] == 0) {
        //   datePentruSofer[NR_SENZORI] = -1; // a iesit din parcare, nu mai are un loc atribuit
        // }

        // WRITE date pentru sofer (adica vectorul senzori)
        int desc_write = write(tdL.cl, senzori, sizeof(senzori));
        if (desc_write < 0)
        {
          printf("[Thread %d] ", tdL.idThread);
          perror("[Thread]Eroare la write() catre client.\n");
        }
        else if (desc_write == 0)
        {
          printf("S-a deconectat soferul %d \n", tdL.idThread);
        }
        else
        {
          printf("[Thread %d]Harta a fost transmisa cu succes.\n", tdL.idThread);
        }
        // CHANGED_MAP = 0;
        changedMap[tdL.idThread] = 0;
        soferNou2[tdL.idThread] = 0;
      }

      // citim
      int de_citit = 0;

      if (read(tdL.cl, &de_citit, sizeof(int)) <= 0)
      {
        printf("S-a deconectat soferul %d \n", tdL.idThread);
        break;
      }

    }
  }
  close(tdL.cl);
}
