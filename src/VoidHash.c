#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseMemoryLib.h>

// Function Prototypes
EFI_STATUS ReadLine(CHAR16 *Buffer, UINTN BufferSize);
// EFI_STATUS ArgumentExtract(CHAR16 *InputCommand, UINTN position) ;
EFI_STATUS StrStrip(CHAR16 *StrInput, CHAR16 C);

EFI_STATUS
EFIAPI
UefiMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  // Using a static array is safer than AllocatePool for simple user input.
  // UEFI specs 2.11, section 2.3.1 Data types.
  CHAR16 InputCommand[255]; 
  CHAR16 *Prompt = L"VoidHash:> ";
  // CHAR16 command[10];

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
    StrStrip(InputCommand, L' ');


    // If the user just pressed Enter without typing, skip to next prompt
    if (StrLen(InputCommand) == 0) {
      continue;
    }

    // Command Parser: Check for "exit"
    if (StrCmp(InputCommand, L"exit") == 0) {
      Print(L"\n[!] Terminating Zero-OS Environment...\n");
      Print(L"[!] Returning control to motherboard. Adios!\n");
      // Stall for 2 seconds so the user can read the exit message
      gBS->Stall(2000000); 
      break; 
    } 
    // Command Parser: Placeholder for other commands
    else {
      Print(L"Unknown command: %s\n", InputCommand);
    }
  }
  
  // 3. Clean exit hands control back to firmware
  return EFI_SUCCESS;
}


// strip spaces from user input string
EFI_STATUS StrStrip(CHAR16 *StrInput, CHAR16 C){
  UINTN lenStrInput = StrLen(StrInput);
  CHAR16 tempStr[lenStrInput+1];
  UINTN i;
  UINTN j;
  UINTN startIndex;
  UINTN endIndex;
  
  if (lenStrInput==0) 
    return EFI_SUCCESS;

  // COPY the str to a tempStr
  for (i=0; i<lenStrInput; i++){
    tempStr[i] = StrInput[i];
  }

  // finding the Starting index without the unwanted character
  i = 0;
  while (StrInput[i] == C && i<lenStrInput){
    i++;
  }
  startIndex = i;
  if (startIndex == lenStrInput){
    for (i=0; i<lenStrInput; i++){
      StrInput[i] = L'\0';
    }
    return EFI_SUCCESS;
  }

  // finding the ending index of actual string without the unwanted character
  i = lenStrInput - 1;
  while (StrInput[i] == C && i>=0) {
    i--;
  }
  endIndex = i;

  //update the string 
  j=0;
  for (i=startIndex; i<=endIndex; i++){
    StrInput[j] = tempStr[i];
    j++;
  }
  // wiping the string clean hahah haha
  for (i=lenStrInput-1; i>(endIndex-startIndex); i--){
    if (i < 0)
      break;
    StrInput[i] = L'\0';
  }
  
  return EFI_SUCCESS;
}


// A robust input buffer engine
EFI_STATUS ReadLine(CHAR16 *Buffer, UINTN BufferSize) {
    // UEFI specs 2.11, section 2.3.1 Data types.
    UINTN Index = 0;
    EFI_INPUT_KEY Key;
    UINTN EventIndex;
    
    while (TRUE) {
        // Correct usage of WaitForEvent: Traps CPU until ConIn registers a keystroke
        gBS->WaitForEvent(1, &(gST->ConIn->WaitForKey), &EventIndex);
        gST->ConOut->EnableCursor(gST->ConOut, TRUE);
        gST->ConIn->ReadKeyStroke(gST->ConIn, &Key);

        // Handle Enter key (Carriage Return)
        if (Key.UnicodeChar == CHAR_CARRIAGE_RETURN) {
            Print(L"\n");
            break;
        }
        
        // Handle Backspace
        if (Key.UnicodeChar == CHAR_BACKSPACE) {
            if (Index > 0) {
                Index--;
                // \b moves cursor left, space overwrites char, \b moves left again
                Print(L"\b \b"); 
            }
            continue;
        }
        
        // Store valid visible characters (ignore weird control keys)
        if (Key.UnicodeChar >= L' ' && Key.UnicodeChar <= L'~') {
            // Prevent buffer overflow
            if (Index < BufferSize - 1) {
                Buffer[Index] = Key.UnicodeChar;
                Index++;
                // Print using format specifier to prevent memory garbage
                Print(L"%c", Key.UnicodeChar); 
            }
        }
    }
    
    // Always null-terminate C strings!
    Buffer[Index] = L'\0'; 
    return EFI_SUCCESS;
}