Files:
ctpbusy.py
static-files/bsyhelp.html
----
ctpwsgi.py
fileserver.py
# To run this example:
python ctpwsgi.py
Hit this url with your browser:
http://hostname:8080/

ctpwsgi.py: is controlled through startClients.bash, also monitored.

------- 14.4.2014
Add miclockgui. .html:
         status        cmd
source   beam1/local  change

reloading page:
- sync: on change
- async: when status changed (websockets ?)
http://www.html5rocks.com/en/tutorials/websockets/basics/
http://www.tutorialspoint.com/html5/html5_websocket.htm
see ~aj/h/ws

firewall:
-A INPUT -m state --state NEW -m tcp -p tcp --dport 8080 -j ACCEPT

