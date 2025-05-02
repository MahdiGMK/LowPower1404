/*
 * Modified on December 2011 by Mostafa Bazzaz (bazzaz@ce.sharif.ir).
 *
 * sim-profile.c - sample functional simulator implementation w/ profiling
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
 * $Id: sim-profile.c,v 1.1.1.1 2000/11/29 14:53:54 cu-cs Exp $
 *
 * $Log: sim-profile.c,v $
 * Revision 1.1.1.1  2000/11/29 14:53:54  cu-cs
 * Grand unification of arm sources.
 *
 *
 * Revision 1.1.1.1.2.1  2000/11/14 05:32:22  taustin
 * Sim-profile now works for ARM (mostly).
 *
 * Revision 1.1.1.1  2000/05/26 15:18:59  taustin
 * SimpleScalar Tool Set
 *
 *
 * Revision 1.4  1999/12/31 18:53:04  taustin
 * quad_t naming conflicts removed
 *
 * Revision 1.3  1999/12/13 18:46:51  taustin
 * cross endian execution support added
 *
 * Revision 1.2  1998/08/27 16:37:03  taustin
 * implemented host interface description in host.h
 * added target interface support
 * added support for register and memory contexts
 * instruction predecoding moved to loader module
 * Alpha target support added
 * added support for qword's
 * added fault support
 * added option ("-max:inst") to limit number of instructions analyzed
 * added target-dependent myprintf() support
 *
 * Revision 1.1  1997/03/11  01:33:36  taustin
 * Initial revision
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "host.h"
#include "misc.h"
#include "machine.h"
#include "regs.h"
#include "memory.h"
#include "loader.h"
#include "syscall.h"
#include "dlite.h"
#include "symbol.h"
#include "options.h"
#include "stats.h"
#include "sim.h"

/*
 * This file implements a functional simulator with profiling support.  Run
 * with the `-h' flag to see profiling options available.
 */

/* simulated registers */
static struct regs_t regs;
/* mostafa: used for computing hamming distance of register bank between two instruction execution */
static struct regs_t oldRegs;

static VarOrFuncMemInfo *blockAddresses=NULL;

/* simulated memory */
static struct mem_t *mem = NULL;

/* track number of refs */
static counter_t sim_num_refs = 0;
static counter_t sim_num_loads = 0;
static counter_t sim_num_flash_loads = 0;	// mostafa: different counter for flash and SRAM
static counter_t sim_num_sram_loads = 0;
static counter_t sim_num_stores = 0;

/* mostafa: activity */
static counter_t regbank_activity = 0;	// number of bit-flips occured in register bank
static counter_t inst_bus_activity=0;	// number of bit-flips occured in instruction bus
static counter_t inst_bus_ones=0;	// number of 1's of the instruction bus during program execution
//static counter_t inst_count_after_meas=0;	// number of executed instruction after start of the measurement
static float sim_total_energy=0.0f;	// total energy consumption of the application until now
/* mostafa: total number of shift operation */
static counter_t num_shift=0;

/* mostafa: if >0 start the program at this address. otherwise start at program starting point*/
static unsigned int initial_pc;

/* mostafa: saves the return address of the procedure that enables the measurement process */
static unsigned int targetPC=0;

/* mostafa: indicates whether we are in the fast_forward phase or not */
static unsigned int fast_forward=0;

/* mostafa: if >0 end the simulation at this address. otherwise continue the execution until program terminates normally */
static unsigned int finish_pc;

/* maximum number of inst's to execute */
static unsigned int max_insts;

/* mostafa: the name of the tracefile */
static char *tracefilename;

/* mostafa: the name of the tracefile */
static char *blockfilename;

// energy consumption of each instruction type. sorted according to order of instructions
// in machine.def file
static float opEnergy[]= {
#include "energyValues.c"
};


/* profiling options */
static int prof_all /* = FALSE */;
static int prof_ic /* = FALSE */;
static int prof_inst /* = FALSE */;
static int prof_unit /* = FALSE */;
static int prof_bc /* = FALSE */;
static int prof_am /* = FALSE */;
static int prof_seg /* = FALSE */;
static int prof_regbank /* = FALSE */;
static int prof_reg /* = FALSE */;
static int prof_tsyms /* = FALSE */;
static int prof_dsyms /* = FALSE */;
static int load_locals /* = FALSE */;
static int prof_taddr /* = FALSE */;
static int traceInst /* = FALSE */;
static int traceData /* = FALSE */;
static int traceBlock;

/* text-based stat profiles */
#define MAX_PCSTAT_VARS 8
static int pcstat_nelt = 0;
static char *pcstat_vars[MAX_PCSTAT_VARS];

/* register simulator-specific options */
void
sim_reg_options(struct opt_odb_t *odb)
{
    opt_reg_header(odb,
                   "sim-profile: This simulator implements a functional simulator with\n"
                   "profiling support.  Run with the `-h' flag to see profiling options\n"
                   "available.\n"
                  );

    /* instruction limit */
    opt_reg_uint(odb, "-max:inst", "maximum number of inst's to execute",
                 &max_insts, /* default */0,
                 /* print */TRUE, /* format */NULL);

//    opt_reg_uint(odb,"-initial:meas","address of the procedure that starts up the measurement procedure",&initial_meas_add, /* default */0,
//                 /* print */TRUE, /* format */NULL);

    /* initial address */
    opt_reg_uint(odb, "-initial:pc", "analysis start address",
                 &initial_pc, /* default */0,
                 /* print */TRUE, /* format */NULL);

    /* end address */
    opt_reg_uint(odb, "-finish:pc", "program execution end address",
                 &finish_pc, /* default */0,
                 /* print */TRUE, /* format */NULL);

  opt_reg_flag(odb, "-regprof", "enable register profiling",
	       &prof_reg, /* default */FALSE, /* print */TRUE, NULL);
    opt_reg_flag(odb, "-all", "enable all profile options",
                 &prof_all, /* default */FALSE, /* print */TRUE, NULL);

    opt_reg_flag(odb, "-iclass", "enable instruction class profiling",
                 &prof_ic, /* default */FALSE, /* print */TRUE, NULL);

    opt_reg_flag(odb, "-iprof", "enable instruction profiling",
                 &prof_inst, /* default */FALSE, /* print */TRUE, NULL);

    opt_reg_flag(odb, "-uprof", "enable unit profiling",
                 &prof_unit, /* default */FALSE, /* print */TRUE, NULL);

    opt_reg_flag(odb, "-brprof", "enable branch instruction profiling",
                 &prof_bc, /* default */FALSE, /* print */TRUE, NULL);

    opt_reg_flag(odb, "-amprof", "enable address mode profiling",
                 &prof_am, /* default */FALSE, /* print */TRUE, NULL);

    opt_reg_flag(odb, "-segprof", "enable load/store address segment profiling",
                 &prof_seg, /* default */FALSE, /* print */TRUE, NULL);

    opt_reg_flag(odb, "-tsymprof", "enable text symbol profiling",
                 &prof_tsyms, /* default */FALSE, /* print */TRUE, NULL);

    opt_reg_flag(odb, "-taddrprof", "enable text address profiling",
                 &prof_taddr, /* default */FALSE, /* print */TRUE, NULL);

    opt_reg_flag(odb, "-dsymprof", "enable data symbol profiling",
                 &prof_dsyms, /* default */FALSE, /* print */TRUE, NULL);

    opt_reg_flag(odb, "-trace:inst", "enable instruction fetch profiling",
                 &traceInst, /* default */FALSE, /* print */TRUE, NULL);

    opt_reg_flag(odb, "-trace:data", "enable data fetch profiling",
                 &traceData, /* default */FALSE, /* print */TRUE, NULL);

    opt_reg_flag(odb, "-internal",
                 "include compiler-internal symbols during symbol profiling",
                 &load_locals, /* default */FALSE, /* print */TRUE, NULL);

    opt_reg_string_list(odb, "-pcstat",
                        "profile stat(s) against text addr's (mult uses ok)",
                        pcstat_vars, MAX_PCSTAT_VARS, &pcstat_nelt, NULL,
                        /* !print */FALSE, /* format */NULL, /* accrue */TRUE);

    opt_reg_string(odb,"-trace:filename","Fullpath of the tracefile",&tracefilename,NULL, /* print */TRUE, NULL);
    opt_reg_flag(odb,"-trace:block","enable basicblock profiling",&traceBlock,FALSE, /* print */TRUE, NULL);
    opt_reg_string(odb,"-trace:blockListFilename","Fullpath of the file containing the address and size of each block",&blockfilename,NULL, /* print */TRUE, NULL);
}

