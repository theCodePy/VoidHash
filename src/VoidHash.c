#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include <voidUtils.h>
#include <voidCommandAndControl.h>

EFI_STATUS
EFIAPI
UefiMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  gBS->SetWatchdogTimer(0, 0, 0, NULL);
  // Using a static array is safer than AllocatePool for simple user input.
  // UEFI specs 2.11, section 2.3.1 Data types.
  CHAR16 InputCommand[255]; 
  CHAR16 *Prompt = L"VoidHash:> ";
  // CHAR16 command[10];
  UINTN Args_Status[2];

  // 1. Clear the screen and print the cosmic UI
  gST->ConOut->ClearScreen(gST->ConOut);
  Print(L"===================================================\n");
  Print(L"                 V O I D H A S H                   \n");
  Print(L"         Bare-Metal Cryptographic Engine           \n");
  Print(L"===================================================\n\n");
  
  // 2. The Interactive Shell Loop
  while (TRUE) {
    Print(Prompt);
    
    // Capture user input
    ReadLine(InputCommand, 255);
    // Strip the extra spacess ;
    StrStrip(InputCommand, L' ');
    // It returns Args_Status[0]=(number of words), Args_Status[1]=(Max word Length)
    StrCountWords(InputCommand, L' ', Args_Status);
    
    // If the user just pressed Enter without typing or typed spaces., skip to next prompt
    // even though we are striping so checking Args_Status[0] wouldn't be required but anyway...
    if (StrLen(InputCommand) == 0 || Args_Status[0] == 0) {
      continue;
    }

    // extra 1 space for NULL teremination in .
    Args_Status[1] += 1;
    // the 2D mtrix to hold the arguments;
    CHAR16 Args_Matrix[Args_Status[0]][Args_Status[1]];
    // seperate the arguments with space, it is same as str.split() in python.
    ArgsSplit(InputCommand, L' ', Args_Status[0], Args_Status[1], Args_Matrix);

    
    // Command Parser: Check for "exit" or "quit"
    if ((StrCmp(InputCommand, L"exit") == 0) || (StrCmp(InputCommand, L"quit") == 0)) {
      Print(L"\n[!] Terminating Zero-OS Environment...\n");
      Print(L"[!] Returning control to motherboard. Adios!\n");
      // Stall for 2 seconds so the user can read the exit message
      gBS->Stall(2000000); 
      break; 
    } 
    // execute the other commands !!! 
    else {
      ExecuteCommand(Args_Status[0], Args_Status[1], Args_Matrix );
    }
  }
  
  // 3. Clean exit hands control back to firmware
  return EFI_SUCCESS;
}


