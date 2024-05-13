#ifndef BMP_CREATE_H

#define PROGRAM_NEV "./chart"
#define VERZIO "1.0"
#define FEJLESZTO "Szilagyi Tibor Abel"
#define DATUM "2024-04-29"
#define PORT_NO 3333


static int s; // socket ID
void verzio();
void help();
int Measurement(int **Values);
int FindPID();
void BMPcreator(int *Values, int NumValues);
void SignalHandeler(int sig);
#endif