/* check simulator-specific option values */
void
sim_check_options(struct opt_odb_t *odb, int argc, char **argv)
{
    if (prof_all)
    {
        /* enable all options */
        prof_ic = TRUE;
        prof_inst = TRUE;
        prof_unit = TRUE;
        prof_bc = TRUE;
        prof_am = TRUE;
        prof_seg = TRUE;
      	prof_reg = TRUE;
        prof_tsyms = TRUE;
        prof_dsyms = TRUE;
        prof_taddr = TRUE;
    }
}

/* instruction classes */
enum inst_class_t {
    ic_load,		/* load inst */
    ic_store,		/* store inst */
    ic_uncond,		/* uncond branch */
    ic_cond,		/* cond branch */
    ic_icomp,		/* all other integer computation */
    ic_fcomp,		/* all floating point computation */
    ic_trap,		/* system call */
    ic_NUM
};

/* instruction class strings */
static char *inst_class_str[ic_NUM] = {
    "load",		/* load inst */
    "store",		/* store inst */
    "uncond branch",	/* uncond branch */
    "cond branch",	/* cond branch */
    "int computation",	/* all other integer computation */
    "fp computation",	/* all floating point computation */
    "trap"		/* system call */
};

/* instruction class profile */
static struct stat_stat_t *ic_prof = NULL;

/* instruction description strings */
static char *inst_str[OP_MAX];

/* unit description strings */
static char *unit_str[NUM_FU_CLASSES];

/* instruction profile */
static struct stat_stat_t *inst_prof = NULL;

/* unit profile */
static struct stat_stat_t *unit_prof = NULL;

/* branch class profile */
enum branch_class_t {
    bc_uncond_dir,	/* direct unconditional branch */
    bc_cond_dir,		/* direct conditional branch */
    bc_call_dir,		/* direct functional call */
    bc_uncond_indir,	/* indirect unconditional branch */
    bc_cond_indir,	/* indirect conditional branch */
    bc_call_indir,	/* indirect function call */
    bc_NUM
};

/* branch class description strings */
static char *branch_class_str[bc_NUM] = {
    "uncond direct",	/* direct unconditional branch */
    "cond direct",	/* direct conditional branch */
    "call direct",	/* direct functional call */
    "uncond indirect",	/* indirect unconditional branch */
    "cond indirect",	/* indirect conditional branch */
    "call indirect"	/* indirect function call */
};

/* branch profile */
static struct stat_stat_t *bc_prof = NULL;

/* addressing mode profile */
static struct stat_stat_t *am_prof = NULL;

/* address segments */
enum addr_seg_t {
    seg_data,		/* data segment */
    seg_heap,		/* heap segment */
    seg_stack,		/* stack segment */
    seg_text,		/* text segment */
    seg_NUM
};

/* address segment strings */
static char *addr_seg_str[seg_NUM] = {
    "data segment",	/* data segment */
    "heap segment",	/* heap segment */
    "stack segment",	/* stack segment */
    "text segment",	/* text segment */
};

/* address segment profile */
static struct stat_stat_t *seg_prof = NULL;

/* bind ADDR to the segment it references */
static enum addr_seg_t			/* segment referenced by ADDR */
bind_to_seg(md_addr_t addr)		/* address to bind to a segment */
{
    if (ld_data_base <= addr && addr < (ld_data_base + ld_data_size))
        return seg_data;
    else if ((ld_data_base + ld_data_size) <= addr && addr < ld_brk_point)
        return seg_heap;
    /* FIXME: ouch! */
    else if ((ld_stack_base - (16*1024*1024)) <= addr && addr < ld_stack_base)
        return seg_stack;
    else if (ld_text_base <= addr && addr < (ld_text_base + ld_text_size))
        return seg_text;
    else
        panic("cannot bind address to segment");
}

/* text symbol profile */
static struct stat_stat_t *tsym_prof = NULL;
static char **tsym_names = NULL;

/* data symbol profile */
static struct stat_stat_t *dsym_prof = NULL;
static char **dsym_names = NULL;
static char **reg_names = NULL;

/* text address profile */
static struct stat_stat_t *taddr_prof = NULL;

/* register profile */
static struct stat_stat_t *register_prof = NULL;

/* text-based stat profiles */
static struct stat_stat_t *pcstat_stats[MAX_PCSTAT_VARS];
static counter_t pcstat_lastvals[MAX_PCSTAT_VARS];
static struct stat_stat_t *pcstat_sdists[MAX_PCSTAT_VARS];

/* wedge all stat values into a counter_t */
#define STATVAL(STAT)							\
  ((STAT)->sc == sc_int							\
   ? (counter_t)*((STAT)->variant.for_int.var)				\
   : ((STAT)->sc == sc_uint						\
      ? (counter_t)*((STAT)->variant.for_uint.var)			\
      : ((STAT)->sc == sc_counter					\
	 ? *((STAT)->variant.for_counter.var)				\
		:((STAT)->sc == sc_float		\
?(float)*((STAT)->variant.for_float.var)\
	: (panic("bad stat class"), 0)))))

int __attribute__((always_inline)) __attribute__((optimize("unroll-loops"))) findCurrentBlockIndex(md_addr_t regs_PC,VarOrFuncMemInfo* list,int numBlocks) {
    register int i;
    if(regs_PC<list[0].address)
        return -1;
    for(i=numBlocks-1; i>=0; i--) {	// FIXME use binary search
        if(regs_PC>=list[i].address && regs_PC<list[i].address+list[i].size)
            return i;
    }
    return -1;
}

