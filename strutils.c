/* $Id: strutils.c,v 1.2 2004/06/22 10:46:35 cvs Exp $ */

#include <stdio.h>
#include <ctype.h>
#include <malloc.h>
#include <string.h>
#include <sys/types.h>
#include <stdlib.h>
#include "strutils.h"

int KillCRLF(char *line)
{
   int j=0;
   char *tempstr=NULL;
   size_t i;
   
   if(strlen(line) == 0)
     return 0;

   tempstr = (char *)malloc(strlen(line) + 1);
   /* TODO */
   
   if(tempstr==NULL)
     {
#ifdef _DEBUG
	fprintf(stderr,"function KillCRLF: Failed to allocate buffer\n");
#endif	  
	return 1;
     }
   
   for(i=0; i<strlen(line); i++)
     {
	if((line[i]!=CR) && (line[i]!=LF) )
	  {
	     tempstr[j]=line[i];
	  }
	else
	  tempstr[j]=' ';
	
	j++;
     }
   tempstr[j]='\0';
   strcpy(line, tempstr);
   
   free(tempstr);
   return 0;
}

int KillComments(char *line)
{
   int in_string=0;
   size_t i;
   
   for(i=0; i<strlen(line); i++)
     {
	switch(line[i])
	  {
	   case '"': in_string++;
	     break;
	  
	   case '#':
	     if((in_string % 2)==0)
	       line[i]='\0';
	     return 0;
	     break;
	     
	   default: break;
	  }
     }
   return 0;
}

int Collapse(char *line)
{
   char *tempstr=NULL;
   int in_string=0;
   int first_space=1;   
   int j=0;
   size_t i;
   
   if(strlen(line) == 0)
     return 0;

   tempstr = (char *)malloc(strlen(line) + 1);
   /* TODO */
   
   if(tempstr==NULL)
     {
#ifdef _DEBUG
	fprintf(stderr,"function Collapse: Failed to allocate buffer\n");
#endif	  
	return 1;
     }
   
   for(i=0; i<strlen(line); i++)
     {
	switch(line[i])
	  {
	   case '\t':
	   case ' ': 
	     if((((in_string % 2)!=0) || first_space) && (j > 0))
	       {
		  first_space = 0;
		  tempstr[j]=line[i];
		  j++;
	       }
	     break;
	   case '"': in_string ++;
	   default:
	     tempstr[j] = line[i];
	     j++;
	     first_space = 1;
	     break;
	  }
     }

   tempstr[j]='\0';
   if(tempstr[j-1]==' ' || tempstr[j-1]=='\t')
     tempstr[j-1]='\0';
   
   strcpy(line, tempstr);
   
   free(tempstr);
   return 0;
}
