// This is for testing if clflush is constant cycle instruction
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static inline void clflush(void *v)
{
    asm volatile("clflush 0(%0)" : : "r"(v) :);
}

static inline uint32_t rdtsc()
{
    uint32_t rv;
    asm volatile("rdtsc" : "=a"(rv)::"edx");
    return rv;
}

void dummy()
{
    int x = 0;
    x++;
}

uint32_t mean(uint32_t *arr, int n)
{
    uint64_t avg = 0;
    for (int i = 0; i < n; i++)
    {
        avg += arr[i];
    }
    return (uint32_t)(avg / n);
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: ./test-clflush <NumOfExperiments>");
        return 1;
    }

    int n = atoi(argv[1]);

    // Get the address of the dummy function
    void (*dummy_ptr)() = dummy;

    uint32_t results[n];

    // Test1: Cache Miss
    clflush(dummy_ptr);
    for (int i = 0; i < n; i++)
    {
        uint32_t start = rdtsc();
        clflush(dummy_ptr);
        uint32_t stop = rdtsc();
        results[i] = stop - start;
    }
    uint32_t miss = mean(results, n);
    printf("Miss: \t%u\n", miss);

    // Test2: Cache Hit
    for (int i = 0; i < n; i++)
    {
        dummy();
        uint64_t start = rdtsc();
        clflush(dummy_ptr);
        uint64_t stop = rdtsc();
        results[i] = stop - start;
    }
    uint32_t hit = mean(results, n);
    printf("Hit: \t%u\n", hit);

    printf("Diff: \t%d\n", hit - miss);

    return 0;
}