/*************************************************************************\
* Copyright (c) 2002 The University of Chicago, as Operator of Argonne
*     National Laboratory.
* Copyright (c) 2002 The Regents of the University of California, as
*     Operator of Los Alamos National Laboratory.
* EPICS BASE Versions 3.13.7
* and higher are distributed subject to a Software License Agreement found
* in file LICENSE that is included with this distribution. 
\*************************************************************************/
/*
 * vms specific socket include
 */

#ifndef osdSockH
#define osdSockH

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <tcp/errno.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/ioctl.h>
#include <tcp/netdb.h>

#ifdef __cplusplus
}
#endif
 
typedef int		SOCKET;
typedef int 		osiSockIoctl_t;
typedef unsigned int 	osiSocklen_t;

#define INVALID_SOCKET		(-1)
#define INADDR_LOOPBACK ((u_long)0x7f000001)

#define socket_close(S) close(S)
#define socket_ioctl(A,B,C) ioctl(A,B,C)

#define SOCKERRNO errno
    
#define MAXHOSTNAMELEN 75

#define SOCK_EWOULDBLOCK EWOULDBLOCK
#define SOCK_ENOBUFS ENOBUFS
#define SOCK_ECONNRESET ECONNRESET
#define SOCK_ETIMEDOUT ETIMEDOUT
#define SOCK_EADDRINUSE EADDRINUSE
#define SOCK_ECONNREFUSED ECONNREFUSED
#define SOCK_ECONNABORTED ECONNABORTED
#define SOCK_EINPROGRESS EINPROGRESS
#define SOCK_EISCONN EISCONN
#define SOCK_EALREADY EALREADY
#define SOCK_EINVAL EINVAL
#define SOCK_EINTR EINTR
#define SOCK_EPIPE EPIPE
#define SOCK_EMFILE EMFILE
#define SOCK_SHUTDOWN ESHUTDOWN
#define SOCK_ENOTSOCK ENOTSOCK
#define SOCK_EBADF EBADF

#define FD_IN_FDSET(FD) ((FD)<FD_SETSIZE&&(FD)>=0)

#define ifreq_size(pifreq) (sizeof(pifreq->ifr_name))

#endif /*osdSockH*/
