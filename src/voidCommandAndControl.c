#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include <voidUtils.h>
#include <Library/BaseLib.h>   
#include <voidCommandAndControl.h>
#include <Library/MemoryAllocationLib.h>
#include <Protocol/LoadedImage.h>
#include <Guid/FileInfo.h>
#include <Library/PrintLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>


#include <Library/TimerLib.h>

// include crypto libs to perform hash operations md5 is deprecated.
#ifndef ENABLE_MD5_DEPRECATED_INTERFACES
#define ENABLE_MD5_DEPRECATED_INTERFACES
#endif

#include <Library/BaseCryptLib.h>
// the hash function we need: Md5HashAll(); Sha1HashAll(); Sha256HashAll(); Sha384HashAll(); Sha512HashAll(); 
// the output size are defined in the librery as [HASHNAME_DIGEST_SIZE] variable e.g. [MD5_DIGEST_SIZE]
// input data for all of them are (CONST VOID *Data, UINTN DataSize, UINT8 *HashValue)

// the faster md5 hash from FastMD5onVoid.c file
#ifndef FASTER_MD5_ON_THE_VOID
#define FASTER_MD5_ON_THE_VOID

BOOLEAN EFIAPI FastMd5HashAll(CONST VOID *Data, UINTN DataSize, UINT8 *HashValue) ;

#endif

// this is to wake up all the core's of cpu to crack the hash...
#include <PiDxe.h>
#include <Protocol/MpService.h>

// The blueprint for what each core needs to know
typedef struct {
    CHAR8 *StartPtr;
    CHAR8 *EndPtr;
    UINT8 TargetHashBinary[64]; // Max size for SHA-512
    UINTN HashByteSize;
    BOOLEAN EFIAPI (*HashAlgo)(CONST VOID *, UINTN, UINT8 *);
    
    // THE KILL SWITCH
    volatile BOOLEAN *GlobalPasswordFound;
    CHAR8 *FoundPasswordDest; 
} AP_CRACKING_TASK;


typedef struct {
    CHAR8 *StartPtr;
    CHAR8 *EndPtr;
    BOOLEAN EFIAPI (*HashAlgo)(CONST VOID *, UINTN, UINT8 *);
    UINTN Iterations;
    UINT64 HashesComputed; // Each core tracks its own total
} AP_TEST_TASK;


typedef struct {
    EFI_MP_SERVICES_PROTOCOL *Mp;
    AP_TEST_TASK *TaskArray;
} MP_TEST_PAYLOAD;

typedef struct {
    EFI_MP_SERVICES_PROTOCOL *Mp;
    AP_CRACKING_TASK *TaskArray;
} MP_CRACKING_PAYLOAD;


EFI_STATUS ExecuteCommand(UINTN No_of_Command, UINTN MaxWordLen, CHAR16 Args_Matrix[No_of_Command][MaxWordLen] ){
    if (StrCmp(Args_Matrix[0], L"help") == 0){
        handleHelp(No_of_Command, MaxWordLen, Args_Matrix );
    } else if (StrCmp(Args_Matrix[0], L"ls") == 0){
        handleLs(No_of_Command, MaxWordLen, Args_Matrix );
    } else if (StrCmp(Args_Matrix[0], L"cat") == 0){
        handleCat(No_of_Command, MaxWordLen, Args_Matrix );
    } else if (StrCmp(Args_Matrix[0], L"test") == 0){
        handleTest(No_of_Command, MaxWordLen, Args_Matrix );
    } else if (StrCmp(Args_Matrix[0], L"clear") == 0 ){
        gST->ConOut->ClearScreen(gST->ConOut);
    } else if (StrCmp(Args_Matrix[0], L"voidhash") == 0){
        handleVoidHash(No_of_Command, MaxWordLen, Args_Matrix );
    } else if (StrCmp(Args_Matrix[0], L"shutdown") == 0){
        handleShutDown(No_of_Command);
    }
    else {
        Print(L"Unknown Command: %s\r\n", Args_Matrix[0]);
    }

    return EFI_SUCCESS;
}

