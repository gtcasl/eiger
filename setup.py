#!/usr/bin/env python

from distutils.core import setup

setup(name='Eiger',
	  version='1.0',
	  description='Eiger Performance Modeling',
	  author='Eric Anger',
	  author_email='eanger@gatech.edu',
	  packages=['eiger'],
	  scripts=['eiger/Eiger.py']
	 )

