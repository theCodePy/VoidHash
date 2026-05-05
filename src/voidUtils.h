#ifndef VOID_HASH_UTILS_HEADER_FILE
#define VOID_HASH_UTILS_HEADER_FILE

// Function Prototypes
EFI_STATUS ReadLine(CHAR16 *Buffer, UINTN BufferSize);
EFI_STATUS StrStrip(CHAR16 *StrInput, CHAR16 C);
EFI_STATUS StrCountWords(CHAR16 *StrInput, CHAR16 separator, UINTN *Args_Status);
// ArgsSplit() funciton prototype...
EFI_STATUS ArgsSplit(CHAR16 *StrInput, CHAR16 separator, UINTN row, UINTN col, CHAR16 Args_Matrix[row][col]);
EFI_STATUS printDirectoryContent(EFI_FILE_PROTOCOL *RootDir, CHAR16 *subDirName) ;
EFI_STATUS GetActiveRootDir(EFI_FILE_PROTOCOL **RootDir) ;
EFI_STATUS StrReplace(CHAR16 *StringInput, CHAR16 replaceChar, CHAR16 WithChar);
EFI_STATUS loadFileToRam(CHAR16 *Path_to_file, VOID **FileBuffer);

#endif

