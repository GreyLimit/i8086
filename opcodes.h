/*
 *	opcodes
 *	=======
 * 
 *	Structures, definitions and tables that capture the whole
 *	mess that is the 16-bit x86 instruction set.
 */

#ifndef _OPCODES_H_
#define _OPCODES_H_

/************************
 *			*
 *	MODIFIERS	*
 *			*
 ************************/


/*
 *	Capture the possible modifiers that can applied.
 */
typedef enum {
	no_modifier		= 000,
	byte_modifier		= 001,
	word_modifier		= 002,
	ptr_modifier		= 004,
	near_modifier		= 010,
	far_modifier		= 020,
	
	size_modifiers		= ( byte_modifier | word_modifier ),
	range_modifiers		= ( near_modifier | far_modifier )
} modifier;

/*
 *	Define a value representing the maximum number of simultaneous
 *	modifiers that can be applied.
 */
#define MAXIMUM_MODIFIERS 3

/*
 *	Define structure and routines that convert a component into
 *	its corresponding modifier value and a matching routine which
 *	performs the reverse operation.
 */
extern modifier map_modifier( component m );
extern void expand_modifier( modifier input, component *output, int max );

/************************
 *			*
 *	PREFIXES	*
 *			*
 ************************/

/*
 *	Prefix bit map.
 */
typedef enum {
	/*
	 *	Note that, in the comments, a number of the outlined
	 *	prefixes have the same prefix values.  This is valid
	 *	because they apply to different instructions and so
	 *	there is no ambiguity about the actions required.
	 */
	no_prefix		= 000000,
	lock_prefix		= 000001,	/* 0xF0: ADC, ADD, AND, BTC, BTR, BTS, CMPXCHG, CMPXCHG8B, CMPXCHG16B, DEC, INC, NEG, NOT, OR, SBB, SUB, XADD, XCHG, XOR */
	rep_prefix		= 000002,	/* 0xF3: INS, LODS, MOVS, OUTS, STOS  */
	rep_eq_prefix		= 000004,	/* 0xF3: CMPS, CMPSB, CMPSD, CMPSW, SCAS, SCASB, SCASD, SCASW */
	rep_ne_prefix		= 000010,	/* 0xF2: CMPS, CMPSB, CMPSD, CMPSW, SCAS, SCASB, SCASD, SCASW */
	CS_prefix		= 000020,	/* 0x2E: ?? */
	DS_prefix		= 000040,	/* 0x3E: ?? */
	SS_prefix		= 000100,	/* 0x36: ?? */
	ES_prefix		= 000200,	/* 0x26: ?? */
	branch_ignored_prefix	= 000400,	/* 0x2E: Not implement in this assembler */
	branch_taken_prefix	= 001000,	/* 0x3E: Not implement in this assembler */
	operand_size_prefix	= 002000,	/* 0x66: */
	address_size_prefix	= 004000,	/* 0x67: */
	/*
	 *	Group prefixes together to simplify opcode encoding.
	 *
	 *	The prefix consolidations relating to the 8086 through
	 *	80286 processors:
	 */
	rep_test_prefix		= ( rep_eq_prefix | rep_ne_prefix ),
	segment_prefixes	= ( CS_prefix | DS_prefix | SS_prefix | ES_prefix ),
	lock_n_segments		= ( lock_prefix | segment_prefixes ),
	repeat_n_segments	= ( rep_test_prefix | segment_prefixes )
} opcode_prefix;

/*
 *	Convert a prefix bitmap into a byte stream.  Return number of bytes
 *	placed into the buffer or ERROR if there has been an error.
 */
extern int encode_prefix_bytes( opcode_prefix prefs, byte *buffer, int max );

/*
 *	Convert component prefix identifiers to an internal
 *	bit mapped value.
 */
extern opcode_prefix map_prefix( component pref );

/*
 *	Convert a segment register number into a prefix bitmap bit
 */
extern opcode_prefix map_segment_prefix( byte segment_reg );


