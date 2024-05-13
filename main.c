#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include <dirent.h>
#include <signal.h>
#include <arpa/inet.h>
#include <omp.h>
#include "bmp_create.h"

void levag(char *str) {
    int len = strlen(str);
    str[len - 16] = '\0';
}

void SendViaFile(int *Values, int NumValues)
{
    pid_t pid = FindPID();

    //    for (int i =0; i<NumValues; i++){
    //         printf("%d\n",Values[i]);
    //    }

    if (pid == -1)
    {
        printf("Hiba a testver folyamat megtalalasaban\n");
        free(Values);
        exit(9);
    }
    char* utvonal = strcat(getenv("HOME"), "/Measurement.txt");
    FILE *file = fopen(utvonal, "w");

    for (int i = 0; i < NumValues; i++)
    {
        fprintf(file, "%d\n", Values[i]);
    }
    fclose(file);
    kill(pid, SIGUSR1);
    //sprintf(utvonal, "%.*s",(int) (strlen(utvonal) - 16), utvonal);
    levag(utvonal);
    printf("Sikeres kuldes\n");
}

void ReceiveViaFile(int sig)
{

    if (sig != SIGUSR1)
    {
        printf("\nNem megfelelo jel\n");
        exit(3);
    }
    FILE *file = NULL;
    char* utvonal = NULL;
    utvonal = strcat(getenv("HOME"), "/Measurement.txt");
    file = fopen(utvonal,"r");

    if (file == NULL)
    {
        fprintf(stderr, "\nHiba a falj megnyitasa kozben!\n");
        exit(4);
    }
    int tmp;
    int NumValues = 0;
    int *Values = NULL;
    while (fscanf(file, "%d", &tmp) != EOF)
    {
        NumValues++;
        Values = realloc(Values, NumValues * sizeof(int));
        Values[NumValues - 1] = tmp;
    }
    fclose(file);
    BMPcreator(Values, NumValues);
    free(Values);
    //sprintf(utvonal, "%.*s",(int) (strlen(utvonal) - 16), utvonal);
    levag(utvonal);
    printf("Sikeres fogadas \nBmp falj elkeszitve\n\n\n");
}

void ReceiveViaSocket()
{
    int *Values = NULL;
    int receive;
    int bytes;
    int err;
    int flag;
    char on;
    unsigned int server_size;
    unsigned int client_size;
    struct sockaddr_in server;
    struct sockaddr_in client;
    on = 1;
    flag = 0;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT_NO);
    server_size = sizeof(server);
    client_size = sizeof(client);
    s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s < 0)
    {
        fprintf(stderr, "Socket creation error.\n");
        exit(5);
    }

    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &on, sizeof on);
    setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, &on, sizeof on);
    err = bind(s, (struct sockaddr *)&server, server_size);
    
    if (err < 0)
    {
        fprintf(stderr, "Binding error.\n");
        exit(8);
    }
    while (1)
    { // Continuous server operation

        printf("\nWaiting for a message...\n");
        signal(SIGUSR1, SignalHandeler);

        bytes = recvfrom(s, &receive, sizeof(receive), flag, (struct sockaddr *)&client, &client_size);

        if (bytes < 0)
        {
            fprintf(stderr, "Receiving error.\n");
            exit(7);
        }
        puts("Number of values received");

        bytes = sendto(s, &receive, sizeof(receive), flag, (struct sockaddr *)&client, client_size);
        puts("Connection established");

        if (bytes <= 0)
        {

            fprintf(stderr, "Sending error.\n");
            exit(6);
        }

        Values = malloc(receive * sizeof(int));

        bytes = recvfrom(s, Values, receive * sizeof(int), flag, (struct sockaddr *)&client, &client_size);

        if (bytes < 0)
        {
            free(Values);
            fprintf(stderr, "Receiving error.\n");
            exit(7);
        }

        BMPcreator(Values, receive);
        puts("Bmp falj elkeszitve");
        free(Values);
        bytes = sendto(s, &receive, sizeof(receive), flag, (struct sockaddr *)&client, client_size);
        if (bytes <= 0)
        {
            fprintf(stderr, "Sending error.\n");
            exit(6);
        }
    }
}

