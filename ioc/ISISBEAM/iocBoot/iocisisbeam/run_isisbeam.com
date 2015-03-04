$!
$! $Id: run_isisbeam.com 58 2014-06-29 23:58:46Z FreddieAkeroyd $
$! $HeadURL: http://svn.isis.rl.ac.uk/freddie_test/trunk/EPICS_VMS/ioc/ISISBEAM/iocBoot/iocisisbeam/run_isisbeam.com $
$!
$ set noon
$ myproc = f$environment("PROCEDURE")
$ myprocdir = f$parse("A.A;1",myproc,,,"SYNTAX_ONLY") - "A.A;1"
$ myroot = "''myprocdir'" - "]" + ".-.-.-.-]"
$ @'myroot'setup
$ ARCH = f$getsyi("ARCH_NAME")
$! we cannot use broadcasts on VMS if we run non-privileged
$! so turn off auto addressing
$ define/nolog EPICS_CA_AUTO_ADDR_LIST "NO"
$! We need to define beacon address as all the possible isis clients machines
$! that might connect to use. We can either set EPICS_CA_ADDR_LIST 
$! or EPICS_CAS_BEACON_ADDR_LIST to achieve this
$! 130.246.39.24 is isis nagios monitoring server, the others are the ISIS beam gateways
$ define/nolog EPICS_CA_ADDR_LIST "130.246.39.24 130.246.39.152 130.246.51.171 130.246.54.107"
$ set def 'myprocdir'
$loop:
$ write sys$output "$Id: run_isisbeam.com 58 2014-06-29 23:58:46Z FreddieAkeroyd $"
$ write sys$output f$time()
$! start a caRepeater
$ spawn/nowait/input=NL:/proc=CAREPEATER -
    run EPICS_ROOT:[BASE.BIN.OpenVMS_'ARCH']caRepeater.exe
$! Finally start real server
$ define/user sys$input sys$command
$ mc [-.-.bin.OpenVMS_'ARCH']isisbeam.exe st.cmd
$ wait 00:00:30
$ goto loop