/********************************
 *				*
 *	EFFECTIVE ADDRESS	*
 *				*
 ********************************/

/*
 * 	Effective Address Syntax
 * 	========================
 *
 *	The following sections define the syntax of various EAs:
 *
 * 	Register Mode:
 * 	--------------
 *
 * 	Registers are simply named.  Their role as source or destination
 *	is implied by their position in the assembly language source
 *	code.  Register AX is the destination (the first argument) and
 *	DI is the source (the second argument):
 *
 *		MOV	AX,DI
 *		ADD	AX,BX
 *
 * 	Immediate Mode:
 *	---------------
 *
 *	Immediate values (numerical constants, addresses of labelled
 *	locations) are simply specified with no decoration:
 *
 *		MOV	AL,45
 *		MOV	SI,DataTable
 *
 *	Immediate data cannot, obviously, be the destination location
 *	and so is ALWAYS the second argument.
 *
 *	Indirect Mode:
 *	--------------
 *
 * 	An immediate value is used as a 16 bit segment offset to where
 *	the data is located (or to be placed) in memory:
 *
 * 		MOV	AX,[DataTable]
 *		MOV	[0200],AX
 *
 * 	Note that access to memory needs the addition of a segment
 *	register to create the actual real memory address (20 bits
 *	on the 8086).  The default for Indirect Mode access is the DS
 *	register (the Data Segment).  However, where labelled memory
 *	is being accessed the assembler will insert appropriate prefix
 *	byte if the label is associated with a different segment register.
 *
 *	Register Indirect Mode:
 *	-----------------------
 *
 *	Also called "Pointer Mode" in the assembler source.
 *
 *	The effective address is in one of the 'indirect' registers
 *	SI, DI or BX.
 *
 *	Note that BP is NOT included in this list.  The position where
 *	this would "logically" sit is used by the "Indirect Mode"
 *	allowing for the direct addressing of data in memory.
 *
 *		ADD	AX,[BX]
 *		MOV	[SI],AL
 *
 *	Implicit segment registers need to be considered.  For these
 *	registers these are as follows:
 *
 *		SI	DS
 *		DI	ES
 *		BX	DS
 *
 *	Base Index Mode:
 *	----------------
 *
 *	In this the effective address is sum of a base register and an
 *	index register.
 *
 *	Base registers: BX, BP
 *	Index registers: SI, DI
 *
 *	The physical memory address is calculated according to the base
 *	register:
 *
 *		MOV AL,[BP+SI]
 *		MOV AX,[BX+DI]
 *
 *	Note default segments:
 *
 *		SI	DS
 *		DI	ES
 *		BX	DS
 *		BP	SS
 *
 *	Indexed Mode:
 *	-------------
 *
 *	In this type of addressing mode the effective
 *	address is sum of index register and displacement:
 *
 *		MOV AX, [SI+2000]
 *		MOV AL, [DI+3000]
 *
 *	Base Mode:
 *	----------
 * 
 *	Based mode – In this the effective address is the sum of base
 *	register and displacement:
 *
 *		MOV AL, [BP+0100]
 *
 *	Base Indexed Mode:
 *	------------------
 * 
 *	Based indexed displacement mode – In this type of addressing
 *	mode the effective address is the sum of index register, base
 *	register and displacement:
 *
 *		MOV AL, [SI+BP+2000]
 */

/*
 *	Capture the argument components we have found (from the above)
 */
typedef enum {
	/*
	 *	Define the various possible components which make up the
	 *	syntax of an effective address:
	 */
	ac_empty		= 00000,	/* Nothing! */
	ac_brackets		= 00001,	/* '[...]' */
	ac_byte_reg		= 00002,	/* AL, BL, CL, DL, AH, BH, CH, DH */
	ac_word_reg		= 00004,	/* AX, BX, CX, DX, SP, BP, SI, DI */
	ac_acc_reg		= 00010,	/* AL, AX */
	ac_pointer_reg		= 00020,	/* BX, SI, DI */
	ac_base_reg		= 00040,	/* BX, BP */
	ac_index_reg		= 00100,	/* DI, SI */
	ac_segment_reg		= 00200,	/* CS, DS, SS, ES */
	ac_immediate		= 00400,	/* Numerical expression */
	ac_seg_override	= 01000,	/* Over-ride default segment register */
} arg_component;

