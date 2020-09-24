$!
$! $Id: run_isisbeam.com 58 2014-06-29 23:58:46Z FreddieAkeroyd $
$! $HeadURL: http://svn.isis.rl.ac.uk/freddie_test/trunk/EPICS_VMS/ioc/ISISBEAM/iocBoot/iocisisbeam/run_isisbeam.com $
$!
$ set noon
$ set proc/parse=extend
$ @con_setup:perl
$ IF f$getsyi("ARCH_TYPE") .EQ. 2
$ THEN
$       @SHARE$DISK:[PSX$ROOT_ALPHA.LIB]GNV_SETUP.COM
$ ELSE
$       @SHARE$DISK:[PSX$ROOT.LIB]GNV_SETUP.COM
$ ENDIF
$ @con_com:con_programmer
$ myproc = f$environment("PROCEDURE")
$ myprocdir = f$parse("A.A;1",myproc,,,"SYNTAX_ONLY") - "A.A;1"
$ myroot = "''myprocdir'" - "]" + ".-.-.-.-]"
$ @'myroot'setup
$ ARCH = f$getsyi("ARCH_NAME")
$ run/detach/out='myprocdir'isisbeam.log/err='myprocdir'isisbeam.log -
    /inp='myprocdir'run_isisbeam.com/proc=isisbeam sys$system:loginout.exe
$!
