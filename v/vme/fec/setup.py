#!/usr/bin/env python
#from distutils.core import setup
from setuptools import setup
setup(name="fec",
      version="1.1",
      description="FEC",
      long_description="""
FEC cmds

news: 
1st version
""",
      author="Luis Perez",
      packages=["fec"],
      package_dir={"fec":"./"},
     )
