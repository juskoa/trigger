# When started on server, execute in addition also on client in 2 cases:
# 1. make without parameters (i.e. make all on server+client)
# 2. make clean (i.e. make clean on server+client)
ifdef MAKECMDGOALS
GOALS = $(MAKECMDGOALS)
ifeq ($(GOALS),clean)
ifdef SERVER_LINK
#ifneq ($(HOSTNAME), alidcscom707)
MAKE_CLIENT_DIR := $(subst $(SERVER_PREF),,$(shell pwd))
MAKE_CLIENT_CMD := '(. /usr/local/trigger/bin/vmebse.bash $(SERVER_BASEDIR) ; cd $(MAKE_CLIENT_DIR) ; make clean)' 
#endif
endif
endif
else
GOALS = all
# if no goal given and we are on server, make also on client:
ifdef SERVER_LINK
ifneq ($(HOSTNAME), alidcscom707)
MAKE_CLIENT_DIR := $(subst $(SERVER_PREF),,$(shell pwd))
MAKE_CLIENT_CMD := '(. /usr/local/trigger/bin/vmebse.bash $(SERVER_BASEDIR) ; cd $(MAKE_CLIENT_DIR) ; make)' 
endif
endif
endif
$(GOALS):
#	@echo t: $@
	cd $(ODIR) && $(MAKE) -f ../make_new $@
ifdef MAKE_CLIENT_DIR
#	echo $(MAKE_CLIENT_DIR)
	echo
	ssh trigger@$(CLIENT_HOST) $(MAKE_CLIENT_CMD)
endif
#%: force
#	cd $(ODIR) && $(MAKE) -f ../make_new $@
#force: ;

