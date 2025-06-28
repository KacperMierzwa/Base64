#include "base64.h"

// to decode a base64 chunk we use this hand made decoding table
// using ascii value of a char - 43 as an index in this table
// which is easier and (not tested) faster than anything used before
// the number 43 comes from the smallest value of a base64 char, which belongs to '+'
static const byte decodeTable[80] = 
{
    62,  0,  0,  0, 63, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61,  0,  0,  0,  0,  0,
     0,  0,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17,
    18, 19, 20, 21, 22, 23, 24, 25,  0,  0,  0,  0,  0,  0, 26, 27, 28, 29, 30, 31,
    32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51
};

// gets index of the character in the base64 table
static inline uint8_t transl(const char c) { return decodeTable[c - 43]; }


static inline void decodeChunk(const char *restrict in, byte *restrict out)
{
    out[0] = (transl(in[0]) << 2) | (transl(in[1]) >> 4);
    out[1] = (transl(in[1]) << 4) | (transl(in[2]) >> 2);
    out[2] = (transl(in[2]) << 6) | (transl(in[3]));
}

byte *decode(const char *restrict data, const size_t size, size_t *restrict destSize)
{
    if(!size || !data)
        return NULL;

    const uint8_t er    = 3 - ((size - 1) % 4); // stupid proof design for when string is not the correct size (not perfect)
    const uint8_t pad   = (data[size - 1] == '=') + (data[size - 2] == '='); // number of '=' chars
    const size_t destSz = ceilf((float)(size - er)  / 4.0f) * 3 + 1; // another memory fuckery

    byte *decoded = (byte *)calloc(destSz, 1);

    if(!decoded)
        return NULL;

    if(destSize)
        *destSize = destSz - pad - 1;
    
    byte *r = decoded;


    for(size_t i = 0; i < size - er - pad; i += 4)
    {
        decodeChunk(data, decoded);
        data += 4;
        decoded += 3;
    }

    if(!pad)
        return r;


    decoded[0] = (transl(data[0]) << 2) | (transl(data[1]) >> 4);
    decoded[1] = (transl(data[1]) << 4) | (transl(data[2]) >> 2);
    decoded[2] = (transl(data[2]) << 6) | (pad == 1 ? 0 : transl(data[3]));
    decoded[3] = '\0';

    return r;
}


byte *decodeFile(FILE *inFile, const size_t fSize, size_t *restrict destSize)
{
    char *fBuff = calloc(fSize, 1);
    
    if(!inFile || !fSize || !fBuff)
        return NULL;

    fread(fBuff, 1, fSize, inFile);

    byte *ret = decode(fBuff, fSize, destSize);
    free(fBuff);
    return ret;
}