/*
 *	Define a bitmap containing the various argument types
 *	that an opcode can have.  I'll call them all effective
 *	addresses but this isn't strictly accurate (an immediate
 *	constant value isn't an address).
 */
typedef enum {
	/*
	 *	For where there is nothing.
	 */
	ea_empty		= 0000000,
	/*
	 *	Now piece together components to specify actual
	 *	effective address combinations valid in an assembly
	 *	instruction.
	 *
	 *	The following block are the individual values which
	 *	are returned from combining argument components into
	 *	a recognised syntactic form.
	 */
	ea_byte_acc		= 0000001,
	ea_byte_reg		= 0000002,
	ea_word_acc		= 0000004,
	ea_word_reg		= 0000010,
	ea_immediate		= 0000020,
	ea_indirect		= 0000040,
	ea_pointer_reg		= 0000100,
	ea_base_disp		= 0000200,
	ea_index_disp		= 0000400,
	ea_base_index_disp	= 0001000,
	ea_segment_reg		= 0002000,
	/*
	 *	To address the difficulty of distinguishing between
	 *	NEAR and FAR subroutine calls, the Effective Address
	 *	categorisation is extended to include specific 'FAR'
	 *	versions of the above 'NEAR' definitions.
	 *
	 *	Of these codes only the FAR IMMEDIATE needs explicit
	 *	code supports as all other FAR EAs are identical to
	 *	their NEAR versions with only the data at the address
	 *	generated being different.
	 */
	ea_far_immediate	= 0010000,
	ea_far_indirect		= 0020000,
	ea_far_pointer_reg	= 0040000,
	ea_far_base_disp	= 0100000,
	ea_far_index_disp	= 0200000,
	ea_far_base_index_disp	= 0400000,
	/*
	 *	Now, group various combinations of the above into
	 *	forms that align with the various machine code
	 *	instructions.
	 *
	 *	These are the values which are used as search parameters
	 *	in the instruction tables and so can contain multiple
	 *	values (from above) combined together (with logical OR).
	 */
	ea_byte_registers	= ( ea_byte_acc | ea_byte_reg ),
	ea_word_registers	= ( ea_word_acc | ea_word_reg ),
	ea_accumulators		= ( ea_byte_acc | ea_word_acc ),
	ea_registers		= ( ea_byte_reg | ea_word_reg ),
	ea_all_reg		= ( ea_byte_registers | ea_word_registers ),
	ea_mod_reg_adrs		= ( ea_all_reg | ea_indirect | ea_pointer_reg | ea_base_disp | ea_index_disp | ea_base_index_disp ),
	ea_mod_wreg_adrs	= ( ea_word_registers | ea_indirect | ea_pointer_reg | ea_base_disp | ea_index_disp | ea_base_index_disp ),
	ea_mem_mod_adrs		= ( ea_indirect | ea_pointer_reg | ea_base_disp | ea_index_disp | ea_base_index_disp ),
	/*
	 *	Wrap up all the FAR EAs which generate a memory address
	 */
	ea_far_mod_reg_adrs	= ( ea_far_indirect | ea_far_pointer_reg | ea_far_base_disp | ea_far_index_disp | ea_far_base_index_disp )
} effective_address;


#if defined( VERIFICATION )||defined( DEBUG )

	/*
	 *	Debugging code to output an AC or EA bit map in text to stdout.
	 */
	extern void show_ac_bitmap( arg_component ac );
	extern void show_ea_bitmap( effective_address ea );

#endif

