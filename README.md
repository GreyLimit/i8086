# i8086
A (not so) simple minded x86 assembler primarily targeting 8086, 80186 and 80286 16-bit CPUs.

Support for the 80286 is still missing, but intended.  Support for 32-bit CPUs is an aspirational objective, but not a primary requirement.

## Disclaimer

I absolutely **know** how inefficient some (many) of the algorithmic elements of this assembler are.  What these simplistic and slow systems provide is two benefits for this software at this point.  They are easy to prove (or more easily), visually, that they are correct and they tend to have a smaller memory foot print.  These benefits naturally come at the cost of CPU time when running the assembler, which isn't an practical issue on a modernish PC.

This all being said, doesn't make it right.

## How did I get here?
Having written (and sometimes) completed a number of assemblers for older CPUs the thought crossed my mind: Why not do an Intel 8086 assmbler, how hard can it be?

The answer is that (strangely reaffirming my opinions formed "back in the day" when 286 PCs were the new thing), these CPUs are damn fiddly and complicated.  While the machine code *does* have some regularity within its definitions, it's not what I would call 'orthogonal' in the way other contemporary CPUs were.  The net result is that where I have been able to capture a functional assembler for other processors in a single managebly sized source file, for this CPU (and relatives) it has required a more complex approach spanning approximately 53 files (both source and header files).

## Where is here (June '24)?

All instructions present in the 8086/88 and 80186/88 have been encoded into the assembler though comprehensive validation and testing that these generate the correct machine instructions has **not** been attempted.

The assembler can *directly* generate DOS '.com' executables (and by inference CP/M86 '.cmd' executables):  "Hello World!" has been written and executed.

Ongoing work on the '--dump-opcodes' option (enabled when compiled with the VERIFICATION macro defined) highlighted a range of errors thus proving how worth while this coding effort has been.  The intended objective of this options is to display all of the instructions which the assembler will recognise in a format that could be the input to an external validation mechanism.

The assembler contains the concepts of segments (associated with segment registers) and segments brought together forming a logical group (where all associated segment registers point to the same paragraph).  Support for object file creation (as input to a separate linker) nor direct '.exe' creation has not been coded.

## Objectives from here

* Encode the missing 286 instructions.
* Provide output to object files and executables.  While, at the moment, the contemporary Intel document on formats for these files is being used as a guide, this might not be the end point of this work.
* Reduce the assemblers final executable footprint such that an operational version can be created with the bounds of a '.com' executable: 64 KBytes.  A key focus on achieveing this go will be the reduction of non-essential static string data (i.e. error messages) through replacement of numerical equivalents.

## Brief Usage

To assemble a file call the assembler with options before the name of the assembler source code file:

```
      i8086 [{options}] {filename}
```
The available list of options can be displayed by running the command with '--help' as the only argument.  This will generate the following (or similar) output:

```
Options:-
	--ignore-keyword-case   Make keywords case insensitive
	--ignore-label-case     Make labels case insensitive
	--com                   Output a '.COM' executable
	--exe                   Output a '.EXE' executable
	--obj                   Output a '.OBJ' linkable file
	--hex                   Output binary files in ASCII
	--ascii                 Output binary files in ASCII
	--listing               Produce detailed listing
	--8086                  Only permit 8086 code
	--8088                  Only permit 8088 code
	--80186                 Only permit 80186 and earlier code
	--80188                 Only permit 80188 and earlier code
	--80286                 Only permit 80286 and earlier code
	--access-segments       Permit assignment to segments
	--position-dependent    Permit fixed/absolute position code
	--version               Display program version details
	--help                  Show this help
	--dump-opcodes          Dump internal opcode table
	--verbose               Show extra details during assembly
	--very-verbose          Show even more detail
```

## Revelations

Perhaps it is a little late chronologically to point out the errors in the book "Programming the 8086 8088" (Sybex 1983), they are numerous, confusing and caused much head scratching.  The genuine Intel documents are, naturally, more reliable if not as easy to comprehend.  Worth noting though that the Intel book "iAPX86/88, 186/188 User's Manual, Programmer's Reference" has **evactly** the same formatting and layout as the earlier Sybex book "Programming the 8086 8088" (which is a far easier read than Intels own 8086 manual) but without the errors.
