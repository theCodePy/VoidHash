
installed packages:
`build-essential`: This is a bundle of core Linux development tools (specifically the make utility and standard C libraries). EDK II uses make to build its own internal tools before it can even look at your VoidHash code.
`uuid-dev`: The Universally Unique Identifier library. In UEFI, everything—every protocol, every driver, every file—is identified by a 128-bit GUID (Globally Unique Identifier). EDK II needs this library to generate and read those GUIDs.
`iasl`: The Intel ACPI Source Language compiler. ACPI handles hardware power management and configuration. Even though VoidHash doesn't focus on power management, the EDK II framework requires this compiler to build the baseline firmware environment.
`git`: The version control system. You need this specifically because EDK II relies on "submodules" (other repositories nested inside it), and standard download links won't fetch them.
`gcc`: The GNU Compiler Collection. This is the actual engine that translates your VoidHash.c human-readable code into machine-readable assembly instructions.
`qemu-system-x86`: The emulator. This acts as your virtual motherboard and CPU so you don't have to constantly reboot your physical laptop to test your code.
`ovmf`: Open Virtual Machine Firmware. By default, QEMU boots using 1980s Legacy BIOS. OVMF is the actual modern UEFI firmware file that you inject into QEMU to make it act like a modern motherboard.
`python3-distutils`: Behind the scenes, EDK II's build system (GenFw, parsing .inf files) is heavily driven by Python scripts. This provides the necessary Python modules for those scripts to execute.