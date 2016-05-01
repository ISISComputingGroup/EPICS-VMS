//
// $Id: isisbeamDriver.cpp 56 2014-06-29 15:45:02Z FreddieAkeroyd $
// $HeadURL: http://svn.isis.rl.ac.uk/freddie_test/trunk/EPICS_VMS/ioc/ISISBEAM/isisbeamApp/src/isisbeamDriver.cpp $
//

/// @file isisbeamDriver.cpp Implementation of EPICS ASYN driver for ISIS beam/accelerator information
/// @author Freddie Akeroyd, STFC ISIS facility

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <math.h>
#include <exception>
#include <stdexcept>
#include <iostream>
#include <fstream>

#include <epicsTypes.h>
#include <errlog.h>
#include <epicsTime.h>
#include <epicsThread.h>
#include <epicsString.h>
#include <epicsTimer.h>
#include <epicsMutex.h>
#include <epicsEvent.h>
#include <epicsExit.h>
#include <iocsh.h>

#include "isisbeamDriver.h"

#include <macLib.h>
#include <epicsGuard.h>

#include <epicsExport.h>

//static epicsThreadOnceId onceId = EPICS_THREAD_ONCE_INIT;

static const char *driverName="isisbeamDriver";

/// Constructor for the isisbeamDriver class.
/// Calls constructor for the asynPortDriver base class.
/// \param[in] portName @copydoc initArg0
/// \param[in] params parameter names to create
isisbeamDriver::isisbeamDriver(const char *portName, const std::list<BeamParam*>& params, double pollTime) 
	: asynPortDriver(portName, 
	1, /* maxAddr */ 
	params.size() + NUM_IB_DRIVER_PARAMS,
	asynInt32Mask | asynFloat64Mask | asynOctetMask | asynDrvUserMask, /* Interface mask */
	asynInt32Mask | asynFloat64Mask | asynOctetMask,  /* Interrupt mask */
	ASYN_CANBLOCK, /* asynFlags.  This driver can block but it is not multi-device */
	1, /* Autoconnect */
	0, /* Default priority */
	0),	/* Default stack size*/
	m_params(params), m_pollTime(pollTime), m_shutdown(false)
{

	const char *functionName = "isisbeamDriver";
	int i = 0;
	epicsAtExit(epicsExitFunc, this);
	if (pollTime <= 1.0e-3)
	{
		m_pollTime = 3.0;
		errlogPrintf("%s:%s: invalid poll time %f using %f seconds\n", driverName, functionName, pollTime, m_pollTime);
	}
	m_driverParamString.resize(m_params.size() + NUM_IB_DRIVER_PARAMS);
	for(std::list<BeamParam*>::iterator it=m_params.begin(); it != m_params.end(); ++it, ++i)
	{
		if ((*it)->type == "float")
		{
			m_driverParamString[i].param = (*it)->param_id = i;
			m_driverParamString[i].paramString = (*it)->param_name.c_str();
			setDoubleParam(i, 0.0);
			//			createParam((*it)->param_name.c_str(), asynParamFloat64, &((*it)->param_id));
		}
		else if ((*it)->type == "long")
		{
			m_driverParamString[i].param = (*it)->param_id = i;
			m_driverParamString[i].paramString = (*it)->param_name.c_str();
			setIntegerParam(i, 0);
			//			createParam((*it)->param_name.c_str(), asynParamInt32, &((*it)->param_id));
		}
		else if ((*it)->type == "string")
		{
			m_driverParamString[i].param = (*it)->param_id = i;
			m_driverParamString[i].paramString = (*it)->param_name.c_str();
			setStringParam(i, "");
			//			createParam((*it)->param_name.c_str(), asynParamOctet, &((*it)->param_id));
		}
		else
		{
			errlogPrintf("%s:%s: unknown type %s for parameter %s\n", driverName, functionName, (*it)->type.c_str(), (*it)->param_name.c_str());
		}
	}
	m_driverParamString[i].param = P_chanErrCnt = i;
	m_driverParamString[i].paramString = P_chanErrCntString;
	setIntegerParam(i, 0);
	++i;
	
	// Create the thread for background tasks 
	if (epicsThreadCreate("isisbeamPoller",
		epicsThreadPriorityMedium,
		epicsThreadGetStackSize(epicsThreadStackMedium),
		(EPICSTHREADFUNC)pollerThreadC, this) == 0)
	{
		errlogPrintf("%s:%s: epicsThreadCreate failure\n", driverName, functionName);
		return;
	}
}

void isisbeamDriver::epicsExitFunc(void* arg)
{
    isisbeamDriver* driver = (isisbeamDriver*)arg;
	delete driver;
}

