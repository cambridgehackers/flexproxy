/* $Id: flexproxy.c,v 1.23 2004/09/15 08:15:43 cvs Exp $ */

#include "config.h"
#include "strutils.h"
#include "flexstruct.h"
#include "conffile.h"
#include "netfunc.h"
#include "global.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/time.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#ifdef linux
#include <getopt.h>
#endif
#include <ctype.h>

#define OPTS "d:b"

#ifdef DEBUG

/*
 * Don't do this
 */

int fdin, fdout;

#endif

/* Global vars */

char *hostsfile;
char *flex_host;
short remote_flex_port;

int acceptloop(int sock, unsigned short flexport)
{
  sflex_proto *p_sfp;
  char buffer[MAXBUF];
  char vdhost[33];
  char *vdaddr;
  fd_set rset, exset;
  struct sockaddr_in sinaddr;
  struct timeval tv;
  size_t sinlen;
  time_t now;
  int newconn, serconn, vdconn, vdserv, vdlist, fdmax;
  int ischild=0;
  int status, msglen, port_changed=0;
  unsigned short vdportr, vdportl;

  p_sfp = NULL;
  vdportr = vdportl = 0;
  fdmax = 0;

  fdmax = serconn = vdconn = vdserv = newconn = vdlist = -1;

  while(1)
  {
    FD_ZERO(&rset);
    FD_ZERO(&exset);

    fdmax = 0;

    if(sock != -1)
    {
      FD_SET(sock, &rset);
      FD_SET(sock, &exset);
      fdmax = sock;
    }

    if(vdserv != -1)
    {
      FD_SET(vdserv, &rset);
      FD_SET(vdserv, &exset);
      fdmax = MAX(fdmax, vdserv);
    }

    if(serconn != -1)
    {
      FD_SET(serconn, &rset);
      FD_SET(serconn, &exset);
      fdmax = MAX(fdmax, serconn);
    }

    if(newconn != -1)
    {
      FD_SET(newconn, &rset);
      FD_SET(newconn, &exset);
      fdmax = MAX(fdmax, newconn);
    }

    if(vdconn != -1)
    {
      FD_SET(vdconn, &rset);
      FD_SET(vdconn, &exset);
      fdmax = MAX(fdmax, vdconn);
    }

    if(vdlist != -1)
    {
      FD_SET(vdlist, &rset);
      FD_SET(vdlist, &exset);
      fdmax = MAX(fdmax, vdlist);
    }

    fdmax++;

    tv.tv_sec = 30;
    tv.tv_usec = 0;

    status = select(fdmax, &rset, NULL, &exset, NULL);

    /*
     * This case should happen only when a timeout occurs
     * No timeout set
     */
    if(status == 0)
    {
      now = time(NULL);
      if(vdlist != -1)
      {
        logmessage(INFO, "vdlist shutdown\n");
        status = shutdown(vdlist, 2);
        status = close(vdlist);
        vdlist = -1;
        vdportr = vdportl = 0;
      }

      /*
       * Close all open connections
       */
      if(newconn != -1)
      {
        shutdown(newconn, 2);
        close(newconn);
        newconn = -1;
      }

      if(serconn != -1)
      {
        shutdown(serconn, 2);
        close(serconn);
        serconn = -1;
      }

      if(vdconn != -1)
      {
        shutdown(vdconn, 2);
        close(vdconn);
        vdconn = -1;
      }

      continue;
    }

    if(status < 0)
    {
      logmessage(INFO, "Select Error\n");
      continue;
    }

    if(sock != -1 && FD_ISSET(sock, &rset)) /* got new connection request */
    {
      if( newconn == -1) /* No connection established yet */
      {
        logmessage(INFO, "Accept\n");
        sinlen = sizeof(sinaddr);
        newconn = accept(sock, (struct sockaddr *)&sinaddr, &sinlen);

        memset(&sinaddr, 0, sizeof(struct sockaddr_in));
        serconn = socket(PF_INET, SOCK_STREAM, IPPROTO_IP);
        sinaddr.sin_family = AF_INET;
        sinaddr.sin_port = htons(flexport);

        vdaddr = dt_get_addr(flex_host);

        status = inet_pton(AF_INET, vdaddr, &sinaddr.sin_addr);

        if(status < 0)
        {
          logmessage(INFO, "NEWCONN inet_pton failed\n");
          logmessage(INFO, "NEWCONN Exit ???, right now ... Continue\n");
        }

        logmessage(INFO, "Connect\n");
        status = connect(serconn, (struct sockaddr *)&sinaddr, 
                   sizeof(sinaddr));

        /*
         * Connection to server failed, close client connection too
         */
        if(status == -1)
        {
          status = shutdown(serconn, 2);
          status = close(serconn);
          serconn = -1;

          status = shutdown(newconn, 2);
          status = close(newconn);
          newconn = -1;
          logmessage(INFO, "SERCONN Server connection failed\n");
        }
        else /* Connection to server succeeded FORK */
        {
          status = fork();
          if(status > 0) /* Fork succeeded - Parent is here */
          {
            close(newconn);
            newconn = -1;
            close(serconn);
            serconn = -1;
          }
          else if ( status == 0) /* Fork succeeded - child is here */
          {
            close(sock);
            sock = -1;
            ischild = 1;
          }
          else /* Fork failed */
          {
            logmessage(INFO, "FORK Failed");
          }
        }
      }
      else /* newconn != -1 */
      {
        /*
         *
         * Received a new connection while old connection has not been
         * spawned yet. Something went wrong somewhere between me and the
         * flexlm server or client
         *
         */
        logmessage(INFO, "SOCK Unhandled exception\n");
      }
    } /* if(FD_ISSET(sock, &rset)) */

    if(newconn != -1 && FD_ISSET(newconn, &rset))
    {
      logmessage(INFO, "newconn\n");
      status = recv(newconn, buffer, MAXBUF, 0);
      if(serconn != -1 && status > 0)
      {
#ifdef DEBUG
        printf("Newconn Got %d bytes\n", status);
        write(fdin, buffer, status);
        fsync(fdin);
#endif
        status = send(serconn, buffer, status, 0);
      }
      else
      {
        if(serconn != -1)
        {
          status = shutdown(serconn, 2);
          status = close(serconn);
          serconn = -1;
        }
        status = shutdown(newconn, 2);
        status = close(newconn);
        newconn = -1;
        if(ischild)
        {
          if(vdconn == -1 && vdserv == -1)
            exit(0);
        }
      }
    }

    if(serconn != -1 && FD_ISSET(serconn, &rset))
    {
      logmessage(INFO, "serconn\n");
      status = recv(serconn, buffer, MAXBUF, 0);
#ifdef DEBUG
      printf("serconn Got %d bytes\n", status);
      write(fdout, buffer, status);
      fsync(fdout);
#endif

      msglen = status;

      if(vdportr == 0 && vdlist == -1)
      {
        /*
         * parse server reply.
         */
        if(status > sizeof(sflex_proto))
        {
          p_sfp = (sflex_proto *)buffer;
          status = atoi(p_sfp->sfp_remote_port);
          strncpy(vdhost, p_sfp->sfp_hostname, 32);
          vdhost[32] = '\0'; /* Just in case */

          logmessage(INFO, "Remote Host ");
          logmessage(INFO, vdhost);
          logmessage(INFO, "  Remote Port ");
          logmessage(INFO, p_sfp->sfp_remote_port);
          logmessage(INFO, "\n");

          if(status < 0)
          {
            vdportr = vdportl = 0;
            vdhost[0] = '\0';
          }
          else
          {
            /*
             * Be ready to listen
             */
            vdportr = vdportl = status;

            vdlist = socket(PF_INET, SOCK_STREAM, IPPROTO_IP);
            memset(&sinaddr, 0, sizeof(struct sockaddr_in));

            sinaddr.sin_family = AF_INET;
            sinaddr.sin_port = htons(vdportr);
            sinaddr.sin_addr.s_addr = htonl(INADDR_ANY);

            status = bind(vdlist, (struct sockaddr *)&sinaddr, 
                          sizeof(sinaddr));

            if(status < 0)
            {
              port_changed = 1;
              sinaddr.sin_port = htons(0);
              status = bind(vdlist, (struct sockaddr *)&sinaddr,
                            sizeof(sinaddr));
            }

            if(status < 0)
            {
              logmessage(INFO, "VDLIST -- bind failed\n");

              shutdown(vdlist, 2);
              close(vdlist);
              vdlist = -1;
              vdportr = vdportl = 0;
              vdhost[0] = '\0';
            }
            else
              listen(vdlist, 3);
          } /* status < 0 */
        } /* status > sizeof(sflex_proto) */
      } /* vdport == 0 && vdlist == -1 */

      if(vdlist > 0 && port_changed == 1)
      {
        /* Get Address of new port */
        struct sockaddr_in l_saddr;
        size_t l_saddrlen;

        l_saddrlen = sizeof(struct sockaddr_in);

        status = getsockname(vdlist, (struct sockaddr *)&l_saddr, &l_saddrlen);

        vdportl = ntohs(l_saddr.sin_port);

        /* printf("New port %d %d\n", vdportl, vdportr); */

        sprintf(p_sfp->sfp_remote_port, "%d", vdportl);
      }

      send(newconn, buffer, msglen, 0);

      if(!(newconn != -1 && status > 0))
      {
        if(newconn != -1)
        {
          status = shutdown(newconn, 2);
          status = close(newconn);
          newconn = -1;
        }
        status = shutdown(serconn, 2);
        status = close(serconn);
        serconn = -1;
      }
    }

    if(vdconn != -1 && FD_ISSET(vdconn, &rset))
    {
      logmessage(INFO, "vdconn\n");
      status = recv(vdconn, buffer, MAXBUF, 0);

      if(vdserv != -1 && status > 0)
      {
        status = send(vdserv, buffer, status, 0);
      }
      else
      {
        if(vdserv != -1)
        {
          status = shutdown(vdserv, 2);
          status = close(vdserv);
          vdserv = -1;
        }
        status = shutdown(vdconn, 2);
        status = close(vdconn);
        vdconn = -1;
        if(ischild)
          exit(0);
      }
    }

    if(vdserv != -1 && FD_ISSET(vdserv, &rset))
    {
      logmessage(INFO, "vdserv\n");
      status = recv(vdserv, buffer, MAXBUF, 0);

      if(vdserv != -1 && status > 0)
      {
        status = send(vdconn, buffer, status, 0);
      }
      else
      {
        if(vdconn != -1)
        {
          status = shutdown(vdserv, 2);
          status = close(vdserv);
          vdserv = -1;
        }
        status = shutdown(vdconn, 2);
        status = close(vdconn);
        vdconn = -1;
        if(ischild)
          exit(0);
      }
    }

    if(vdlist != -1 && (FD_ISSET(vdlist, &rset)) && (vdconn == -1) && (vdportr > 0))
    {
      logmessage(INFO, "vdlist\n");
      sinlen = sizeof(struct sockaddr_in);
      vdconn = accept(vdlist, (struct sockaddr *)&sinaddr, &sinlen);

      memset(&sinaddr, 0, sizeof(sinaddr));
      vdserv = socket(PF_INET, SOCK_STREAM, IPPROTO_IP);
      sinaddr.sin_family = AF_INET;
      sinaddr.sin_port = htons(vdportr);

      vdaddr = dt_get_addr(flex_host);

      status = inet_pton(AF_INET, vdaddr, &sinaddr.sin_addr);

      if(status < 0)
      {
        logmessage(INFO, "VDLIST inet_pton failed\n");
        logmessage(INFO, "VDLIST Exit ???, right now ... Continue\n");
      }
      logmessage(INFO, "vdlist connect\n");
      status = connect(vdserv, (struct sockaddr *)&sinaddr,
                 sizeof(struct sockaddr_in));

      /*
       * Connection to server failed, close client connection too
       */
      if(status == -1)
      {
        status = shutdown(vdconn, 2);
        status = close(vdconn);
        vdconn = -1;

        status = shutdown(vdserv, 2);
        status = close(vdserv);
        vdserv = -1;
        logmessage(INFO, "VDSERV Server connection failed\n");
      }
      else
      {
        status = 0;

        if(!ischild) /* do fork only if first fork try failed */
          status = fork();

        if(status < 0)
        {
          logmessage(INFO, "Fork Failed\n");
          logmessage(INFO, "EXIT ???\n");
        }
        else if(status == 0)
        {
          /*
           * Child is here ...
           */
          close(vdlist);
          close(sock);

          logmessage(INFO, "Child Spawned\n");
          child_forwarder(newconn, serconn, vdconn, vdserv);
          exit(0); /* Do not return here */
        }
        else
        {
          /*
           * Parent is here ...
           */
          logmessage(INFO, "Parent Running\n");
          close(vdserv);
          close(vdconn);
          close(newconn);
          close(serconn);
          vdserv = vdconn = newconn = serconn = -1;
        } /* status < 0 */
      } /* if status == -1 */
    } /* FD_ISSET */
  } /* while(1) */
  return 0;
}

