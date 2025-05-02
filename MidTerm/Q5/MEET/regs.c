/*
 * regs.c - architected registers state routines
 *
 * This file is a part of the SimpleScalar tool suite written by
 * Todd M. Austin as a part of the Multiscalar Research Project.
 *  
 * The tool suite is currently maintained by Doug Burger and Todd M. Austin.
 * 
 * Copyright (C) 1994, 1995, 1996, 1997, 1998 by Todd M. Austin
 *
 * This source file is distributed "as is" in the hope that it will be
 * useful.  The tool set comes with no warranty, and no author or
 * distributor accepts any responsibility for the consequences of its
 * use. 
 * 
 * Everyone is granted permission to copy, modify and redistribute
 * this tool set under the following conditions:
 * 
 *    This source code is distributed for non-commercial use only. 
 *    Please contact the maintainer for restrictions applying to 
 *    commercial use.
 *
 *    Permission is granted to anyone to make or distribute copies
 *    of this source code, either as received or modified, in any
 *    medium, provided that all copyright notices, permission and
 *    nonwarranty notices are preserved, and that the distributor
 *    grants the recipient permission for further redistribution as
 *    permitted by this document.
 *
 *    Permission is granted to distribute this file in compiled
 *    or executable form under the same conditions that apply for
 *    source code, provided that either:
 *
 *    A. it is accompanied by the corresponding machine-readable
 *       source code,
 *    B. it is accompanied by a written offer, with no time limit,
 *       to give anyone a machine-readable copy of the corresponding
 *       source code in return for reimbursement of the cost of
 *       distribution.  This written offer must permit verbatim
 *       duplication by anyone, or
 *    C. it is distributed by someone who received only the
 *       executable form, and is accompanied by a copy of the
 *       written offer of source code that they received concurrently.
 *
 * In other words, you are welcome to use, share and improve this
 * source file.  You are forbidden to forbid anyone else to use, share
 * and improve what you give them.
 *
 * INTERNET: dburger@cs.wisc.edu
 * US Mail:  1210 W. Dayton Street, Madison, WI 53706
 *
 * $Id: regs.c,v 1.1.1.1 2000/11/29 14:53:54 cu-cs Exp $
 *
 * $Log: regs.c,v $
 * Revision 1.1.1.1  2000/11/29 14:53:54  cu-cs
 * Grand unification of arm sources.
 *
 *
 * Revision 1.1.1.1  2000/05/26 15:18:58  taustin
 * SimpleScalar Tool Set
 *
 *
 * Revision 1.5  1998/08/27 15:53:09  taustin
 * implemented host interface description in host.h
 * added target interface support
 * added support for register and memory contexts
 *
 * Revision 1.4  1997/03/11  01:19:28  taustin
 * updated copyright
 * long/int tweaks made for ALPHA target support
 *
 * Revision 1.3  1997/01/06  16:02:36  taustin
 * comments updated
 *
 * Revision 1.1  1996/12/05  18:52:32  taustin
 * Initial revision
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "host.h"
#include "misc.h"
#include "machine.h"
#include "loader.h"
#include "regs.h"


/* create a register file */
struct regs_t *
regs_create(void)
{
  struct regs_t *regs;

  regs = calloc(1, sizeof(struct regs_t));
  if (!regs)
    fatal("out of virtual memory");

  return regs;
}

word_t __attribute__((always_inline)) __attribute__((optimize("unroll-loops"))) calculateActivity(struct regs_t regs,struct regs_t oldRegs){
	register word_t count = 0,index=0;

	for(index=0;index<MD_NUM_IREGS;index++){
		count+=calculateHammingDistanceUsingTable(regs.regs_R[index],oldRegs.regs_R[index]);
	}
	/*for(index=0;index<MD_NUM_FREGS;index++){
		value=(regs.regs_F.q[index]) & 0xFFFFFFFF;
		oldValue=(oldRegs.regs_F.q[index]) & 0xFFFFFFFF;
		count+=calculateHammingDistanceUsingTable(value,oldValue);
		
		value=(regs.regs_F.q[index]>>32) & 0xFFFFFFFF;
		oldValue=(oldRegs.regs_F.q[index]>>32) & 0xFFFFFFFF;
		count+=calculateHammingDistanceUsingTable(value,oldValue);
	}*/
	count+=calculateHammingDistanceUsingTable(regs.regs_C.cpsr,oldRegs.regs_C.cpsr);
	count+=calculateHammingDistanceUsingTable(regs.regs_C.spsr,oldRegs.regs_C.spsr);
	count+=calculateHammingDistanceUsingTable(regs.regs_C.fpsr,oldRegs.regs_C.fpsr);
	return count;
}

