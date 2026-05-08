#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include <voidUtils.h>
#include <Library/BaseLib.h>   
#include <voidCommandAndControl.h>
#include <Protocol/LoadedImage.h>
#include <Guid/FileInfo.h>

#include <Library/TimerLib.h>

// include crypto libs to perform hash operations md5 is deprecated.
#ifndef ENABLE_MD5_DEPRECATED_INTERFACES
#define ENABLE_MD5_DEPRECATED_INTERFACES
#endif

#include <Library/BaseCryptLib.h>
// the hash function we need: Md5HashAll(); Sha1HashAll(); Sha256HashAll(); Sha384HashAll(); Sha512HashAll(); 
// the output size are defined in the librery as [HASHNAME_DIGEST_SIZE] variable e.g. [MD5_DIGEST_SIZE]
// input data for all of them are (CONST VOID *Data, UINTN DataSize, UINT8 *HashValue)


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
    }
    else {
        Print(L"Unknown Command: %s\r\n", Args_Matrix[0]);
    }

    return EFI_SUCCESS;
}

EFI_STATUS handleHelp(UINTN No_of_Command, UINTN MaxWordLen, CHAR16 Args_Matrix[No_of_Command][MaxWordLen] ){
    CHAR16 *help0 = L"\n list of commands you can use:\n\r\n  help\r\n  test\r\n  cat\r\n  ls    (to list wordlists and hash files)\r\n  voidhash    (to crack hashes)\r\n\n";
    CHAR16 *testHelp = L" test command help:\r\n";

    if (No_of_Command==1){
        Print(L"%s", help0);
    } else if (No_of_Command > 1){
        if ((StrCmp(Args_Matrix[1], L"test") == 0)){
            Print(L"%s", testHelp);
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
    
    printDirectoryContent(RootDir, L"HashFiles");
    printDirectoryContent(RootDir, L"WordLists");

    return EFI_SUCCESS;
}


EFI_STATUS handleCat(UINTN No_of_Command, UINTN MaxWordLen, CHAR16 Args_Matrix[No_of_Command][MaxWordLen] ){
    VOID *FileBuffer = NULL;

    StrReplace(Args_Matrix[1], L'/', L'\\');

    EFI_STATUS Status = loadFileToRam(Args_Matrix[1], &FileBuffer);

    if EFI_ERROR(Status){
        return EFI_SUCCESS;
    }

    Print(L"\r\n here is the content of the file which loaded to RAM :\r\n\n");
    UINTN i=0;
    CHAR8 *Buffer = (CHAR8 *)FileBuffer;
    while (Buffer[i] != '\0') {
        if (Buffer[i] == '\n' ){
            Print(L"\r\n");
        } else {
            Print(L"%c", (CHAR16)Buffer[i]);
        }
        i++;
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
    UINTN hashsize = 0;
    UINTN i; 

    // hash algorithm options to choose from 
    if (StrCmp(Args_Matrix[1], L"md5") == 0) {
        HashAlgo = Md5HashAll ;
        hashsize = MD5_DIGEST_SIZE;
    } else if (StrCmp(Args_Matrix[1], L"sha1") == 0) {
        HashAlgo =  Sha1HashAll ;
        hashsize = SHA1_DIGEST_SIZE;
    } else if (StrCmp(Args_Matrix[1], L"sha256") == 0) {
        HashAlgo =  Sha256HashAll ;
        hashsize = SHA256_DIGEST_SIZE;
    } else if (StrCmp(Args_Matrix[1], L"sha512") == 0) {
        HashAlgo =  Sha512HashAll ;
        hashsize = SHA512_DIGEST_SIZE;
    } else if (StrCmp(Args_Matrix[1], L"sha384") == 0) {
        HashAlgo =  Sha384HashAll ;
        hashsize = SHA384_DIGEST_SIZE;
    } else {
        Print(L"\r\nOnly supported algorithms: md5, sha1, sha256, sha384, sha512\r\n\r\n");
        return EFI_INVALID_PARAMETER;
    }

    // loading the file into the buffer inside RAM;
    VOID *FileBuffer = NULL;
    UINTN s = StrLen(Args_Matrix[2]);
    if (s<1){
        Print(L"\r\nINVALID fileName\r\n\r\n");
        return EFI_INVALID_PARAMETER;
    }
    CHAR16 wordListName[s +10] ;
    wordListName[0] = L'W';
    wordListName[1] = L'o';
    wordListName[2] = L'r';
    wordListName[3] = L'd';
    wordListName[4] = L'L';
    wordListName[5] = L'i';
    wordListName[6] = L's';
    wordListName[7] = L't';
    wordListName[8] = L's';
    wordListName[9] = L'\\';
    i=0;  
    while (Args_Matrix[2][i] != L'\0'){
        wordListName[10+i]=Args_Matrix[2][i] ;
        i++;
    }
    wordListName[10+i] = L'\0';
    Status = loadFileToRam(wordListName, &FileBuffer);
    if EFI_ERROR (Status){
        return EFI_SUCCESS;
    }
    CHAR8 *fBuffer = (CHAR8 *)FileBuffer;

    // CHAR8 *word1 = "password";
    // CHAR8 word10[4][15] = {"password", "P@ssw0rd", "123456789", "rohan@2002"};
    UINT8 HashValue[hashsize + 1];
    // CHAR8 hexDigest[(hashsize * 2) + 1];
    UINTN buffPtr=0;
    UINTN wLen;
    UINT64 totalHash=0;

    //  replace \n with \0
    AsciiCharReplace(fBuffer, '\n', '\0');

    UINT64 Frequency;
    UINT64 StartTicks, StartCycles;
    UINT64 EndTicks, EndCycles;
    UINT64 TotalTicks, TotalCycles;
    
    // 2. Query the Motherboard for the Timer Frequency
    // If running in QEMU, this will likely return 0.
    Frequency = GetPerformanceCounterProperties(NULL, NULL);

    // 3. START BOTH CLOCKS
    StartTicks = GetPerformanceCounter();
    StartCycles = AsmReadTsc();


    for (i=0; i<ittr; i++){
        while (fBuffer[buffPtr] != '\0'){
            wLen = AsciiStrLen(&fBuffer[buffPtr]);

            HashAlgo(&(fBuffer[buffPtr]), wLen, HashValue);
            buffPtr += wLen + 1;
            totalHash++;
        }
        buffPtr = 0;
        Print(L"progress itteration: %d  \r", i+1);
    }

    EndCycles = AsmReadTsc();
    EndTicks = GetPerformanceCounter();

    // 5. CALCULATE RAW DIFFERENCES
    TotalCycles = EndCycles - StartCycles;
    TotalTicks = EndTicks - StartTicks;

    // 6. REPORTING AND SAFE MATH
    Print(L"\r\n--- Benchmark Results ---\r\n");
    Print(L"Total CPU Cycles: %lu    ", TotalCycles);
    
    if (totalHash > 0) {
        Print(L"    Speed: %lu Cycles/Hash\r\n", (TotalCycles / totalHash));
    }

    Print(L"-------------------------\r\n");

    // 7. THE SAFETY VALVE (Preventing the Divide-by-Zero crash)
    if (Frequency == 0) {
        Print(L"[!] Unable to get base clock speed, might be running on a VM??\r\n");
        Print(L"[!] Skipping time calculation\r\n");
    } else {
        // Because Frequency is NOT zero, this math is now 100% safe
        UINT64 TotalTimeNs = (TotalTicks * 1000000000ULL) / Frequency;
        
        // Convert to human readable formats
        UINT64 TimeMs = TotalTimeNs / 1000000;
        UINT64 TimeSec = TimeMs / 1000;
        
        Print(L"Hash/Sec:%lu     Time Elapsed: %lu Seconds  (%lu ms)  [%lu ns]\r\n",(totalHash / TimeSec), TimeSec, TimeMs, TotalTimeNs);
        Print(L": %lu Seconds   (%lu ms)   [%lu ns]\r\n", TimeSec, TimeMs, TotalTimeNs);
    }

    return EFI_SUCCESS;
}