/*
 *	The following data structure is used to bring together
 *	elements of the instruction construction which rely
 *	of the choice of registers used.
 *
 *	The following table (derived from the Intel 80186 programming
 *	manual) captures the relationships between segment registers,
 *	offset registers and possible segment over-ride prefix bytes
 *	(Table 3-3, Page 3-11).
 *
 *	Type of memory		Default		Alternate	Offset
 *	reference		Segment base	segment	base	Register
 * 	--------------		------------	------------	--------
 *	Instruction Fetch	CS		None		IP
 *	Stack Operation		SS		None		SP
 *	Variable		DS		CS,ES,SS	Effective Adrs
 *	String Source		DS		CS,ES,SS	SI
 *	String Destination	ES		None		DI
 *	BP Used as Base Reg	SS		CS,DS,ES	Effective Adrs
 *	BX Used as Base Reg	DS		CS,ES,SS	Effective Adrs
 */
typedef struct {
	component	comp;
	arg_component	ac;
	byte		reg_no,
			ptr_reg_no,
			base_index_reg_no,
			segment;
} register_data;

/*
 *	A routine that returns the address of the register_data
 * 	record for a specific register.
 */
extern register_data *register_component( component comp );

/*
 *	Define a data structure used while breaking the
 *	opcode into 'effective address' components.
 */
typedef struct {
	effective_address	ea;			/* What has been found as a bitmap. */
	modifier		mod;			/* Explicit modifiers specified. */
	byte			registers;		/* Number of registers supplied */
	register_data		*reg[ MAX_REGISTERS ];	/* Details about each each register. */
	byte			segment_override;	/* Over-ride the default segment register. */
	constant_value		immediate_arg;		/* Any numerical constant value */
} ea_breakdown;

/*
 *	Define the structures which are used to hold the machine code
 *	definitions allowing the assembler to build the output instructions.
 */
typedef struct _opcode {
	/*
	 *	Identification components:
	 */
	component		op;
	mnemonic_flags		flags;
	opcode_prefix		prefs;
	modifier		mods;
	byte			args;
	effective_address	arg[ MAX_OPCODE_ARGS ];
	/*
	 *	Encoding components:
	 */
	int			encoded;
	word			encode[ MAX_OPCODE_ENCODING ];
} opcode;


/*
 *	The encoding of the instruction is captured as a series of
 *	16-bit 'instruction' words which combine an instruction code
 *	(the ACTion) and one or more arguments to that instruction.
 */

/*
 *	Encoding of the high level action codes: the ACTions.  These
 *	are fixed to 4 bits occupying the top four bits of the
 *	encoded word value.
 */
#define ACT_LSB		12
#define ACT_BITS	4
#define ACT(n)		VALUE((n),ACT_BITS,ACT_LSB)
#define GET_ACT(w)	EXTRACT((w),ACT_BITS,ACT_LSB)

/*
 *	Across the actions are a number of specific constant values
 *	with associated meanings.  The following macros assign symbols
 *	to these values (hopefully) reducing the opportunity for coding
 *	errors in the opcode table.
 */
 
/*
 *	Actions where the signed status of a numerical value is pertinent
 *	will use one of these values to enumerate the appropiate
 *	interpretation (contributes towards range checking and error
 *	detection)
 */
#define SIGN_IGNORED	0
#define SIGN_UNSIGNED	1
#define SIGN_SIGNED	2

/*
 *	Actions where the data size is pertinent will use one of these
 *	values to capture the appropiate data sizing requirement.
 */
#define DATA_SIZE_BYTE	0
#define DATA_SIZE_WORD	1
#define DATA_SIZE_NEAR	2
#define DATA_SIZE_FAR	3

/*
 *	Relative ranching instructions can be found using byte, word or
 *	*both* options (depending on the instruction), use as appropiate.
 */
#define RANGE_BYTE	1
#define RANGE_WORD	2
#define RANGE_BOTH	3

/*
 *	Instructions with two arguments (eg 'mov') need to explicitly
 *	have the direction of data movement specified.  These values
 *	serve that role.
 */
#define DIRECT_TO_EA	0
#define DIRECT_TO_REG	1

