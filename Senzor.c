#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <time.h>
#include <sys/select.h>
#include <arpa/inet.h>

/* codul de eroare returnat de anumite apeluri */
extern int errno;

/* portul de conectare la server*/
int port;

int status;         // starea senzorului: 1 -> ocupat, 0 -> liber
int changed_status; // se schimba cand senzorul isi schimba starea
int alarma_sofer_nou;
double interval_timp = 30.0;

float RandomFloat();
int RandomInt();

int main()
{
    int sd;                    // descriptorul de socket
    struct sockaddr_in server; // structura folosita pentru conectare

    // stabilim portul
    port = 2908;

    /* creem socketul */
    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("Eroare la socket().\n");
        return errno;
    }

    /* umplem structura folosita pentru realizarea conexiunii cu serverul */
    /* familia socket-ului */
    server.sin_family = AF_INET;
    /* adresa IP a serverului */
    server.sin_addr.s_addr = inet_addr("192.168.0.165"); // DE SCHIMBAT CAND TREBUIE
    /* portul de conectare */
    server.sin_port = htons(port);

    // ne conectam la server
    if (connect(sd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
    {
        perror("[client]Eroare la connect().\n");
        return errno;
    }

    // initializam srand
    srand(time(0));

    // timer
    time_t startExec, prezent;
    double timp_trecut;
    time(&startExec);
    // BUCLA INFINITA
    while (1)
    {
        time(&prezent);
        changed_status = 0;
        int forRead;

        fd_set in;
        FD_ZERO(&in);
        FD_SET(sd, &in);

        struct timeval timeout; // interval de timp de asteptare a mesajului de la server
        timeout.tv_sec = 1;     // 1 secunda
        timeout.tv_usec = 0;    // + 0 milisecunde

        forRead = select(sd + 1, &in, NULL, NULL, &timeout);

        // printf("forRead este %d \n", forRead);

        if (forRead < 0)
        {
            perror("[client]Eroare la select().\n");
            return errno;
        }
        else if (forRead > 0)
        {
            // citim
            if (FD_ISSET(sd, &in))
            {
                int desc_read = read(sd, &alarma_sofer_nou, sizeof(int));

                if (desc_read < 0)
                {
                    perror("[client]Eroare la read() de la server.\n");
                    return errno;
                }
                else if (desc_read == 0)
                {
                    printf("[senzor] Serverul a picat, inchidem senzor \n");
                    break;
                }
                else
                {
                    printf("[senzor] Am citit de la server %d\n", alarma_sofer_nou);
                }
            }
        }
        // if alarma_sofer_nou e 1 daca e alarma ca soferul vrea sa parcheze pe locul ala
        if (alarma_sofer_nou != 1)
        {
            timp_trecut = difftime(prezent, startExec);
            if (timp_trecut >= interval_timp)
            {
                if (status != RandomInt())
                {
                    status = 1 - status;
                    changed_status = 1;
                }
                // resetam timer
                time(&startExec);
            }
        }
        else
        {
            printf("Intra sofer nou ? \n");
            // if probabilitate sa parcheze e mai mica de 0.9, change status
            if (RandomFloat() <= 0.9)
            {
                printf("da \n");
                status = 1;
                changed_status = 1;
            } else {
                printf("nu \n");
            }
            alarma_sofer_nou = 0;
        }

        // printf("Senzorul nu e blocat de read \n");
        if (changed_status)
        {
            // Trimitem changed_status la server sa actualizeze harta in caz ca trebuie
            if (write(sd, &changed_status, sizeof(int)) <= 0)
            {
                perror("[client]Eroare la write() spre server.\n");
                return errno;
            }
            printf("[senzor] Am dat schimbare status \n");
        }
    }
    // inchidem conexiunea, am terminat
    close(sd);
}

float RandomFloat()
{
    return ((float)rand() / (float)RAND_MAX);
}

int RandomInt()
{
    return (rand() % 2);
}
