g++ \
/h/aj/git/trigger/v/vmeb/cmdbase.o ctp_cf.o ctp.o intint.o toobusy.o ssmbrowser.o ssm.o \
-Lctplib/linux_c -lctp \
-L/h/aj/git/trigger/v/vmeb/vmeblib/linux_c -lvmeb -lpthread -lm \
-L/opt/dim/linux -ldim \
-o ctp.exe