EFI_STATUS handleHelp(UINTN No_of_Command, UINTN MaxWordLen, CHAR16 Args_Matrix[No_of_Command][MaxWordLen] ){
    CHAR16 *help0 = L"\r\nVOIDHASH is an UEFI password bruteforcing tool\r\n"
                    L"Developed by Rohan Maji \r\n\n"
                    L"type `help Name` to find out more information about the command `name`\r\n\n  ";
    CHAR16 *help1 = L"list of commands:\r\n\n  "
                    L"help        (to view information)\r\n  ";
    CHAR16 *help2 = L"shutdown    (to shutdown the mahchine\r\n  "
                    L"test        (to test hash rates)\r\n  "
                    L"cat         (print file contents)\r\n  ";
    CHAR16 *help3 = L"ls          (to list file and directories)\r\n  "
                    L"voidhash    (to crack hashes)\r\n  "
                    L"clear       (to clear the screen)\r\n\n";

    CHAR16 *testHelp0 = L"  test command: test <hashName> <wordlistName> <itteration>\r\n"
                        L"      <HashName>:    choose from these available hashes [md5, sha1, sha256, sha384, sha512]\r\n";
    CHAR16 *testHelp1 = L"      <WordlistName: only name of the existing wordlist files from `\\WordLists` directory (e.g. rockyou.txt)\r\n"
                        L"      <itteration>:  number of itteration of the whole file (e.g. 1, 2, 3...)\r\n";
    CHAR16 *testHelp2 = L"  the test results will be appended into `\\benchmark_results.csv` file\r\n";

    CHAR16 *shutdownHelp0 = L"  shutdown: This command doesn't take any aurguments\r\n"
                            L"  type `shutdown` to poweroff the machine\r\n";

    if (No_of_Command==1){
        Print(L"%s", help0);
        Print(L"%s", help1);
        Print(L"%s", help2);
        Print(L"%s", help3);
    } else if (No_of_Command > 1){
        if ((StrCmp(Args_Matrix[1], L"test") == 0)){
            Print(L"%s", testHelp0);
            Print(L"%s", testHelp1);
            Print(L"%s", testHelp2);
        } else if ((StrCmp(Args_Matrix[1], L"shutdown") == 0)){
            Print(L"%s", shutdownHelp0);
        }

        else {
            Print(L"Can't Help with Unknown Command: %s\r\n", Args_Matrix[1]);
        }
    }

    return EFI_SUCCESS;
}

EFI_STATUS handleLs(UINTN No_of_Command, UINTN MaxWordLen, CHAR16 Args_Matrix[No_of_Command][MaxWordLen] ){
    EFI_FILE_PROTOCOL *RootDir = NULL;

    GetActiveRootDir(&RootDir);
    if (No_of_Command > 1){
        for (UINTN i=1; i<No_of_Command; i++){
            printDirectoryContent(RootDir, Args_Matrix[i]);
        }
    }
    else {
        printDirectoryContent(RootDir, L"");
        printDirectoryContent(RootDir, L"HashFiles");
        printDirectoryContent(RootDir, L"WordLists");
    }

    return EFI_SUCCESS;
}


EFI_STATUS handleCat(UINTN No_of_Command, UINTN MaxWordLen, CHAR16 Args_Matrix[No_of_Command][MaxWordLen] ){
    VOID *FileBuffer = NULL;
    UINTN fileSize;
    StrReplace(Args_Matrix[1], L'/', L'\\');

    EFI_STATUS Status = loadFileToRam(Args_Matrix[1], &FileBuffer, &fileSize);

    if EFI_ERROR(Status){
        return EFI_SUCCESS;
    }

    // Print(L"\r\n here is the content of the file which loaded to RAM :\r\n\n");
    UINTN i=0;
    CHAR8 *Buffer = (CHAR8 *)FileBuffer;
    while (Buffer[i] != '\0') {
        if (Buffer[i] == '\n' ){
            Print(L"\r\n ");
        } else {
            Print(L"%c", (CHAR16)Buffer[i]);
        }
        i++;
    }
    Print(L"\r\n");

    if (FileBuffer != NULL) {
        FreePool(FileBuffer);
    }

    return EFI_SUCCESS;
}






