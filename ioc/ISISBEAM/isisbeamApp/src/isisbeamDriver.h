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

extern "C" {
#if defined(__VMS) && !defined(TESTING)

#include    <vsys$include:vdb_descrip.h>
#include    <vsys$include:vdb_routines.h>
#include    <vsys$include:vdb_structure.h>
#include    <include:usrlib.h>
#include    <include:hndlib.h>
#include	<descrip.h>

#else

#define DSC$K_CLASS_S 1
#define DSC$K_DTYPE_L 8
#define DSC$K_DTYPE_F 10
#define DSC$K_DTYPE_T 14

struct vdb_descrip_v
{
    unsigned short dsc_w_length;
	char dsc_b_dtype;
	char dsc_b_class;
	void* dsc_a_pointer;
};

struct vdb_descrip_s
{
    unsigned short dsc_w_length;
	char dsc_b_dtype;
	char dsc_b_class;
	char* dsc_a_pointer;
};

struct vista_chix 
{ 
    int _dummy; 
};

static int vdbc_channel_index(char* name, vista_chix* chix)
{
    chix->_dummy = 1234;
    return 0; 
}

static int check_hardware_error(vista_chix* chix) 
{
    if (chix->_dummy == 1234)
	{
        return 0; // no errors
	}
	else
	{
        return 1;
	}
}

static int user_initialize_nofins() 
{ 
    return 0; 
}

// READCHAN_SUCCESS on success, 0 on failure
static int read_chan(struct vdb_descrip_s* name, struct vdb_descrip_v* value)
{
    if (value->dsc_b_class != DSC$K_CLASS_S)
	{
	    return 0;
	}
    switch(value->dsc_b_dtype)
	{
	    case DSC$K_DTYPE_L:
			if (value->dsc_w_length != sizeof(long))
			{
				return 0;
			}
		    *((long*)value->dsc_a_pointer) = rand();
			break;

		case DSC$K_DTYPE_F:
			if (value->dsc_w_length != sizeof(float))
			{
				return 0;
			}
		    *((float*)value->dsc_a_pointer) = rand();
//		    *((float*)value->dsc_a_pointer) = 1.0;
			break;

		case DSC$K_DTYPE_T:
		    snprintf((char*)value->dsc_a_pointer, value->dsc_w_length, "%ld", (long)rand());
			break;
			
		default:
			return 0;
	}
	return 1; /* READCHAN_SUCCESS */
}

#endif /* defined(__VMS) && !defined(TESTING) */
}

#include <list>
#include <string>
#include <vector>

#include "asynPortDriver.h"

#define SVAL_SIZE 512

static const char* TS1_INST_LIST = "ALF,ARGUS,CHRONUS,CRISP,EMMA,EMU,ENGINX,EVS,GEM,HIFI,HRPD,INES,IRIS,LOQ,MAPS,MARI,"
                  "MERLIN,MUSR,OSIRIS,PEARL,POLARIS,PRISMA,ROTAX,SANDALS,SURF,SXD,TOSCA,VESUVIO";

static const char* TS2_INST_LIST = "CHIPIR,IMAT,INTER,LARMOR,LET,NIMROD,OFFSPEC,POLREF,SANS2D,WISH,ZOOM";

/// Describes a beam/accelerator parameter in VSystem/VISTA 
class BeamParam
{
private:
	struct vdb_descrip_v lval_dsc;
	struct vdb_descrip_v fval_dsc;
	struct vdb_descrip_v sval_dsc;
	BeamParam(const BeamParam&) { }
	void operator=(const BeamParam&) { }
public:
	std::string param_name;
	std::string type;
	std::string vista_name;
	int update_freq;
	int param_id;
	long lval;
	float fval;
	time_t updtime;   // used by UPDTIME and UPDTIMET, timestamp read from Vsystem
	time_t updtime_old;   // used by UPDTIME and UPDTIMET
	time_t updtimev;  // time value last changed, calculated by actually comparing values read
	static time_t g_updtime; // global timestamp
    static unsigned long error_count;
    static const int READCHAN_SUCCESS;
	bool chan_ok;
	char sval[SVAL_SIZE];
	BeamParam(const char* pn, const char* t, const char* vn, int uf) :
	    param_name(pn), type(t), vista_name(vn), update_freq(uf), param_id(-1), lval(0), 
        fval(0.0), updtime(0), updtime_old(0), updtimev(0), chan_ok(true)
	{
		sval[0] = '\0';
		bool valid_type = (type == "long" || type == "float" || type == "string");
		if (!valid_type)
		{
			throw std::runtime_error("Invalid parameter data type in input file");
		}
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
	}
	
