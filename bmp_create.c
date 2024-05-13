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



void verzio()
{
    #pragma omp parallel sections
    {

    #pragma omp section
    {
        printf("Program: %s\n", PROGRAM_NEV);
    }
    #pragma omp section
    {
        printf("Version: %s\n", VERZIO);
    }
    #pragma omp section
    {
        printf("Developer: %s\n", FEJLESZTO);
    }
    #pragma omp section
    {
        printf("Date: %s\n", DATUM);
    }
    }

}

void help()
{
    printf("Usage: %s [OPTIONS]\n", PROGRAM_NEV);
    printf("Options:\n");
    printf("  --version          Verzio informacio \n");
    printf("  --help             Segitseg keres\n");
    printf("  -send              Kuldes (alap)\n");
    printf("  -receive           Fogadas \n");
    printf("  -file              Fajl kommunikacio (alap)\n");
    printf("  -socket            Socket kommunikacio \n");
}
void SignalHandeler(int sig)
{

    if (sig == SIGUSR1)
    {
        printf("\nFalj muvelet nemelerheto!\n");
        exit(3);
    }
    else if (sig == SIGINT)
    {
        printf("\n\nProgram befejezodott\nSSzia\n\n");
        close(s);
        exit(0);
    }
    else if (sig == SIGALRM)
    {
        printf("A szerver nem valaszol\n");
        exit(10);
    }
}
int Measurement(int **Values)
{

    int mp = time(NULL) % 900;
    int meret = mp + 1;
    int *tomb = (int *)malloc(meret * sizeof(int));

    tomb[0] = 0;

    srand(time(NULL));

    double nagyobb = 0.428571;
    double kisebb = 0.3548;

    for (int i = 1; i < meret; i++)
    {
        double n = (double)rand() / RAND_MAX;

        if (n < nagyobb)
        {
            tomb[i] = tomb[i - 1] + 1;
        }
        else if (n < nagyobb + kisebb)
        {
            tomb[i] = tomb[i - 1] - 1;
        }
        else
        {
            tomb[i] = tomb[i - 1];
        }
    }

    *Values = tomb;

    return meret;
}

int FindPID()
{
    DIR *dir;

    struct dirent *asd;
    char path[1000];

    FILE *f;

    char line[100];
    int pid = -1;

    dir = opendir("/proc");

    if (dir == NULL)
    {
        printf("Hiba\n");
        return -1;
    }

    while ((asd = readdir(dir)) != NULL)
    {

        if ((*asd).d_name[0] > '0' && (*asd).d_name[0] < '9')
        {
            sprintf(path, "/proc/%s/status", asd->d_name);
            f = fopen(path, "r");

            if (f == NULL)
            {
                printf("Hiba\n");
                closedir(dir);
                return -1;
            }

            fgets(line, sizeof(line), f);

            if (strncmp(line, "Name:", 5) == 0 && strstr(line, "chart") != NULL)
            {

                while (fgets(line, sizeof(line), f))
                {
                    if (strncmp(line, "Pid:", 4) == 0)
                    {
                        int pid0 = atoi(line + 4);
                        if (pid0 != getpid())
                        {
                            pid = pid0;
                        }
                        break;
                    }
                }
            }
            fclose(f);
        }
    }

    closedir(dir);
    return pid;
}

void BMPcreator(int *Values, int NumValues)
{
    int meret;
    int kep = NumValues;
    int csusztatas = 0;

    if (kep % 32 == 0)
    {
        meret = (kep * kep) / 8 + 62;
    }
    else
    {
        meret = ((kep + (32 - (kep % 32))) * (kep)) / 8 + 62;
        csusztatas = 32 - (kep % 32);
    }

    unsigned char *bitmap = (unsigned char *)calloc(meret, 1);

    bitmap[0] = 'B';
    bitmap[1] = 'M';

    bitmap[2] = meret;
    bitmap[3] = meret >> 8;
    bitmap[4] = meret >> 16;
    bitmap[5] = meret >> 24;

    bitmap[6] = 0b0000000;
    bitmap[7] = 0b0000000;
    bitmap[8] = 0b0000000;
    bitmap[9] = 0b0000000;

    bitmap[10] = 62;
    bitmap[11] = 62 >> 8;
    bitmap[12] = 62 >> 16;
    bitmap[13] = 62 >> 24;

    bitmap[14] = 40;
    bitmap[15] = 40 >> 8;
    bitmap[16] = 40 >> 16;
    bitmap[17] = 40 >> 24;

    bitmap[18] = kep;
    bitmap[19] = kep >> 8;
    bitmap[20] = kep >> 16;
    bitmap[21] = kep >> 24;

    bitmap[22] = kep;
    bitmap[23] = kep >> 8;
    bitmap[24] = kep >> 16;
    bitmap[25] = kep >> 24;

    bitmap[26] = 1;
    bitmap[27] = 1 >> 8;

    bitmap[28] = 1;
    bitmap[29] = 1 >> 8;

    bitmap[30] = 0;
    bitmap[31] = 0 >> 8;
    bitmap[32] = 0 >> 16;
    bitmap[33] = 0 >> 24;

    bitmap[34] = 0;
    bitmap[35] = 0 >> 8;
    bitmap[36] = 0 >> 16;
    bitmap[37] = 0 >> 24;

    bitmap[38] = (char)3937;
    bitmap[39] = (char)3937 >> 8;
    bitmap[40] = (char)3937 >> 16;
    bitmap[41] = (char)3937 >> 24;

    bitmap[42] = (char)3937;
    bitmap[43] = (char)3937 >> 8;
    bitmap[44] = (char)3937 >> 16;
    bitmap[45] = (char)3937 >> 24;

    bitmap[46] = 0;
    bitmap[47] = 0 >> 8;
    bitmap[48] = 0 >> 16;
    bitmap[49] = 0 >> 24;

    bitmap[50] = 0;
    bitmap[51] = 0 >> 8;
    bitmap[52] = 0 >> 16;
    bitmap[53] = 0 >> 24;

    bitmap[54] = 0b00000000;
    bitmap[55] = 0b00000000;
    bitmap[56] = 0b00000000;
    bitmap[57] = 0b11111111;

    bitmap[58] = 0b00000000;
    bitmap[59] = 0b01111100;
    bitmap[60] = 0b00000000;
    bitmap[61] = 0b00000000;

    for (int i = 0; i < NumValues; i++)
    {
        if (Values[i] > (kep / 2) - 2)
        {
            Values[i] = (kep / 2) - 2;
        }
    }

    for (int i = 0; i < NumValues; i++)
    {
        if (Values[i] < -((kep / 2) - 2))
        {
            Values[i] = -((kep / 2) - 2);
        }
    }

    for (int i = 0; i < kep; i++)
    {
        bitmap[62 + ((((kep + csusztatas) / 8) * ((kep / 2 + 1) + Values[i])) + (i / 8))] |= ((char)((int)pow(2, 7 - (i % 8))));
    }

    int f = open("chart.bmp", O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

    if (f == -1)
    {
        printf("Hiba\n");
        exit(3);
    }

    write(f, bitmap, meret);
    close(f);
    free(bitmap);
}
