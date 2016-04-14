/* $Id: strutils.h,v 1.1 2004/01/03 12:45:46 cvs Exp $ */

#ifndef __T_STRUTILS_H__
#define __T_STRUTILS_H__

#define CR '\r'
#define LF '\n'

#ifdef __cplusplus
extern "C" {
#endif

int KillCRLF(char *);
int KillComments(char *);
int Collapse(char *);
int PrintString(const char *);
int PrintVariable(const char *vname);

#ifdef __cplusplus
};
#endif

#endif