EFI_STATUS handleTest( UINTN No_of_Command, UINTN MaxWordLen, CHAR16 Args_Matrix[No_of_Command][MaxWordLen]) {
    // test command architecture
    // test  <algorithm>  <wordlist>  <iterations>
    if (No_of_Command != 4) {
        Print(L"\r\nWrong command!!!\r\n to see the correct argument try: `help test`\r\n");
        return EFI_INVALID_PARAMETER;
    }

    UINTN ittr = StrDecimalToUintn(Args_Matrix[3]);
    if (ittr == 0){
        Print(L"\r\nZero itteretion: nothing to do!!! \r\n\r\n");
        return EFI_INVALID_PARAMETER;
    }
    
    BOOLEAN EFIAPI (*HashAlgo) (CONST VOID *Data, UINTN DataSize, UINT8 *HashValue);
    EFI_STATUS Status;
    // UINTN hashsize = 0;
    UINTN i; 

    // hash algorithm options to choose from 
    if (StrCmp(Args_Matrix[1], L"md5") == 0) {HashAlgo = FastMd5HashAll ;} 
    else if (StrCmp(Args_Matrix[1], L"sha1") == 0) { HashAlgo =  Sha1HashAll ; } 
    else if (StrCmp(Args_Matrix[1], L"sha256") == 0) { HashAlgo =  Sha256HashAll ;} 
    else if (StrCmp(Args_Matrix[1], L"sha512") == 0) { HashAlgo =  Sha512HashAll ;} 
    else if (StrCmp(Args_Matrix[1], L"sha384") == 0) { HashAlgo =  Sha384HashAll ;} 
    else {
        Print(L"\r\nOnly supported algorithms: md5, sha1, sha256, sha384, sha512\r\n\r\n");
        return EFI_INVALID_PARAMETER;
    }

    Print(L"loading file...\r\n");
    // loading the file into the buffer inside RAM;
    VOID *FileBuffer = NULL;
    UINTN fileSize;
    UINTN s = StrLen(Args_Matrix[2]);
    if (s<1){
        Print(L"\r\nINVALID fileName\r\n\r\n");
        return EFI_INVALID_PARAMETER;
    }
    CHAR16 wordListName[255];

    // concatinate 2 strings,,, woww this function is awsome..
    UnicodeSPrint(wordListName, sizeof(wordListName), L"WordLists\\%s", Args_Matrix[2]);

    Status = loadFileToRam(wordListName, &FileBuffer, &fileSize);
    if EFI_ERROR (Status){
        return EFI_SUCCESS;
    }
    CHAR8 *fBuffer = (CHAR8 *)FileBuffer;
    Print(L"fileSize=%d\r\n", fileSize);
    // CHAR8 *word1 = "password";
    // CHAR8 word10[4][15] = {"password", "P@ssw0rd", "123456789", "rohan@2002"};
    // UINT8 HashValue[hashsize + 1];
    // CHAR8 hexDigest[(hashsize * 2) + 1];
    // UINTN buffPtr=0;
    // UINTN wLen;
    UINT64 totalHash=0;

    //  replace \n with \0
    Print(L"formatting the file...\r\n");
    AsciiCharReplace(fBuffer, '\n', '\0');

    // --- MP SERVICES SETUP ---
    EFI_MP_SERVICES_PROTOCOL *MpServices = NULL;
    Status = gBS->LocateProtocol(&gEfiMpServiceProtocolGuid, NULL, (VOID **)&MpServices);
    if (EFI_ERROR(Status)) {
        Print(L"[!] Fatal: Multiprocessor Protocol missing!\r\n");
        if (FileBuffer != NULL) FreePool(FileBuffer);
        return Status;
    }

    UINTN NumProcessors, NumEnabledProcessors;
    MpServices->GetNumberOfProcessors(MpServices, &NumProcessors, &NumEnabledProcessors);
    
    EFI_EVENT ApSyncEvent;
    gBS->CreateEvent(0, TPL_APPLICATION, NULL, NULL, &ApSyncEvent);
    AP_TEST_TASK *Tasks = AllocateZeroPool(sizeof(AP_TEST_TASK) * NumEnabledProcessors);

    // Calculate memory chunks
    UINTN ChunkSize = fileSize / NumEnabledProcessors;
    CHAR8 *ChunkCursor = fBuffer;

    for (i = 0; i < NumEnabledProcessors; i++) {
        Tasks[i].StartPtr = ChunkCursor;
        Tasks[i].HashAlgo = HashAlgo;
        Tasks[i].Iterations = ittr;
        Tasks[i].HashesComputed = 0;

        if (i == NumEnabledProcessors - 1) {
            Tasks[i].EndPtr = fBuffer + fileSize;
        } else {
            CHAR8 *TempEnd = ChunkCursor + ChunkSize;
            while (*TempEnd != '\0' && TempEnd < (fBuffer + fileSize)) TempEnd++;
            Tasks[i].EndPtr = TempEnd;
            ChunkCursor = TempEnd + 1;
        }
    }

    Print(L"Benchmarking across %d Active CPU Cores...\r\n", NumEnabledProcessors);

    UINT64 Frequency;
    UINT64 StartTicks, StartCycles;
    UINT64 EndTicks, EndCycles;
    UINT64 TotalTicks, TotalCycles;
    
    // query the Motherboard for the Timer Frequency
    // If running in QEMU, this will likely return 0.
    Frequency = GetPerformanceCounterProperties(NULL, NULL);

    // start the clocks. what's the time?
    StartTicks = GetPerformanceCounter();
    StartCycles = AsmReadTsc();

    // initialize the payload
    MP_TEST_PAYLOAD Payload;
    Payload.Mp = MpServices;
    Payload.TaskArray = Tasks;
    
    // Wake the APs (Cores 1 through N) asynchronously
    EFI_STATUS ApStatus = MpServices->StartupAllAPs(
        MpServices, 
        TestCoreHashingFunction, 
        FALSE, 
        ApSyncEvent, 
        0, 
        (VOID *)&Payload, // Broadcast the entire payload to everyone
        NULL
    );

    // The BSP instantly joins the fight (Core 0)
    TestCoreHashingFunction((VOID *)&Payload);

    // Wait for all APs to finish
    if (ApStatus == EFI_SUCCESS) {
        while (gBS->CheckEvent(ApSyncEvent) == EFI_NOT_READY) {}
    }

    // stop clocks
    EndCycles = AsmReadTsc();
    EndTicks = GetPerformanceCounter();

    // Sum up all the hashes processed by every core
    totalHash = 0;
    for (i = 0; i < NumEnabledProcessors; i++) {
        totalHash += Tasks[i].HashesComputed;
    }

    // 5. CALCULATE RAW DIFFERENCES
    TotalCycles = EndCycles - StartCycles;
    TotalTicks = EndTicks - StartTicks;
    UINT64 CyclesPerHash = 0;
    if (totalHash > 0) {
        CyclesPerHash = (TotalCycles / totalHash);
    }
    
    UINT64 HashesPerSec = 0;
    UINT64 HashesPerUs  = 0;

    // 6. REPORTING AND SAFE MATH
    Print(L"\r\n--- Benchmark Results ---\r\n");
    Print(L"Total Hashes: %lu   \r\n", totalHash);
    Print(L"Total CPU Cycles: %lu    ", TotalCycles);
    
    if (totalHash > 0) {
        Print(L"    Speed: %lu Cycles/Hash\r\n", CyclesPerHash);
    }

    Print(L"-------------------------\r\n");

    // 7. THE SAFETY VALVE (Preventing the Divide-by-Zero crash)
    if (Frequency == 0) {
        Print(L"[!] Unable to get base clock speed, might be running on a VM??\r\n");
        Print(L"[!] Skipping time calculation\r\n");
    } else {
        // --- 1. SAFE NANOSECOND CALCULATION (Prevents 6-second overflow) ---
        UINT64 SecondsFull     = TotalTicks / Frequency;
        UINT64 RemainderTicks  = TotalTicks % Frequency;
        
        // Calculate nanoseconds safely
        UINT64 TotalTimeNs = (SecondsFull * 1000000000ULL) + ((RemainderTicks * 1000000000ULL) / Frequency);
        
        // Convert to human-readable formats
        UINT64 TimeUs  = TotalTimeNs / 1000;
        UINT64 TimeMs  = TimeUs / 1000;
        UINT64 TimeSec = TimeMs / 1000;

        // --- 2. HIGH-PRECISION RATE CALCULATIONS ---
        // UINT64 HashesPerSec = 0;
        // UINT64 HashesPerUs  = 0;

        // Safety catch: Ensure we don't divide by zero if the program was unimaginably fast
        if (TotalTimeNs > 0) {
            HashesPerSec = (totalHash * 1000000000ULL) / TotalTimeNs;
            HashesPerUs  = (totalHash * 1000000ULL) / TotalTimeNs;
        }

        // --- 3. THE FINAL PRINTOUT ---
        Print(L"Time Elapsed: %lu sec | %lu ms | %lu us | %lu ns\r\n", TimeSec, TimeMs, TimeUs, TotalTimeNs);
        
        if (TotalTimeNs > 0) {
            Print(L"Performance:  %lu Hash/Sec  |  %lu Hash/us\r\n", HashesPerSec, HashesPerUs);
        } else {
            Print(L"Performance:  Execution too fast to measure rate!\r\n");
        }

    }
    
    //saving the bechmark test result to the csv file.....
    EFI_FILE_PROTOCOL *RootDir = NULL;
    CHAR8 lineToWrite[512];
    CHAR16 *FileName = L"benchmark_results.csv";
    CHAR8 *Header = "hashname,wordlist_Name,cycles/hash,hash/sec,hash/microSec\r\n";
    AsciiSPrint(lineToWrite, sizeof(lineToWrite), "%s,%s,%lu,%lu,%lu\r\n", 
        Args_Matrix[1], Args_Matrix[2], CyclesPerHash, HashesPerSec, HashesPerUs );
    GetActiveRootDir(&RootDir);
    saveToFile_inAppendMode(RootDir, FileName, Header, lineToWrite);
    
    // Cleanup MP Services memory allocations
    gBS->CloseEvent(ApSyncEvent);
    FreePool(Tasks);

    if (FileBuffer != NULL) {
        FreePool(FileBuffer);
    }

    return EFI_SUCCESS;
}   