/*
 *	Set Byte (Action 0)
 *
 *	Provide static data forming the basic for the machine
 *	code instruction.
 *
 *	SB(v)		v = value to add to instruction
 */
#define SB_ACT		0
#define SB_VALUE_LSB	0
#define SB_VALUE_BITS	8

#define SB(v)		(ACT(SB_ACT)|VALUE((v),SB_VALUE_BITS,SB_VALUE_LSB))
#define SB_VALUE(w)	EXTRACT((w),SB_VALUE_BITS,SB_VALUE_LSB)

/*
 *	Identify Data Size (Action 1)
 *
 *	Determine, from the indicated argument, the size of the
 *	data to be handled (byte or word).
 *
 *	IDS(a,g)	a = Argument Number
 *			g = Sign,	0:Ignore
 *					1:Unsigned
 *					2:Signed
 */
#define IDS_ACT		1
#define IDS_ARG_LSB	0
#define IDS_ARG_BITS	3
#define IDS_SIGN_LSB	3
#define IDS_SIGN_BITS	2

#define IDS(a,g)	(ACT(IDS_ACT)|VALUE((a),IDS_ARG_BITS,IDS_ARG_LSB)|VALUE((g),IDS_SIGN_BITS,IDS_SIGN_LSB))

#define IDS_ARG(w)	EXTRACT((w),IDS_ARG_BITS,IDS_ARG_LSB)
#define IDS_SIGN(w)	EXTRACT((w),IDS_SIGN_BITS,IDS_SIGN_LSB)

/*
 *	Fix Data Size (Action 2)
 *
 *	Set the internal structure for working on Bytes, Words or
 *	Double Words (only used in FAR calculation).
 *
 *	FDS(s,g)	s = Size,	0:Byte
 *					1:Word
 *					2:Near
 *					3:Far
 *			g = Sign,	0:Ignore
 *					1:Unsigned
 *					2:Signed
 */
#define FDS_ACT		2
#define FDS_SIZE_LSB	0
#define FDS_SIZE_BITS	2
#define FDS_SIGN_LSB	2
#define FDS_SIGN_BITS	2

#define FDS(s,g)		(ACT(FDS_ACT)|VALUE((s),FDS_SIZE_BITS,FDS_SIZE_LSB)|VALUE((g),FDS_SIGN_BITS,FDS_SIGN_LSB))

#define FDS_SIZE(w)	EXTRACT((w),FDS_SIZE_BITS,FDS_SIZE_LSB)
#define FDS_SIGN(w)	EXTRACT((w),FDS_SIGN_BITS,FDS_SIGN_LSB)

/*
 *	Immediate Data (Action 3)
 *
 *	Encode immediate data into the instruction from the
 *	indicated argument.  Data size to be taken from the
 *	data gathered by IDS or FDS defined above.
 *
 *	IMM(a)		a = Argument Number
 */
#define IMM_ACT		3
#define IMM_ARG_LSB	0
#define IMM_ARG_BITS	3

#define IMM(a)		(ACT(IMM_ACT)|VALUE((a),IMM_ARG_BITS,IMM_ARG_LSB))

#define IMM_ARG(w)	EXTRACT((w),IMM_ARG_BITS,IMM_ARG_LSB)

/*
 *	Effective Address (Action 4)
 *
 *	Generate the effective address byte (mod reg rm) in the
 *	instruction.
 *
 *	EA(r,a)		r = argument that is the register component
 *			a = argument number where the EA can be found
 */
#define EA_ACT		4
#define EA_REG_LSB	0
#define EA_REG_BITS	3
#define EA_EADRS_LSB	3
#define EA_EADRS_BITS	3

#define EA(r,a)		(ACT(EA_ACT)|VALUE((r),EA_REG_BITS,EA_REG_LSB)|VALUE((a),EA_EADRS_BITS,EA_EADRS_LSB))

#define EA_REG(w)	EXTRACT((w),EA_REG_BITS,EA_REG_LSB)
#define EA_EADRS(w)	EXTRACT((w),EA_EADRS_BITS,EA_EADRS_LSB)

