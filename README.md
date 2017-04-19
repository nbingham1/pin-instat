# pin-instat: X86 Instruction Profiler

**pin-instat** is a [PIN tool](https://software.intel.com/en-us/articles/pintool/)
to collect runtime information of each instruction.
Its main purpose is gathering program execution data to inform computer architecture design choices.
The information collected includes:

* **Opcode:**
* **The opcode's category**
* **Instruction execution count by opcode**
* **Instruction-operand max-bitwidth distribution by opcode**
* **Instruction execution count by address**

## How to Use?

For Linux build:

1. Copy `make.example` to `make.sh` and make your local changes.
2. Run `bash make.sh`.
3. The pin tool will be built as `obj-intel64/instat.so` for x64.
4. To profile a program, say `ls`, run `PINDIR/pin.sh -t obj-intel64/instat.so -- ls`

For Windows build:

1. Copy `makefile.nmake.sample` to `makefile.nmake`.
Note that this make file is very dependent on the version of MSVC.
If you are not using Microsoft Visual C++ 2010 Express, you probably have to tweak it.
2. Open a MSVC command prompt (from start menu) and run `nmake -f makefile.nmake`.
3. The pin tool will be `instat.dll`.
4. To profile a program, say `notepad.exe`, run `PINDIR\pin.bat -t TOOLDIR\instat.dll -- C:\windows\notepad.exe`. The output will be saved only after notepad exits.
