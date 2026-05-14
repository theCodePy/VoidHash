// Compile with: gcc host_benchmark.c -o host_benchmark -O3 -lpthread -lssl -lcrypto -Wno-deprecated-declarations

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include <x86intrin.h> // For __rdtsc() hardware cycle counting

// OpenSSL libraries for identical hash operations
#include <openssl/sha.h>

// Hash Size Macros to match EDK II
#define MD5_DIGEST_SIZE    16
#define SHA1_DIGEST_SIZE   20
#define SHA256_DIGEST_SIZE 32
#define SHA384_DIGEST_SIZE 48
#define SHA512_DIGEST_SIZE 64

// --- Custom Unrolled MD5 (Replaces OpenSSL for 1-to-1 comparison) ---

// MD5 magic math macros
#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | (~z)))
#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32-(n))))

#define FF(a, b, c, d, x, s, ac) { \
    (a) += F((b), (c), (d)) + (x) + (uint32_t)(ac); \
    (a) = ROTATE_LEFT((a), (s)); \
    (a) += (b); \
  }
#define GG(a, b, c, d, x, s, ac) { \
    (a) += G((b), (c), (d)) + (x) + (uint32_t)(ac); \
    (a) = ROTATE_LEFT((a), (s)); \
    (a) += (b); \
  }
#define HH(a, b, c, d, x, s, ac) { \
    (a) += H((b), (c), (d)) + (x) + (uint32_t)(ac); \
    (a) = ROTATE_LEFT((a), (s)); \
    (a) += (b); \
  }
#define II(a, b, c, d, x, s, ac) { \
    (a) += I((b), (c), (d)) + (x) + (uint32_t)(ac); \
    (a) = ROTATE_LEFT((a), (s)); \
    (a) += (b); \
  }

// The highly unrolled core transformation
void FastMd5Transform(uint32_t state[4], const uint8_t block[64]) {
    uint32_t a = state[0], b = state[1], c = state[2], d = state[3], x[16];
    memcpy(x, block, 64); // Fast memory copy into local registers

    // Round 1
    FF(a, b, c, d, x[ 0],  7, 0xd76aa478); FF(d, a, b, c, x[ 1], 12, 0xe8c7b756);
    FF(c, d, a, b, x[ 2], 17, 0x242070db); FF(b, c, d, a, x[ 3], 22, 0xc1bdceee);
    FF(a, b, c, d, x[ 4],  7, 0xf57c0faf); FF(d, a, b, c, x[ 5], 12, 0x4787c62a);
    FF(c, d, a, b, x[ 6], 17, 0xa8304613); FF(b, c, d, a, x[ 7], 22, 0xfd469501);
    FF(a, b, c, d, x[ 8],  7, 0x698098d8); FF(d, a, b, c, x[ 9], 12, 0x8b44f7af);
    FF(c, d, a, b, x[10], 17, 0xffff5bb1); FF(b, c, d, a, x[11], 22, 0x895cd7be);
    FF(a, b, c, d, x[12],  7, 0x6b901122); FF(d, a, b, c, x[13], 12, 0xfd987193);
    FF(c, d, a, b, x[14], 17, 0xa679438e); FF(b, c, d, a, x[15], 22, 0x49b40821);

    // Round 2
    GG(a, b, c, d, x[ 1],  5, 0xf61e2562); GG(d, a, b, c, x[ 6],  9, 0xc040b340);
    GG(c, d, a, b, x[11], 14, 0x265e5a51); GG(b, c, d, a, x[ 0], 20, 0xe9b6c7aa);
    GG(a, b, c, d, x[ 5],  5, 0xd62f105d); GG(d, a, b, c, x[10],  9, 0x02441453);
    GG(c, d, a, b, x[15], 14, 0xd8a1e681); GG(b, c, d, a, x[ 4], 20, 0xe7d3fbc8);
    GG(a, b, c, d, x[ 9],  5, 0x21e1cde6); GG(d, a, b, c, x[14],  9, 0xc33707d6);
    GG(c, d, a, b, x[ 3], 14, 0xf4d50d87); GG(b, c, d, a, x[ 8], 20, 0x455a14ed);
    GG(a, b, c, d, x[13],  5, 0xa9e3e905); GG(d, a, b, c, x[ 2],  9, 0xfcefa3f8);
    GG(c, d, a, b, x[ 7], 14, 0x676f02d9); GG(b, c, d, a, x[12], 20, 0x8d2a4c8a);

    // Round 3
    HH(a, b, c, d, x[ 5],  4, 0xfffa3942); HH(d, a, b, c, x[ 8], 11, 0x8771f681);
    HH(c, d, a, b, x[11], 16, 0x6d9d6122); HH(b, c, d, a, x[14], 23, 0xfde5380c);
    HH(a, b, c, d, x[ 1],  4, 0xa4beea44); HH(d, a, b, c, x[ 4], 11, 0x4bdecfa9);
    HH(c, d, a, b, x[ 7], 16, 0xf6bb4b60); HH(b, c, d, a, x[10], 23, 0xbebfbc70);
    HH(a, b, c, d, x[13],  4, 0x289b7ec6); HH(d, a, b, c, x[ 0], 11, 0xeaa127fa);
    HH(c, d, a, b, x[ 3], 16, 0xd4ef3085); HH(b, c, d, a, x[ 6], 23, 0x04881d05);
    HH(a, b, c, d, x[ 9],  4, 0xd9d4d039); HH(d, a, b, c, x[12], 11, 0xe6db99e5);
    HH(c, d, a, b, x[15], 16, 0x1fa27cf8); HH(b, c, d, a, x[ 2], 23, 0xc4ac5665);

    // Round 4
    II(a, b, c, d, x[ 0],  6, 0xf4292244); II(d, a, b, c, x[ 7], 10, 0x432aff97);
    II(c, d, a, b, x[14], 15, 0xab9423a7); II(b, c, d, a, x[ 5], 21, 0xfc93a039);
    II(a, b, c, d, x[12],  6, 0x655b59c3); II(d, a, b, c, x[ 3], 10, 0x8f0ccc92);
    II(c, d, a, b, x[10], 15, 0xffeff47d); II(b, c, d, a, x[ 1], 21, 0x85845dd1);
    II(a, b, c, d, x[ 8],  6, 0x6fa87e4f); II(d, a, b, c, x[15], 10, 0xfe2ce6e0);
    II(c, d, a, b, x[ 6], 15, 0xa3014314); II(b, c, d, a, x[13], 21, 0x4e0811a1);
    II(a, b, c, d, x[ 4],  6, 0xf7537e82); II(d, a, b, c, x[11], 10, 0xbd3af235);
    II(c, d, a, b, x[ 2], 15, 0x2ad7d2bb); II(b, c, d, a, x[ 9], 21, 0xeb86d391);

    state[0] += a; state[1] += b; state[2] += c; state[3] += d;
}

