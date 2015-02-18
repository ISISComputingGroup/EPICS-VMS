/*************************************************************************\
* Copyright (c) 2002 The University of Chicago, as Operator of Argonne
*     National Laboratory.
* Copyright (c) 2002 The Regents of the University of California, as
*     Operator of Los Alamos National Laboratory.
* EPICS BASE Versions 3.13.7
* and higher are distributed subject to a Software License Agreement found
* in file LICENSE that is included with this distribution. 
\*************************************************************************/


#ifndef osiWireFormat
#define osiWireFormat

#include <cvt$routines.h>
#include <cvtdef.h>
#include <inet.h>

#include "epicsTypes.h"


inline void osiConvertToWireFormat ( const epicsFloat32 &value, unsigned char *pWire )
{
#if __IEEE_FLOAT
    union {
	epicsUInt32 utmp;
	epicsFloat32 ftmp;
    } wireFloat32;
    wireFloat32.ftmp = value;
    pWire[0] = static_cast < epicsUInt8 > ( wireFloat32.utmp >> 24u );
    pWire[1] = static_cast < epicsUInt8 > ( wireFloat32.utmp >> 16u );
    pWire[2] = static_cast < epicsUInt8 > ( wireFloat32.utmp >> 8u );
    pWire[3] = static_cast < epicsUInt8 > ( wireFloat32.utmp >> 0u );
#else
    cvt$convert_float ( &value, CVT$K_VAX_F , pWire, CVT$K_IEEE_S, CVT$M_BIG_ENDIAN );
#endif
}

inline void osiConvertToWireFormat ( const epicsFloat64 &value, unsigned char *pWire )
{
#if __IEEE_FLOAT
    union {
        epicsUInt8 btmp[8];
        epicsFloat64 ftmp;
    } wireFloat64;
    wireFloat64.ftmp = value;
        pWire[0] = wireFloat64.btmp[7];
        pWire[1] = wireFloat64.btmp[6];
        pWire[2] = wireFloat64.btmp[5];
        pWire[3] = wireFloat64.btmp[4];
        pWire[4] = wireFloat64.btmp[3];
        pWire[5] = wireFloat64.btmp[2];
        pWire[6] = wireFloat64.btmp[1];
        pWire[7] = wireFloat64.btmp[0];
#else
#   if defined ( __G_FLOAT ) && ( __G_FLOAT == 1 )
        cvt$convert_float ( &value, CVT$K_VAX_G , pWire, CVT$K_IEEE_T, CVT$M_BIG_ENDIAN );
#   else
        cvt$convert_float ( &value, CVT$K_VAX_D , pWire, CVT$K_IEEE_T, CVT$M_BIG_ENDIAN );
#   endif
#endif
}

inline void osiConvertFromWireFormat ( epicsFloat32 &value, epicsUInt8 *pWire )
{
#if __IEEE_FLOAT
    value = *(epicsFloat32*) pWire;
#else
    cvt$convert_float ( &value, CVT$K_IEEE_S, pWire, CVT$K_VAX_F, CVT$M_BIG_ENDIAN );
#endif
}

inline void osiConvertFromWireFormat ( epicsFloat64 &value, epicsUInt8 *pWire )
{
#if __IEEE_FLOAT
    value = *(epicsFloat64*) pWire;
#else
#   if defined ( __G_FLOAT ) && ( __G_FLOAT == 1 )
        cvt$convert_float ( pWire, CVT$K_IEEE_T, &value, CVT$K_VAX_G, CVT$M_BIG_ENDIAN );
#   else
        cvt$convert_float ( pWire, CVT$K_IEEE_T, &value, CVT$K_VAX_D, CVT$M_BIG_ENDIAN );
#   endif
#endif
}

inline epicsUInt16 epicsHTON16 ( epicsUInt16 in )
{
#if 0
    unsigned tmp = in; // avoid the usual unary conversions
	register union {
		epicsUInt8 bytes[2];
		epicsUInt16 word;
	} result;

	result.bytes[0] = static_cast <epicsUInt8> ( tmp >> 8 );
	result.bytes[1] = static_cast <epicsUInt8> ( tmp );

	return result.word;
#endif
    return htons( in );
}

inline epicsUInt16 epicsNTOH16 ( epicsUInt16 in )
{
#if 0
	return epicsHTON16 ( in );
#endif
        return ntohs( in );
}

inline epicsUInt32 epicsHTON32 ( epicsUInt32 in )
{
#if 0
    unsigned tmp = in; // avoid the usual unary conversions
	register union {
		epicsUInt8 bytes[4];
		epicsUInt32 longWord;
	} result;

	result.bytes[0] = static_cast <epicsUInt8> ( tmp >> 24 );
	result.bytes[1] = static_cast <epicsUInt8> ( tmp >> 16 );
	result.bytes[2] = static_cast <epicsUInt8> ( tmp >> 8 );
	result.bytes[3] = static_cast <epicsUInt8> ( tmp >> 0 );

	return result.longWord;
#endif
   return htonl( in );
}

inline epicsUInt32 epicsNTOH32 ( epicsUInt32 in )
{
#if 0
	return epicsHTON32 ( in );
#endif
        return ntohl( in );
}

#endif // osiWireFormat
