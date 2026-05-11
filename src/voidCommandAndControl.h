#ifndef COMMAND_AND_CONTROL_PROTOTYPES_IN_VOID
#define COMMAND_AND_CONTROL_PROTOTYPES_IN_VOID

EFI_STATUS ExecuteCommand(UINTN No_of_Command, UINTN MaxWordLen, CHAR16 Args_Matrix[No_of_Command][MaxWordLen] );
EFI_STATUS handleHelp(UINTN No_of_Command, UINTN MaxWordLen, CHAR16 Args_Matrix[No_of_Command][MaxWordLen]);
EFI_STATUS handleLs(UINTN No_of_Command, UINTN MaxWordLen, CHAR16 Args_Matrix[No_of_Command][MaxWordLen] );
EFI_STATUS handleCat(UINTN No_of_Command, UINTN MaxWordLen, CHAR16 Args_Matrix[No_of_Command][MaxWordLen] );
EFI_STATUS handleTest( UINTN No_of_Command, UINTN MaxWordLen, CHAR16 Args_Matrix[No_of_Command][MaxWordLen]);
EFI_STATUS handleVoidHash( UINTN No_of_Command, UINTN MaxWordLen, CHAR16 Args_Matrix[No_of_Command][MaxWordLen]) ;
VOID EFIAPI CoreCrackingFunction(VOID *Buffer) ;


#endif