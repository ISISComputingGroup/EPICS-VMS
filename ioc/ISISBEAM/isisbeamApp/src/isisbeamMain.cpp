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
    int reset_error_count = atoi(getenv("RESET_ERROR_COUNT") != NULL ? getenv("RESET_ERROR_COUNT") : "50");
    double reset_time_diff = atof(getenv("RESET_TIME_DIFF") != NULL ? getenv("RESET_TIME_DIFF") : "300.0");
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
        } while(isisbeamDriver::g_error_count < reset_error_count && tdiff < reset_time_diff /* && isisbeamDriver::g_chan_err_cnt < 20 */ );

        errlogPrintf("Exiting IOC: total error count=%lu, chan error count=%lu, TIMET diff=%f\n", 
                       isisbeamDriver::g_error_count, isisbeamDriver::g_chan_err_cnt, tdiff);
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
