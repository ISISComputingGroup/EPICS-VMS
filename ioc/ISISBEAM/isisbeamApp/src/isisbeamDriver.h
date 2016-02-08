//
// $Id: isisbeamDriver.h 57 2014-06-29 18:24:33Z FreddieAkeroyd $
// $HeadURL: http://svn.isis.rl.ac.uk/freddie_test/trunk/EPICS_VMS/ioc/ISISBEAM/isisbeamApp/src/isisbeamDriver.h $
//

/// @file isisbeamDriver.h Header file of EPICS ASYN driver for ISIS beam/accelerator information
/// @author Freddie Akeroyd, STFC ISIS facility

#ifndef ISISBEAMDRIVER_H
#define ISISBEAMDRIVER_H

#define read_chan READ_CHAN
#define user_initialize_nofins USER_INITIALIZE_NOFINS

#if defined(__VMS) && !defined(TESTING)
extern "C" {
#include    <vsys$include:vdb_descrip.h>
#include    <vsys$include:vdb_routines.h>
#include    <vsys$include:vdb_structure.h>
#include    <include:usrlib.h>
#include    <include:hndlib.h>
#include	<descrip.h>
}
#endif /* defined(__VMS) && !defined(TESTING) */

#include <list>
#include <string>

#include "asynPortDriver.h"

static const char* TS1_INST_LIST = "ALF,ARGUS,CHRONUS,CRISP,EMMA,EMU,ENGINX,EVS,GEM,HIFI,HRPD,INES,IRIS,LOQ,MAPS,MARI,"
                  "MERLIN,MUSR,OSIRIS,PEARL,POLARIS,PRISMA,ROTAX,SANDALS,SURF,SXD,TOSCA,VESUVIO";

static const char* TS2_INST_LIST = "CHIPIR,IMAT,INTER,LARMOR,LET,NIMROD,OFFSPEC,POLREF,SANS2D,WISH,ZOOM";

/// Describes a beam/accelerator parameter in VSystem/VISTA 
class BeamParam
{
private:
#if defined(__VMS) && !defined(TESTING)
	struct vdb_descrip_s vista_name_dsc;
	struct vdb_descrip_v lval_dsc;
	struct vdb_descrip_v fval_dsc;
	struct vdb_descrip_v sval_dsc;
#endif /* defined(__VMS) && !defined(TESTING) */
	BeamParam(const BeamParam&) { }
	void operator=(const BeamParam&) { }
public:
	std::string param_name;
	std::string type;
	std::string vista_name;
	int param_id;
	long lval;
	float fval;
	static time_t updtime;   // used by UPDTIME and UPDTIMET
    static unsigned long error_count;
    int read_chan_status;
    static const int READCHAN_SUCCESS;
	char sval[512];
	BeamParam(const char* pn, const char* t, const char* vn) :
	    param_name(pn), type(t), vista_name(vn), param_id(-1), lval(0), 
        fval(0.0), read_chan_status(READCHAN_SUCCESS)
	{
		sval[0] = '\0';
		bool valid_type = (type == "long" || type == "float" || type == "string");
		if (!valid_type)
		{
			throw std::runtime_error("Invalid parameter data type in input file");
		}
#if defined(__VMS) && !defined(TESTING)
		vista_name_dsc.dsc_w_length = vista_name.size();
		vista_name_dsc.dsc_b_dtype = DSC$K_DTYPE_T;
		vista_name_dsc.dsc_b_class =  DSC$K_CLASS_S;
		vista_name_dsc.dsc_a_pointer = const_cast<char*>(vista_name.c_str());
		fval_dsc.dsc_w_length = sizeof(float);
		fval_dsc.dsc_b_dtype = DSC$K_DTYPE_F;
		fval_dsc.dsc_b_class =  DSC$K_CLASS_S;
		fval_dsc.dsc_a_pointer = &fval;
		lval_dsc.dsc_w_length = sizeof(long);
		lval_dsc.dsc_b_dtype = DSC$K_DTYPE_L;
		lval_dsc.dsc_b_class =  DSC$K_CLASS_S;
		lval_dsc.dsc_a_pointer = &lval;
		sval_dsc.dsc_w_length = sizeof(sval)-1;
		sval_dsc.dsc_b_dtype = DSC$K_DTYPE_T;
		sval_dsc.dsc_b_class =  DSC$K_CLASS_S;
		sval_dsc.dsc_a_pointer = sval;
#endif /* defined(__VMS) && !defined(TESTING) */
	}
	void read()
	{
		read_chan_status = READCHAN_SUCCESS;
		if (type == "float")
		{
#if defined(__VMS) && !defined(TESTING)
			read_chan_status = read_chan(&vista_name_dsc, &fval_dsc);
#else
			fval = rand();
#endif /* defined(__VMS) && !defined(TESTING) */
		}
		else if (type == "long")
		{
			if (param_name == "UPDTIMET")
			{
			    time(&updtime);
				lval = updtime;
			}
			else
			{
#if defined(__VMS) && !defined(TESTING)
			    read_chan_status = read_chan(&vista_name_dsc, &lval_dsc);
#else
			    lval = rand();
#endif /* defined(__VMS) && !defined(TESTING) */
			}
		}
		else if (type == "string")
		{
			if (param_name == "INSTTS1")
			{
			    strncpy(sval, TS1_INST_LIST, sizeof(sval)); 
			}
			else if (param_name == "INSTTS2")
			{
			    strncpy(sval, TS2_INST_LIST, sizeof(sval)); 
			}
			else if (param_name == "UPDTIME")
			{
			    // relies on being called after UPDTIMET which sets updtime
				strftime(sval, sizeof(sval), "%Y-%m-%dT%H:%M:%S", localtime(&updtime));
			}
			else
			{
#if defined(__VMS) && !defined(TESTING)
			    read_chan_status = read_chan(&vista_name_dsc, &sval_dsc);
#else
			    sprintf(sval, "%d", rand());
#endif /* defined(__VMS) && !defined(TESTING) */
			}
			sval[sizeof(sval)-1] = '\0';
		}
		if (!(read_chan_status & 0x1)) // errors are even numbers on VMS
		{
			errlogPrintf("isisbeamDriver:BeamParam:read: Error %d reading channel \"%s\"\n", read_chan_status, vista_name.c_str());
            ++error_count;
		}
	}
	void update(asynPortDriver* driver) const
	{
        if (read_chan_status != READCHAN_SUCCESS)
        {
            return; // don't update if not read a sensible value
        }
		if (type == "float")
		{
			driver->setDoubleParam(param_id, fval);
		}
		else if (type == "long")
		{
			driver->setIntegerParam(param_id, lval);
		}
		else if (type == "string")
		{
			driver->setStringParam(param_id, sval);
		}
	}
};

class isisbeamDriver : public asynPortDriver 
{
public:
	isisbeamDriver(const char* portName, const std::list<BeamParam*>& params, double pollTime);
	virtual asynStatus drvUserCreate(asynUser *pasynUser, const char *drvInfo, 
		const char **pptypeName, size_t *psize);

	static void pollerThreadC(void* arg);
	static void epicsExitFunc(void* arg);
	~isisbeamDriver();

private:
	std::list<BeamParam*> m_params;
	asynParamString_t* m_driverParamString;
	double m_pollTime;
	bool m_shutdown;  ///< false initially, set to true to request shutdown, and then wait until it goes false again
	
	void pollerThread();
};

#endif /* ISISBEAMDRIVER_H */