	// true if channel OK, false if in error
	bool getChan(const std::string& vista_name, vdb_descrip_v& value_dsc, time_t& updtime)
	{
		std::string vista_ts = vista_name + "@TIMESTAMP";
	    char tval[30]; // timestamp, which should change whether value does or not
	    struct vdb_descrip_s vista_name_dsc;
	    struct vdb_descrip_s vista_ts_dsc;
	    struct vdb_descrip_v tval_dsc;
		vista_name_dsc.dsc_w_length = vista_name.size();
		vista_name_dsc.dsc_b_dtype = DSC$K_DTYPE_T;
		vista_name_dsc.dsc_b_class =  DSC$K_CLASS_S;
		vista_name_dsc.dsc_a_pointer = const_cast<char*>(vista_name.c_str());
		vista_ts_dsc.dsc_w_length = vista_ts.size();
		vista_ts_dsc.dsc_b_dtype = DSC$K_DTYPE_T;
		vista_ts_dsc.dsc_b_class =  DSC$K_CLASS_S;
		vista_ts_dsc.dsc_a_pointer = const_cast<char*>(vista_ts.c_str());
		tval_dsc.dsc_w_length = sizeof(tval)-1;
		tval_dsc.dsc_b_dtype = DSC$K_DTYPE_T;
		tval_dsc.dsc_b_class =  DSC$K_CLASS_S;
		tval_dsc.dsc_a_pointer = tval;
		vista_chix chix;
		vdbc_channel_index(const_cast<char*>(vista_name.c_str()), &chix);
		if (check_hardware_error(&chix) != 0)
		{
			errlogPrintf("isisbeamDriver:BeamParam:getChan: hardware IO error on channel \"%s\"\n", vista_name.c_str());
		    return false;
		}	
		int read_chan_status = read_chan(&vista_name_dsc, &value_dsc);
		if (read_chan_status != READCHAN_SUCCESS)
		{
			errlogPrintf("isisbeamDriver:BeamParam:getChan: read_chan error %d on reading value for channel \"%s\"\n", read_chan_status, vista_name.c_str());
		    return false;
		}
		memset(tval, 0, sizeof(tval));
		read_chan_status = read_chan(&vista_ts_dsc, &tval_dsc);
		if (read_chan_status != READCHAN_SUCCESS)
		{
			errlogPrintf("isisbeamDriver:BeamParam:getChan: read_chan error %d on reading timestamp for channel \"%s\"\n", read_chan_status, vista_name.c_str());
		    return false;
		}
		tval[sizeof(tval)-1] = '\0';
		VMSToTimeT(updtime, tval);
#if !defined(__VMS) || defined(TESTING)
		time(&updtime);
#endif
		return true;
	}
	
