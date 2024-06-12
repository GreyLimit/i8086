/*
 *	component
 *	=========
 *
 *	Define symbolic values for opcodes and syntactic elements.
 */

#ifndef _COMPONENT_H_
#define _COMPONENT_H_

/*
 *	Declare a complete list of the assembly language components.
 *	These values will be used in both the source code syntax
 *	processing and the subsequent semantic analysis.
 */
typedef enum {
	/*
	 *	The "no op" component ID
	 */
	nothing = 0,
	/*
	 *	Assembly Language op codes
	 */
	op_aaa,		op_aad,		op_aam,		op_aas,
	op_adc,		op_add,		op_and,
	op_bound,	op_break,	op_call,	op_lcall,
	op_cbw,		op_clc,		op_cld,		op_cli,
	op_cmc,		op_cmp,		op_cmps,	op_cwd,
	op_daa,		op_das,		op_dec,		op_div,
	op_esc,		op_enter,	op_hlt,		op_idiv,	op_imul,
	op_in,		op_inc,		op_ins,		op_int,		op_intr,
	op_into,	op_iret,	op_ja,		op_jnbe,	op_jbe,
	op_jae,		op_jna,		op_jnb,		op_jb,		op_jnae,
	op_jc,		op_jcxz,	op_je,		op_jz,
	op_jg,		op_jnle,	op_jge,		op_jnl,
	op_jl,		op_jnge,	op_jle,		op_jng,
	op_jmp,		op_ljmp,
	op_jnc,		op_jne,		op_jnz,
	op_jno,		op_jnp,		op_jpo,		op_jns,
	op_jo,		op_jp,		op_jpe,		op_js,
	op_lahf,	op_lds,		op_lea,		op_leave,
	op_les,		op_lods,	op_looppe,	op_looppz,	op_loopnz,
	op_loop,	op_loope,	op_loopz,	op_loopne,	op_loopna,
	op_mov,		op_movs,	op_movsb,	op_movsw,
	op_mul,		op_neg,		op_nop,		op_not,
	op_or,		op_out,		op_pop,		op_popf,
	op_push,	op_pushf,	op_rcl,		op_rcr,
	op_ret,		op_lret,	op_rol,		op_ror,		op_sahf,
	op_sal,		op_shl,		op_sar,		op_sbb,
	op_scas,	op_shr,		op_stc,		op_std,
	op_sti,		op_stos,	op_sub,		op_test,
	op_wait,	op_xchg,	op_xlat,	op_xor,
	/*
	 *	Assembly language explicit Prefix Ops.
	 */
	pref_lock,	pref_rep,					/* is_prefix() dependent */
	pref_repe,	pref_repz,	pref_repne,	pref_repnz,
	/*
	 *	Registers.
	 *
	 *	8 bit registers.
	 */
	reg_al,		reg_cl,		reg_dl,		reg_bl,		/* is_byte_reg() dependency */
	reg_ah,		reg_ch,		reg_dh,		reg_bh,
	/*
	 *	16 bit registers.
	 *
	 *	Divided into two ranges:
	 *
	 *	Base Registers: SP, BP, SI and DI
	 *
	 *	Index Registers: AX, CX, DX and BX
	 */
	reg_ax,		reg_cx,		reg_dx,		reg_bx,		/* is_index_reg() dependent */
	reg_sp,		reg_bp,		reg_si,		reg_di,		/* is_base_reg() dependent */
	/*
	 *	Finally Segment registers size
	 *	depend on the CPU being targeted.
	 */
	reg_cs,		reg_ds,		reg_ss,		reg_es,
	/*
	 *	Assembler directives
	 */
	asm_segment,	asm_group,
	asm_db,		asm_dw,		asm_dd,		asm_reserve,
	asm_equ,	asm_org,	asm_align,
	asm_include,	asm_export,	asm_import,	asm_end,
	/*
	 *	Syntactic elements that are used to build up
	 *	the assembly languages' more complex elements.
	 */
	tok_semicolon,	tok_colon,
	tok_comma,	tok_period,
	tok_oparen,	tok_cparen,
	tok_obracket,	tok_cbracket,
	tok_plus,	tok_minus,
	tok_mul,	tok_div,
	tok_and,	tok_or,
	tok_not,	tok_xor,
	tok_shl,	tok_shr,
	/*
	 *	Complex tokens.
	 */
	tok_immediate,	tok_label,	tok_string,
	/*
	 *	Modifier keywords used to explicitly set characteristics
	 *	of an instruction if these are ambigious at source level.
	 */
	mod_byte,	mod_word,	mod_ptr,		/* is_modifier() dependency */
	mod_near,	mod_far,
	/*
	 *	Synthetic component numbers used as part of the
	 *	syntax analysis process allowing the tokens to
	 *	be converted from a list into a tree structure.
	 */
	end_of_line		/* In some code it is easier to check */
				/* for this instead of checking for a */
				/* NULL pointer. Algorithm dependent. */
} component;

/*
 *	Component testing and filtering macros/routines
 */
extern boolean is_opcode( component comp );
extern boolean is_directive( component comp );
extern boolean is_prefix( component comp );
extern boolean is_reg_segment( component comp );
extern boolean is_modifier( component comp );


#endif

/*
 *	EOF
 */