/* register simulator-specific statistics */
void
sim_reg_stats(struct stat_sdb_t *sdb)
{
    int i;
    stat_reg_counter(sdb, "sim_num_insn",
                     "total number of instructions executed after analysis started",
                     &sim_num_insn, sim_num_insn, NULL);
/*    stat_reg_counter(sdb, "inst_count_after_meas",
                     "total number of instructions executed after measurement",&inst_count_after_meas,0,NULL);*/
    stat_reg_counter(sdb, "sim_num_refs",
                     "total number of loads and stores executed",
                     &sim_num_refs, 0, NULL);
    stat_reg_counter(sdb, "sim_num_loads",
                     "total number of read memory accesses",
                     &sim_num_loads, 0, NULL);
    stat_reg_counter(sdb, "sim_num_flash_loads",
                     "total number of Flash read memory accesses",
                     &sim_num_flash_loads, 0, NULL);
    stat_reg_counter(sdb, "sim_num_sram_loads",
                     "total number of SRAM read memory accesses",
                     &sim_num_sram_loads, 0, NULL);
    stat_reg_counter(sdb, "sim_num_stores",
                     "total number of write memory accesses",
                     &sim_num_stores, 0, NULL);
    stat_reg_float(sdb,"sim_total_energy","total energy consumption (nJ)",&sim_total_energy,0.0f,NULL);
    stat_reg_int(sdb, "sim_elapsed_time",
                 "total simulation time in seconds",
                 &sim_elapsed_time, 0, NULL);
    stat_reg_formula(sdb, "sim_inst_rate",
                     "simulation speed (in insts/sec)",
                     "sim_num_insn / sim_elapsed_time", NULL);
    stat_reg_counter(sdb,"instruction_bus_activity",
                     "total number of bit flip in instruction bus",
                     &inst_bus_activity,0,NULL);
    stat_reg_counter(sdb,"instruction_bus_weight",
                     "total number of 1 count in instruction bus",
                     &inst_bus_ones,0,NULL);
    stat_reg_counter(sdb,"regbank_activity",
                     "total number of bit flip in register bank",
                     &regbank_activity,0,NULL);
    if (prof_ic)
    {
        /* instruction class profile */
        ic_prof = stat_reg_dist(sdb, "sim_inst_class_prof",
                                "instruction class profile",
                                /* initial value */0,
                                /* array size */ic_NUM,
                                /* bucket size */1,
                                /* print format */(PF_COUNT|PF_PDF),
                                /* format */NULL,
                                /* index map */inst_class_str,
                                /* print fn */NULL);
    }

    if (prof_inst)
    {
        int i;
        char buf[512];

        /* conjure up appropriate instruction description strings */
        for (i=0; i < /* skip NA */OP_MAX-1; i++)
        {
            sprintf(buf, "%-8s %-6s", md_op2name[i+1], md_op2format[i+1]);
            //sprintf(buf, "instcount,%s,", md_op2name[i+1]);
            //sprintf(buf, "instcount,%s_%d,", md_op2name[i+1],i+1);
            inst_str[i] = mystrdup(buf);
        }

        /* instruction profile */
        inst_prof = stat_reg_dist(sdb, "sim_inst_prof",
                                  "instruction profile",
                                  /* initial value */0,
                                  /* array size */ /* skip NA */OP_MAX-1,
                                  /* bucket size */1,
                                  /* print format */(PF_COUNT|PF_PDF),
                                  /* format */NULL,
                                  /* index map */inst_str,
                                  /* print fn */NULL);
        stat_reg_counter(sdb,"shift_activity",
                         "total number of shift operations",
                         &num_shift,0,NULL);
//		stat_reg_counter(sdb,"shift_bits_activity",
//							  "total number of shift bits",
//			&num_shift_bits,0,NULL);
    }

    if (prof_unit)
    {
        int i;
        char buf[512];

        /* conjure up appropriate instruction description strings */
        for (i=0; i < /* skip NA */NUM_FU_CLASSES-1; i++)
        {
            //sprintf(buf, "%-8s %-6s", md_op2name[i+1], md_op2format[i+1]);
            sprintf(buf, "unitcount,%s@%d,", md_fu2name[i+1], i);
            unit_str[i] = mystrdup(buf);
        }

        /* instruction profile */
        unit_prof = stat_reg_dist(sdb, "sim_unit_prof",
                                  "unit profile",
                                  /* initial value */0,
                                  /* array size */ /* skip NA */NUM_FU_CLASSES-1,
                                  /* bucket size */1,
                                  /* print format */(PF_COUNT|PF_PDF),
                                  /* format */NULL,
                                  /* index map */unit_str,
                                  /* print fn */NULL);
    }
    if (prof_bc)
    {
        /* instruction branch profile */
        bc_prof = stat_reg_dist(sdb, "sim_branch_prof",
                                "branch instruction profile",
                                /* initial value */0,
                                /* array size */bc_NUM,
                                /* bucket size */1,
                                /* print format */(PF_COUNT|PF_PDF),
                                /* format */NULL,
                                /* index map */branch_class_str,
                                /* print fn */NULL);
    }

    if (prof_am)
    {
        /* instruction branch profile */
        am_prof = stat_reg_dist(sdb, "sim_addr_mode_prof",
                                "addressing mode profile",
                                /* initial value */0,
                                /* array size */md_amode_NUM,
                                /* bucket size */1,
                                /* print format */(PF_COUNT|PF_PDF),
                                /* format */NULL,
                                /* index map */md_amode_str,
                                /* print fn */NULL);
    }

    if (prof_seg)
    {
        /* instruction branch profile */
        seg_prof = stat_reg_dist(sdb, "sim_addr_seg_prof",
                                 "load/store address segment profile",
                                 /* initial value */0,
                                 /* array size */seg_NUM,
                                 /* bucket size */1,
                                 /* print format */(PF_COUNT|PF_PDF),
                                 /* format */NULL,
                                 /* index map */addr_seg_str,
                                 /* print fn */NULL);
    }

    if (prof_tsyms && sym_ntextsyms != 0)
    {
        int i;

        /* load program symbols */
        sym_loadsyms(ld_prog_fname, load_locals);

        /* conjure up appropriate instruction description strings */
        tsym_names = (char **)calloc(sym_ntextsyms, sizeof(char *));

        for (i=0; i < sym_ntextsyms; i++)
            tsym_names[i] = sym_textsyms[i]->name;

        /* text symbol profile */
        tsym_prof = stat_reg_dist(sdb, "sim_text_sym_prof",
                                  "text symbol profile",
                                  /* initial value */0,
                                  /* array size */sym_ntextsyms,
                                  /* bucket size */1,
                                  /* print format */(PF_COUNT|PF_PDF),
                                  /* format */NULL,
                                  /* index map */tsym_names,
                                  /* print fn */NULL);
    }

    if (prof_dsyms && sym_ndatasyms != 0)
    {
        int i;

        /* load program symbols */
        sym_loadsyms(ld_prog_fname, load_locals);

        /* conjure up appropriate instruction description strings */
        dsym_names = (char **)calloc(sym_ndatasyms, sizeof(char *));

        for (i=0; i < sym_ndatasyms; i++)
            dsym_names[i] = sym_datasyms[i]->name;

        /* data symbol profile */
        dsym_prof = stat_reg_dist(sdb, "sim_data_sym_prof",
                                  "data symbol profile",
                                  /* initial value */0,
                                  /* array size */sym_ndatasyms,
                                  /* bucket size */1,
                                  /* print format */(PF_COUNT|PF_PDF),
                                  /* format */NULL,
                                  /* index map */dsym_names,
                                  /* print fn */NULL);
    }

    if (prof_taddr)
    {
        /* text address profile (sparse profile), NOTE: a dense print format
           is used, its more difficult to read, but the profiles are *much*
        smaller, I've assumed that the profiles are read by programs, at
        least for your sake I hope this is the case!! */
        taddr_prof = stat_reg_sdist(sdb, "sim_text_addr_prof",
                                    "text address profile",
                                    /* initial value */0,
                                    /* print format */(PF_COUNT|PF_PDF),
                                    /* format */"0x%p %u %.2f",
                                    /* print fn */NULL);
    }

    if(prof_reg){
      int i,nRegs;

      nRegs=sizeof(regs.regs_R)/sizeof(regs.regs_R[0]);
      /* conjure up appropriate register description strings */
      reg_names = (char **)calloc(nRegs, sizeof(char *));

      for (i=0; i < nRegs; i++){
	reg_names[i] = (char *)calloc(5, sizeof(char *));
	sprintf(reg_names[i],"R%i",i);
      }
      
      /* data symbol profile */
      register_prof = stat_reg_dist(sdb, "sim_reg_prof",
				"register profile",
				/* initial value */0,
				/* array size */nRegs,
				/* bucket size */1,
				/* print format */(PF_COUNT|PF_PDF),
				/* format */NULL,
				/* index map */reg_names,
				/* print fn */NULL);
    }
    
  for (i=0; i<pcstat_nelt; i++)
    {
        char buf[512], buf1[512];
        struct stat_stat_t *stat;

        /* track the named statistical variable by text address */

        /* find it... */
        stat = stat_find_stat(sdb, pcstat_vars[i]);
        if (!stat)
            fatal("cannot locate any statistic named `%s'", pcstat_vars[i]);

        /* stat must be an integral type */
        //if (stat->sc != sc_int && stat->sc != sc_uint && stat->sc != sc_counter)
        //fatal("`-pcstat' statistical variable `%s' is not an integral type",
        //      stat->name);

        /* register this stat */
        pcstat_stats[i] = stat;
        pcstat_lastvals[i] = STATVAL(stat);

        /* declare the sparce text distribution */
        sprintf(buf, "%s_by_pc", stat->name);
        sprintf(buf1, "%s (by text address)", stat->desc);
        pcstat_sdists[i] = stat_reg_sdist(sdb, buf, buf1,
                                          /* initial value */0,
                                          /* print format */(PF_COUNT|PF_PDF),
                                          /* format */"0x%p %u %.2f",
                                          /* print fn */NULL);
    }
    ld_reg_stats(sdb);
    mem_reg_stats(mem, sdb);
}

