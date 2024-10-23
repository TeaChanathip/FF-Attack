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

// Function to compare two integers for qsort
int compare_ull(const void *a, const void *b)
{
    int32_t int_a = *(int32_t *)a;
    int32_t int_b = *(int32_t *)b;
    if (int_a < int_b)
        return -1;
    if (int_a > int_b)
        return 1;
    return 0;
}

int32_t median(int32_t *sorted_arr, int n)
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

double variance(int32_t *arr, int n)
{
    int32_t sum = 0;
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

int32_t mean(int32_t *arr, int n)
{
    int32_t avg = 0;
    for (int i = 0; i < n; i++)
    {
        avg += arr[i];
    }
    return avg / n;
}

volatile void perform_test(int n)
{
    // Allocate memory for results
    int32_t *hit_results = malloc(n * sizeof(int32_t));
    int32_t *miss_results = malloc(n * sizeof(int32_t));

    // Get the address of the dummy function
    void (*dummy_ptr)() = dummy;

    unsigned int aux;
    for (int i = 0; i < n; i++)
    {
        delayloop(1000);

        // Make sure that dummy will be in the cache
        asm volatile("movq (%0), %%rax\n"
                     :
                     : "c"(dummy_ptr)
                     : "rax");
        asm volatile("movq (%0), %%rax\n"
                     :
                     : "c"(dummy_ptr)
                     : "rax");

        // Test hit
        asm volatile(
            "mfence\n"
            "lfence\n"
            "rdtscp\n"
            "mov %%eax, %%esi\n"
            "clflush (%1)\n"
            "mfence\n"
            "rdtscp\n"
            "lfence\n"
            "sub %%esi, %%eax\n"
            : "=&a"(hit_results[i]) : "r"(dummy_ptr) : "ecx", "edx", "esi");

        // Make sure that dummy will NOT be in the cache
        _mm_clflush(dummy_ptr);
        _mm_mfence();

        // Test miss
        asm volatile(
            "mfence\n"
            "lfence\n"
            "rdtscp\n"
            "mov %%eax, %%esi\n"
            "clflush (%1)\n"
            "mfence\n"
            "rdtscp\n"
            "lfence\n"
            "sub %%esi, %%eax\n"
            : "=&a"(miss_results[i]) : "r"(dummy_ptr) : "ecx", "edx", "esi");
    }

    // Caculate Min, Max, Mean, Median, Variance of each result arrays
    printf("====== Hit ======\n");
    qsort(hit_results, n, sizeof(int32_t), compare_ull);
    printf("Min: %u\n", hit_results[0]);
    printf("Max: %u\n", hit_results[n - 1]);
    printf("Mean: %u\n", mean(hit_results, n));
    printf("Median: %u\n", median(hit_results, n));
    printf("Variance: %f\n", variance(hit_results, n));

    printf("\n====== Miss ======\n");
    qsort(miss_results, n, sizeof(int32_t), compare_ull);
    printf("Min: %u\n", miss_results[0]);
    printf("Max: %u\n", miss_results[n - 1]);
    printf("Mean: %u\n", mean(miss_results, n));
    printf("Median: %u\n", median(miss_results, n));
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