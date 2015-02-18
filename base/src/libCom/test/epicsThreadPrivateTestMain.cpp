/*************************************************************************\
* Copyright (c) 2002 The University of Chicago, as Operator of Argonne
*     National Laboratory.
* Copyright (c) 2002 The Regents of the University of California, as
*     Operator of Los Alamos National Laboratory.
* EPICS BASE Versions 3.13.7
* and higher are distributed subject to a Software License Agreement found
* in file LICENSE that is included with this distribution. 
\*************************************************************************/

/* epicsThreadPrivateTestMain.cpp,v 1.3.2.1 2004/09/16 14:03:03 norume Exp */

/* Author: Jeff Hill Date:    March 28 2001 */

extern "C" void epicsThreadPrivateTest ();

int main ()
{
    epicsThreadPrivateTest ();
    return 0;
}

