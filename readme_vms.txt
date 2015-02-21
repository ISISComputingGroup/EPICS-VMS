Please see [.base.documentation]README.VMS for original notes and requirements

Basically you need to be using an ODS5 filesystem with GNV and perl installed

    unpack kit (using GNV tar/zip, not vmstar or VMS zip as these will replace . with _ in some extracted path names)
    @setup
    bash
    make MakefileInclude
    make

If you get an error like:

    Failure spawning child process; VMS condition code 28
    Program terminated prior to normal completion

Try increasing your VMS account subprocess quota - I think make generates a 
subprocess per level of the build tree. A quick workaround is to execute
separate makes in the subdirectories themselves. Error code 28 is a general
"process quota exceeded" error, subprocess count is the most likely cause, but
it could be one of the other possible process quotas.

On VMS creating a broadcast socket is a privileged operation - the 
included EPICS BASE has been modified (online_notify.c) to make this a 
warning rather than a fatal error, so you can run an IOC from a non-privileged 
account but you will be unable to use broadcasts for beacons or PV name lookups 
so will need to set EPICS_CA_AUTO_ADDR_LIST=NO

Main changes to original SLAC epics-vms distribution are:

* back merged to full EPICS 3.14.8.2 so it has other files not used on VMS
* made setup.com more generic
* added asyn and devIocStats modules to support and msi to extensions
* added -fpermissive to CONFIG.gnuCommon to allow building on Linux
* commented out SHRLIB_VERSION use from RULES_BUILD as it caused problems
* removed LIB_PREFIX from CONFIG.Common.OpenVMSCommon
* needed to change private to protected inheritance in ioBlocked.h
* explicitly set MAKE and SHELL environment variables
* added note on use of BUILD_PREFIX to support/README_VMS.txt
* Back ported fix for EPICS time epoch (only needed in UK) 

Freddie Akeroyd (freddie.akeroyd@stfc.ac.uk)

