#!/usr/bin/python
class Slmdefs:
  version=1
  wordseq=8
  L1cls= (3,0,50)   # (word, start bit,length)
  L1cls2= 6   # the last 2 L1-classes left-shift in word 0
  CITflg= (0, 0x4100, 4, 0x1800)   # (L1word,bits, L2word, bits)
  L2cls= (7,3,50)
  Clust= (4,5,6, 0x3f)   # word, start bit, length, mask
  rocsh= 10
  esr2= (4, 0x2000)
  l2swc= (4, 0x800)
  spare23w= (4, 0x4000, 0x2000)
class Slmdefs_run2a:
  # obsolete (replaced in jan2014 by Slmdefs_run2.
  version=2
  wordseq=16
  L1cls= (7,14,100)  
  L1cls2= 6   
  CITflg= (0, 0x4100, 8, 0x1800)
  L2cls= (15,15,100)
  Clust= (8,3,8, 0xff)
  rocsh= 10
  esr2= (8, 0x2000)
  l2swc= (8, 0x800)
  spare23w= (8, 0x4000, 0x2000)
class Slmdefs_run2:
  # the same as version=2, but L2Cluster,L2class bits+esr,CIT
  # L2SwC are placed
  # in words 0..7, bits 31..16
  version=3
  wordseq=8
  L1cls= (7,14,100)  
  L1cls2= 6   
  CITflg= (0, 0x4100, 0, 0x18000000)
  L2cls= (7,31,100)
  Clust= (0,19,8, 0xff)
  rocsh= 10
  esr2= (0, 0x20000000)
  l2swc= (0, 0x8000000)
  spare23w= (0, 0x40000000, 0x20000000)
