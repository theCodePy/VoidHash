#ifndef VOID_HASH_UTILS_HEADER_FILE
#define VOID_HASH_UTILS_HEADER_FILE

// Function Prototypes
EFI_STATUS ReadLine(CHAR16 *Buffer, UINTN BufferSize);
EFI_STATUS StrStrip(CHAR16 *StrInput, CHAR16 C);
EFI_STATUS StrCountWords(CHAR16 *StrInput, CHAR16 separator, UINTN *Args_Status);
// ArgsSplit() funciton prototype...
EFI_STATUS ArgsSplit(CHAR16 *StrInput, CHAR16 separator, UINTN row, UINTN col, CHAR16 Args_Matrix[row][col]);

#endif