EFI_STATUS handleVoidHash(UINTN No_of_Command, UINTN MaxWordLen, CHAR16 Args_Matrix[No_of_Command][MaxWordLen]) {
    // voidhash <hashfile> <wordlist>
    if (No_of_Command != 3) {
        Print(L"\r\nWrong command!!! Try: `help voidhash`\r\n");
        return EFI_INVALID_PARAMETER;
    }

    EFI_STATUS Status;

    Print(L"Loading Files, it may take a while...\r\n");

    // loading hashfile into RAM
    CHAR16 HashFileName[255];
    VOID *voidHashFileBuffer = NULL;
    UINTN HashFileSize;
    UnicodeSPrint(HashFileName, sizeof(HashFileName), L"HashFiles\\%s", Args_Matrix[1]);
    Status = loadFileToRam(HashFileName, &voidHashFileBuffer, &HashFileSize);
    if (EFI_ERROR(Status)) return Status;
    CHAR8 *HashFileBuffer = (CHAR8 *)voidHashFileBuffer;

    // loading wordlist file into RAM
    CHAR16 wordListName[255];
    VOID *voidWordListBuffer = NULL;
    UINTN WordlistFileSize;
    UnicodeSPrint(wordListName, sizeof(wordListName), L"WordLists\\%s", Args_Matrix[2]);
    Status = loadFileToRam(wordListName, &voidWordListBuffer, &WordlistFileSize);
    if (EFI_ERROR(Status)) return Status;
    CHAR8 *WordListBuffer = (CHAR8 *)voidWordListBuffer;

    Print(L"Formatting files...\r\n");
    AsciiCharReplace(HashFileBuffer, '\n', '\0');
    AsciiCharReplace(WordListBuffer, '\n', '\0');
    
    // find MP service and get number of processors in the cpu
    EFI_MP_SERVICES_PROTOCOL *MpServices = NULL;
    Status = gBS->LocateProtocol(&gEfiMpServiceProtocolGuid, NULL, (VOID **)&MpServices);
    if (EFI_ERROR(Status)) {
        Print(L"[FATAL] Motherboard does not support MP Services!\r\n");
        return Status;
    }
    UINTN NumProcessors, NumEnabledProcessors;
    MpServices->GetNumberOfProcessors(MpServices, &NumProcessors, &NumEnabledProcessors);
    Print(L"number of CPU Cores: %d\r\n", NumEnabledProcessors);

    // Create the Asynchronous Event Bell 
    EFI_EVENT ApSyncEvent;
    gBS->CreateEvent(0, TPL_APPLICATION, NULL, NULL, &ApSyncEvent);

    // creating the shared variables and allocating memories to pass to the all processors
    AP_CRACKING_TASK *Tasks = AllocateZeroPool(sizeof(AP_CRACKING_TASK) * NumEnabledProcessors);
    volatile BOOLEAN GlobalPasswordFound;
    CHAR8 CrackedPassword[255];

    // main loop to crack the hashes with multi processors
    UINTN hashbuffPtr = 0;
    BOOLEAN EFIAPI (*HashAlgo) (CONST VOID *Data, UINTN DataSize, UINT8 *HashValue);
    UINTN HashByteSize = 0;
    
    while (hashbuffPtr < HashFileSize) {
        if (HashFileBuffer[hashbuffPtr] == '\0' ){
            hashbuffPtr++;
            continue;
        }
        
        AsciiStrStrip(&HashFileBuffer[hashbuffPtr], ' ');
        UINTN hashLen = AsciiStrLen(&HashFileBuffer[hashbuffPtr]);
        CHAR8 *CurrentTargetStr = &HashFileBuffer[hashbuffPtr];
        CHAR16 hashName[10] ;
        
        if (hashLen / 2 == MD5_DIGEST_SIZE) { HashAlgo = FastMd5HashAll; HashByteSize = MD5_DIGEST_SIZE; StrCpyS(hashName, sizeof(hashName), L"md5");} 
        else if (hashLen / 2 == SHA1_DIGEST_SIZE) { HashAlgo = Sha1HashAll; HashByteSize = SHA1_DIGEST_SIZE; StrCpyS(hashName, sizeof(hashName), L"sha1");} 
        else if (hashLen / 2 == SHA256_DIGEST_SIZE) { HashAlgo = Sha256HashAll; HashByteSize = SHA256_DIGEST_SIZE; StrCpyS(hashName, sizeof(hashName), L"sha256"); } 
        else if (hashLen / 2 == SHA512_DIGEST_SIZE) { HashAlgo = Sha512HashAll; HashByteSize = SHA512_DIGEST_SIZE; StrCpyS(hashName, sizeof(hashName), L"sha512");} 
        else if (hashLen / 2 == SHA384_DIGEST_SIZE) { HashAlgo = Sha384HashAll; HashByteSize = SHA384_DIGEST_SIZE; StrCpyS(hashName, sizeof(hashName), L"sha384"); } 
        else {
            Print(L"Unsupported Hash Length: %a... Skipping.\r\n", CurrentTargetStr);
            hashbuffPtr += hashLen + 1;
            continue;
        }

        Print(L"cracking: %a\r\n", CurrentTargetStr);

        // resetting variables for this specific hash
        GlobalPasswordFound = FALSE;
        CrackedPassword[0] = '\0';
        UINT8 TargetBinary[64];
        HexStringToByteArray(CurrentTargetStr, TargetBinary, HashByteSize);

        // calculating the chunk of memory for each cores...
        UINTN ChunkSize = WordlistFileSize / NumEnabledProcessors;
        CHAR8 *ChunkCursor = WordListBuffer;

        for (UINTN i = 0; i < NumEnabledProcessors; i++) {
            Tasks[i].TargetHashBinary[0] = '\0'; // clear
            CopyMem(Tasks[i].TargetHashBinary, TargetBinary, HashByteSize);
            Tasks[i].HashByteSize = HashByteSize;
            Tasks[i].HashAlgo = HashAlgo;
            Tasks[i].GlobalPasswordFound = &GlobalPasswordFound;
            Tasks[i].FoundPasswordDest = CrackedPassword;
            Tasks[i].StartPtr = ChunkCursor;

            // to ensure the endPtr always points to \0, (don't wanna end a word inna middle)
            if (i == NumEnabledProcessors - 1) {
                Tasks[i].EndPtr = WordListBuffer + WordlistFileSize;
            } else {
                CHAR8 *TempEnd = ChunkCursor + ChunkSize;
                while (*TempEnd != '\0' && TempEnd < (WordListBuffer + WordlistFileSize)) TempEnd++;
                Tasks[i].EndPtr = TempEnd;
                ChunkCursor = TempEnd + 1;
            }
        }

        MP_CRACKING_PAYLOAD Payload;
        Payload.Mp = MpServices;
        Payload.TaskArray = Tasks;

        // waking up all the APs (Cores 1 through N) asynchronously... for cracking, baby,, yeee boy.
        EFI_STATUS ApStatus = MpServices->StartupAllAPs(
            MpServices, 
            CoreCrackingFunction, 
            FALSE,          // SingleThread = FALSE (Wake them all)
            ApSyncEvent,    // The asynchronous bell
            0,              // Timeout (0 = infinite)
            (VOID *)&Payload, // Pass the shared payload to work with
            NULL
        );

        // 2. The BSP instantly joins the fight (Core 0)
        CoreCrackingFunction((VOID *)&Payload);

        // 3. The Synchronization Phase
        // The manager waits until either someone finds it, or everyone naturally finishes.
        if (ApStatus == EFI_SUCCESS){ 
            while (GlobalPasswordFound == FALSE && gBS->CheckEvent(ApSyncEvent) == EFI_NOT_READY) {}
        }
        

        // Print the result!
        if (GlobalPasswordFound) {
            Print(L"PASSWORD FOUND: %a\r\n\r\n", CrackedPassword);

            // saving the cracked hash to a file.
            EFI_FILE_PROTOCOL *RootDir = NULL;
            CHAR8 lineToWrite[1024];
            CHAR16 *FileName = L"cracked_hashes.txt";
            CHAR8 *Header = NULL;
            AsciiSPrint(lineToWrite, sizeof(lineToWrite), "%s:%a:%a\r\n", 
                hashName, CrackedPassword , CurrentTargetStr);
            GetActiveRootDir(&RootDir);
            saveToFile_inAppendMode(RootDir, FileName, Header, lineToWrite);
        } else {
            Print(L"password NOT FOUND in wordlist\r\n\r\n");
        }

        // Jump to the next hash in the file
        hashbuffPtr += hashLen + 1;
    }

    // Cleanup
    gBS->CloseEvent(ApSyncEvent);
    FreePool(Tasks);
    if (voidHashFileBuffer != NULL) FreePool(voidHashFileBuffer);
    if (voidWordListBuffer != NULL) FreePool(voidWordListBuffer);

    Print(L"VoidHash Complete.\r\n");
    return EFI_SUCCESS;
}


