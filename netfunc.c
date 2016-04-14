/* $Id: netfunc.c,v 1.4 2004/09/15 09:33:08 cvs Exp $ */

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include "strutils.h"
#include "config.h"
#include "global.h"

char *dt_get_addr(const char *hname)
{
  static char h_addr[20];
  char buffer[256]; /* this could be a BUG */
  char *host_addr, *host_tok;
  FILE * fp;
 
  memset(h_addr, 0, 20);
 
  if(strlen(hname) <= 0)
    return FLEXHOST;
 
  fp = fopen(hostsfile, "r");
 
  if(fp == NULL)
  {
    logmessage(INFO, "Error opening Hostfile\n");
    return FLEXHOST; /* Return default Flex host */
  }
 
  while(!feof(fp))
  {
    fgets(buffer, 256, fp);
    KillCRLF(buffer);
    KillComments(buffer);
    Collapse(buffer);
 
    if(!isdigit(buffer[0]))
      continue;
 
    host_addr = strtok(buffer, " ");
    while((host_tok = strtok(NULL, " ")))
    {
      if(strcmp(hname, host_tok) == 0)
      {
        strncpy(h_addr, host_addr, 20);
        fclose(fp);
        return h_addr;
      }
    }
  }
  return h_addr;
}

void child_forwarder(int flconn, int flserv, int vdconn, int vdserv)
{
  char buffer[MAXBUF];
  fd_set rset;
  int fdmax;
  int status;

  fdmax = MAX(flconn, flserv);
  fdmax = MAX(fdmax, vdconn);
  fdmax = MAX(fdmax, vdserv);
  fdmax++;

  while(1)
  {
    FD_ZERO(&rset);

    if(flconn != -1)
      FD_SET(flconn, &rset);

    if(flserv != -1)
      FD_SET(flserv, &rset);

    if(vdconn != -1)
      FD_SET(vdconn, &rset);

    if(vdserv != -1)
      FD_SET(vdserv, &rset);

    logmessage(INFO, "Enter Select\n");
    status = select(fdmax, &rset, NULL, NULL, NULL);

    if(flconn != -1 && FD_ISSET(flconn, &rset))
    {
      logmessage(INFO, "flconn\n");
      status = recv(flconn, buffer, MAXBUF, 0);
      if(status > 0)
        status = send(flserv, buffer, status, 0);
      else if(status == 0)
        break;
    }

    if(flserv != -1 && FD_ISSET(flserv, &rset))
    {
      logmessage(INFO, "flserv\n");
      status = recv(flserv, buffer, MAXBUF, 0);
      if(status > 0)
        status = send(flconn, buffer, status, 0);
      else if(status == 0)
        break;
    }

    if(vdconn != -1 && FD_ISSET(vdconn, &rset))
    {
      logmessage(INFO, "vdconn\n");
      status = recv(vdconn, buffer, MAXBUF, 0);
      if(status > 0)
        status = send(vdserv, buffer, status, 0);
      else if(status == 0)
        break;
    }

    if(vdserv != -1 && FD_ISSET(vdserv, &rset))
    {
      logmessage(INFO, "vdserv\n");
      status = recv(vdserv, buffer, MAXBUF, 0);
      if(status > 0)
        status = send(vdconn, buffer, status, 0);
      else if(status == 0)
        break;
    }
  }

  /* Close all sockets and exit */

  logmessage(INFO, "CHILD -- someone closed the connection\n");
  logmessage(INFO, "CHILD -- close sockets and exit\n");

  shutdown(vdconn, 2);
  shutdown(vdserv, 2);
  shutdown(flconn, 2);
  shutdown(flserv, 2);
  close(vdconn);
  close(vdserv);
  close(flconn);
  close(flserv);

  exit(0);
}

int rundaemon()
{
  int cpid;
  int newfd;

  chdir("/");
  setsid(); /* Become session leader */
  setpgid(0, 0); /* set process group id */

  /*
   * Open /dev/null
   */
  newfd = open("/dev/null", O_RDWR);

  close(0); /* Close stdin  */
  close(1); /* Close stdout */
  close(2); /* Close stderr */

  dup2(newfd, STDIN_FILENO);
  dup2(newfd, STDOUT_FILENO);
  dup2(newfd, STDERR_FILENO);

  cpid = fork();
  if(cpid < 0)
    return -1; /* fork failed */

  if(cpid == 0)
    return 0;
  else
    exit(0); /* parent exit */
}
