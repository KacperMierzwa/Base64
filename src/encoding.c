#include "base64.h"

static const char base64[64] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";


// encode 3 bytes of data into 4 bytes of base64
inline static void encodeChunk(const byte *restrict in, char *restrict out)
{
    out[0] = base64[(in[0] >> 2)];                                  // first 6 bits
    out[1] = base64[((in[0] & 0x03) << 4) | ((in[1] & 0xf0) >> 4)]; // rest 2 bits + 4 bits from next element
    out[2] = base64[((in[1] & 0x0f) << 2) | ((in[2] & 0xc0) >> 6)]; // rest 4 bits + 2 next
    out[3] = base64[(in[2] & 0x3f)];                                // last 4 bits
}

// returns allocated buffer with encoded data with optional param that recieves size of the buffer
char *encode(const byte *restrict data, const size_t size, size_t *restrict destSize)
{
    if(!size || !data)
        return NULL;

    const uint8_t pad   = size % 3;
    const size_t destSz = ceilf((float)size / 3.0f) * 4 + 1; // crazy memory alignment cause im retarded perfectionist

    char *encoded  = (char *)calloc(destSz, 1);

    if(!encoded)
        return NULL;
    
    // if provided a pointer, it assigns a size of decoded string to it
    if(destSize)
        *destSize = destSz - 1;
    
    char *r = encoded; // we will return this value since we shift pointer during encoding


    // if size isnt divisible by 3 add '=' for padding
    for(size_t i = 0; i < size - pad; i += 3)
    {
        encodeChunk(data, encoded);
        data += 3;
        encoded += 4;
    }

    if(!pad)
        return r;

    // last bytes + padding
    encoded[0] = base64[(data[0] >> 2)];
    encoded[1] = base64[((data[0] & 0x03) << 4) | ((data[1] & 0xf0) >> 4)];
    encoded[2] = (pad == 1 ? '=' : base64[((data[1] & 0x0f) << 2) | ((data[2] & 0xc0) >> 6)]);
    encoded[3] = '=';
    encoded[4] = '\0';

    return r;
}

// encodes fSize bytes from thed opened file
char *encodeFile(FILE *inFile, const size_t fSize, size_t *restrict destSize)
{             
    byte *fBuff = calloc(fSize, 1);

    if(!inFile || !fSize || !fBuff)
        return NULL;

    fread(fBuff, 1, fSize, inFile);

    char *ret = encode(fBuff, fSize, destSize);
    free(fBuff);

    return ret;
}