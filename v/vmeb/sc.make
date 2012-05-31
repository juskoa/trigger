# .o in linux_s or linux_c directory:
ifeq ($(findstring linux_, $(notdir $(CURDIR))), linux_)
$(info ===================== running make from $(notdir $(CURDIR)) directory. Host:$(HOSTNAME) $(vmewro))
 ifdef SERVER_LINK
  ifneq ($(findstring linux_s, $(notdir $(CURDIR))), linux_s)
   $(error run make from linux_s directory: make -f ../makefile)
  endif
 else
  ifneq ($(findstring linux_c, $(notdir $(CURDIR))), linux_c)
   $(error run make from linux_c directory: make -f ../makefile)
  endif
 endif
VPATH := ../
else
$(info CURDIR:$(CURDIR))
$(error run make from linux_s or linux_c directory: make -f ../makefile)
endif

