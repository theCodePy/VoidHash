#include <uchar.h>
#include <stdint.h>

// common uefi data Types: UEFI Specs 2.10 section 2.3.1
typedef uint16_t UINT16;
typedef uint32_t UINT32;
typedef uint64_t UINT64;
typedef uint64_t UINTN;
typedef char16_t CHAR16; //UTF-16, But will use UCS-2 code points 0x0000-0xFFFF

typedef UINTN   EFI_STATUS;
typedef VOID*   EFI_HANDLE;

//Taken from EDK-II at
// https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Base.h
#define IN
#define OUT
#define OPTIONAL
#define CONST const

// Taken from gnu-efi at
// https://github.com/vathpela/gnu-efi/blob/master/inc/x86_64/efibind.h
#define EFIAPI __attribute__((ms_abi)) // x86_64 MicroSoft calling convention

// EFI_STATUS codes: UEFI Specs 2.10  Appendix D
#define EFI_SUCCESS 0

//EFI Simple Text Input Protocol: UEFI Specs 2.10 Section 12.3 
// Forward declare struct for this to work and compile
typedef struct  EFI_SIMPLE_TEXT_INPUT_PROTOCOL
EFI_SIMPLE_TEXT_INPUT_PROTOCOL;

typedef struct {
    UINT16 ScanCode;
    CHAR16 UnicodeChar;
}EFI_INPUT_KEY ;

typedef EFI_STATUS (EFIAPI *EFI_INPUT_READ_KEY) (
    IN EFI_SIMPLE_TEXT_INPUT_PROTOCOL  *This,
    OUT EFI_INPUT_KEY                  *Key
);

typedef struct EFI_SIMPLE_TEXT_INPUT_PROTOCOL {
    void*       Reset;
    EFI_INPUT_READ_KEY  ReadKeyStroke;
    void*       WaitForKey;

} EFI_SIMPLE_TEXT_INPUT_PROTOCOL;