VOID EFIAPI CoreCrackingFunction(VOID *Buffer) {
    MP_CRACKING_PAYLOAD *Payload = (MP_CRACKING_PAYLOAD *)Buffer;

    // getting my core number
    UINTN MyCoreNumber;
    Payload->Mp->WhoAmI(Payload->Mp, &MyCoreNumber);

    AP_CRACKING_TASK *Task = &Payload->TaskArray[MyCoreNumber];

    CHAR8 *CurrentWord = Task->StartPtr;
    UINT8 GeneratedHash[64];
    UINTN WordLen;

    while (CurrentWord < Task->EndPtr) {
        // Check the Kill Switch! If another core found the password or not, go to sleep instantly if found.
        if (*(Task->GlobalPasswordFound) == TRUE) {
            return; 
        }

        // empty password line in a dirty wordlist. skipping themm .
        if (*CurrentWord == '\0') {
            CurrentWord++;
            continue;
        }

        WordLen = AsciiStrLen(CurrentWord);

        // calling hashing algorithm, what? to generate hash off course.
        Task->HashAlgo(CurrentWord, WordLen, GeneratedHash);

        // compare if found, fast comparing with this method.. chuck chuck
        if (CompareMem(GeneratedHash, Task->TargetHashBinary, Task->HashByteSize) == 0) {
            // the AHA moment, gotcha!!!!
            *(Task->GlobalPasswordFound) = TRUE;
            AsciiStrCpyS(Task->FoundPasswordDest, 255, CurrentWord);
            return;
        }

        // not found on to the next one...
        CurrentWord += WordLen + 1;
    }
}



