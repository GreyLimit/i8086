# i8086
A (not so) simple minded x86 assembler primarily targeting 8086, 80186 and 80286 16-bit CPUs.

Support for 32-bit CPUs is an aspirational objective, but not a primary requirement.

## How did I get here?
Having written (and sometimes) completed a number of assemblers for older CPUs the thought crossed my mind: Why not do an Intel 8086 assembler, how hard can it be?

The answer is that (strangely reaffirming my opinions formed "back in the day" when 286 PCs were the new thing), these CPUs are damn fiddly and complicated.  While the machine code *does* have some regularity within its definitions, it's not what I would call 'orthogonal' in the way other contemporary CPU were.  The net result is that where I have been able to capture a functional assembler for other processors in a single manage-ably sized source file, for this CPU (and relatives) it has required a more complex approach spanning approximately 55 source and header files.

And it's still not finished.

## Where is here (June '24)?

Within the limits of the x86 instructions so far encoded the assembler can *directly* generate DOS '.COM' executables (and by inference CP/M86 executables):  "Hello World!" has been written and executed.

All instructions present in the 8086/88 and 80186(88) have been encoded into the assembler though comprehensive validation and testing that these generate the correct machine instructions has not been attempted.

As part of an effort to provide tooling to enable this verification work on a '--dump-opcodes' option (enabled when compiled with VERIFICATION defined) has been undertaken.

This has already highlighted a range of errors thus proving how valuable this coding effort has been.  The purpose of this options is to display all of the instructions which the assembler will recognise providing a direct input to an external validation mechanism.  When combined with the '--verbose' or '--very-verbose' options a more detailed and longer output can be generated.

No support for object file creation (as input to a separate linker) or direct '.exe' creation has been coded so far.

## Revelations

Perhaps a little late to point out the errors in the book "Programming the 8086 8088" (Sybex 1983).  They are numerous, confusing and caused much head scratching.  The genuine Intel documents are, naturally, more reliable if not as easy to comprehend.

It is interesting to note that the Intel manual released for the 80186 adopted the Sybex manual layout, and that Sybex doesn't appear to released any further books on this subject.


