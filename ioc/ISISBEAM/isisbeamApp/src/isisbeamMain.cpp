// $Id: isisbeamMain.cpp 55 2014-06-27 14:12:54Z FreddieAkeroyd $
// $HeadURL: http://svn.isis.rl.ac.uk/freddie_test/trunk/EPICS_VMS/ioc/ISISBEAM/isisbeamApp/src/isisbeamMain.cpp $

#include <stddef.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <stdexcept>

#ifdef __VMS
#include <starlet.h>
#include <delprcsymdef.h>
#else
#define        sys$delprc(a, b, c)
#endif

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
        // g_chan_err_cnt includes timestamp not updating errors
        double tdiff;
        do {
            epicsThreadSleep(10.0);
            tdiff = difftime(time(NULL), BeamParam::g_updtime);
        } while(isisbeamDriver::error_count < 40 && tdiff < 300.0 /* && isisbeamDriver::g_chan_err_cnt < 20 */ );

        errlogPrintf("Exiting IOC: total error count=%lu, chan error count=%lu, TIMET diff=%f\n", 
                       isisbeamDriver::error_count, isisbeamDriver::g_chan_err_cnt, tdiff);
        //_exit(EXIT_SUCCESS);
        //sys$delprc(NULL, NULL, DELPRC$M_NOEXIT);
        sys$delprc(NULL, NULL, 0); // this is equivalent to a DCL stop/id
    }
    else
    {
        iocsh(NULL);
    }
//  epicsExit(EXIT_SUCCESS);
    return(0);
}
