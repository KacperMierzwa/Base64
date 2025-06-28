#ifndef BASE64_H
#define BASE64_H

#include <stdio.h>
#include <malloc.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>


typedef unsigned char byte;

char *encode(const byte *restrict data, const size_t size, size_t *restrict destSize);
char *encodeFile(FILE *inFile, const size_t fSize, size_t *restrict destSize);

byte *decode(const char *restrict data, const size_t size, size_t *restrict destSize);
byte *decodeFile(FILE *inFile, const size_t fSize, size_t *restrict destSize);

#endif