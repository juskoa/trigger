#!/usr/bin/python
class Slmdefs:
  wordseq=8
  L1cls= (3,0,50)   # (word, start bit,length)
  L1cls2= 6   # the last 2 classes left-shift in word 0
  CITflg= (0, 0x4100, 4, 0x1800)   # (L1word,bits, L2word, bits)
  L2cls= (7,3,50)
  Clust= (4,5,6, 0x3f)   # word, start bit, length, mask
  rocsh= 10
  esr2= (4, 0x2000)
  l2swc= (4, 0x800)
  spare23w= 4
class Slmdefs_run2:
  wordseq=16
  L1cls= (7,14,100)  
  L1cls2= 6   
  CITflg= (0, 0x4100, 8, 0x1800)
  L2cls= (15,15,100)
  Clust= (8,3,8, 0xff)
  rocsh= 10
  esr2= (8, 0x2000)
  l2swc= (8, 0x800)
  spare23w= 8