/* initialize the simulator */
void
sim_init(void)
{
    sim_num_refs = 0;
    sim_num_loads=0;
    sim_num_flash_loads=0;
    sim_num_sram_loads=0;
    sim_num_stores=0;
    /* allocate and initialize register file */
    regs_init(&regs);

    /* allocate and initialize memory space */
    mem = mem_create("mem");
    mem_init(mem);
}

/* local machine state accessor */
static char *					/* err str, NULL for no err */
profile_mstate_obj(FILE *stream,		/* output stream */
                   char *cmd,			/* optional command string */
                   struct regs_t *regs,		/* registers to access */
                   struct mem_t *mem)		/* memory to access */
{
    /* just dump intermediate stats */
    sim_print_stats(stream);

    /* no error */
    return NULL;
}

/* load program into simulated state */
void
sim_load_prog(char *fname,		/* program to load */
              int argc, char **argv,	/* program arguments */
              char **envp)		/* program environment */
{
    /* load program text and data, set up environment, memory, and regs */
    ld_load_prog(fname, argc, argv, envp, &regs, mem, TRUE);

    /* initialize the DLite debugger */
    dlite_init(md_reg_obj, dlite_mem_obj, profile_mstate_obj);
}


/* print simulator-specific configuration information */
void
sim_aux_config(FILE *stream)		/* output stream */
{
    /* nothing currently */
}

/* dump simulator-specific auxiliary simulator statistics */
void
sim_aux_stats(FILE *stream)		/* output stream */
{
}

/* un-initialize simulator-specific state */
void
sim_uninit(void)
{
    /* nada */
}
void
stat_add_sample_if_necessary(int prof_reg, struct stat_stat_t *stat,/* stat variable */
		md_addr_t N){
  if(!fast_forward && prof_reg)
    stat_add_sample(stat, N);
}

/*
 * configure the execution engine
 */

/*
 * precise architected register accessors
 */

/* next program counter */
#define SET_NPC(EXPR)		(regs.regs_NPC = (EXPR))

/* current program counter */
#define CPC			(regs.regs_PC)

/* general purpose registers */
#define GPR(N)			(stat_add_sample_if_necessary(prof_reg,register_prof, N),regs.regs_R[N])
#define SET_GPR(N,EXPR)                                                 \
  ((void)(((N) == 15) ? setPC++ : 0), stat_add_sample_if_necessary(prof_reg,register_prof, N), regs.regs_R[N] = (EXPR))

#if defined(TARGET_PISA)

/* floating point registers, L->word, F->single-prec, D->double-prec */
#define FPR_L(N)		(regs.regs_F.l[(N)])
#define SET_FPR_L(N,EXPR)	(regs.regs_F.l[(N)] = (EXPR))
#define FPR_F(N)		(regs.regs_F.f[(N)])
#define SET_FPR_F(N,EXPR)	(regs.regs_F.f[(N)] = (EXPR))
#define FPR_D(N)		(regs.regs_F.d[(N) >> 1])
#define SET_FPR_D(N,EXPR)	(regs.regs_F.d[(N) >> 1] = (EXPR))

/* miscellaneous register accessors */
#define SET_HI(EXPR)		(regs.regs_C.hi = (EXPR))
#define HI			(regs.regs_C.hi)
#define SET_LO(EXPR)		(regs.regs_C.lo = (EXPR))
#define LO			(regs.regs_C.lo)
#define FCC			(regs.regs_C.fcc)
#define SET_FCC(EXPR)		(regs.regs_C.fcc = (EXPR))

#elif defined(TARGET_ALPHA)

/* floating point registers, L->word, F->single-prec, D->double-prec */
#define FPR_Q(N)		(regs.regs_F.q[N])
#define SET_FPR_Q(N,EXPR)	(regs.regs_F.q[N] = (EXPR))
#define FPR(N)			(regs.regs_F.d[N])
#define SET_FPR(N,EXPR)		(regs.regs_F.d[N] = (EXPR))

/* miscellaneous register accessors */
#define FPCR			(regs.regs_C.fpcr)
#define SET_FPCR(EXPR)		(regs.regs_C.fpcr = (EXPR))
#define UNIQ			(regs.regs_C.uniq)
#define SET_UNIQ(EXPR)		(regs.regs_C.uniq = (EXPR))

#elif defined(TARGET_ARM)

/* processor status register */
#define PSR                     (regs.regs_C.cpsr)
#define SET_PSR(EXPR)           (regs.regs_C.cpsr = (EXPR))

#define PSR_N                   _PSR_N(regs.regs_C.cpsr)
#define SET_PSR_N(EXPR)         _SET_PSR_N(regs.regs_C.cpsr, (EXPR))
#define PSR_C                   _PSR_C(regs.regs_C.cpsr)
#define SET_PSR_C(EXPR)         _SET_PSR_C(regs.regs_C.cpsr, (EXPR))
#define PSR_Z                   _PSR_Z(regs.regs_C.cpsr)
#define SET_PSR_Z(EXPR)         _SET_PSR_Z(regs.regs_C.cpsr, (EXPR))
#define PSR_V                   _PSR_V(regs.regs_C.cpsr)
#define SET_PSR_V(EXPR)         _SET_PSR_V(regs.regs_C.cpsr, (EXPR))

