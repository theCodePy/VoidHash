#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include <voidUtils.h>
#include <Library/BaseLib.h>   
#include <voidCommandAndControl.h>
#include <Protocol/LoadedImage.h>
#include <Guid/FileInfo.h>

// include crypto libs to perform hash operations md5 and sha1 are deprecated.
#define DISABLE_SHA1_DEPRECATED_INTERFACES
#define ENABLE_MD5_DEPRECATED_INTERFACES
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

    loadFileToRam(Args_Matrix[1], &FileBuffer);

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
    if (No_of_Command != 4) {
        Print(L"\r\nWrong command!!!\r\n to see the correct argument try: `help test`\r\n");
        return EFI_SUCCESS;
    }

    UINTN ittr = StrDecimalToUintn(Args_Matrix[3]);
    if (ittr == 0){
        Print(L"\r\nWrong command!!!\r\n to see the correct argument try: `help test`\r\n");
        // I'm working here
    }

    return EFI_SUCCESS;
}