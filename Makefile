SUBDIRS=base extensions support ioc

.PHONY : $(SUBDIRS) subdirs clean install all MakefileInclude

subdirs : $(SUBDIRS)

$(SUBDIRS) :
	$(MAKE) -C $@ $(MAKECMDGOALS)

extensions : base
support : extensions
ioc : support

clean : $(SUBDIRS)
all : $(SUBDIRS)
install : $(SUBDIRS)
uninstall : $(SUBDIRS)
MakefileInclude : $(SUBDIRS)