bool FastMd5HashAll(const void *Data, size_t DataSize, uint8_t *HashValue) {
    uint32_t state[4] = {0x67452301, 0xefcdab89, 0x98badcfe, 0x10325476};
    uint8_t buffer[64];
    size_t i = 0;
    uint64_t totalBits = (uint64_t)DataSize * 8; 

    // 1. Process all full 64-byte blocks directly from memory
    for (i = 0; i + 63 < DataSize; i += 64) {
        FastMd5Transform(state, (const uint8_t *)Data + i);
    }

    // 2. Handle the remaining bytes
    size_t remaining = DataSize - i;
    memcpy(buffer, (const uint8_t *)Data + i, remaining);

    // 3. Padding Step 1: Append the mandatory 0x80 byte
    buffer[remaining] = 0x80;
    remaining++;

    // 4. Padding Step 2: If we don't have exactly 8 bytes left for the length, 
    // we must pad to 64, process it, and start a fresh final block.
    if (remaining > 56) {
        memset(buffer + remaining, 0, 64 - remaining);
        FastMd5Transform(state, buffer);
        remaining = 0; // Reset for the final block
    }

    // 5. Padding Step 3: Pad with zeros up to byte 56
    memset(buffer + remaining, 0, 56 - remaining);

    // 6. Padding Step 4: Append the original length in bits (64-bit integer)
    memcpy(buffer + 56, &totalBits, 8);

    // 7. Final transform
    FastMd5Transform(state, buffer);

    // 8. Output the 16-byte result
    memcpy(HashValue, state, 16);
    return true;
}

// --- EDK II Hash Function Wrappers ---
// We wrap OpenSSL functions to perfectly match your UEFI function signatures
bool Sha1HashAll(const void *Data, size_t DataSize, uint8_t *HashValue) {
    SHA1((const unsigned char*)Data, DataSize, HashValue); return true;
}
bool Sha256HashAll(const void *Data, size_t DataSize, uint8_t *HashValue) {
    SHA256((const unsigned char*)Data, DataSize, HashValue); return true;
}
bool Sha384HashAll(const void *Data, size_t DataSize, uint8_t *HashValue) {
    SHA384((const unsigned char*)Data, DataSize, HashValue); return true;
}
bool Sha512HashAll(const void *Data, size_t DataSize, uint8_t *HashValue) {
    SHA512((const unsigned char*)Data, DataSize, HashValue); return true;
}

