# Using VM as a shared library

This is for shared library only. Outputs `libizmirvmlib.so` in the build directory.
Check the checks files for how to use this shared library.

For reference, after running `cmake`, there will be a file created `izmirvm-vm-main.c` which is a CLI tool for interacting with VM, (running instructions/inspecting assembly etc.). You can use this

All the `*-syntax.*` files and lex/bison files are not needed here.
The exported symbols from the VM shared library can be imported into any other language and can be used.
