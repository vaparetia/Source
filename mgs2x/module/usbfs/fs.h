/* SCEI CONFIDENTIAL
 $PSLibId$
 */
/* $Id: fs.h,v 1.1 2001/02/14 01:20:41 usr01475 Exp $ */

/*
 *                     I/O Processor System Services
 *
 *      Copyright (C) 1998-1999 Sony Computer Entertainment Inc.
 *                        All Rights Reserved.
 *
 *                         fs.h
 *                         IO manager interface
 *
 *       Version        Date            Design      Log
 *  --------------------------------------------------------------------
 *       1.00           1999/11/09      hakama
 */

#ifdef LANGUAGE_C

/* device table */
struct device_table {
	char 	*dt_string;
	int	dt_type;
	int	dt_size;
	char	*dt_desc;
	struct device_function *dt_func;
};

struct device_function {
	int	(*df_init)();	/* ・f・o・C・X・・・・・・・・・(AddDev()) */
	int	(*df_exit)();	/* ・f・o・C・X・・I・・・・・(DelDev()) */
	int	(*df_format)();	/* ・f・o・C・X・・t・H・[・}・b・g */
	int	(*df_open)();	/* ・t・@・C・・・・I・[・v・・ */
	int	(*df_close)();	/* ・t・@・C・・・・N・・・[・Y */
	int	(*df_read)();	/* ・t・@・C・・・・・・[・h */
	int	(*df_write)();	/* ・t・@・C・・・・・・C・g */
	int	(*df_lseek)();	/* ・t・@・C・・・|・C・・・^・・V・[・N */
	int	(*df_ioctl)();	/* ・t・@・C・・・・R・・・g・・・[・・ */
	int	(*df_remove)();	/* ・t・@・C・・・・・・*/
	int	(*df_mkdir)();	/* ・f・B・・・N・g・・・・・・*/
	int	(*df_rmdir)();	/* ・f・B・・・N・g・・・・・・*/
	int	(*df_dopen)();	/* ・f・B・・・N・g・・・・I・[・v・・ */
	int	(*df_dclose)();	/* ・f・B・・・N・g・・・・N・・・[・Y */
	int	(*df_dread)();	/* ・f・B・・・N・g・・・・・・[・h */
	int	(*df_getstat)();/* F/D ・X・e・[・^・X・・・・*/
	int	(*df_chstat)();	/* F/D ・X・e・[・^・X・・・X */
};

#endif LANGUAGE_C

/* device types */
#define	DTTYPE_CHAR	0x1	/* character device */
#define	DTTYPE_CONS	0x2	/* can be console */
#define	DTTYPE_BLOCK	0x4	/* block device */
#define DTTYPE_RAW	0x8	/* raw device that uses fs switch */
#define DTTYPE_FS	0x10


/* io block */
#ifdef LANGUAGE_C
struct iob {
	int i_flgs;
	int i_unit;
	struct device_table	*i_dp;
	void			*i_private;
};
#endif LANGUAGE_C

#ifndef NULL
#define NULL 0
#endif

/* Request codes */
#define	READ	1
#define	WRITE	2

#define NDTBL	16	/* max number of device table entroy */
#define NIOB	16	/* max number of open files */