/* floating point conversions */
union x {
    float f;
    word_t w;
};
#define DTOW(D)         ({ union x fw; fw.f = (float)(D); fw.w; })
#define WTOD(W)         ({ union x fw; fw.w = (W); (double)fw.f; })
#define QSWP(Q)                                                         \
  ((((Q) << 32) & ULL(0xffffffff00000000)) | (((Q) >> 32) & ULL(0xffffffff)))

/* floating point registers, L->word, F->single-prec, D->double-prec */
#define FPR_Q(N)                (QSWP(regs.regs_F.q[N]))
#define SET_FPR_Q(N,EXPR)       (regs.regs_F.q[N] = QSWP((EXPR)))
#define FPR_W(N)                (DTOW(regs.regs_F.d[N]))
#define SET_FPR_W(N,EXPR)       (regs.regs_F.d[N] = (WTOD(EXPR)))
#define FPR(N)                  (regs.regs_F.d[(N)])
#define SET_FPR(N,EXPR)         (regs.regs_F.d[(N)] = (EXPR))

/* miscellaneous register accessors */
#define FPSR                    (regs.regs_C.fpsr)
#define SET_FPSR(EXPR)          (regs.regs_C.fpsr = (EXPR))

#else
#error No ISA target defined...
#endif

/* precise architected memory state accessor macros */
#define READ_BYTE(SRC, FAULT)						\
  ((FAULT) = md_fault_none, MEM_READ_BYTE(mem, addr = (SRC)))
#define READ_HALF(SRC, FAULT)						\
  ((FAULT) = md_fault_none, MEM_READ_HALF(mem, addr = (SRC)))
#define READ_WORD(SRC, FAULT)						\
  ((FAULT) = md_fault_none, MEM_READ_WORD(mem, addr = (SRC)))
#ifdef HOST_HAS_QWORD
#define READ_QWORD(SRC, FAULT)						\
  ((FAULT) = md_fault_none, MEM_READ_QWORD(mem, addr = (SRC)))
#endif /* HOST_HAS_QWORD */

#define WRITE_BYTE(SRC, DST, FAULT)					\
  ((FAULT) = md_fault_none, MEM_WRITE_BYTE(mem, addr = (DST), (SRC)))
#define WRITE_HALF(SRC, DST, FAULT)					\
  ((FAULT) = md_fault_none, MEM_WRITE_HALF(mem, addr = (DST), (SRC)))
#define WRITE_WORD(SRC, DST, FAULT)					\
  ((FAULT) = md_fault_none, MEM_WRITE_WORD(mem, addr = (DST), (SRC)))
#ifdef HOST_HAS_QWORD
#define WRITE_QWORD(SRC, DST, FAULT)					\
  ((FAULT) = md_fault_none, MEM_WRITE_QWORD(mem, addr = (DST), (SRC)))
#endif /* HOST_HAS_QWORD */

/* system call handler macro */
#define SYSCALL(INST)	sys_syscall(&regs, mem_access, mem, INST, TRUE)

#define INST_TYPE 1
#define DATA_TYPE 2
#define BLOCK_TYPE 3
#define READ_TYPE 1
#define WRITE_TYPE 2
#define TRACE_BUFFER_LENGTH 20

void fillTraceBuffer(unsigned char *buffer,unsigned int type, unsigned int PC, unsigned int opType,unsigned int NPC, unsigned int bytes) {
    //  buffer[0]=type;
    memcpy(buffer,&type,4);
    memcpy(buffer+4,&PC,4);
    memcpy(buffer+8,&opType,4);
    memcpy(buffer+12,&NPC,4);
    memcpy(buffer+16,&bytes,4);
}

/* addressing mode FSM (dest of last LUI, used for decoding addr modes) */
static unsigned int fsm = 0;

