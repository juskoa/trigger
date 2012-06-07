Contents:
---------
List of files, detector names
Config files format
Software
How to setup parted
History

See also: 
DOC/rundemo*    -for latest versions of .rcfg file
ctp_proxy/readme -description of .pcfg file

List of files, detector names
------------------------------
input files:
DB/VALID.LTUS       -LTUs wireing: fo, busy, I2C connections
   VALID.CTPINPUTS  -CTP inputs connections, symbolic 
                     names, signature info, connections
                    -aliases for possible l0fun1/2, combined
                     from names in this file
   VALID.PFS        -all possible PF protection circuit definitions,
                     symbolic name assigned to each
   VALID.DESCRIPTORS-all recommended trigger descriptors. 

in/out file:  $VMECFDIR/CFG/ctp/pardefs/*.partition   
output files: $VMECFDIR/CFG/ctp/pardefs/*.pcfg
              $VMEWORKDIR/WORK/RCFG/rN.rcfg   -see DOC/

parted.py   partition editor. Called:
 -interactively when creating/modifying .partition file 
 -from pydim server when creating .pcfg file (savepcfg)
 -from pydim server when creating .rcfg file (savercfg)
           
scanrcfg.py scans set of .rcfg files
ctp.py      loader   (not used -CTP is loaded from ctp_proxy)

Detector names:
We use DAQ names:
spd sdd ssd tpc trd tof phos cpv hmpid fmd pmd v0 t0 muon_trk muon_trg
zdc emcal daq acorde

DCS recommedation:
spd sdd ssd tpc trd tof phs cpv hmp fmd pmd v00 t00 mch mtr
zdc emc tri ...
 
Config files format
-------------------
VALID.LTUS   -the names of all the ALICE detectors. Format:
# detname=DAQdet fo focon bsyinp ltubase i2cchan i2cbran
# detname -as used by ECS/DAQ
# DAQdet  -ECS/DAQ detector number (Franco's table): 0-23
# fo      -FO number: 1-6       0 (or not present):LTU not connected
# focon   -FO connector number: 1-4 (upper one is 1)
# bsyinp  -BUSY input: 1-24 (very bottom one is 24)
#          0 (or not present):busy not connected
# ltubase -LTU base if in the CTP crate or LTUDIMserver name
#          0 (or not present): LTU doesn't exist/not connected
# i2cchan -i2c channel 0-7.  N (or not present): not connected
# i2cbran -i2c branch 0-7. (valid if i2cchan is 0-7)
              
VALID.CTPINPUTS
Consists of a single line for each CTP input.
# InName = Symbolic name of CTP input (Unique)
# Det = Detector Name as used by DAQ
# Level = 0,1,2
# Signature = input signature 1-119  (Unique)
# Inpnum =1-24 for L0,L1; 1-12 for L2
# Dimnum = 1-N N=number of CABLES from triggering detetctor to CTP
#          This is the order number as used in communication with TIN proxy
#           (i.e. in status word, in setting state)
# Configured = 0-1;  1 - signature is found on this cable
#                     (i.e. this cable is used for this input)

   2 'L0 special functions' (according to TB approval 
from 12. July 2005) can be defined and used by any class as L0 input. 
The are defined as lookup table or logical expression.
Their names start with l0f.
An example: l0fvt= V0mb | T0
The names of 4 logical variables used in logical expression defining l0f* 
have to be defined and connected as 0.1-0.4 CTP inputs 
in earlier lines of VALID.CTPINPUTS file.

TRIGGER.PFS   
The symbolic names for possible settings of P/F protection circuits:

pf1 tha1 tha2 thb1 thb2 resolution interval
pf2 ...

TRIGGER.DESCRIPTORS defines all the  default trigger descriptors 
available for Trigger classes definition.
1 line describes 1 trigger descriptor.
First identifier in the line is the Trigger descriptor name 
followed by list of CTP inputs. The list of CTP inputs is composed from:
- identifiers defined in VALID.CTPINPUTS file 
- identifiers l0f* (defined at the end of VALID.CTPINPUTS file).

   The use of the CTP input not defined in VALID.CTPINPUTS file is illegal).
Inverted inputs (i.e. active in case the input signal is not present) 
are marked by '*' in front of the name (for inverted inputs, classes 45-50
will be assigned during CTP configuration).

For example trigger descriptors with the names MB, SC, CE are
described by the following lines in TRIGGER.DESCRIPTORS file:

MB T0 V0mb TRDpre ZDC1_l1 l0fvt
SC T0 V0sc TRDpre ZDC2_l1 l0fvt l0f1
CE T0 V0ce *TRDpre ZDC3_l1

.pcfg file:
similar to .cfg file, but not the same. Printed together with .partition file.
See readme in v/vme/ctp_proxy directory.

Software
--------
Scripts:
parted.py -partition editor
scanrcfg.py -scans set of .rcfg files
ctp.py    -CTP loader (obsolete)

How to setup parted
-------------------

export TRIGGER_DIR="<git-trigger>"
# export TRIGGER_DIR="/tmp/trigger"
export VMEBDIR=$TRIGGER_DIR/v/vmeb
export VMECFDIR=$TRIGGER_DIR/v/vme
export dbctp=$TRIGGER_DIR/v/vme/CFG/ctp/DB
export PYTHONPATH="$VMEBDIR"

$VMECFDIR/TRG_DBED/parted.py example

History
-------
11.5.2012
parted.py modifications started using github.
Goal: lean git. Modifications in git version (not in vd on pcalicebhm10):
    1. unknown cluster name: only warning 'Strange cluster name' on stdout
    2. bug 'not filtering classes using inverted inputs' fixed

see git log
started work on: simpler class def. syntax:
1. reading .partition
OK
2. writing .partition .pcfg .rcfg
seems OK:
7.6.2012:
a -ok
a1 a saved as. ok
a2 a1 saved as. ok (i.e. a1=a2 .partition and .pcfg)

3. editing class in parted
goals:
class name:
- if empty:
   -show built-name class name in class button  OK
   -show empty class field in 'Class' widget    OK
  else:
   -show class name in class button  OK
   -show class name field in 'Class' widget    OK

