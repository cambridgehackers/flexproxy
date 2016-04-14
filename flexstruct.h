/* $Id: flexstruct.h,v 1.2 2003/12/31 16:01:49 cvs Exp $ */

#ifndef _FLEXSTRUCT_H_
#define _FLEXSTRUCT_H_

typedef struct sflex_proto
{
  char sfp_a; /* ???? */
  char sfp_b; /* ???? */
  char sfp_hostname[33]; /* Hostname is 32 char long + trailing \0 */
  char sfp_remote_port[16];
} sflex_proto;

#endif
