#ifndef COMMAND_AND_CONTROL_PROTOTYPES_IN_VOID
#define COMMAND_AND_CONTROL_PROTOTYPES_IN_VOID

EFI_STATUS ExecuteCommand(UINTN No_of_Command, UINTN MaxWordLen, CHAR16 Args_Matrix[No_of_Command][MaxWordLen] );
EFI_STATUS handleHelp(UINTN No_of_Command, UINTN MaxWordLen, CHAR16 Args_Matrix[No_of_Command][MaxWordLen]);
EFI_STATUS handleLs(UINTN No_of_Command, UINTN MaxWordLen, CHAR16 Args_Matrix[No_of_Command][MaxWordLen] );
EFI_STATUS printDirectoryContent(EFI_FILE_PROTOCOL *RootDir, CHAR16 *subDirName) ;

#endif