void SendViaSocket(int *Values, int NumValues)
{
    int valasz;
    int s;
    int bytes;
    int flag = 0;
    char on = 1;
    unsigned int server_size;
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_port = htons(PORT_NO);
    server_size = sizeof(server);
    s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s < 0)
    {
        free(Values);
        fprintf(stderr, "Socket creation error.\n");
        exit(5);
    }
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &on, sizeof on);
    setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, &on, sizeof on);
    bytes = sendto(s, &NumValues, 4, flag, (struct sockaddr *)&server, server_size);
    if (bytes <= 0)
    {
        free(Values);
        fprintf(stderr, "Sending error at connection\n\n");
        exit(6);
    }
    puts("NumValues elkuldese sikeres");
    signal(SIGALRM, SignalHandeler);
    alarm(1);

    bytes = recvfrom(s, &valasz, sizeof(long), flag, (struct sockaddr *)&server, &server_size);
    if (bytes < 0)
    {
        free(Values);
        fprintf(stderr, "Receiving error at connection\n\n");
        exit(7);
    };
    if (NumValues != valasz)
    {
        puts("Rossz szerver!");
        exit(11);
    }
    valasz = 0;
    puts("Csatlakozas megszakitva");

    bytes = sendto(s, Values, NumValues * sizeof(int), flag, (struct sockaddr *)&server, server_size);
    if (bytes <= 0)
    {
        free(Values);
        fprintf(stderr, "Sending error at values\n");
        exit(6);
    }
    printf("Az adatok sikeresen el lettek kuldve\n");
    bytes = recvfrom(s, &valasz, sizeof(long), flag, (struct sockaddr *)&server, &server_size);
    printf("Szerver %d db szamot kapott\n", valasz);
    if (bytes <= 0)
    {
        free(Values);
        fprintf(stderr, "Receive error at check num\n");
        exit(7);
    }
    if (NumValues != valasz)
    {
        puts("Nem egzeznek az ertekek!");
        exit(12);
    }
    close(s);
}

int main(int argc, char *argv[])
{
    srand(time(NULL));
    // BMPcreator(values, num_values);
    int kuldo = 1;
    int fajl = 1;
    signal(SIGINT, SignalHandeler);

    if (strcmp(argv[0], PROGRAM_NEV) != 0)
    {
        printf("Hiba! A programnak ennek kellene lennie: '%s'\n", PROGRAM_NEV);
        return 1;
    }

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "--version") == 0)
        {
            verzio();
            return 0;
        }
        else if (strcmp(argv[i], "--help") == 0)
        {
            help();
            return 0;
        }
        else if (strcmp(argv[i], "-send") == 0)
        {
            kuldo = 1;
        }
        else if (strcmp(argv[i], "-receive") == 0)
        {
            kuldo = 0;
        }
        else if (strcmp(argv[i], "-file") == 0)
        {
            fajl = 1;
        }
        else if (strcmp(argv[i], "-socket") == 0)
        {
            fajl = 0;
        }
        else
        {
            printf("Hiba! Az argumentum nemjo '%s'\n", argv[i]);
            help();
            return 1;
        }
    }

    if (kuldo && fajl)
    {
        // printf("Kuldes \n");
        int *Values;
        int NumValues = Measurement(&Values);

        SendViaFile(Values, NumValues);
        free(Values);
    }
    else if (kuldo && !fajl)
    {
        // printf("Kuldes \n");
        int *Values;
        int NumValues = Measurement(&Values);
        SendViaSocket(Values, NumValues);
        free(Values);
    }
    else if (!kuldo && fajl)
    {
        // printf("Fogadas \n");
        while (1)
        {
            printf("Varakozas a jelre\n");
            signal(SIGUSR1, ReceiveViaFile);
            pause();
        }
    }
    else
    {
        // printf("Fogadas \n");
        ReceiveViaSocket();
    }

    return 0;
}
