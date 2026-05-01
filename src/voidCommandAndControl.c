#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include <voidUtils.h>
#include <voidCommandAndControl.h>

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
    }

    else {
        Print(L"Unknown Command: %s", Args_Matrix[0]);
    }

    return EFI_SUCCESS;
}

EFI_STATUS handleHelp(UINTN No_of_Command, UINTN MaxWordLen, CHAR16 Args_Matrix[No_of_Command][MaxWordLen] ){
    CHAR16 *help0 = L"list of commands you can use:\n\n help\n test\n ls\t\t(to list wordlists and hash files)\n voidhash\t\t(to crack hashes)\n\n";
    CHAR16 *testHelp = L"test command help:";

    if (No_of_Command==1){
        Print(L"%s", help0);
    } else if (No_of_Command > 1){
        if ((StrCmp(Args_Matrix[1], L"test") == 0)){
            Print(L"%s", testHelp);
        }

        else {
            Print(L"Can't Help with Unknown Command: %s", Args_Matrix[1]);
        }
    }

    return EFI_SUCCESS;
}