// --- Multi-Threading Task Structure (Identical to UEFI AP_TEST_TASK) ---
typedef struct {
    char *StartPtr;
    char *EndPtr;
    bool (*HashAlgo)(const void *, size_t, uint8_t *);
    size_t Iterations;
    uint64_t HashesComputed;
} THREAD_TEST_TASK;

// --- The Core Worker Function ---
void* ThreadHashingFunction(void *Buffer) {
    THREAD_TEST_TASK *Task = (THREAD_TEST_TASK *)Buffer;
    char *CurrentWord;
    uint8_t GeneratedHash[64];
    size_t WordLen;
    size_t i;

    Task->HashesComputed = 0;

    for (i = 0; i < Task->Iterations; i++) {
        CurrentWord = Task->StartPtr;
        
        while (CurrentWord < Task->EndPtr) {
            // Skip empty lines safely
            if (*CurrentWord == '\0') {
                CurrentWord++;
                continue;
            }
            
            WordLen = strlen(CurrentWord);
            Task->HashAlgo(CurrentWord, WordLen, GeneratedHash);
            
            Task->HashesComputed++;
            CurrentWord += WordLen + 1; // Jump to next word
        }
    }
    return NULL;
}

// --- Helper: Load File to RAM ---
bool loadFileToRam(const char *filename, void **buffer, size_t *fileSize) {
    FILE *f = fopen(filename, "rb");
    if (!f) {
        printf("[!] Error: Could not open file %s\n", filename);
        return false;
    }
    
    fseek(f, 0, SEEK_END);
    *fileSize = ftell(f);
    rewind(f);
    
    // Allocate buffer with +1 padding to ensure a final null terminator
    *buffer = calloc(*fileSize + 1, 1);
    if (!*buffer) {
        printf("[!] Fatal: RAM Allocation Failed\n");
        fclose(f);
        return false;
    }
    
    fread(*buffer, 1, *fileSize, f);
    fclose(f);
    return true;
}

