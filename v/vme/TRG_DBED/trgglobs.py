#!/usr/bin/python

NCLS= 100          # 50: run1   100:run2
# FPGAVERSION env. variable has priority over L0VER set here (validate)
#             FPGAVERSION is taken from ctp.h sent to pydimserver
#             using command 'rcfgdel ALL version' to be set there
# L0VER. >= 0xc0 LM0 board
#L0VER=0xc605 # old L0FUN1/2 (4 inputs)
#L0VER=0xc606 # new L0FUN1/2/3/4 (8 inputs)
#      0xc60a   pf (spd-like)
L0VER=0xc707  # DDL2 (from c705)
