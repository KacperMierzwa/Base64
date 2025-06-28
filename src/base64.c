#include "base64.h"

#define printElapsedTime(s)                     \
printf(                                         \
    "Execution time: %4.3fs\n",                 \
    ((double)(clock() - (s))) / CLOCKS_PER_SEC  \
);


// tests if file is accessible
uint8_t verifyFile(const char *restrict fName)
{
    // if file doesnt exist
    if(_access(fName, F_OK))
        return 1;

    // if file cant be read
    if(_access(fName, R_OK))
        return 2;

    return 0;
}


size_t getFileSize(FILE *f)
{
    if(fseek(f, 0L, SEEK_END))
    {
        fclose(f);
        return 0;
    }

    size_t r = ftell(f);
    rewind(f);
    return r;
}


// I swear if telthar accuses me of using Chat GPT ONE MORE FUCKING TIME im killing myself
int main(int argc, char **argv)
{
    if(argc < 2 || argc > 7)
    {
        printf("Usage: %s [-d] [-t] [-o <file>] {[-f <file>] | [-s <string>]}\n", argv[0]);
        return 1;
    }

    char *str =     NULL;       // input string
    size_t strSz =  0;          // size of the string
    char *outBuff = NULL;       // output buffer
    size_t outSz =  0;          // size of the buffer

    bool dec =   false;         // decode flag
    bool bench = false;         // benchmark flag (very simplistic)

    // file handles
    FILE *inFile;
    FILE *outFile;

    // filenames
    char fNameIn[_MAX_FNAME]  = {0};
    char fNameOut[_MAX_FNAME] = {0};

    clock_t tStart = 0; // benchmarking
    size_t fSize;       // size of the input file
    opterr = 0;         // no getopt diagnostics

    char opt;
    while((opt = getopt(argc, argv, "hdtf:o:s:")) != -1)
    {
        switch(opt)
        {
            case 'd': dec = true;               break;  // decryption
            case 't': bench = true;             break;  // benchmarking
            case 'f': strcpy(fNameIn, optarg);  break;  // file input
            case 'o': strcpy(fNameOut, optarg); break;  // file output
            case 's':
                strSz = strlen(optarg);
                str = (char *)calloc(strSz + 1, 1);

                if(!str)
                {
                    perror("Memory error\n");
                    return 1;
                }
                
                strcpy(str, optarg);
                break;
            case 'h':
                printf(
                    "Usage: %s [-d] [-t] [-o <file>] {[-f <file>] | [-s <string>]}\n"
                    "-h : display this message\n"
                    "-d : enable decoding (encoding by default)\n"
                    "-s <string> : input string for encoding/decoding\n"
                    "-f <file> : input file for encoding/decoding\n"
                    "-o <file> : output file (required with [-f] option)\n"
                    "-t : measure execution time (in miliseconds)\n"
                    , argv[0]
                );
                
                return 0;
            default: 
                printf("Usage: %s [-d] [-t] {[-f <file>] [-o] <file> | [-s <string>]}\n", argv[0]);
                return 1;
        }
    }

    if(bench)
        tStart = clock(); // start timer


    if(!str && !fNameIn[0])
    {
        perror("No input defined\n");
        return 1;
    }


    /* == String input == */

    if(str)
    {
        // determine output type with dec flag
        outBuff = dec ? (char *)decode(str, strSz, &outSz) : encode((byte *)str, strSz, &outSz);
        free(str);

        if(!outBuff)
        {
            perror(dec ? "error during decoding" : "error during encoding\n");
            return 1;
        }

        /* === File output === */

        if(fNameOut[0])
        {
            if(!(outFile = fopen(fNameOut, "w")))
            {
                free(outBuff);
                perror("Error while creating output file\n");
                return 1;
            }

            printf("\nCreated file: %s\nFile size: %zuB\n", fNameOut, outSz);
            fwrite(outBuff, 1, outSz, outFile);

            fclose(outFile);
        }
        else
            printf("\nReturned string: %s\nstring length: %zu\n", outBuff, outSz);

        free(outBuff);

        if(bench)
            printElapsedTime(tStart);

        return 0;
    }
    

    /* == File input == */

    if(!fNameOut[0])
    {
        perror("Please specify output file\n");
        return 1;
    }

    if(verifyFile(fNameIn))
        return 1;
    
    if(!(inFile = fopen(fNameIn, "rb")))
    {
        perror("Error while opening input file\n");
        return 1;
    }

    fSize = getFileSize(inFile);

    // determine output type with dec flag
    outBuff = (dec ? (char *)decodeFile(inFile, fSize, &outSz) : encodeFile(inFile, fSize, &outSz));
    fclose(inFile);

    if(!outBuff)
    {
        perror(dec ? "Error during file decoding\n" : "error during file encoding\n");
        return 1;
    }

    if(!(outFile = fopen(fNameOut, "w")))
    {
        free(outBuff);
        perror("Error while creating output file\n");
        return 1;
    }

    printf("\nCreated file: %s\nFile size: %zuB\n", fNameOut, outSz);

    fwrite(outBuff, 1, outSz, outFile);

    fclose(outFile);
    free(outBuff);

    if(bench)
        printElapsedTime(tStart);

    return 0;
}