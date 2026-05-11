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
#include <openssl/md5.h>
#include <openssl/sha.h>

// Hash Size Macros to match EDK II
#define MD5_DIGEST_SIZE    16
#define SHA1_DIGEST_SIZE   20
#define SHA256_DIGEST_SIZE 32
#define SHA384_DIGEST_SIZE 48
#define SHA512_DIGEST_SIZE 64

// --- EDK II Hash Function Wrappers ---
// We wrap OpenSSL functions to perfectly match your UEFI function signatures
bool Md5HashAll(const void *Data, size_t DataSize, uint8_t *HashValue) {
    MD5((const unsigned char*)Data, DataSize, HashValue); return true;
}
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
    if (strcmp(hashAlgoStr, "md5") == 0) { HashAlgo = Md5HashAll; hashsize = MD5_DIGEST_SIZE; } 
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
    uint64_t HashesPerUs  = 0;

    if (TotalTimeNs > 0) {
        HashesPerSec = (totalHash * 1000000000ULL) / TotalTimeNs;
        HashesPerUs  = (totalHash * 1000000ULL) / TotalTimeNs;
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
        printf("Performance:  %lu Hash/Sec  |  %lu Hash/us\n", HashesPerSec, HashesPerUs);
    } else {
        printf("Performance:  Execution too fast to measure rate!\n");
    }

    // --- SAVE TO CSV ---
    FILE *csv = fopen("benchmark_results_os.csv", "a");
    if (csv) {
        // If file is empty/new, write headers
        fseek(csv, 0, SEEK_END);
        if (ftell(csv) == 0) {
            fprintf(csv, "hashname,wordlist_Name,cycles/hash,hash/sec,hash/microSec\n");
        }
        
        fprintf(csv, "%s,%s,%lu,%lu,%lu\n", 
            hashAlgoStr, wordListName, CyclesPerHash, HashesPerSec, HashesPerUs);
        
        fclose(csv);
        printf("[*] Results appended to benchmark_results_os.csv successfully!\n");
    } else {
        printf("[!] Failed to save CSV results.\n");
    }

    // Cleanup
    free(threads);
    free(Tasks);
    if (FileBuffer) free(FileBuffer);

    return 0;
}