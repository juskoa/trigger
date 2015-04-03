#!/usr/bin/python
import os,smtplib,subprocess
def gmonscal():
  out=subprocess.call("gmonscal.sh")
  print out
def sendmail(subject,text):
  SENDMAIL = "/usr/sbin/sendmail" # sendmail location
  p = os.popen("%s -t" % SENDMAIL, "w")
  p.write("To: rl@hep.ph.bham.ac.uk\n")
  p.write("From:lietava\n")
  p.write("Subject:"+subject+"\n")
  p.write("\n") # blank line separating headers from body
  p.write(text+"\n")
  sts = p.close()
  if sts != 0:
   print "Sendmail exit status", sts
  return
def main():
  #sendmail("aaa","ddd")
  gmonscal()
if __name__=="__main__":
   main()
