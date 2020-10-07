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
	P_chanErrCnt = i++;
	m_driverParamString[P_chanErrCnt].param = P_chanErrCnt;
	m_driverParamString[P_chanErrCnt].paramString = P_chanErrCntString;
	setIntegerParam(P_chanErrCnt, 0);
	P_errCnt = i++;
	m_driverParamString[P_errCnt].param = P_errCnt;
	m_driverParamString[P_errCnt].paramString = P_errCntString;
	setIntegerParam(P_errCnt, 0);
	P_xml = i++;
	m_driverParamString[P_xml].param = P_xml;
	m_driverParamString[P_xml].paramString = P_xmlString;
	setStringParam(P_xml, "");
	
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
	static char xml_buffer[6000];
	user_initialize_nofins();
	while(!m_shutdown)
	{
	    g_chan_err_cnt = 0;
		for(std::list<BeamParam*>::iterator it=m_params.begin(); it != m_params.end(); ++it)
		{
			if ( !(*it)->read() )
			{
			    ++g_chan_err_cnt;
			}
		}
		lock();
		for(std::list<BeamParam*>::iterator it=m_params.begin(); it != m_params.end(); ++it)
		{
			(*it)->update(this);
		}
		setIntegerParam(P_chanErrCnt, g_chan_err_cnt);
		setIntegerParam(P_errCnt, BeamParam::error_count);
		getXML(xml_buffer, sizeof(xml_buffer));
		setStringParam(P_xml, xml_buffer);
		callParamCallbacks();
		unlock();
		epicsThreadSleep(m_pollTime);
	}
	m_shutdown = false; // signal we have finished
}	

