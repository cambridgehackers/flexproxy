/* $Id: global.h,v 1.3 2004/06/24 12:53:04 cvs Exp $ */

#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#ifndef MAX
#define MAX(a,b) a > b ? a : b
#endif

#ifdef DEBUG
 
/*
 * Don't do this
 */
 
extern int fdin, fdout;
 
#endif
 
/* Global vars */
 
extern char *hostsfile;
extern char *flex_host;
extern short remote_flex_port;

#endif