/* start simulation, program loaded, processor precise state initialized */
void
sim_main(void)
{
    int i,additinalCount,temp;
    int lastVisitedBlock=-1,currentVisitedBlock;
    md_inst_t inst,oldInst=0;
    int measurementStarted=TRUE;
    register md_addr_t addr,oldSRAMAddr=0,oldFLASHAddr=0;
    register int is_write;
    char t[255];
    enum md_opcode op;
    unsigned int flags;
    enum md_fault_type fault;
    md_addr_t autoFinalPC=0;
    int setPC;
    int numBlocks=0;
    int numInstsInCurrentBlock=0;
    FILE *tracefilePointer=NULL,*blockFilePointer=NULL;
    unsigned char traceBuffer[TRACE_BUFFER_LENGTH];

    init_NUM_BITS();
    fprintf(stderr, "sim: ** starting functional simulation **\n");

    /* set up initial default next PC */
    regs.regs_NPC = regs.regs_PC + sizeof(md_inst_t);

    /* synchronize register files... */
    regs.regs_R[MD_REG_PC] = regs.regs_PC;

    /* check for DLite debugger entry condition */
    if (dlite_check_break(regs.regs_PC, /* no access */0, /* addr */0, 0, 0))
        dlite_main(regs.regs_PC - sizeof(md_inst_t), regs.regs_PC,
                   sim_num_insn, &regs, mem);
	
    if(initial_pc>0) {
	fprintf(stderr, "sim: ** fast forwarding to 0x%08X **\n", initial_pc);
	fast_forward = 1;
	while(regs.regs_PC!=initial_pc)
	{
	  autoFinalPC = regs.regs_PC + 4;
	    /* maintain $r0 semantics */
    #ifndef TARGET_ARM
	    regs.regs_R[MD_REG_ZERO] = 0;
    #endif
    #ifdef TARGET_ALPHA
	    regs.regs_F.d[MD_REG_ZERO] = 0.0;
    #endif /* TARGET_ALPHA */
	    /* get the next instruction to execute */
	    MD_FETCH_INST(inst, mem, regs.regs_PC);
	    if (verbose)
	    {
		myfprintf(stderr, "%10n @ 0x%08x: (0x%08x) (energy=%0.1f)", sim_num_insn, regs.regs_PC,inst,sim_total_energy);
		md_print_insn(inst, regs.regs_PC, stderr);
		fprintf(stderr, "\n");
		/* fflush(stderr); */
	    }
	    /* set default reference address and access mode */
	    addr = 0;
	    is_write = FALSE;

	    /* set default fault - none */
	    fault = md_fault_none;

	    /* decode the instruction */
	    MD_SET_OPCODE(op, inst);

	    if (op == NA)
		fatal("bogus opcode detected @ 0x%08p", regs.regs_PC);

	    setPC = 0;
	    regs.regs_R[MD_REG_PC] = regs.regs_PC;
	    /* execute the instruction */
	    switch (op)
	    {
    #define DEFINST(OP,MSK,NAME,OPFORM,RES,FLAGS,O1,O2,O3,I1,I2,I3,I4)	\
	    case OP:							\
	      SYMCAT(OP,_IMPL);						\
	      if(measurementStarted==TRUE) \
			    sim_total_energy+=opEnergy[OP];\
	      break;
    #define DEFLINK(OP,MSK,NAME,MASK,SHIFT)					\
	    case OP:							\
	      panic("attempted to execute a linking opcode");
    #define CONNECT(OP)
    #define DECLARE_FAULT(FAULT)						\
	      { fault = (FAULT); break; }
    #include "machine.def"
	    default:
		panic("attempted to execute a bogus opcode");
	    }

	    if (fault != md_fault_none)
		fatal("fault (%d) detected @ 0x%08p", fault, regs.regs_PC);

	    if (setPC != 0/* regs.regs_R[MD_REG_PC] != regs.regs_PC */)
		regs.regs_NPC = regs.regs_R[MD_REG_PC];

	    /* check for DLite debugger entry condition */
	    if (dlite_check_break(regs.regs_NPC,
				  is_write ? ACCESS_WRITE : ACCESS_READ,
				  addr, sim_num_insn, sim_num_insn))
		dlite_main(regs.regs_PC, regs.regs_NPC, sim_num_insn, &regs, mem);


	    /* go to the next instruction */
	    regs.regs_PC = regs.regs_NPC;
	    regs.regs_NPC += sizeof(md_inst_t);

	    /* finish early? */
	    if ((max_insts && sim_num_insn >= max_insts)){
	      fprintf(stderr, "sim: ** max instructions limit reached **\n");
	      return;
	    }
	    if((regs.regs_PC==finish_pc) || (regs.regs_PC==0)) {
		fprintf(stderr, "sim: ** 0x%08X visited **\n",regs.regs_PC);
		return;
	    }
	}
	fast_forward = 0;
    }
    sim_num_insn=0;

    if((initial_pc > 0) && (finish_pc ==0 ))
	finish_pc = autoFinalPC;

    fprintf(stderr, "sim: ** starting analysis **\n");

    if(tracefilename!=NULL && strlen(tracefilename)>0) {	// FIXME use better check
        tracefilePointer=fopen(tracefilename,"wb");
        if(tracefilePointer==NULL) {
            panic("cannot open %s for writing!",tracefilename);
        } else {
            //fprintf(tracefilePointer,"I/D/B , PC , R/W/Block No. , Address/NPC , Byte/Inst Count\n");
        }
    }

    if(tracefilePointer!=NULL && blockfilename!=NULL && strlen(blockfilename)>0) {	// FIXME use better check
        blockFilePointer=fopen(blockfilename,"r");
        if(blockFilePointer==NULL) {
            panic("cannot open %s for reading!",blockfilename);
        } else {
            fscanf(blockFilePointer,"%i",&numBlocks);
            if(numBlocks>0) {
//	  if(numBlocks>65535){
//		panic("Too much blocks!");
//	  }
                blockAddresses=malloc(numBlocks*sizeof(VarOrFuncMemInfo));
                if(blockAddresses!=NULL) {
                    for(i=0; i<numBlocks; i++) {
                        fscanf(blockFilePointer,"%s %i %i",blockAddresses[i].name,&blockAddresses[i].address,&blockAddresses[i].size);
                    }
                } else {
                    panic("cannot allocate memory!");
                }
            }
            fclose(blockFilePointer);
        }
    }

    if(tracefilePointer!=NULL) {
        if(blockAddresses!=NULL) {
            currentVisitedBlock=findCurrentBlockIndex(regs.regs_PC,blockAddresses,numBlocks);
            if(currentVisitedBlock!=lastVisitedBlock) {
                if(verbose)
                    fprintf(stderr, "B,0,%i,%p,%i\n", currentVisitedBlock,regs.regs_PC,numInstsInCurrentBlock);
                //fprintf(tracefilePointer, "B,0,%i,%p,%i\n", currentVisitedBlock,regs.regs_PC,numInstsInCurrentBlock);
                fillTraceBuffer(traceBuffer,BLOCK_TYPE,0,currentVisitedBlock,regs.regs_PC,numInstsInCurrentBlock);
                fwrite(traceBuffer,1,TRACE_BUFFER_LENGTH,tracefilePointer);
                lastVisitedBlock=currentVisitedBlock;
                numInstsInCurrentBlock=0;
            }
        } else if(traceBlock) {
            if(verbose)
                myfprintf(stderr, "%10n @ visiting new block (PC=0x0, target PC=0x%p, setPC=%n)\n", sim_num_insn,regs.regs_PC,setPC);
            //fprintf(tracefilePointer, "B,0,v,%p,0\n",regs.regs_PC);
            fillTraceBuffer(traceBuffer,BLOCK_TYPE,0,0,regs.regs_PC,0);
            fwrite(traceBuffer,1,TRACE_BUFFER_LENGTH,tracefilePointer);
            numInstsInCurrentBlock=0;
        }
    }
    while (TRUE)
    {
        /* maintain $r0 semantics */
#ifndef TARGET_ARM
        regs.regs_R[MD_REG_ZERO] = 0;
#endif
#ifdef TARGET_ALPHA
        regs.regs_F.d[MD_REG_ZERO] = 0.0;
#endif /* TARGET_ALPHA */
        /* get the next instruction to execute */
        MD_FETCH_INST(inst, mem, regs.regs_PC);
        if(tracefilePointer!=NULL) {
            if (traceInst){
                fillTraceBuffer(traceBuffer,INST_TYPE,regs.regs_PC,inst,regs.regs_PC,0);
                fwrite(traceBuffer,1,TRACE_BUFFER_LENGTH,tracefilePointer);
}
        }
        if (verbose)
        {
            myfprintf(stderr, "%10n @ 0x%08x: (0x%08x) (energy=%0.1f)", sim_num_insn, regs.regs_PC,inst,sim_total_energy);
            md_print_insn(inst, regs.regs_PC, stderr);
            fprintf(stderr, "\n");
            /* fflush(stderr); */
        }
        if(measurementStarted==TRUE){
	    temp=calculateHammingDistanceUsingTable(inst,oldInst);
	    inst_bus_activity+=temp;
            sim_total_energy+=temp*0.009228902;
	}
        if(measurementStarted==TRUE){
	    temp=calculateHammingDistanceUsingTable(inst,0x00000000);
	    inst_bus_ones+=temp;
            sim_total_energy+=temp*0.023649076;
	}
        oldInst=inst;
        /* keep an instruction count */
        sim_num_insn++;

        /* set default reference address and access mode */
        addr = 0;
        is_write = FALSE;

        /* set default fault - none */
        fault = md_fault_none;

        /* decode the instruction */
        MD_SET_OPCODE(op, inst);

        if (op == NA)
            fatal("bogus opcode detected @ 0x%08p", regs.regs_PC);

        setPC = 0;
        regs.regs_R[MD_REG_PC] = regs.regs_PC;
        memcpy(&oldRegs,&regs,sizeof(regs));
        /* execute the instruction */
        switch (op)
        {
#define DEFINST(OP,MSK,NAME,OPFORM,RES,FLAGS,O1,O2,O3,I1,I2,I3,I4)	\
	case OP:							\
          SYMCAT(OP,_IMPL);						\
          if(measurementStarted==TRUE) \
			sim_total_energy+=opEnergy[OP];\
          break;
#define DEFLINK(OP,MSK,NAME,MASK,SHIFT)					\
        case OP:							\
          panic("attempted to execute a linking opcode");
#define CONNECT(OP)
#define DECLARE_FAULT(FAULT)						\
	  { fault = (FAULT); break; }
#include "machine.def"
        default:
            panic("attempted to execute a bogus opcode");
        }

        if (fault != md_fault_none)
            fatal("fault (%d) detected @ 0x%08p", fault, regs.regs_PC);

        if(measurementStarted==TRUE){
	    temp=calculateActivity(regs,oldRegs);
	    regbank_activity+=temp;
            sim_total_energy+=temp*0.002808035;
	}
        memcpy(&oldRegs,&regs,sizeof(regs));

        if (setPC != 0/* regs.regs_R[MD_REG_PC] != regs.regs_PC */)
            regs.regs_NPC = regs.regs_R[MD_REG_PC];

        if (MD_OP_FLAGS(op) & F_MEM)
        {
            sim_num_refs++;
            if ((inst & (1<<27))==0) {
                if (MD_OP_FLAGS(op) & F_STORE) {
                    is_write = TRUE;
                    sim_num_stores++;
                    if(tracefilePointer!=NULL && traceData) {
                        if(verbose)
                            fprintf(stderr, "D,0x%x,%c,0x%x,%i\n", oldRegs.regs_PC,'w',addr,sizeof(regs.regs_R[0]));
//fprintf(tracefilePointer, "D,0x%x,%c,0x%x,%i\n", oldRegs.regs_PC,'w',addr,sizeof(regs.regs_R[0]));
                        fillTraceBuffer(traceBuffer,DATA_TYPE,oldRegs.regs_PC,WRITE_TYPE,addr,sizeof(regs.regs_R[0]));
                        fwrite(traceBuffer,1,TRACE_BUFFER_LENGTH,tracefilePointer);

                    }
                    if(measurementStarted==TRUE)
                        sim_total_energy+=0.983744242;
                } else {
                    sim_num_loads++;
                    if(tracefilePointer!=NULL && traceData) { //   fprintf(tracefilePointer, "D,0x%x,%c,0x%x,%i\n", oldRegs.regs_PC,'r',addr,sizeof(regs.regs_R[0]));
                        if(verbose)
                            fprintf(stderr, "D,0x%x,%c,0x%x,%i\n", oldRegs.regs_PC,'r',addr,sizeof(regs.regs_R[0]));

                        fillTraceBuffer(traceBuffer,DATA_TYPE,oldRegs.regs_PC,READ_TYPE,addr,sizeof(regs.regs_R[0]));
                        fwrite(traceBuffer,1,TRACE_BUFFER_LENGTH,tracefilePointer);
                    }
                    if (addr>=0x200000) {
                        sim_num_sram_loads++;
                        if(measurementStarted==TRUE)
                            sim_total_energy+=0.724368949;
                    } else {
                        sim_num_flash_loads++;
                        if(measurementStarted==TRUE)
                            sim_total_energy+=2.254368949;
                    }
                }
            } else {
                additinalCount=calculateHammingDistanceUsingTable(inst & 0xFFFF,0x00000000);
                if (inst & (1 << 20)) {	// FIXME why not use F_STORE?
                    sim_num_loads+=additinalCount;
                    if(tracefilePointer!=NULL && traceData)   {//fprintf(tracefilePointer, "D,0x%x,%c,0x%x,%i\n", oldRegs.regs_PC,'r',addr,additinalCount*sizeof(regs.regs_R[0]));
                        if(verbose)
                            fprintf(stderr, "D,0x%x,%c,0x%x,%i\n", oldRegs.regs_PC,'r',addr,sizeof(regs.regs_R[0])*additinalCount);
                        fillTraceBuffer(traceBuffer,DATA_TYPE,oldRegs.regs_PC,READ_TYPE,addr,additinalCount*sizeof(regs.regs_R[0]));
                        fwrite(traceBuffer,1,TRACE_BUFFER_LENGTH,tracefilePointer);
                    }
                    if (addr>=0x200000) {
                        sim_num_sram_loads+=additinalCount;
                        if(measurementStarted==TRUE)
                            sim_total_energy+=0.724368949*additinalCount;
                    } else {
                        sim_num_flash_loads+=additinalCount;
                        if(measurementStarted==TRUE)
                            sim_total_energy+=2.254368949*additinalCount;
                    }
                } else {
                    is_write = TRUE;
                    sim_num_stores+=additinalCount;
                    if(tracefilePointer!=NULL && traceData) {//  fprintf(tracefilePointer, "D,0x%x,%c,0x%x,%i\n", oldRegs.regs_PC,'w',addr,additinalCount*sizeof(regs.regs_R[0]));
                        if(verbose)
                            fprintf(stderr, "D,0x%x,%c,0x%x,%i\n", oldRegs.regs_PC,'w',addr,additinalCount*sizeof(regs.regs_R[0]));
                        fillTraceBuffer(traceBuffer,DATA_TYPE,oldRegs.regs_PC,WRITE_TYPE,addr,additinalCount*sizeof(regs.regs_R[0]));
                        fwrite(traceBuffer,1,TRACE_BUFFER_LENGTH,tracefilePointer);
                    }
                    if(measurementStarted==TRUE)
                        sim_total_energy+=0.983744242*additinalCount;
                }
            }
            if (addr>=0x200000) {
                oldSRAMAddr=addr;
            } else {
                oldFLASHAddr=addr;
            }
            if (MD_OP_FLAGS(op) & F_STORE)
                is_write = TRUE;
        }
//	flash_addr_bus_activity+=calculateInstActivity(regs.regs_R[MD_REG_PC],oldFLASHAddr);
        //oldFLASHAddr=regs.regs_R[MD_REG_PC];

        /*
         * profile this instruction
         */
        flags = MD_OP_FLAGS(op);

        if(tracefilePointer!=NULL) {
            numInstsInCurrentBlock++;
            if(blockAddresses!=NULL) {
                currentVisitedBlock=findCurrentBlockIndex(regs.regs_NPC,blockAddresses,numBlocks);
                if(currentVisitedBlock>-1)
		  if((currentVisitedBlock!=lastVisitedBlock) || traceBlock) {
		      if(verbose)
			  fprintf(stderr, "B,%p,%i,%p,%i\n", regs.regs_PC,currentVisitedBlock,regs.regs_NPC,numInstsInCurrentBlock);
		      //fprintf(tracefilePointer, "B,%p,%i,%p,%i\n", regs.regs_PC,currentVisitedBlock,regs.regs_NPC,numInstsInCurrentBlock);
		      fillTraceBuffer(traceBuffer,BLOCK_TYPE,regs.regs_PC,currentVisitedBlock,regs.regs_NPC,numInstsInCurrentBlock);
		      fwrite(traceBuffer,1,TRACE_BUFFER_LENGTH,tracefilePointer);
		      lastVisitedBlock=currentVisitedBlock;
		      numInstsInCurrentBlock=0;
		  }
            } else if(traceBlock) {
                if(flags & (F_CTRL)) {
                    if(verbose)
                        myfprintf(stderr, "%10n @ visiting new block (PC=0x%p, target PC=0x%p, setPC=%n)\n", sim_num_insn,regs.regs_PC,regs.regs_NPC,setPC);
                    //fprintf(tracefilePointer, "B,%p,v,%p,%i\n",regs.regs_PC, regs.regs_NPC,numInstsInCurrentBlock);
                    fillTraceBuffer(traceBuffer,BLOCK_TYPE,regs.regs_PC,0,regs.regs_NPC,numInstsInCurrentBlock);
                    fwrite(traceBuffer,1,TRACE_BUFFER_LENGTH,tracefilePointer);
                    numInstsInCurrentBlock=0;
                }
            }
        }
        if (prof_ic)
        {
            enum inst_class_t ic;

            /* compute instruction class */
            if (flags & F_LOAD) {
                ic = ic_load;
            } else if (flags & F_STORE) {
                ic = ic_store;
            }
#if 0
            else if (flags & F_UNCOND)
                ic = ic_uncond;
            else if (flags & F_COND)
                ic = ic_cond;
#endif
            else if (flags & F_CTRL && COND == COND_AL)
                ic = ic_uncond;
            else if (flags & F_CTRL && COND != COND_AL)
                ic = ic_cond;
            else if (flags & F_ICOMP)
                ic = ic_icomp;
            else if (flags & F_FCOMP)
                ic = ic_fcomp;
            else if (flags & F_TRAP)
                ic = ic_trap;
            else
                panic("instruction has no class");

            /* update instruction class profile */
            stat_add_sample(ic_prof, (int)ic);
        }

        if (prof_inst)
        {
            /* update instruction profile */
            stat_add_sample(inst_prof, (int)op - /* skip NA */1);
            
        }

        if ((MD_OP_FLAGS(op) & (F_ICOMP)) && (MD_OP_FUCLASS(op) !=IntMULT)) {
	    if ((inst&(1 << 25))) {
		if ((inst&0xf00)>>8) {
		    num_shift++;
		    if(measurementStarted==TRUE)
			sim_total_energy+=0.28835849;
		}
//			  num_shift_bits+=((inst&0xf00)>>8);
	    } else {
		if ((inst&0xff0)>>4) {
		    num_shift++;
		    if(measurementStarted==TRUE)
			sim_total_energy+=0.28835849;
		}
//			  num_shift_bits+=((inst&0xff0)>>4);
	    }
	}
        
        if (prof_unit)
        {
            /* update unit profile */
            stat_add_sample(unit_prof, (int)md_op2fu[op] - /* skip NA */1);
        }

#if FIXME
        if (prof_bc)
        {
            enum branch_class_t bc;

            /* compute instruction class */
            if (flags & F_CTRL)
            {
                if ((flags & (F_CALL|F_DIRJMP)) == (F_CALL|F_DIRJMP))
                    bc = bc_call_dir;
                else if ((flags & (F_CALL|F_INDIRJMP)) == (F_CALL|F_INDIRJMP))
                    bc = bc_call_indir;
                else if ((flags & (F_UNCOND|F_DIRJMP)) == (F_UNCOND|F_DIRJMP))
                    bc = bc_uncond_dir;
                else if ((flags & (F_UNCOND|F_INDIRJMP))== (F_UNCOND|F_INDIRJMP))
                    bc = bc_uncond_indir;
                else if ((flags & (F_COND|F_DIRJMP)) == (F_COND|F_DIRJMP))
                    bc = bc_cond_dir;
                else if ((flags & (F_COND|F_INDIRJMP)) == (F_COND|F_INDIRJMP))
                    bc = bc_cond_indir;
                else
                    panic("branch has no class");

                /* update instruction class profile */
                stat_add_sample(bc_prof, (int)bc);
            }
        }
#endif

#if FIXME
        if (prof_am)
        {
            enum md_amode_type am;

            /* update addressing mode pre-probe FSM */
            MD_AMODE_PREPROBE(op, fsm);

            /* compute addressing mode */
            if (flags & F_MEM)
            {
                /* compute addressing mode */
                MD_AMODE_PROBE(am, op, fsm);

                /* update the addressing mode profile */
                stat_add_sample(am_prof, (int)am);

                /* addressing mode pre-probe FSM, after all loads and stores */
                MD_AMODE_POSTPROBE(fsm);
            }
        }
#endif

#if FIXME
        if (prof_seg)
        {
            if (flags & F_MEM)
            {
                /* update instruction profile */
                stat_add_sample(seg_prof, (int)bind_to_seg(addr));
            }
        }
#endif

        if (prof_tsyms)
        {
            int tindex;

            /* attempt to bind inst address to a text segment symbol */
            sym_bind_addr(regs.regs_PC, &tindex, /* !exact */FALSE, sdb_text);

            if (tindex >= 0)
            {
                if (tindex > sym_ntextsyms)
                    panic("bogus text symbol index");

                stat_add_sample(tsym_prof, tindex);
            }
            /* else, could not bind to a symbol */
        }

        if (prof_dsyms)
        {
            int dindex;

            if (flags & F_MEM)
            {
                /* attempt to bind inst address to a text segment symbol */
                sym_bind_addr(addr, &dindex, /* !exact */FALSE, sdb_data);

                if (dindex >= 0)
                {
                    if (dindex > sym_ndatasyms)
                        panic("bogus data symbol index");

                    stat_add_sample(dsym_prof, dindex);
                }
                /* else, could not bind to a symbol */
            }
        }

        if (prof_taddr)
        {
            /* add regs_PC exec event to text address profile */
            stat_add_sample(taddr_prof, regs.regs_PC);
        }

        /* update any stats tracked by PC */
        for (i=0; i<pcstat_nelt; i++)
        {
            counter_t newval;
            int delta;

            /* check if any tracked stats changed */
            newval = STATVAL(pcstat_stats[i]);
            delta = newval - pcstat_lastvals[i];
            if (delta != 0)
            {
                stat_add_samples(pcstat_sdists[i], regs.regs_PC, delta);
                pcstat_lastvals[i] = newval;
            }

        }

        /* check for DLite debugger entry condition */
        if (dlite_check_break(regs.regs_NPC,
                              is_write ? ACCESS_WRITE : ACCESS_READ,
                              addr, sim_num_insn, sim_num_insn))
            dlite_main(regs.regs_PC, regs.regs_NPC, sim_num_insn, &regs, mem);


        /*if(measurementStarted==FALSE) {
            if(initial_meas_add==regs.regs_NPC) {
                measurementStarted=TRUE;
                targetPC=regs.regs_PC+sizeof(md_inst_t);
                myfprintf(stderr, "%10n @ Measurement enabled\n", sim_num_insn);
                inst_count_after_meas++;
            }
        } else {
            //myfprintf(stderr, "current PC = %d but target PC = %d\n", regs.regs_PC,finish_meas_add);
            if(targetPC==regs.regs_NPC) {
                measurementStarted=FALSE;
                myfprintf(stderr, "%10n @ Measurement disabled\n", sim_num_insn);
            } else
                inst_count_after_meas++;
        }*/

        /* go to the next instruction */
        regs.regs_PC = regs.regs_NPC;
        regs.regs_NPC += sizeof(md_inst_t);

        /* finish early? */
        if ((max_insts && sim_num_insn >= max_insts) || (regs.regs_PC==finish_pc) || (regs.regs_PC==0)) {
            if(tracefilePointer!=NULL) {
                fclose(tracefilePointer);
                if(blockAddresses!=NULL)
                    free(blockAddresses);
                tracefilePointer=NULL;
            }
	    /* finish early? */
	    if ((max_insts && sim_num_insn >= max_insts)){
	      fprintf(stderr, "sim: ** max instructions limit reached **\n");
	      return;
	    }
	    if((regs.regs_PC==finish_pc) || (regs.regs_PC==0)) {
		fprintf(stderr, "sim: ** 0x%08X visited **\n",regs.regs_PC);
		return;
	    }
            return;
        }
    }
}