static const char* xml_format = 
	"<?xml version=\"1.0\"?>" 
	"<ISISBEAM>"
	"<BEAMS>%.1f</BEAMS>"
	"<BEAME1>%.1f</BEAME1>"
	"<BEAMT>%.1f</BEAMT>"
	"<BEAMT2>%.1f</BEAMT2>"
	"<INJE>%.0f</INJE>"
	"<ACCE>%.0f</ACCE>"
	"<EXTE>%.0f</EXTE>"
	"<REPR>%.1f</REPR>"
	"<REPR2>%.1f</REPR2>"
	"<MODE>%d</MODE>"
	"<GMS1>%d</GMS1>"
	"<GMS2>%d</GMS2>"
	"<TS1ON>%s</TS1ON>"
	"<TS1OFF>%s</TS1OFF>"
	"<TS2ON>%s</TS2ON>"
	"<TS2OFF>%s</TS2OFF>"
	"<SHUTN>%d</SHUTN>"
	"<SHUTS>%d</SHUTS>"
	"<TS1SHUTTERS>"
	  "<SANDALS>%s</SANDALS>" /* N1 */
	  "<PRISMA>%s</PRISMA>" /* N2 */
	  "<ALF>%s</ALF>" /* N2 */
	  "<ROTAX>%s</ROTAX>" /* N2 */
	  "<SURF>%s</SURF>" /* N3 */
	  "<CRISP>%s</CRISP>" /* N4 */
	  "<LOQ>%s</LOQ>" /* N5 */
	  "<IRIS>%s</IRIS>" /* N6 */
	  "<OSIRIS>%s</OSIRIS>" /* N6 */
	  "<POLARIS>%s</POLARIS>" /* N7 */
	  "<TOSCA>%s</TOSCA>" /* N8 */
	  "<INES>%s</INES>" /* N8 */
	  "<HET>%s</HET>" /* N9 */
	  "<MAPS>%s</MAPS>" /* S1 */
	  "<EVS>%s</EVS>" /* S2 */
	  "<SXD>%s</SXD>" /* S3 */
	  "<MERLIN>%s</MERLIN>" /* S4 */
	  "<MARI>%s</MARI>" /* S6 */
	  "<GEM>%s</GEM>" /* S7 */
	  "<HRPD>%s</HRPD>" /* S8 */
	  "<ENGINX>%s</ENGINX>" /* S8 */
	  "<PEARL>%s</PEARL>" /* S9 */
	"</TS1SHUTTERS>"
	"<MTEMP>%.1f</MTEMP>"
    "<HTEMP>%.1f</HTEMP>"
	"<MUONKICKER>%d</MUONKICKER>"
	"<TS1_TOTAL>%.1f</TS1_TOTAL>"
    "<TS1_TOTAL_YEST>%.1f</TS1_TOTAL_YEST>"
	"<TS2_TOTAL>%.1f</TS2_TOTAL>"
    "<TS2_TOTAL_YEST>%.1f</TS2_TOTAL_YEST>"
	"<TS2SHUTTERS>"
	  "<ZOOM>%s</ZOOM>" /* E1 */
	  "<SANS2D>%s</SANS2D>" /* E2 */
	  "<POLREF>%s</POLREF>" /* E3 */
	  "<INTER>%s</INTER>" /* E4 */
	  "<OFFSPEC>%s</OFFSPEC>" /* E5 */
	  "<LARMOR>%s</LARMOR>" /* E6 */
	  "<WISH>%s</WISH>" /* E8 */
	  "<CHIPIR>%s</CHIPIR>" /* W1 */
	  "<IMAT>%s</IMAT>" /* W5 */
	  "<LET>%s</LET>" /* W6 */
	  "<NIMROD>%s</NIMROD>" /* W7 */
	"</TS2SHUTTERS>"
	"<T2MTEMP1>%.1f</T2MTEMP1>" /* decoupled methane, TE842 */
	"<T2MTEMP2>%.1f</T2MTEMP2>" /* coupled methane, TE852 */ 
	"<T2HTEMP1>%.1f</T2HTEMP1>" /* hydrogen TT706 */
	"<BEAMI>%.1f</BEAMI>"
	"<BEAMR>%.1f</BEAMR>"
	"<BEAML>%.1f</BEAML>"
	"<TS2VAT>"
	  "<ZOOM>%s</ZOOM>" /* E1 */
	  "<SANS2D>%s</SANS2D>" /* E2 */
	  "<POLREF>%s</POLREF>" /* E3 */
	  "<INTER>%s</INTER>" /* E4 */
	  "<OFFSPEC>%s</OFFSPEC>" /* E5 */
	  "<LARMOR>%s</LARMOR>" /* E6 */
	  "<WISH>%s</WISH>" /* E8 */
	  "<CHIPIR>%s</CHIPIR>" /* W1 */
	  "<IMAT>%s</IMAT>" /* W5 */
	  "<LET>%s</LET>" /* W6 */
	  "<NIMROD>%s</NIMROD>" /* W7 */
	"</TS2VAT>"
	"<TS2SHUTTERMODES>"
	  "<ZOOM>%s</ZOOM>" /* E1 */
	  "<SANS2D>%s</SANS2D>" /* E2 */
	  "<POLREF>%s</POLREF>" /* E3 */
	  "<INTER>%s</INTER>" /* E4 */
	  "<OFFSPEC>%s</OFFSPEC>" /* E5 */
	  "<LARMOR>%s</LARMOR>" /* E6 */
	  "<WISH>%s</WISH>" /* E8 */
	  "<CHIPIR>%s</CHIPIR>" /* W1 */
	  "<IMAT>%s</IMAT>" /* W5 */
	  "<LET>%s</LET>" /* W6 */
	  "<NIMROD>%s</NIMROD>" /* W7 */
	"</TS2SHUTTERMODES>"
    "<DMOD_RUNTIME>%d</DMOD_RUNTIME>"
    "<DMOD_RUNTIME_LIM>%d</DMOD_RUNTIME_LIM>"
    "<DMOD_UABEAM>%.1f</DMOD_UABEAM>"
    "<DMOD_ANNLOW1>%d</DMOD_ANNLOW1>"
    "<DMOD_FILL_MASS>%.1f</DMOD_FILL_MASS>"
    "<BEAM_ENERGY>%d</BEAM_ENERGY>"
	"<TS1CH4TRANS>%d</TS1CH4TRANS>"
	"<TS1CH4TRANSW>%s</TS1CH4TRANSW>"
	"<TIME>%u</TIME>"
	"<TIMEF>%s</TIMEF>"
	"</ISISBEAM>"; 

static std::string as_iso(time_t t)
{
	char time_buffer[64];
	struct tm* pstm = localtime(&t);
	strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%dT%H:%M:%S", pstm);
	return std::string(time_buffer);
}

static float floatParam(const std::string& name)
{
    return atof(BeamParam::paramValues[name].c_str());
}	

static int intParam(const std::string& name)
{
    return atoi(BeamParam::paramValues[name].c_str());
}

static const char* ts2_shutter_mode(const std::string& beamline)
{
	int stat = intParam(beamline + "_mode");
    switch( stat )
    {
                case 0:
                    return "DEACT"; // De-Activated
                    break;
                case 1:
                    return "LOCAL-HMI"; // Manual (Control Room HMI)
                    break;
                case 2:
                    return "REMOTE-HMI"; // Remote Manual (used for shutter scanning)
                    break;
                case 3:
                    return "SHIELD-TOP"; // Shield Top Control (used for maintenance)
                    break;
                case 4:
                    return "BEAMLINE"; // Opening (beam line request)
                    break;
                default:
                    return "INVALID";
                    break;

    }	    
    return "INVALID"; /*NOTREACHED*/
}