VOID EFIAPI TestCoreHashingFunction(VOID *Buffer) {
    MP_TEST_PAYLOAD *Payload = (MP_TEST_PAYLOAD *)Buffer;

    // getting my core number
    UINTN MyCoreNumber;
    Payload->Mp->WhoAmI(Payload->Mp, &MyCoreNumber);

    AP_TEST_TASK *Task = &Payload->TaskArray[MyCoreNumber];

    CHAR8 *CurrentWord;
    UINT8 GeneratedHash[64];
    UINTN WordLen;
    UINTN i;

    Task->HashesComputed = 0;

    // Run the requested number of iterations for this specific memory chunk
    for (i = 0; i < Task->Iterations; i++) {
        CurrentWord = Task->StartPtr;
        
        while (CurrentWord < Task->EndPtr) {
            // Skip empty lines safely
            if (*CurrentWord == '\0') {
                CurrentWord++;
                continue;
            }
            
            WordLen = AsciiStrLen(CurrentWord);
            Task->HashAlgo(CurrentWord, WordLen, GeneratedHash);
            
            Task->HashesComputed++;
            CurrentWord += WordLen + 1; // Jump to next word
        }
    }
}


EFI_STATUS handleShutDown(UINTN No_of_Command){

    if (No_of_Command > 1) {
        Print(L"Told you `shutdown` command doesn't take any aurguments\r\n");
        Print(L"if I didn't told you before, type `help shutdown`\r\n\n");
        return EFI_SUCCESS;
    }

    Print(L"shutdown the Computer");
    gRT->ResetSystem (
         EfiResetShutdown, 
         EFI_SUCCESS, 
         0, 
         NULL
         );

    return EFI_SUCCESS;
}