int main(int argc, char *argv[])
{
  char *p_tmp;
  char *confdir;
#ifdef INET6
  struct sockaddr_in6 flsin;
#else
  struct sockaddr_in flsin;
#endif
  int flsock;
  int status, one;
  int op, daemonize = 0;
  in_port_t flexport;

#ifdef DEBUG
  fdin = -1;
  fdout = -1;

  if((fdin == -1) && (fdout == -1))
  {
    fdin = open("flexin.out", O_APPEND | O_CREAT | O_RDWR, 
                 S_IRGRP | S_IRUSR | S_IWUSR | S_IWGRP);
    fdout = open("flexout.out", O_APPEND | O_CREAT | O_RDWR,
                 S_IRGRP | S_IRUSR | S_IWUSR | S_IWGRP);
  }
#endif

  one = 1;
  confdir = CONFDIR;
  opterr = 0;

  while((op = getopt(argc, argv, OPTS)) != EOF)
  {
    switch(op)
    {
      case 'd' : confdir = optarg; break;
      case 'b' : daemonize = 1; break;
      default : break;
    }
  }

  /*
   * Read Config file
   */


  p_tmp = string_get_config(confdir, CONFFILE, PROXY_HOSTS);

  if(p_tmp == NULL)
    hostsfile = HOSTFILE;
  else
  {
    hostsfile = malloc(strlen(p_tmp)+1);
    strcpy(hostsfile, p_tmp);
  }

  p_tmp = string_get_config(confdir, CONFFILE, FLEX_HOST);

  if(p_tmp == NULL)
    flex_host = FLEXHOST;
  else
  {
    flex_host = malloc(strlen(p_tmp)+1);
    strcpy(flex_host, p_tmp);
  }

  status = int_get_config(confdir, CONFFILE, PROXY_PORT);
  if(status < 0)
    flexport = FLEXPORT;
  else
    flexport = status;

  status = int_get_config(confdir, CONFFILE, REMOTE_FLEX_PORT);
  if(status < 0)
    remote_flex_port = flexport;
  else
    remote_flex_port = status;

  signal(SIGCHLD, SIG_IGN);

#ifndef DEBUG
  if(daemonize)
  {
    rundaemon();
  }
#endif

#ifdef INET6
  flsock = socket(PF_INET6, SOCK_STREAM, IPPROTO_IP);
#else
  flsock = socket(PF_INET, SOCK_STREAM, IPPROTO_IP);
#endif

  memset(&flsin, 0, sizeof(flsin));

#ifdef INET6
  flsin.sin6_family = AF_INET;
  flsin.sin6_port = htons(flexport);
  flsin.sin6_addr = in6addr_any;
#else
  flsin.sin_family = AF_INET;
  flsin.sin_port = htons(flexport);
  flsin.sin_addr.s_addr = INADDR_ANY;
#endif

  status = setsockopt(flsock, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
  status = bind(flsock, (struct sockaddr *)&flsin, sizeof(flsin));
  if(status < 0)
  {
    logmessage(INFO, "FL Bind Error\n");
    return -1;
  }

  /*
   * Socket is ready, now listen
   */
  status = listen(flsock, 3);

  acceptloop(flsock, remote_flex_port);

  shutdown(flsock, 2);
  close(flsock);

  return 0;
}
