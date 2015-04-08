// $Id: isisbeamMain.cpp 55 2014-06-27 14:12:54Z FreddieAkeroyd $
// $HeadURL: http://svn.isis.rl.ac.uk/freddie_test/trunk/EPICS_VMS/ioc/ISISBEAM/isisbeamApp/src/isisbeamMain.cpp $

#include <stddef.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>

#include <stdexcept>

#include "epicsThread.h"
#include "epicsExit.h"
#include "iocsh.h"
#include "errlog.h"

#include "isisbeamDriver.h"

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
        while(BeamParam::error_count < 20)
        {
            epicsThreadSleep(10.0);
        }
        errlogPrintf("Exiting IOC as error count=%lu too high\n", BeamParam::error_count);
	}
	else
	{
        iocsh(NULL);
	}
	epicsExit(EXIT_SUCCESS);
    return(0);
}
