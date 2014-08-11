# When started on server, execute in addition also on client in 2 cases:
# 1. make without parameters (i.e. make all on server+client)
# 2. make clean (i.e. make clean on server+client)
ifdef MAKECMDGOALS
GOALS = $(MAKECMDGOALS)
ifeq ($(GOALS),clean)
ifdef SERVER_LINK
#ifneq ($(HOSTNAME), alidcscom707)
MAKE_CLIENT_DIR := $(subst $(SERVER_PREF),,$(shell pwd))
#MAKE_CLIENT_CMD := '(. /usr/local/trigger/bin/vmebse.bash $(SERVER_BASEDIR) ; cd $(MAKE_CLIENT_DIR) ; make clean)' 
#MAKE_CLIENT_CMD := '(vmedirs ; cd /usr/local/trigger/devel ; . v/bin/vmebse.bash ; cd $(MAKE_CLIENT_DIR) ; make clean)' 
MAKE_CLIENT_CMD := '(cd $(MAKE_CLIENT_DIR) ; make clean)' 
#endif
endif
endif
else
GOALS = all
# if no goal given and we are on server, make also on client:
ifdef SERVER_LINK
ifneq ($(HOSTNAME), alidcscom707)
MAKE_CLIENT_DIR := $(subst $(SERVER_PREF),,$(shell pwd))
#MAKE_CLIENT_CMD := '(. /usr/local/trigger/bin/vmebse.bash $(SERVER_BASEDIR) ; cd $(MAKE_CLIENT_DIR) ; make)' 
#MAKE_CLIENT_CMD := '(cd $(MAKE_CLIENT_DIR) ; mkdir -p linux; make)' 
MAKE_CLIENT_CMD := '(cd $(MAKE_CLIENT_DIR) ; mkdir -p linux_c; make)' 
endif
endif
endif
$(GOALS):
	@echo lib_stuff.make: $@ $(MAKECMDGOALS) making:$(ODIR) pwd:`pwd`
	@mkdir -p $(ODIR)
	- cd $(ODIR) && $(MAKE) -f ../make_new $@
ifdef MAKE_CLIENT_DIR
#	echo $(MAKE_CLIENT_DIR)
	@echo
#	ssh trigger@$(CLIENT_HOST) $(MAKE_CLIENT_CMD)
	ssh $(CLIENT_HOST) $(MAKE_CLIENT_CMD)         # trigger or run1
endif
#%: force
#	cd $(ODIR) && $(MAKE) -f ../make_new $@
#force: ;

