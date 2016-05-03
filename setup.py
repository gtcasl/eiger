#!/usr/bin/env python

from setuptools import setup

setup(name='Eiger',
	  version='3.0',
	  description='Eiger Performance Modeling',
          url='https://github.com/gtcasl/eiger',
	  author='Eric Anger',
	  author_email='eanger@gatech.edu',
	  packages=['eiger'],
	  scripts=['Eiger.py'],
          install_requires=[
              'sklearn',
              'scipy',
              'matplotlib',
              'tabulate',
          ],
          zip_safe=False
	 )

