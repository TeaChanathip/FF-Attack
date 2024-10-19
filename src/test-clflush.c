// This is for testing if clflush is constant cycle instruction
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <x86intrin.h>

void delayloop(uint32_t cycles)
{
    unsigned int aux;
    uint64_t start = __rdtscp(&aux);
    while ((__rdtscp(&aux) - start) < cycles)
        ;
}

void dummy()
{
    int x = 0;
    x++;
    x++;
    x++;
    x++;
    x++;
    x++;
    x++;
    x++;
    x++;
    x++;
}

// Function to compare two unsigned long long integers for qsort
int compare_ull(const void *a, const void *b)
{
    unsigned long long int_a = *(unsigned long long *)a;
    unsigned long long int_b = *(unsigned long long *)b;
    if (int_a < int_b)
        return -1;
    if (int_a > int_b)
        return 1;
    return 0;
}

unsigned long long median(unsigned long long *sorted_arr, int n)
{
    if (n % 2 == 0)
    {
        return (sorted_arr[n / 2 - 1] + sorted_arr[n / 2]) / 2;
    }
    else
    {
        return sorted_arr[n / 2];
    }
}

double variance(unsigned long long *arr, int n)
{
    unsigned long long sum = 0;
    for (int i = 0; i < n; i++)
    {
        sum += arr[i];
    }
    double mean = (double)sum / n;

    double variance_sum = 0;
    for (int i = 0; i < n; i++)
    {
        variance_sum += ((double)arr[i] - mean) * ((double)arr[i] - mean);
    }
    return variance_sum / n;
}

unsigned long long mean(unsigned long long *arr, int n)
{
    unsigned long long avg = 0;
    for (int i = 0; i < n; i++)
    {
        avg += arr[i];
    }
    return avg / n;
}

volatile void perform_test(int n)
{
    // Allocate memory for results
    unsigned long long *hit_results = malloc(n * sizeof(unsigned long long));
    unsigned long long *miss_results = malloc(n * sizeof(unsigned long long));

    // Get the address of the dummy function
    void (*dummy_ptr)() = dummy;

    unsigned int aux;
    unsigned long long start, stop;
    for (int i = 0; i < n; i++)
    {
        delayloop(1000);
        dummy(); // Make sure that dummy will be in the cache

        // Test hit
        start = __rdtscp(&aux);
        _mm_clflush(dummy_ptr);
        _mm_mfence();
        stop = __rdtscp(&aux);
        hit_results[i] = stop - start;

        // Make sure that dummy will NOT be in the cache
        _mm_clflush(dummy_ptr);
        _mm_mfence();

        // Test miss
        start = __rdtscp(&aux);
        _mm_clflush(dummy_ptr);
        _mm_mfence();
        stop = __rdtscp(&aux);
        miss_results[i] = stop - start;
    }

    // Caculate Min, Max, Mean, Median, Variance of each result arrays
    printf("====== Hit ======\n");
    qsort(hit_results, n, sizeof(unsigned long long), compare_ull);
    printf("Min: %llu\n", hit_results[0]);
    printf("Max: %llu\n", hit_results[n - 1]);
    printf("Mean: %llu\n", mean(hit_results, n));
    printf("Median: %llu\n", median(hit_results, n));
    printf("Variance: %f\n", variance(hit_results, n));

    printf("\n====== Miss ======\n");
    qsort(miss_results, n, sizeof(unsigned long long), compare_ull);
    printf("Min: %llu\n", miss_results[0]);
    printf("Max: %llu\n", miss_results[n - 1]);
    printf("Mean: %llu\n", mean(miss_results, n));
    printf("Median: %llu\n", median(miss_results, n));
    printf("Variance: %f\n", variance(miss_results, n));

    // Free results
    free(hit_results);
    free(miss_results);
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: ./test-clflush <NumOfExperiments>\n");
        return 1;
    }

    int n = atoi(argv[1]);

    perform_test(n);

    return 0;
}