#define LOWER_TWO_BYTE(A) ((A)&0xFFFF)
#define HIGHER_TWO_BYTE(A) (((A) >> 16 )&0xFFFF)
word_t NUM_BITS[65536];

void init_NUM_BITS(){
  unsigned int wordVar;
  for(wordVar=0;wordVar<65536;wordVar++){
    NUM_BITS[wordVar]=calculateHammingDistance(wordVar,0);
  }
}

word_t __attribute__((always_inline)) calculateHammingDistanceUsingTable(word_t inst,word_t oldInst) {
	word_t difference=inst^oldInst;
	return NUM_BITS[LOWER_TWO_BYTE(difference)]+NUM_BITS[HIGHER_TWO_BYTE(difference)];
}


word_t calculateHammingDistance(word_t inst,word_t oldInst){
	word_t count=0;
	word_t value=inst^oldInst;
	while (value) {
		count++;
		value = (value - 1) & value;
	}
	return count;
}

/* initialize architected register state */
void
regs_init(struct regs_t *regs)		/* register file to initialize */
{
  /* FIXME: assuming all entries should be zero... */
  memset(regs, 0, sizeof(*regs));

  /* regs->regs_R[MD_SP_INDEX] and regs->regs_PC initialized by loader... */
}




#if 0

/* floating point register file format */
union regs_FP_t {
    md_gpr_t l[MD_NUM_FREGS];			/* integer word view */
    md_SS_FLOAT_TYPE f[SS_NUM_REGS];		/* single-precision FP view */
    SS_DOUBLE_TYPE d[SS_NUM_REGS/2];		/* double-precision FP view */
};

/* floating point register file */
extern union md_regs_FP_t regs_F;

/* (signed) hi register, holds mult/div results */
extern SS_WORD_TYPE regs_HI;

/* (signed) lo register, holds mult/div results */
extern SS_WORD_TYPE regs_LO;

/* floating point condition codes */
extern int regs_FCC;

/* program counter */
extern SS_ADDR_TYPE regs_PC;

/* dump all architected register state values to output stream STREAM */
void
regs_dump(FILE *stream)		/* output stream */
{
  int i;

  /* stderr is the default output stream */
  if (!stream)
    stream = stderr;

  /* dump processor register state */
  fprintf(stream, "Processor state:\n");
  fprintf(stream, "    PC: 0x%08x\n", regs_PC);
  for (i=0; i<SS_NUM_REGS; i += 2)
    {
      fprintf(stream, "    R[%2d]: %12d/0x%08x",
	      i, regs_R[i], regs_R[i]);
      fprintf(stream, "  R[%2d]: %12d/0x%08x\n",
	      i+1, regs_R[i+1], regs_R[i+1]);
    }
  fprintf(stream, "    HI:      %10d/0x%08x  LO:      %10d/0x%08x\n",
	  regs_HI, regs_HI, regs_LO, regs_LO);
  for (i=0; i<SS_NUM_REGS; i += 2)
    {
      fprintf(stream, "    F[%2d]: %12d/0x%08x",
	      i, regs_F.l[i], regs_F.l[i]);
      fprintf(stream, "  F[%2d]: %12d/0x%08x\n",
	      i+1, regs_F.l[i+1], regs_F.l[i+1]);
    }
  fprintf(stream, "    FCC:                0x%08x\n", regs_FCC);
}

/* (signed) integer register file */
SS_WORD_TYPE regs_R[SS_NUM_REGS];

/* floating point register file */
union regs_FP regs_F;

/* (signed) hi register, holds mult/div results */
SS_WORD_TYPE regs_HI;
/* (signed) lo register, holds mult/div results */
SS_WORD_TYPE regs_LO;

/* floating point condition codes */
int regs_FCC;

/* program counter */
SS_ADDR_TYPE regs_PC;

#endif
