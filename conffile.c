/* $Id: conffile.c,v 1.1 2004/06/22 10:43:38 cvs Exp $ */

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include "strutils.h"

char *string_get_config(const char *confdir, const char *conffile,
  const char * string)
{
  char *str_tok;
  char *fname, *presult;
  FILE *fp;
  char buffer[256];
  static char result[256];
  int fnamelen;
                                                                                
  fnamelen = strlen(confdir) + strlen(conffile) + 2;
  fname = malloc(fnamelen);
  snprintf(fname, fnamelen, "%s/%s", confdir, conffile);
                                                                                
  fp = fopen(fname, "r");
  if(fp == NULL)
  {
    free(fname);
    return NULL;
  }
                                                                                
  while(!feof(fp))
  {
    fgets(buffer, 256, fp);
                                                                                
    KillCRLF(buffer);
    KillComments(buffer);
    Collapse(buffer);
                                                                                
    str_tok = strtok(buffer,"=");
                                                                                
    if(str_tok == NULL)
      continue;
                                                                                
    Collapse(str_tok);
                                                                                
    if(strcasecmp(buffer, string) == 0)
    {
      presult = strtok(NULL, " ");
      strncpy(result, presult, 256);
      fclose(fp);
      free(fname);
                                                                                
      return result;
    }
  }
  return NULL;
}

int int_get_config(const char *confdir, const char *conffile,
  const char * string)
{
  char *str_tok;
  char *fname;
  FILE *fp;
  char buffer[256];
  int value, fnamelen;
                                                                                
  fnamelen = strlen(confdir) + strlen(conffile) + 2;
  fname = malloc(fnamelen);
  snprintf(fname, fnamelen, "%s/%s", confdir, conffile);
                                                                                
  fp = fopen(fname, "r");
  if(fp == NULL)
  {
    free(fname);
    return -1;
  }
                                                                                
  while(!feof(fp))
  {
    fgets(buffer, 256, fp);
    KillCRLF(buffer);
    KillComments(buffer);
    Collapse(buffer);
                                                                                
    str_tok = strtok(buffer,"=");
                                                                                
    if(str_tok == NULL)
      continue;
                                                                                
    Collapse(str_tok);
                                                                                
    if(strcasecmp(buffer, string) == 0)
    {
      value = atoi(strtok(NULL, " "));
      if(value > 0)
      {
        fclose(fp);
        free(fname);
        return value;
      }
    }
  }
  return -1;
}