	bool read()
	{
	    time_t now;
		bool non_zero = false;
		updtime_old = updtime;
		chan_ok = true;
		if (type == "float")
		{
		    float fval_old = fval;
			chan_ok = getChan(vista_name, fval_dsc, updtime);
			if (chan_ok)
			{
				if (fval != fval_old)
				{
				    time(&updtimev);
				}
			}
			if (fval != 0.0)
			{
			    non_zero = true;
			}
		}
		else if (type == "long")
		{
		    long lval_old = lval;
			if (param_name == "UPDTIMET")
			{
                updtime = g_updtime;
				lval = updtime;
				if (lval != lval_old)
				{
					time(&updtimev);
				}
			}
			else if (param_name == "ERRCNT")
			{
                updtime = g_updtime;
				lval = error_count;
				if (lval != lval_old)
				{
					time(&updtimev);
				}
			}
			else
			{
			    chan_ok = getChan(vista_name, lval_dsc, updtime);
				if (chan_ok)
				{
					if (lval != lval_old)
					{
						time(&updtimev);
					}
			    }
			}
			if (lval != 0.0)
			{
			    non_zero = true;
			}
		}
		else if (type == "string")
		{
		    char sval_old[SVAL_SIZE];
			memcpy(sval_old, sval, SVAL_SIZE);
			memset(sval, 0, SVAL_SIZE);
			if (param_name == "INSTTS1")
			{
                updtime = g_updtime;
			    strncpy(sval, TS1_INST_LIST, sizeof(sval)); 
			}
			else if (param_name == "INSTTS2")
			{
                updtime = g_updtime;
			    strncpy(sval, TS2_INST_LIST, sizeof(sval)); 
			}
			else if (param_name == "UPDTIME")
			{
                updtime = g_updtime;
				strftime(sval, sizeof(sval), "%Y-%m-%dT%H:%M:%S", localtime(&updtime));
				if (strncmp(sval, sval_old, SVAL_SIZE))
				{
                    time(&updtimev);
				}
			}
			else
			{
				chan_ok = getChan(vista_name, sval_dsc, updtime);
				if (chan_ok)
				{
					if (strncmp(sval, sval_old, SVAL_SIZE))
					{
                        time(&updtimev);
					}
			    }
			}
			sval[sizeof(sval)-1] = '\0';
			if (strlen(sval) > 0)
			{
			    non_zero = true;
			}
		}
		time(&now);
		if (!chan_ok)
		{
			errlogPrintf("isisbeamDriver:BeamParam:read: Error reading channel \"%s\"\n", vista_name.c_str());
            ++error_count;
		}
		else if ( (now - updtime) > 10 ) // vista timestamp should always update
		{
			errlogPrintf("isisbeamDriver:BeamParam:read: channel \"%s\" timestamp not updating\n", vista_name.c_str());
			chan_ok = false;
            ++error_count;
		}
		else if ( (update_freq > 0) && ((now - updtimev) > update_freq) )
		{
			errlogPrintf("isisbeamDriver:BeamParam:read: channel \"%s\" value not updating\n", vista_name.c_str());
			chan_ok = false;
            ++error_count;
		}
		else if ( (update_freq < 0) && non_zero && ((now - updtimev) > (-update_freq)) )
		{
			errlogPrintf("isisbeamDriver:BeamParam:read: channel \"%s\" value not updating and non-zero\n", vista_name.c_str());
			chan_ok = false;
            ++error_count;
		}
		else if (updtime > g_updtime)
		{
		    g_updtime = updtime;
		}
		return chan_ok;
	}
	
	bool update(asynPortDriver* driver) const
	{
        if (!chan_ok)
        {
            return false; // don't update if not read a sensible value
        }
		if (updtime <= updtime_old)
		{
		    return false; // don't update if timestamp not changed
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
		return true;
	}
	
	// 15-APR-2016 13:22:19.619
	void VMSToTimeT(time_t& updtime, const char* tval)
	{
	    static const char* months[] = { "JAN", "FEB", "MAR", "APR", "MAY", "JUN", 
								"JUL", "AUG", "SEP", "OCT", "NOV", "DEC", NULL };
	    char month[4];
	    int year, i, n;
	    struct tm tms;
	    n = sscanf(tval, "%d-%3c-%d %d:%d:%d", &(tms.tm_mday), month, &year, 
				&(tms.tm_hour), &(tms.tm_min), &(tms.tm_sec));   // this ignores possible milliseconds on end
	    if (n != 6)
	    {
		    updtime = 0;
			return;
	    }
	    month[3] = '\0';
	    tms.tm_year = year - 1900;
	    tms.tm_isdst = -1;
	    tms.tm_mon = 0;
	    for(i = 0; months[i] != NULL; i++)
	    {
		    if (!strcmp(months[i], month))
			{
			    tms.tm_mon = i;
			}
	    }
	    updtime = mktime(&tms);
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
#define FIRST_IB_DRIVER_PARAM P_chanErrCnt
	int P_chanErrCnt; // int, number of channels in error
#define LAST_IB_DRIVER_PARAM P_chanErrCnt
	std::vector<asynParamString_t> m_driverParamString;
	double m_pollTime;
	bool m_shutdown;  ///< false initially, set to true to request shutdown, and then wait until it goes false again
	void pollerThread();
};

#define NUM_IB_DRIVER_PARAMS (&LAST_IB_DRIVER_PARAM - &FIRST_IB_DRIVER_PARAM + 1)	

#define P_chanErrCntString  "CHANERRCNT"

#endif /* ISISBEAMDRIVER_H */