// --- Main Program ---
int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("\nWrong command!!!\n Usage: %s <algorithm> <wordlist> <iterations>\n", argv[0]);
        return 1;
    }

    char *hashAlgoStr = argv[1];
    char *wordListName = argv[2];
    size_t ittr = strtoull(argv[3], NULL, 10);

    if (ittr == 0) {
        printf("\nZero iterations: nothing to do!!!\n\n");
        return 1;
    }

    bool (*HashAlgo)(const void *, size_t, uint8_t *);
    size_t hashsize = 0;

    // Route algorithms
    if (strcmp(hashAlgoStr, "md5") == 0) { HashAlgo = FastMd5HashAll; hashsize = MD5_DIGEST_SIZE; } 
    else if (strcmp(hashAlgoStr, "sha1") == 0) { HashAlgo = Sha1HashAll; hashsize = SHA1_DIGEST_SIZE; } 
    else if (strcmp(hashAlgoStr, "sha256") == 0) { HashAlgo = Sha256HashAll; hashsize = SHA256_DIGEST_SIZE; } 
    else if (strcmp(hashAlgoStr, "sha512") == 0) { HashAlgo = Sha512HashAll; hashsize = SHA512_DIGEST_SIZE; } 
    else if (strcmp(hashAlgoStr, "sha384") == 0) { HashAlgo = Sha384HashAll; hashsize = SHA384_DIGEST_SIZE; } 
    else {
        printf("\nOnly supported algorithms: md5, sha1, sha256, sha384, sha512\n\n");
        return 1;
    }

    printf("loading file...\n");
    void *FileBuffer = NULL;
    size_t fileSize;
    
    if (!loadFileToRam(wordListName, &FileBuffer, &fileSize)) {
        return 1;
    }
    
    char *fBuffer = (char *)FileBuffer;
    printf("fileSize=%zu\n", fileSize);

    printf("formatting the file...\n");
    // Replace \n with \0 exactly like UEFI
    for (size_t i = 0; i < fileSize; i++) {
        if (fBuffer[i] == '\n') {
            fBuffer[i] = '\0';
        }
    }

    // --- OS MULTI-THREADING SETUP ---
    // sysconf queries the OS for the number of active CPU cores
    long NumProcessors = sysconf(_SC_NPROCESSORS_ONLN);
    if (NumProcessors < 1) NumProcessors = 1;

    pthread_t *threads = malloc(sizeof(pthread_t) * NumProcessors);
    THREAD_TEST_TASK *Tasks = calloc(NumProcessors, sizeof(THREAD_TEST_TASK));

    // Calculate memory chunks
    size_t ChunkSize = fileSize / NumProcessors;
    char *ChunkCursor = fBuffer;

    for (int i = 0; i < NumProcessors; i++) {
        Tasks[i].StartPtr = ChunkCursor;
        Tasks[i].HashAlgo = HashAlgo;
        Tasks[i].Iterations = ittr;
        Tasks[i].HashesComputed = 0;

        if (i == NumProcessors - 1) {
            Tasks[i].EndPtr = fBuffer + fileSize;
        } else {
            char *TempEnd = ChunkCursor + ChunkSize;
            while (*TempEnd != '\0' && TempEnd < (fBuffer + fileSize)) TempEnd++;
            Tasks[i].EndPtr = TempEnd;
            ChunkCursor = TempEnd + 1;
        }
    }

    printf("Benchmarking across %ld Active CPU Cores (OS Level)...\n", NumProcessors);

    uint64_t totalHash = 0;
    struct timespec start, end;
    uint64_t StartCycles, EndCycles, TotalCycles;

    // START THE CLOCKS
    clock_gettime(CLOCK_MONOTONIC, &start); // High-res OS timer
    StartCycles = __rdtsc();                // Raw hardware CPU cycles

    // 1. Wake all threads
    for (int i = 0; i < NumProcessors; i++) {
        pthread_create(&threads[i], NULL, ThreadHashingFunction, &Tasks[i]);
    }

    // 2. Wait for all threads to finish (Synchronization)
    for (int i = 0; i < NumProcessors; i++) {
        pthread_join(threads[i], NULL);
    }

    // STOP CLOCKS
    EndCycles = __rdtsc();
    clock_gettime(CLOCK_MONOTONIC, &end);

    // Sum up all the hashes processed by every core
    for (int i = 0; i < NumProcessors; i++) {
        totalHash += Tasks[i].HashesComputed;
    }

    // --- CALCULATE PERFORMANCE ---
    TotalCycles = EndCycles - StartCycles;
    uint64_t CyclesPerHash = 0;
    if (totalHash > 0) {
        CyclesPerHash = (TotalCycles / totalHash);
    }

    // Convert OS monotonic time natively to nanoseconds (Bypasses UEFI Frequency math!)
    uint64_t TotalTimeNs = (end.tv_sec - start.tv_sec) * 1000000000ULL + (end.tv_nsec - start.tv_nsec);
    
    uint64_t TimeUs  = TotalTimeNs / 1000;
    uint64_t TimeMs  = TimeUs / 1000;
    uint64_t TimeSec = TimeMs / 1000;

    uint64_t HashesPerSec = 0;
    uint64_t HashesPerMs  = 0;

    if (TotalTimeNs > 0) {
        HashesPerSec = (totalHash * 1000000000ULL) / TotalTimeNs;
        HashesPerMs  = (totalHash * 1000000ULL) / TotalTimeNs;
    }

    // --- REPORTING ---
    printf("\n--- OS Benchmark Results ---\n");
    printf("Total Hashes: %lu   \n", totalHash);
    printf("Total CPU Cycles: %lu    ", TotalCycles);
    
    if (totalHash > 0) {
        printf("    Speed: %lu Cycles/Hash\n", CyclesPerHash);
    }
    printf("-------------------------\n");

    printf("Time Elapsed: %lu sec | %lu ms | %lu us | %lu ns\n", TimeSec, TimeMs, TimeUs, TotalTimeNs);
    
    if (TotalTimeNs > 0) {
        printf("Performance:  %lu Hash/Sec  |  %lu Hash/ms\n", HashesPerSec, HashesPerMs);
    } else {
        printf("Performance:  Execution too fast to measure rate!\n");
    }

    // --- SAVE TO CSV ---
    FILE *csv = fopen("benchmark_results_os.csv", "a");
    if (csv) {
        // If file is empty/new, write headers
        fseek(csv, 0, SEEK_END);
        if (ftell(csv) == 0) {
            fprintf(csv, "hashname,wordlist_Name,cycles/hash,hash/sec,hash/Mili-Sec\n");
        }
        
        fprintf(csv, "%s,%s,%lu,%lu,%lu\n", 
            hashAlgoStr, wordListName, CyclesPerHash, HashesPerSec, HashesPerMs);
        
        fclose(csv);
        printf("Results appended to benchmark_results_os.csv successfully!\n");
    } else {
        printf("Failed to save CSV results.\n");
    }

    // Cleanup
    free(threads);
    free(Tasks);
    if (FileBuffer) free(FileBuffer);

    return 0;
}