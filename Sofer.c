#include <ncurses.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/select.h>

#define NR_SENZORI 8

// primeste harta actualizata imediat ce e rulat

/* codul de eroare returnat de anumite apeluri */
extern int errno;

/* portul de conectare la server*/
int port;
int harta[53];
int in_viata = 1;
int NR_LOCURI_LIBERE;
int LinTerm, ColTerm;

void afisare();
void dreptunghi_loc(int y1, int x1, int y2, int x2, int nr_loc);

int main(int argc, char *argv[])
{

  int sd;                    // descriptorul de socket
  struct sockaddr_in server; // structura folosita pentru conectare

  // stabilim portul
  port = 2908;

  // cream socketul
  if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
  {
    perror("Eroare la socket().\n");
    return errno;
  }

  // umplem structura folosita pentru realizarea conexiunii cu serverul
  // familia socket-ului
  server.sin_family = AF_INET;
  // adresa IP a serverului
  server.sin_addr.s_addr = inet_addr("192.168.0.165"); // DE SCHIMBAT CAND TREBUIE
  // portul de conectare
  server.sin_port = htons(port);

  // ne conectam la server
  if (connect(sd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
  {
    perror("[sofer]Eroare la connect().\n");
    return errno;
  }

  // pregatire interfata
  initscr();
  start_color();                          // initializare culori
  init_pair(1, COLOR_GREEN, COLOR_BLACK); // creare pereche culoare locuri libere
  init_pair(2, COLOR_RED, COLOR_BLACK);   // creare pereche culoare locuri ocupate
  init_pair(3, COLOR_YELLOW, COLOR_BLACK); //creare culoare pentru intrare si iesire

  // calculare marimi terminal pentru centrare interfata
  getmaxyx(stdscr, LinTerm, ColTerm);

  while (1)
  {

    // FACEM READ NEBLOCANT
    int forReadSofer;

    fd_set inS;
    FD_ZERO(&inS);
    FD_SET(sd, &inS);

    struct timeval timeoutS; // interval de timp de asteptare a mesajului de la server
    timeoutS.tv_sec = 1;     // 1 secunda
    timeoutS.tv_usec = 0;    // + 0 milisecunde

    forReadSofer = select(sd + 1, &inS, NULL, NULL, &timeoutS);

    if (forReadSofer == -1)
    {
      perror("[server]Eroare la select().\n");
      // return errno;
    }
    else if (forReadSofer)
    {
      // citim
      if (FD_ISSET(sd, &inS))
      {
        if (read(sd, harta, sizeof(harta)) < 0)
        {
          perror("[senzor]Eroare la read().\n");
          // return errno;
        }

        // Print harta
        afisare();
      }
    }

    //

    // un write de verificare daca e conectat clientul
    if (write(sd, &in_viata, sizeof(int)) <= 0)
    {
      perror("[client]Eroare la write() spre server.\n");
      return errno;
    }
  }

  // inchidere interfata
  endwin();
  // inchidem conexiunea, am terminat
  close(sd);
}

//functii interfata

void dreptunghi_loc(int y1, int x1, int y2, int x2, int nr_loc)
{
  mvhline(y1, x1, 0, x2 - x1);
  mvhline(y2, x1, 0, x2 - x1);
  mvvline(y1, x1, 0, y2 - y1);
  mvvline(y1, x2, 0, y2 - y1);
  mvaddch(y1, x1, ACS_ULCORNER);
  mvaddch(y2, x1, ACS_LLCORNER);
  mvaddch(y1, x2, ACS_URCORNER);
  mvaddch(y2, x2, ACS_LRCORNER);
  if (nr_loc <= 8)
  { // e numar de loc
    mvprintw((y1 + y2) / 2, (x1 + x2) / 2, "%d", nr_loc);
  }
  else if (nr_loc == 9)
  { // e casuta de intrare
    mvprintw((y1 + y2) / 2, x1+2, "ENTR");
  }
  else
  { // e casuta de iesire
    mvprintw((y1 + y2) / 2, x1+2, "EXIT");
  }
}

void afisare()
{
  clear();
  int lungime = NR_SENZORI;
  int x = ColTerm / 2 - 14;
  int y1 = 3;
  int y2 = 7;

  // locuri
  NR_LOCURI_LIBERE = 0;

  for (int z2 = 0; z2 < lungime; z2++)
  {
    if (harta[z2] == 0)
    {
      NR_LOCURI_LIBERE++;
    }
  }
  // centrare text: 30 de spatii (80/2 - 21/2)
  printw("\n                              ");
  printw("Locuri disponibile: %d", NR_LOCURI_LIBERE);

  mvhline(2, 0, ' ', COLS);

  // harta
  for (int i = 0; i < lungime; i++)
  {
    if (harta[i] == 0)
    {
      attron(COLOR_PAIR(1)); // culoare verde pentru locuri libere
      dreptunghi_loc(y1, x, y2, x + 4, i + 1);
      attroff(COLOR_PAIR(1));
    }
    else
    {
      attron(COLOR_PAIR(2)); // culoare rosie pentru locuri ocupate
      dreptunghi_loc(y1, x, y2, x + 4, i + 1);
      attroff(COLOR_PAIR(2));
    }

    x += 8; // spatiere

    // pentru 10 senzori
    if (i == 3)
    {

      // cod intrare + iesire
      attron(COLOR_PAIR(3));
      dreptunghi_loc(y2, 12, y2+6, 19, 9);
      dreptunghi_loc(y2, 60, y2+6, 67, 10);
      attroff(COLOR_PAIR(3));

      // strada principala care delimiteaza casutele 1-4 de casutele de mai jos 5-8
      mvwhline(stdscr, y2 + 1, 20, ACS_HLINE, 40);

      // pentru linie intrerupta
      for (int f = 21; f <= 59; f += 4)
      {
        mvwhline(stdscr, y2 + 3, f, ACS_HLINE, 2);
      }

      mvwhline(stdscr, y2 + 5, 20, ACS_HLINE, 40);

      //actualizare coordonate pentru a creea casutele de pe linia 2
      x = ColTerm / 2 - 14;
      y1 = y2 + 6;
      y2 = y1 + 4;
    }
  }

  // linie necesara pentru a delimita casutele de outputul de mai jos
  mvhline(y2 + 1, 0, ' ', COLS);
  printw("\n\n                   ");
  printw("Pentru a parasi aplicatia, apasati Ctrl + C\n\n");
  refresh();
}
