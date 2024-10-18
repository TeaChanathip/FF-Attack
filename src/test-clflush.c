// This is for testing if clflush is constant cycle instruction
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <x86intrin.h>

void dummy()
{
    int x = 0;
    x++;
}

unsigned long long mean(unsigned long long *arr, int n)
{
    unsigned long long avg = 0;
    for (int i = 0; i < n; i++)
    {
        avg += arr[i];
    }
    return avg/n;
}

void test_miss(void *dummy_ptr, int n)
{
    unsigned long long results[n];
    unsigned int aux;

    _mm_clflush(dummy_ptr);
    _mm_mfence();
    for (int i=0; i<n; i++)
    {   
        unsigned long long start = __rdtscp(&aux);
        _mm_clflush(dummy_ptr);
        _mm_mfence();
        unsigned long long stop = __rdtscp(&aux);
        results[i] = stop - start;
    }
    
    printf("Mean Miss: %llu\n", mean(results, n));
}

void test_hit(void *dummy_ptr, int n)
{
    unsigned long long results[n];
    unsigned int aux;

    for (int i=0; i<n; i++)
    {   
        dummy();
        _mm_mfence();
        unsigned long long start = __rdtscp(&aux);
        _mm_clflush(dummy_ptr);
        _mm_mfence();
        unsigned long long stop = __rdtscp(&aux);
        results[i] = stop - start;
    }
    
    printf("Mean Hit: %llu\n", mean(results, n));
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

    test_miss(dummy_ptr, n);
    test_hit(dummy_ptr, n);

    return 0;
}