/*
 *	Effective Address Operator (Action 5)
 *
 *	Generate the effective address byte (mod opcode rm) in the
 *	instruction.
 *
 *	EAO(o,a)	o = 3 bit opcode to insert into the byte
 *			a = argument number where the EA can be found
 */
#define EAO_ACT		5
#define EAO_OPCODE_LSB	0
#define EAO_OPCODE_BITS	3
#define EAO_EADRS_LSB	3
#define EAO_EADRS_BITS	3

#define EAO(o,a)	(ACT(EAO_ACT)|VALUE((o),EAO_OPCODE_BITS,EAO_OPCODE_LSB)|VALUE((a),EAO_EADRS_BITS,EAO_EADRS_LSB))

#define EAO_OPCODE(w)	EXTRACT((w),EAO_OPCODE_BITS,EAO_OPCODE_LSB)
#define EAO_EADRS(w)	EXTRACT((w),EAO_EADRS_BITS,EAO_EADRS_LSB)

/*
 *	Save Data Size (Action 6)
 *
 *	Set data size location providing a byte index and 'bit in byte'
 *	position where a 0 (for bytes) or 1 (for words) will be placed.
 *
 *	SDS(i,b)	i = index into machine instruction (0..7)
 *			b = bit number in byte (0..7)
 */
#define SDS_ACT		6
#define SDS_INDEX_LSB	0
#define SDS_INDEX_BITS	3
#define SDS_BIT_LSB	3
#define SDS_BIT_BITS	3

#define SDS(i,b)	(ACT(SDS_ACT)|VALUE((i),SDS_INDEX_BITS,SDS_INDEX_LSB)|VALUE((b),SDS_BIT_BITS,SDS_BIT_LSB))

#define SDS_INDEX(w)	EXTRACT((w),SDS_INDEX_BITS,SDS_INDEX_LSB)
#define SDS_BIT(w)	EXTRACT((w),SDS_BIT_BITS,SDS_BIT_LSB)

/*
 *	Set DiRection (Action 7)
 *
 *	Set data direction providing a byte index and 'bit in byte'
 *	position where a 0 (EA is Destination) or 1 (EA is Source).
 *
 *	SDR(d,i,b)	d = direction bit
 *				0 = EA<-Reg (Reg->Source, EA->Destination)
 *				1 = Reg<-EA (EA->Source, Reg->Destination)
 *			i = index into machine instruction (0..7)
 *			b = bit number in byte (0..7)
 */
#define SDR_ACT		7
#define SDR_DIR_LSB	0
#define SDR_DIR_BITS	1
#define SDR_INDEX_LSB	1
#define SDR_INDEX_BITS	3
#define SDR_BIT_LSB	4
#define SDR_BIT_BITS	3

#define SDR(d,i,b)	(ACT(SDR_ACT)|VALUE((d),SDR_DIR_BITS,SDR_DIR_LSB)|VALUE((i),SDR_INDEX_BITS,SDR_INDEX_LSB)|VALUE((b),SDR_BIT_BITS,SDR_BIT_LSB))

#define SDR_DIR(w)	EXTRACT((w),SDR_DIR_BITS,SDR_DIR_LSB)
#define SDR_INDEX(w)	EXTRACT((w),SDR_INDEX_BITS,SDR_INDEX_LSB)
#define SDR_BIT(w)	EXTRACT((w),SDR_BIT_BITS,SDR_BIT_LSB)

/*
 *	Register (Action 8)
 *
 *	Take the register number from the argument a (three bit
 *	value 0..7) and place this value into instruction byte i
 *	at bit offset b.
 *
 *	REG(a,i,b)	a = Argument number (0..7)
 *			i = index into machine instruction (0..7)
 *			b = bit number in byte (0..7)
 */
#define REG_ACT		8
#define REG_ARG_LSB	0
#define REG_ARG_BITS	3
#define REG_INDEX_LSB	3
#define REG_INDEX_BITS	3
#define REG_BIT_LSB	6
#define REG_BIT_BITS	3

