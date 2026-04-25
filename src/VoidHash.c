#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>

EFI_STATUS
EFIAPI
UefiMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  // UEFI Specs 2.11 sectioin 2.3.1 Data Types
  EFI_INPUT_KEY Key;
  CHAR16 *inputCommand = AllocatePool(1000 * sizeof(CHAR16));
  CHAR16 *Command = AllocatePool(10 * sizeof(CHAR16));
  CHAR16 *c ;

  // 1. Clear the BIOS boot text from the screen
  SystemTable->ConOut->ClearScreen(SystemTable->ConOut);

  // 2. Print your custom bare-metal interface
  Print(L"===================================================\n");
  Print(L"                 V O I D H A S H                   \n");
  Print(L"         Bare-Metal Cryptographic Engine           \n");
  Print(L"===================================================\n\n");
  
  Print(L"[+] System Boot Hijacked Successfully.\n");
  Print(L"[+] Zero-OS Environment Initialized.\n\n");
  
//   Print(L"Press any key to return control to the firmware...\n");

  // 3. The "Wait" Logic - Trap the CPU until a key is pressed
  // We stall for 10 milliseconds between checks to prevent CPU thrashing
  while ( !StrCmp(command, L"exit") ){
    while (SystemTable->ConIn->ReadKeyStroke(SystemTable->ConIn, &Key) == EFI_NOT_READY) {
      // gBS->Stall(10000); 
      Print(L"VoidHash:> ");
    }
  }
  // 4. Clean exit
  return EFI_SUCCESS;
}