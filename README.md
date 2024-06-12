# i8086
A (not so) simple minded x86 assembler primarily targeting 8086, 80186 and 80286 16-bit CPUs.

Support for 32-bit CPUs is an aspirational objective, but not aprimary requirement.

## How did I get here?
Having written (and sometimes) completed a number of assemblers for older CPUs the thought crossed my mind: Why not do an Intel 8086 assmbler, how hard can it be?

The answer is that (strangely reaffirming my opinions formed "back in the day" when 286 PCs were the new thing), these CPUs are damn fiddly and complicated.  While the machine code *does* have some regularity within its definitions, it's not what I would call 'orthoganal' in the way other contemporary CPU were.  The net result is that where I have been able to capture a functional assembler for other processors in a single managebly sized source file, for this CPU (and relatives) it has required a more complex approach spanning approximately 25 source files.

And it's still not finished.

## Where is here?

Within the limits of the x86 instructions so far encoded the assembler can *directly* generate DOS '.COM' executables (and by inference CP/M86 executables):  "Hello World!" has been written and executed.

Support for object file creation (as input to a separate linker) has not been coded.

June '24.
