#include <efi.h>

// EFI Image Entry piont
EFI_STATUS EFIAPI efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    (void)ImageHandle; //prevet compiler warning
    
    // Set text to yellow fg/ green bg
    SystemTable->ConOut->SetAttribute( SystemTable->ConOut, EFI_TEXT_ATTR(EFI_YELLOW, EFI_GREEN));
    
    // Clear screen to bg colour
    SystemTable->ConOut->ClearScreen(SystemTable->ConOut);

    SystemTable->ConOut->OutputString(SystemTable->ConOut, u"Hello World\r\n\r\n");

    // Set text to REG fg/ BLACK bg
    SystemTable->ConOut->SetAttribute( SystemTable->ConOut, EFI_TEXT_ATTR(EFI_RED, EFI_BLACK));
    SystemTable->ConOut->OutputString(SystemTable->ConOut, u"press any key to shutdown");

    // Wait Until key press, then return
    EFI_INPUT_KEY key;
    while (SystemTable->ConIn->ReadKeyStroke(SystemTable->ConIn, &key) != EFI_SUCCESS);

    // Shutdown does not return
    SystemTable->RunTimeServices->ResetSystem(EfiResetShutdown, EFI_SUCCESS, 0, NULL);

    //Should never get here
    return EFI_SUCCESS;
}