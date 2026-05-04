#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include <voidUtils.h>
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
    }
    else {
        Print(L"Unknown Command: %s\r\n", Args_Matrix[0]);
    }

    return EFI_SUCCESS;
}

EFI_STATUS handleHelp(UINTN No_of_Command, UINTN MaxWordLen, CHAR16 Args_Matrix[No_of_Command][MaxWordLen] ){
    CHAR16 *help0 = L"\n list of commands you can use:\n\r\n  help\r\n test\r\n  ls    (to list wordlists and hash files)\r\n  voidhash    (to crack hashes)\r\n\n";
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
    EFI_STATUS Status;
    // 1. Corrected the struct type
    EFI_LOADED_IMAGE_PROTOCOL *LoadedImage; 
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *FileSystem;
    EFI_FILE_PROTOCOL *RootDir;

    Status = gBS->HandleProtocol(
        gImageHandle, 
        // 2. Corrected the GUID
        &gEfiLoadedImageProtocolGuid, 
        (VOID **)&LoadedImage
    );

    if (EFI_ERROR (Status)){
        Print(L"ERROR loading Image!!\r\n\n");
        return EFI_SUCCESS;
    }

    Status = gBS->HandleProtocol(
        LoadedImage->DeviceHandle, 
        &gEfiSimpleFileSystemProtocolGuid, 
        (VOID **)&FileSystem
    );

    if (EFI_ERROR(Status) ) {
        Print(L"No File System Found !!!\r\n\n");
        return EFI_SUCCESS;
    }

    Status = FileSystem->OpenVolume(FileSystem, &RootDir);

    if (EFI_ERROR(Status) ) {
        Print(L"Unable to open Volume!!!\r\n\n");
        return EFI_SUCCESS;
    }

    Print(L"so far so good\r\n");
    
    printDirectoryContent(RootDir, L"HashFiles");
    printDirectoryContent(RootDir, L"WordLists");

    return EFI_SUCCESS;
}

EFI_STATUS printDirectoryContent(EFI_FILE_PROTOCOL *RootDir, CHAR16 *subDirName) {
    EFI_FILE_PROTOCOL *SubFolder;
    EFI_STATUS Status;

    Status = RootDir->Open(RootDir, &SubFolder, subDirName, EFI_FILE_MODE_READ, 0);
    
    if (EFI_ERROR(Status) ) {
        Print(L"No Such directory /%s\r\n\n", subDirName);
        return EFI_SUCCESS;
    }

    UINT8 Buffer[1024]; 
    UINTN BufferSize = sizeof(Buffer);
    EFI_FILE_INFO *FileInfo;
    Print(L" Contents of %s :\r\n",subDirName)
    while (TRUE) {
        Status = SubFolder->Read(SubFolder, &BufferSize, Buffer);
        if (EFI_ERROR(Status) ) {
            Print(L"Can't Read the folder /%s\r\n\n", subDirName);
            return EFI_SUCCESS;
        }
        if ((Status==EFI_SUCCESS) && (BufferSize==0)){
            break;
        }
        FileInfo = (EFI_FILE_INFO *)Buffer;
        Print(L" %s FileSize=%d\r\n", FileInfo->FileName, FileInfo->FileSize);
        BufferSize = sizeof(Buffer);

    }

    return EFI_SUCCESS;
}