static const char* ts2_shutter_status(const std::string& beamline)
{
	int stat = intParam(beamline + "_shut");
    switch( stat )
    {
    	case 0:
    	    /* return "DEACTIVE"; */
    	    return "DEACT";
    	    break;
    	case 1:
    	    return "OPEN";
    	    break;
    	case 2:
    	    return "CLOSED";
    	    break;
    	case 3:
    	    return "MOVING";
    	    break;
    	case 4:
    	    return "FAULT";
    	    break;
    	default:
    	    return "INVALID";
    	    break;
    }	    
    return "INVALID"; /*NOTREACHED*/
}

static const char* ts2_vat_status(const std::string& beamline)
{
	int stat = intParam(beamline + "_vat");
    switch( stat )
    {
    	case 0:
    	    /* return "DEACTIVE"; */
    	    return "DEACT";
    	    break;
    	case 1:
    	    return "OPEN";
    	    break;
    	case 2:
    	    return "MOVING";
    	    break;
    	case 3:
    	    return "CLOSED";
    	    break;
    	case 4:
    	    return "FAULT";
    	    break;
    	default:
    	    return "INVALID";
    	    break;
    }	    
    return "INVALID"; /*NOTREACHED*/
}

static const char* ts1_shutter_status(const std::string& beamline)
{
	int stat = intParam(beamline + "_shut");
    switch( stat )
    {
    	case 0:
    	    return "CLOSED";
    	    break;
    	case 1:
    	    return "OPEN";
    	    break;
    	case 2:
    	    return "FAULT";
    	    break;
    	case 3:
    	    return "MOVING";
    	    break;
    	case 4:
    	    return "INVALID";
    	    break;
    	default:
    	    return "INVALID";
    	    break;
    }	    
    return "INVALID"; /*NOTREACHED*/
}


void isisbeamDriver::getXML(char* xml_buffer, int len)
{
    static time_t ts1_off, ts1_on, ts2_off, ts2_on, ch4_transfer_warn;
	static double beamt_old, beamt2_old;
	static int ch4_transfer_old;
	double beamt, beamt2;
	int ch4_transfer;
	time_t timer;
	beamt = floatParam("beam_tgt");
	beamt2 = floatParam("beam_tgt2");
	ch4_transfer = intParam("ch4_transfer");
	time(&timer);
	if (ch4_transfer_old == 0 && ch4_transfer == 1)
    {
	    ch4_transfer_warn = timer;
    }
    ch4_transfer_old = ch4_transfer;	
	memset(xml_buffer, 0, len);
		if (beamt2 == 0.0 && beamt2_old > 0.0)
		{
			ts2_off = timer;
		}
		if (beamt == 0.0 && beamt_old > 0.0)
		{
			ts1_off = timer;
		}
		if (beamt2 > 0.0 && beamt2_old == 0.0)
		{
			ts2_on = timer;
		}
		if (beamt > 0.0 && beamt_old == 0.0)
		{
			ts1_on = timer;
		}
		beamt_old = beamt;
		beamt2_old = beamt2;
		snprintf(xml_buffer, len, xml_format, 
		    floatParam("beam_sync"), 
		    floatParam("beam_e1"), 
		    floatParam("beam_tgt"), 
		    floatParam("beam_tgt2"), 
		    floatParam("inj_eff"), 
		    floatParam("acc_eff"), 
		    floatParam("ext_eff"), 
		    floatParam("rep_rate"), 
		    floatParam("rep_rate2"), 
		    intParam("mode"), 
		    intParam("gms1on"), 
		    intParam("gms2on"), 
			as_iso(ts1_on).c_str(),
			as_iso(ts1_off).c_str(),
			as_iso(ts2_on).c_str(),
			as_iso(ts2_off).c_str(),
			intParam("shut_north"),
			intParam("shut_south"),
			ts1_shutter_status("n1"),
			ts1_shutter_status("n2"),
			ts1_shutter_status("n2"),
			ts1_shutter_status("n2"),
			ts1_shutter_status("n3"),
			ts1_shutter_status("n4"),
			ts1_shutter_status("n5"),
			ts1_shutter_status("n6"),
			ts1_shutter_status("n6"),
			ts1_shutter_status("n7"),
			ts1_shutter_status("n8"),
			ts1_shutter_status("n8"),
			ts1_shutter_status("n9"),
			ts1_shutter_status("s1"),
			ts1_shutter_status("s2"),
			ts1_shutter_status("s3"),
			ts1_shutter_status("s4"),
			ts1_shutter_status("s6"),
			ts1_shutter_status("s7"),
			ts1_shutter_status("s8"),
			ts1_shutter_status("s8"),
			ts1_shutter_status("s9"),
			floatParam("mtemp"),
			floatParam("htemp"),
			intParam("muon_kicker"),
			floatParam("ts1_total"),
			floatParam("ts1_total_yest"),
			floatParam("ts2_total"),
			floatParam("ts2_total_yest"),
			ts2_shutter_status("e1"),
			ts2_shutter_status("e2"),
			ts2_shutter_status("e3"),
			ts2_shutter_status("e4"),
			ts2_shutter_status("e5"),
			ts2_shutter_status("e6"),
			ts2_shutter_status("e8"),
			ts2_shutter_status("w1"),
			ts2_shutter_status("w5"),
			ts2_shutter_status("w6"),
			ts2_shutter_status("w7"),
			floatParam("t2_mtemp1"),
			floatParam("t2_mtemp2"),
			floatParam("t2_htemp1"),
			floatParam("beam_ions"),  
			floatParam("beam_rfq"),  
			floatParam("beam_linac"),  
			ts2_vat_status("e1"),
			ts2_vat_status("e2"),
			ts2_vat_status("e3"),
			ts2_vat_status("e4"),
			ts2_vat_status("e5"),
			ts2_vat_status("e6"),
			ts2_vat_status("e8"),
			ts2_vat_status("w1"),
			ts2_vat_status("w5"),
			ts2_vat_status("w6"),
			ts2_vat_status("w7"),
			ts2_shutter_mode("e1"),
			ts2_shutter_mode("e2"),
			ts2_shutter_mode("e3"),
			ts2_shutter_mode("e4"),
			ts2_shutter_mode("e5"),
			ts2_shutter_mode("e6"),
			ts2_shutter_mode("e8"),
			ts2_shutter_mode("w1"),
			ts2_shutter_mode("w5"),
			ts2_shutter_mode("w6"),
			ts2_shutter_mode("w7"),
			intParam("dmod_runtime"),
			intParam("dmod_runtime_lim"),
			floatParam("dmod_uabeam"),
			intParam("dmod_annlow1"),
			floatParam("dmod_fill_mass"),
			intParam("beam_energy"),
			ch4_transfer,
			as_iso(ch4_transfer_warn).c_str(),
			(unsigned)intParam("UPDTIMET"),
			BeamParam::paramValues["UPDTIME"].c_str());			
	xml_buffer[len-1] = '\0';
}

