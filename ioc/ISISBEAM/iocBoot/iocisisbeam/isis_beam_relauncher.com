$ set verify
$ beam_relauncher = f$parse(";0",f$environment("PROCEDURE"))
$ logfile = $parse("relauncher.log;0",f$environment("PROCEDURE"),,,"SYNTAX_ONLY")
$ submit /q=cilla$fast /restart /name=isisbeam_restart /after="+00:15:00" /log='logfile' 'beam_relauncher'
$ run_isisbeam = f$parse("run_isisbeam_detach.com;0",f$environment("PROCEDURE"))
$ process_name = "ISISBEAM"
$ call GET_PROCESS_ID "''process_name'"
$ if ("''process_id'" .eqs. "")
$ then
$    @'run_isisbeam'
$ endif
$ exit
$!
$ GET_PROCESS_ID: SUBROUTINE
$ context = ""
$ name = "''p1'"
$ x = f$context("PROCESS", context, "PRCNAM", name, "EQL")
$ process_id == f$pid(context)
$ exit
$ endsubroutine
$!
