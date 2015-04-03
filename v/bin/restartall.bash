echo "not prepared yet..."
exit
#miclock
startClients.bash ctpproxy stop
startClients.bash gcalib stop
startClients.bash pydim stop
startClients.bash rrd stop
startClients.bash ctpdim stop
startClients.bash ttcmidim stop
startClients.bash gmonscal stop
startClients.bash monscal stop
#startClients.bash html stop  zdochnuty aj tak
#startClients.bash irdim stop
startClients.bash xcounters stop
startClients.bash udpmon stop
#startClients.bash diprfrx stop

# start: in this order tested in lab 3.9.2013
startClients.bash pydim start
startClients.bash ttcmidim start
startClients.bash ctpproxy start
startClients.bash ctpdim start
startClients.bash rrd start
# must be after rrd:
startClients.bash html start
# available only in the pit:
startClients.bash xcounters start
startClients.bash gmonscal start
startClients.bash monscal start
#startClients.bash irdim start
#startClients.bash diprfrx start
# end of daemons availale only in the pit
startClients.bash gcalib start
startClients.bash udpmon start
#fanio
#sctelServer.bash
