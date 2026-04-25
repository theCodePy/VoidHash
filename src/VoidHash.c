#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>

EFI_STATUS
EFIAPI
UefiMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_INPUT_KEY Key;

  // 1. Clear the BIOS boot text from the screen
  SystemTable->ConOut->ClearScreen(SystemTable->ConOut);

  // 2. Print your custom bare-metal interface
  Print(L"===================================================\n");
  Print(L"                 V O I D H A S H                   \n");
  Print(L"         Bare-Metal Cryptographic Engine           \n");
  Print(L"===================================================\n\n");
  
  Print(L"[+] System Boot Hijacked Successfully.\n");
  Print(L"[+] Zero-OS Environment Initialized.\n\n");
  
  Print(L"Press any key to return control to the firmware...\n");

  // 3. The "Wait" Logic - Trap the CPU until a key is pressed
  // We stall for 10 milliseconds between checks to prevent CPU thrashing
  while (SystemTable->ConIn->ReadKeyStroke(SystemTable->ConIn, &Key) == EFI_NOT_READY) {
    gBS->Stall(10000); 
  }

  // 4. Clean exit
  return EFI_SUCCESS;
}