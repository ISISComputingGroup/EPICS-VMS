// $Id: isisbeamMain.cpp 55 2014-06-27 14:12:54Z FreddieAkeroyd $
// $HeadURL: http://svn.isis.rl.ac.uk/freddie_test/trunk/EPICS_VMS/ioc/ISISBEAM/isisbeamApp/src/isisbeamMain.cpp $

#include <stddef.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>

#include "epicsThread.h"
#include "epicsExit.h"
#include "iocsh.h"

int main(int argc,char *argv[])
{
#ifdef __VMS
    bool non_interactive = true;
#else
    bool non_interactive = false;
#endif
    if(argc>=2) {    
        iocsh(argv[1]);
        epicsThreadSleep(.2);
    }
	if (non_interactive)
	{
        epicsThreadSleep(1e9);
	}
	else
	{
        iocsh(NULL);
	}
	epicsExit(EXIT_SUCCESS);
    return(0);
}
