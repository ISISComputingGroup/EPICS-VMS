/*************************************************************************\
* Copyright (c) 2002 The University of Chicago, as Operator of Argonne
*     National Laboratory.
* Copyright (c) 2002 The Regents of the University of California, as
*     Operator of Los Alamos National Laboratory.
* EPICS BASE Versions 3.13.7
* and higher are distributed subject to a Software License Agreement found
* in file LICENSE that is included with this distribution. 
\*************************************************************************/
/* macEnvExpandTestMain.cpp,v 1.1.2.1 2003/07/15 19:04:29 norume Exp
 */

extern "C" {
int macEnvExpandTest ( void );
}

int main ( int  , char *[] )
{
    return macEnvExpandTest ();
}
