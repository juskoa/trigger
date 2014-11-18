CC=$(VMEGCC)
#COMMONCFLAGS= -Wall -g -D$(VMEDRIVER)
COMMONCFLAGS= -Wall -Wno-write-strings -g -D$(VMEDRIVER)
ifeq ($(CC),g++)
#CCDEFS +=-DCPLUSPLUS
COMMONCFLAGS +=-DCPLUSPLUS
endif
include $(VMEBDIR)/vmeai.make.$(VMEDRIVER)

SERVER_BASEDIR := 
odl64 := /opt/dip/lib64
HOSTNAME:=$(shell hostname)
ifeq ($(HOSTNAME), alidcscom188)
SERVER_LINK := yes
CLIENT_HOST := alidcsvme008
#SERVER_PREF := /data/dl/root
#SERVER_PREF := /data/dl/root/usr/local/trigger/...
# provided, theres is a link: /usr/local/trigger ->...
SERVER_PREF :=
odl64 := /opt/dip/lib
endif
ifeq ($(HOSTNAME), alidcscom835)
SERVER_LINK := yes
CLIENT_HOST := alidcsvme008
SERVER_PREF := /home/dl6
endif
ifeq ($(HOSTNAME), pcalicebhm10)
SERVER_LINK := yes
CLIENT_HOST := altri2
SERVER_PREF := /home/dl6
endif
ifeq ($(HOSTNAME), avmes)
SERVER_LINK := yes
CLIENT_HOST := altri1
SERVER_PREF := /home/dl6
endif

EXEDIR= ../linux
ifeq ($(HOSTNAME), alidcscom707)
SERVER_LINK := yes
SERVER_PREF :=
SERVER_BASEDIR := v
endif

ifdef SERVER_LINK
ODIR = linux_s
#  all exe on 64 bit machines go to _s:
EXEDIR= ../linux_s
CTPLIB= $(VMECFDIR)/ctp/ctplib/linux_s
LTULIB= $(VMECFDIR)/ltu/ltulib/linux_s
VMEBLIB= $(VMEBDIR)/vmeblib/linux_s
else
ODIR = linux_c
CTPLIB= $(VMECFDIR)/ctp/ctplib/linux_c
LTULIB= $(VMECFDIR)/ltu/ltulib/linux_c
VMEBLIB= $(VMEBDIR)/vmeblib/linux_c
endif

#$(info allEXES:$(allEXES))
# check if target is all or part of allEXES:
#ifeq ($(findstring linux_, $(notdir $(CURDIR))), linux_)

# compile:
SMIinc = $(SMIDIR)/smixx
DIMinc = $(DIMDIR)/dim
CTPinc = $(CTPLIB)/..
VMEBinc = $(VMEBDIR)/vmeblib

# link:
# VMERCCLD is in vmeai.make.VMERCC:
#VMERCCLD := -L/lib/modules/daq -lvme_rcc -lrcc_error -lio_rcc -lcmem_rcc
CTPLD= -L$(CTPLIB) -lctp
LTULD= -L$(LTULIB) -lltu
DIMLD= -L$(DIMDIR)/linux -ldim
SMILD= -L$(SMIDIR)/linux -lsmi
VMEBLD= -L$(VMEBLIB) -lvmeb
LDFLAGS += -lpthread


