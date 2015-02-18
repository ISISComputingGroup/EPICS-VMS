/*************************************************************************\
* Copyright (c) 2002 The University of Chicago, as Operator of Argonne
*     National Laboratory.
* Copyright (c) 2002 The Regents of the University of California, as
*     Operator of Los Alamos National Laboratory.
* EPICS BASE Versions 3.13.7
* and higher are distributed subject to a Software License Agreement found
* in file LICENSE that is included with this distribution. 
\*************************************************************************/
#ifndef osdThreadh
#define osdThreadh

#include <signal.h>
#include <pthread.h>

#undef sched_get_priority_max
#define sched_get_priority_max(p) \
	( p==SCHED_OTHER ? PRI_FG_MAX_NP : PRI_FIFO_MAX )

#undef sched_get_priority_min
#define sched_get_priority_min(p) \
	( p==SCHED_OTHER ? PRI_FG_MIN_NP : PRI_FIFO_MIN )

#ifdef __cplusplus
extern "C" {
#endif

pthread_t epicsThreadGetPosixThreadId ( epicsThreadId id );

#ifdef __cplusplus
}
#endif

#endif /* osdThreadh */
