# An example from using Jitter

The main idea behind is creating a `<LANGNAME>vm.jitter` file that spits out `vm-main.c` file that can read from a file or from console bunch of VM instructions and execute them. You can also look at assembly generated from the JIT.

`izmir` stuff is taken from the jitter workshop. Fiddling with `autoconf` and `automake` was a nightmare, I prefer using `cmake` instead.

First build, `izmir` (interpreter) and `izmirvm` (VM) targets.

Interpreter spits out VM instructions, and VM binary takes instructions from console and executes them.

For example:

Interpreter only:

```sh
$ echo 'print 2; print 7;' | ./build/izmir -
pushconstant 2
print
pushconstant 7
print
```

Interpreter with VM:

```sh
echo 'print 2; print 7;' | ./build/izmir - | ./build/izmirvm -
2
7
```

Interpreter with VM disassemble:

```sh
$ echo 'print 2; print 7;' | ./build/izmir - | ./build/izmirvm - --disassemb
le
# !BEGINBASICBLOCK 0x7dc2a837f680 (4 B):
    0x00007dc2a837f680 f3 0f 1e fa          	endbr64
# pushconstant/nR 0x2 (21 B):
    0x00007dc2a837f684 41 bc 02 00 00 00    	movl   $0x2,%r12d
    0x00007dc2a837f68a f3 0f 1e fa
.....
```

## Prereqs:

```sh
sudo apt install texinfo libtool flex bison help2man libgc1 libgc-dev autoconf automake cmake
```

## Automatic installation

1. Install jitter, build vm and interpreter all at once

```
mkdir build && cd build
cmake ..
make
```

For manual installation (go to [this](https://github.com/trickster/jitter-using-cmake/tree/3988147a90f68a89930facafc417adae4663f14c) commit)
