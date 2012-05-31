partition manager (= ctp_proxy.c)

GENERAL:
The goal is to:
- add partitition to hardware
- remove partition from hardware
- pause
- resume.

The partition is defined in .pcfg file, similar BUT NOT THE SAME to
cfg file in WORK directory.
This file is input to partition manager.
I call this 'partition cfg file'.
The code for this is in cfg2part.c
This can be changed later for reading database or output from parted
or anything.

The hardware configuration is described as cfg file,
but the INTERPRETATION of items in file is DIFFERENT.
This file is output of partition manager.
I call this file 'hardware cfg file'.

In the code the hardware and partitions are described
as different structures ! 

My interpretation of .pcfg as partition definition is following:

CLASSES:
Classes are in CLA.xx lines of .pcfg file. 
Classes with bcmask==0 define partition classes.
Let call this classes allocated classes.
All other classes in .pcfg file are ignored. 
CLA.01 0xefffffff 0x0 0xfff1 0x0 0x1fffffff 0x0 0x1f000fff
    cl# (skipped)
       l0inputs
                  l0inverted
                      l0vetos   -bit17 (0x10000) is CLASS MASK (1-class not val)
                             l0scaler
                                 l1def
                                            l1inverted
                                                l2def
CLUSTERS and DETECTORS:
Clusters in allocated classes define allocated partition clusters.
Detectors of partition are describes in Fan Out FO line of .pcfg file.
FO words in FO line gives us detector - cluster relation.
Every first word in FO line represents clusters of 4 detectors 
(8 bits per detector as described in cluster.doc CLUSTER word). 
FO.1: 3210     detectors 3-0
FO.2: 7654     detectors 7-4
.
FO.6: 23222120 detectors 23-20
These numbers are inrepreted as numeraical names of detectors.
Table between numerical names and real names already exist
- given by Franco - see appendix 1. 
Example of detectors to cluster association: 
FO.1: 0x109 detectors 0 belongs to clusters 1,4
            detector  1 belongs to clusters 1
FO.2: 0x030000: detector 6 belong to cluster 1,2 

To make hardware configuration or 'hardware cfg file'
we need the correspodence between hardware fo connectors
and detectors = numeric detector names.
This is done at the moment in Detector2Connector() routine in ctp_proxy.c.
(Note that to specify the configuration you may need to edit partition
 cfg file by hand.)

 
SHARED RESOURCES:
Shared resources are described in .pcfg file in RBIF,INTSEL,BCMASK,PFL lines.
Almost nothing is implemented in code.
The idea is that loop over classes gives you the allocated shared resources.
You save them to partition structure- you need to ditinquish 0 from not allocated!


CODE STRUCTURES (in c language sense):
Two main structures are defined:
- Tpartition
- HardWare

Tpartition describes partition.
The pointers to partitions which are active 
(i.e. somehow added together and loaded in hardware)
are kept in array Partitions[0..5].

HardWare strucutre describes the unification of all partition.
There is only one instance of it.

The more description is in Tpartition.h,Tpartition.c
 
START PARTITION ALGORITHM:
 *Read description of new partition from 'partition .pcfg file'
 *Check conflict of resources
 *If conflict return loading_failure
  else if any other error return error
  else continue
 *add partition to Partiitons[]
 *generate start of data
 *merge all Partitions[] to HardWare HW
 *load hardware

STOP PARTITION ALGORITHM
 *get name of partition to stop
 *generate end of data
 *remove it from Partitions[]
 *merge Partitions[] to HardWare HW
 *load hardware

PAUSE PARTITION
 *mask DAQBUSY for clusters in that partiton

RESUME PARTITION
 *unmask DAQBUSY for clusters in that partition
 


Appendix 1.
Correspondence between numerical detector names and real detector names.
Table given by Franco Carena.
Num name   Name     Code
0          SPD      SPD
1          SDD      SDD
2          SSD      SSD
3          TPC      TPC
4          TRD      TRD
5          TOF      TOF
6         PHOS      PHS
7          CPV      CPV
8        HMPID      HMP
9     MUON_TRK      MCH
10    MUON_TRG      MTR
11         PMD      PMD
12         TRG      TRG
13         FMD      FMD
14          T0      T00
15          V0      V00
16         ZDC      ZDC
17      ACORDE      ACO
18       EMCal      EMC
19         HLT      HLT
99     GENERIC      ABC