asynStatus isisbeamDriver::drvUserCreate(asynUser *pasynUser, const char *drvInfo, 
	const char **pptypeName, size_t *psize)
{
	return this->drvUserCreateParam(pasynUser, drvInfo, pptypeName, psize,
		&(m_driverParamString[0]), m_driverParamString.size());
}

void isisbeamDriver::pollerThreadC(void* arg)
{ 
	isisbeamDriver* driver = (isisbeamDriver*)arg; 
	driver->pollerThread();
}

isisbeamDriver::~isisbeamDriver()
{
	m_shutdown = true;
	while(m_shutdown)
	{
		epicsThreadSleep(0.5); // wait for pollerThread() to exit
	}
	for(std::list<BeamParam*>::iterator it=m_params.begin(); it != m_params.end(); ++it)
	{
		delete *it;
	}
}

void isisbeamDriver::pollerThread()
{
	//    static const char* functionName = "isisbeamPoller";
	user_initialize_nofins();
    int chan_err_cnt;
	while(!m_shutdown)
	{
	    chan_err_cnt = 0;
		for(std::list<BeamParam*>::iterator it=m_params.begin(); it != m_params.end(); ++it)
		{
			if ( !(*it)->read() )
			{
			    ++chan_err_cnt;
			}
		}
		lock();
		for(std::list<BeamParam*>::const_iterator it=m_params.begin(); it != m_params.end(); ++it)
		{
			(*it)->update(this);
		}
		setIntegerParam(P_chanErrCnt, chan_err_cnt);
		callParamCallbacks();
		unlock();
		epicsThreadSleep(m_pollTime);
	}
	m_shutdown = false; // signal we have finished
}	

extern "C" {

	/// EPICS iocsh callable function to call constructor of isisbeamConfigure().
	/// \param[in] portName @copydoc initArg0
	int isisbeamConfigure(const char *portName, const char* paramFile, double pollTime)
	{
		std::list<BeamParam*> params; // a copy of this list's contents will be kept by isisbeamDriver when we go out of scope
		char param_name[129], param_type[129], vista_name[129];
		int update_freq;
		FILE* f = fopen(paramFile, "r"); // VMS does not support "rt"
		if (f == NULL)
		{
			errlogPrintf("isisbeamDriver:isisbeamConfigure: cannot open paramFile \"%s\"", paramFile);
			return(asynError);
		}
		while(fscanf(f, " %128s %128s %d %128s ", param_name, param_type, &update_freq, vista_name) == 4)
		{
			std::cout << "Adding parameter " << param_name << " (" << param_type << ") -> " << vista_name << "(expected update freq = " << update_freq << "s)" << std::endl;
			params.push_back(new BeamParam(param_name, param_type, vista_name, update_freq));
		}
		fclose(f);
		std::cout << "Loaded " << params.size() << " parameters from " << paramFile << std::endl;
		// now  create our special parameters
		params.push_back(new BeamParam("UPDTIMET", "long", "(UPDTIMET)", 10));
		params.push_back(new BeamParam("UPDTIME", "string", "(UPDTIME)", 10));
		params.push_back(new BeamParam("INSTTS1", "string", "(INSTTS1)", 0));
		params.push_back(new BeamParam("INSTTS2", "string", "(INSTTS2)", 0));
		params.push_back(new BeamParam("ERRCNT", "long", "(ERRCNT)", 0));
		try
		{
			new isisbeamDriver(portName, params, pollTime);
			return(asynSuccess);
		}
		catch(const std::exception& ex)
		{
			errlogPrintf("isisbeamDriver:isisbeamConfigure: %s", ex.what());
			return(asynError);
		}
	}

	// EPICS iocsh shell commands 

	static const iocshArg initArg0 = { "portName", iocshArgString};			///< The name of the asyn driver port we will create
	static const iocshArg initArg1 = { "paramFile", iocshArgString};			///< The name of the asyn driver port we will create
	static const iocshArg initArg2 = { "pollTime", iocshArgDouble};			///< poll time in seconds

	static const iocshArg * const initArgs[] = { &initArg0, &initArg1, &initArg2 };

	static const iocshFuncDef initFuncDef = {"isisbeamConfigure", sizeof(initArgs) / sizeof(iocshArg*), initArgs};

	static void initCallFunc(const iocshArgBuf *args)
	{
		isisbeamConfigure(args[0].sval, args[1].sval, args[2].dval);
	}

	static void isisbeamRegister(void)
	{
		iocshRegister(&initFuncDef, initCallFunc);
	}

	epicsExportRegistrar(isisbeamRegister);

}

unsigned long BeamParam::error_count = 0;
const int BeamParam::READCHAN_SUCCESS = 0x1;  /* on VMS 1 is success, errors are even numbers */

time_t BeamParam::g_updtime = 0;
