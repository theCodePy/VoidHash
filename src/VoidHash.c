#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseMemoryLib.h>

// Function Prototypes
EFI_STATUS ReadLine(CHAR16 *Buffer, UINTN BufferSize);
EFI_STATUS StrStrip(CHAR16 *StrInput, CHAR16 C);
EFI_STATUS StrCountWords(CHAR16 *StrInput, CHAR16 separator, UINTN *Args_Status);
// ArgsSplit() funciton prototype...
EFI_STATUS ArgsSplit(CHAR16 *StrInput, CHAR16 separator, UINTN row, UINTN col, CHAR16 Args_Matrix[row][col]);

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
    ArgsSplit(InputCommand, L' ', Args_Status[0], Args_Status[1], Args_Matrix);

    // checking and testing the output
    for (UINTN i = 0; i<Args_Status[0]; i++){
      Print(L"Row %d = %s\n", i, Args_Matrix[i]);
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

// to split the arguments for easiear access. 
EFI_STATUS ArgsSplit(CHAR16 *StrInput, CHAR16 separator, UINTN row, UINTN col, CHAR16 Args_Matrix[row][col]){
  UINTN sepFlag = 0;
  UINTN Rows=0;
  UINTN Columns;
  UINTN i=0;

  while (StrInput[i] != L'\0'){
    if (StrInput[i] != separator){
      if (sepFlag == 0){
        Columns = 0;
        sepFlag = 1;
      }
      Args_Matrix[Rows][Columns] = StrInput[i];
      Columns++ ;
      
    } else {
      if (sepFlag == 1){
        sepFlag = 0;
        Args_Matrix[Rows][Columns] = L'\0';
        Rows++;
      }
    }
    i++;
  }
  // Adding null terminator to the one last word..
  if (sepFlag == 1){
    Args_Matrix[Rows][Columns] = L'\0';
  }

  return EFI_SUCCESS;
}


// to Count how many words are there in a sentance.
// It returns Args_Status[0]=(number of words), Args_Status[1]=(Max word Length)
EFI_STATUS StrCountWords(CHAR16 *StrInput, CHAR16 separator, UINTN *Args_Status){
  // sefty check first
  if (Args_Status == NULL || StrInput == NULL){
    return EFI_SUCCESS;
  }
  
  // resetting counters && discarding garbage
  Args_Status[0]=0;
  Args_Status[1]=0;

  UINTN sepO_F = 1;
  UINTN Count = 0;
  UINTN i = 0;
  UINTN MaxW = 0;

  // Counting words 
  while (StrInput[i] != L'\0'){
    if (StrInput[i] != separator){
      // this checks if we are inside a new word after a separator 
      if (sepO_F == 1){
        Count ++;
        sepO_F = 0;
        MaxW = 1;
      } else {
        MaxW++;
      }
    } else {
        if ((sepO_F==0) && (Args_Status[1] < MaxW)){
          Args_Status[1] = MaxW;
        }
        sepO_F=1;
      }
    i++;
  }
  // updating the word count and word length. 
  Args_Status[0] = Count;
  if (Args_Status[1] < MaxW){
    Args_Status[1] = MaxW;
  }

  return EFI_SUCCESS;
}


// strip spaces from user input string
EFI_STATUS StrStrip(CHAR16 *StrInput, CHAR16 C){
  // Some Safty checks... blah blah blah..
  if (StrInput==NULL){
    return EFI_INVALID_PARAMETER;;
  }
  UINTN lenStrInput = StrLen(StrInput);
  if (lenStrInput==0) return EFI_SUCCESS;

  UINTN i;
  UINTN startIndex;
  UINTN endIndex;

  startIndex = 0;
  while (StrInput[startIndex] == C && startIndex < lenStrInput){
    startIndex ++;
  }
  
  if (startIndex == lenStrInput){
    StrInput[0] = L'\0';
    return EFI_SUCCESS;
  }

  // finding the ending index of actual string without the unwanted character
  endIndex = lenStrInput - 1;
  while (StrInput[endIndex] == C && endIndex > startIndex) {
    endIndex--;
  }
 
  //update the string 
  if (startIndex!=0){
    for (i=0; i<=(endIndex-startIndex); i++){
      StrInput[i] = StrInput[i + startIndex];
    }
  }

  // terminate the string with '\0'
  StrInput[endIndex - startIndex + 1] = L'\0';
  
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