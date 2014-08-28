#!/usr/bin/env python
#from distutils.core import setup
from setuptools import setup
setup(name="validate",
      version="3.0",
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
0         -the names of triggering detectors printed to stdout
8         -error message printed to stdout

news: 
from version 3.0: 100 classes
""",
      author="Anton Jusko",
      packages=["validate"],
      package_dir={"validate":"./"},
     )
#      package_dir={"validate":"./"},
#nebavi:      setup_requires=["tkinter"],