extern "C" {

	/// EPICS iocsh callable function to call constructor of isisbeamConfigure().
	/// \param[in] portName @copydoc initArg0
	int isisbeamConfigure(const char *portName, const char* paramFile, double pollTime)
	{
		std::list<BeamParam*> params; // a copy of this list's contents will be kept by isisbeamDriver when we go out of scope
		char param_name[129], param_type[129], param_opts[129], vista_name[129];
		int update_freq;
		FILE* f = fopen(paramFile, "r"); // VMS does not support "rt"
		if (f == NULL)
		{
			errlogPrintf("isisbeamDriver:isisbeamConfigure: cannot open paramFile \"%s\"", paramFile);
			return(asynError);
		}
		while(fscanf(f, " %128s %128s %128s %d %128s ", param_name, param_type, param_opts, &update_freq, vista_name) == 5)
		{
			std::cout << "Adding parameter " << param_name << " (" << param_type << ") -> " << vista_name << "(expected update freq = " << update_freq << "s)" << std::endl;
			params.push_back(new BeamParam(param_name, param_type, vista_name, param_opts, update_freq));
		}
		fclose(f);
		std::cout << "Loaded " << params.size() << " parameters from " << paramFile << std::endl;
		// now  create our special parameters
		params.push_back(new BeamParam("UPDTIMET", "long", "(UPDTIMET)", "n", 10));
		params.push_back(new BeamParam("UPDTIME", "string", "(UPDTIME)", "n", 10));
		params.push_back(new BeamParam("INSTTS1", "string", "(INSTTS1)", "n", 0));
		params.push_back(new BeamParam("INSTTS2", "string", "(INSTTS2)", "n", 0));
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

unsigned long isisbeamDriver::g_chan_err_cnt = 0;
unsigned long BeamParam::error_count = 0;
const int BeamParam::READCHAN_SUCCESS = 0x1;  /* on VMS 1 is success, errors are even numbers */
time_t BeamParam::g_updtime = time(NULL);
std::map<std::string,std::string> BeamParam::paramValues; 
