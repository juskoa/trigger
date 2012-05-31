# daq:
ifdef DATE_INFOLOGGER_DIR
daqINCDIRS=-I$(DATE_INFOLOGGER_DIR)
daqCCDEFS=
infoLD= -L$(DATE_INFOLOGGER_DIR) -lInfo
daqLD= $(infoLD)
else
daqINCDIRS=
daqCCDEFS=-DNOINFOLOGGER
daqLD=
endif

ifdef DATE_DAQLOGBOOK_DIR
daqINCDIRS += -I$(DATE_DAQLOGBOOK_DIR)
daqCCDEFS +=-DDAQLOGBOOK
daqLD += -L$(DATE_DAQLOGBOOK_DIR) -lDAQlogbook
MYSQLLIBS=`/usr/bin/mysql_config --libs`
endif

ifdef ACT_DB
daqINCDIRS += -I/opt/act
daqCCDEFS +=-DACT_DB
actLD = -L/opt/act -lACT
daqLD += $(actLD)
endif