#define REG(a,i,b)	(ACT(REG_ACT)|VALUE((a),REG_ARG_BITS,REG_ARG_LSB)|VALUE((i),REG_INDEX_BITS,REG_INDEX_LSB)|VALUE((b),REG_BIT_BITS,REG_BIT_LSB))

#define REG_ARG(w)	EXTRACT((w),REG_ARG_BITS,REG_ARG_LSB)
#define REG_INDEX(w)	EXTRACT((w),REG_INDEX_BITS,REG_INDEX_LSB)
#define REG_BIT(w)	EXTRACT((w),REG_BIT_BITS,REG_BIT_LSB)

/*
 *	ESCape Data (Action 9)
 *
 *	Provides a mechanism to capture the co-processor opcode and
 *	position it within the output code generated.
 *
 *	ESC(a)		a = Argument number of immediate data
 */
#define ESC_ACT		9
#define ESC_ARG_LSB	0
#define ESC_ARG_BITS	3

#define ESC(a)		(ACT(ESC_ACT)|VALUE((a),ESC_ARG_BITS,ESC_ARG_LSB))

#define ESC_ARG(w)	EXTRACT((w),ESC_ARG_BITS,ESC_ARG_LSB)

/*
 *	RELative reference (Action 10)
 *
 *	Handles the conversion of an immediate label reference into
 *	a relative IP offset value.
 *
 *	REL(a,s,i,b)	a = Argument number of immediate data
 *			s = Size of relative reference allowed
 *				0 - invalid
 *				1 - Byte only
 *				2 - Word only
 *				3 - Byte or word (if range is invalid for byte)
 *			i = index of code to adjust for word disp
 *			b = bit in index byte to flip.
 */
#define REL_ACT		10
#define REL_ARG_LSB	0
#define REL_ARG_BITS	3
#define REL_RANGE_LSB	3
#define REL_RANGE_BITS	2
#define REL_INDEX_LSB	5
#define REL_INDEX_BITS	3
#define REL_BIT_LSB	8
#define REL_BIT_BITS	1

#define REL(a,s,i,b)	(ACT(REL_ACT)|VALUE((a),REL_ARG_BITS,REL_ARG_LSB)|VALUE((s),REL_RANGE_BITS,REL_RANGE_LSB)|VALUE((i),REL_INDEX_BITS,REL_INDEX_LSB)|VALUE((b),REL_BIT_BITS,REL_BIT_LSB))

#define REL_ARG(w)	EXTRACT((w),REL_ARG_BITS,REL_ARG_LSB)
#define REL_RANGE(w)	EXTRACT((w),REL_RANGE_BITS,REL_RANGE_LSB)
#define REL_INDEX(w)	EXTRACT((w),REL_INDEX_BITS,REL_INDEX_LSB)
#define REL_BIT(w)	EXTRACT((w),REL_BIT_BITS,REL_BIT_LSB)

/*
 *	Verify Data Size (Action 11)
 *
 *	Check that the argument specified has the compatible size
 *	configuration as the size data recorded in the constructed
 *	instruction data.
 *
 *	VDS(a)		a = Argument number of immediate data
 */
#define VDS_ACT		11
#define VDS_ARG_LSB	0
#define VDS_ARG_BITS	3

#define VDS(a)		(ACT(VDS_ACT)|VALUE((a),VDS_ARG_BITS,VDS_ARG_LSB))

#define VDS_ARG(w)	EXTRACT((w),VDS_ARG_BITS,VDS_ARG_LSB)

/*
 *	Look for an instruction definition given the details provided.
 */
extern opcode *find_opcode( modifier mods, component op, int args, ea_breakdown *format );



/************************************************************************
************************************************************************/

#ifdef VERIFICATION

/*
 *	If the VERIFICATION macro has been defined then we will
 *	allow direct access to the opcode data.
 */
extern opcode opcodes[];
	
#endif



#endif

/*
 *	EOF
 */
