#!/usr/bin/python
import os,sys,glob,string
partition={'inputs':[],
           'interactions':[],
           'descriptors':[],
           'clusters':[],
           'pfs':[],
           'bcmasks':[],
           'classes':[]
}

def parsePartition(name):
  f= open(name,"r")
  clusterflag=0
  for line in f.readlines():
      if line[0] == "\n": continue
      if line[0] == "#": continue
      ls = string.split(line,' ')
      lss=[]
      for i in ls:
        lss.append(i.strip('\n'))
      print lss 
      if lss[0] == 'BC1':
         partition['inputs'].append('BC1 CTP 0 '+lss[1]+' 0') 
      if lss[0] == 'BC2':
         partition['inputs'].append('BC2 CTP 0 '+lss[1]+' 0') 
      if lss[0] == 'RND1':
         partition['inputs'].append('RND1 CTP 0 '+lss[1]+' 0') 
      if lss[0] == 'RND2':
         partition['inputs'].append('RND2 CTP 0 '+lss[1]+' 0') 
      if lss[0] == 'BCM1':
         partition['inputs'].append('BCM1 '+lss[1]) 
      if lss[0] == 'BCM2':
         partition['inputs'].append('BCM2 '+lss[1]) 
      if lss[0] == 'BCM3':
         partition['inputs'].append('BCM2 '+lss[1]) 
      if lss[0] == 'BCM4':
         partition['inputs'].append('BCM4 '+lss[1]) 
      if lss[0] == 'Clusters:':
         print ' I have cluster:'
         if clusterflag:
          for klass in lss:
             klassplit=klass.split('(')
             print klassplit
         else: clusterflag=1
         
if __name__ == "__main__":
    parsePartition("p33.partition")
    print partition
  
