#!/usr/bin/env python
#from distutils.core import setup
from setuptools import setup
setup(name="validate",
      version="5.11",
      description="Validate .partition file with CTP config",
      long_description="""
Validate .partition file with CTP config
Environment variables:
dbctp          directory containing CTP configuration files
dbctp_pardefs  directory containing .partition file

Validation:
python /usr/lib/python2.6/site-packages/validate/validate.py partname [strict]

-validates partname.partition file. 
Optional strict parameter forces 'strict validation', i.e.:
- IR_MONITOR/CTP/Luminosity DIM service has to be available in case
  FIXLUM/FIXLOSS/FIXPOWER (automatic downscaling) is used in validated file

return code:
0         -the names of triggering detectors printed to stdout: det1 *det2
8         -error message printed to stdout

Validation of aliases.txt file:
./aliases.exe
return code:
0         -ok, note about number of aliases printed to stdout
8         -error message printed to stdout

news: 
from v4.0: ctpinputs.cfg instead of CTP.SWITCH VALID.CTPINPUTS L0.INPUTS
from v5.0: $dbctp/filter preferred, if not available trgInput_* (ON/OFF) used
from v5.1: 0HWU check removed, i.. TRD cluster allowed with non-0HWU classes
from v5.2: the names of effectively filtered out trig. detectors preceded by '*'
from v5.4: 4x 8-inputs l0f, new INRND1 option in .partition definition
from v5.5: IR1/2 defined by 8 inputs, new PF on LM board               
     v5.6: minor changes (better syntax check of .partition file)
     v5.7: minor change: the max. SDG name length check added (max. is 23)
     v5.8: minor change: check if # of LTUs>0 for each cluster
     v5.9: minor change: added syntax checks (i.e. BCM1=0.5% now error)
     v5.10: aliases.exe (x86_64) 
     v5.11: FIXPOWER N df: now, N can be also float number
""",
      author="Anton Jusko",
      packages=["validate"],
      package_dir={"validate":"./"},
      data_files=[("bin", ["/home/alice/trigger/validate/aliases.exe"])]
     )
#      package_dir={"validate":"./"},
#nebavi:      setup_requires=["tkinter"],
