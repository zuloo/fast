#include <stdlib.h>
#include <stdio.h>

int main(void);

void printSpeed(int speed);

// read all the stuff from the pipe
char *inputString(FILE* fp, size_t size);

// which letter to paint red
int pivotLetter(int l);

// strpbrk backwards, data marks minimal index
char *backwards(char *data, char *index,const char *accept);

// get the number of bytes in utf8-char
int byteInChar(unsigned char v);

// fast read looper
void fastread(char *data, int x, int y, int speed);

