#!/usr/bin/env python
from distutils.core import setup
setup(name="validate",
      version="2.2",
      description="Validate .partition file with CTP config",
      long_description="""
Validate .partition file with CTP config
Environment variables:
dbctp          directory containing CTP configuration files
dbctp_pardefs  directory containing .partition file

Validation:
python /usr/lib/python2.3/site-packages/validate/validate.py partname

-validates partname.partition file

return code:
0         -the names of triggering detectors printed to stdout
8         -error message printed to stdout
""",
      author="Anton Jusko",
      packages=["validate"],
      package_dir={"validate":"./"},
     )
#      package_dir={"validate":"./"},
