SET(SRCS actdb.c
detectfile.c 
doItemName.c  
lexan.c 
ranlux.c 
timeroutines.c
vmewrap.c
bakery.c 
dimwrap.c 
environment.c 
loadFPGA.c
shmaccess.c
udplib.c
daqlogbook.c 
dodif32.c 
infolog.c 
prtError.c 
signature.c 
vmesim.c)

SET(HDRS bakery.h
infolog.h
lexan.h
shmaccess.h
udplib.h
vmeblib.h
vmesim.h
vmewrapdefs.h
vmewrap.h
)

MESSAGE(STATUS "zdrojaky su ${SRCS}")

set(INCLUDE_DIRECTORIES
  ${CMAKE_SOURCE_DIR}/vmeb/vmelib
)
include_directories( ${INCLUDE_DIRECTORIES})

set(LINK_DIRECTORIES
  ${CMAKE_SOURCE_DIR}/vmeb/vmelib
) 
link_directories( ${LINK_DIRECTORIES})


add_library(vmeb ${SRCS} ${HDRS})

set(DAQ_LIB /opt/libDAQlogbook/libDAQlogbook.a)
set(INFO_LOGGER_LIB /opt/infoLogger/libInfo.a)

#target_link_libraries( ${DAQ_LIB} ${INFO_LOGGER_LIB})
