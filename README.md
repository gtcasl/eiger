# Eiger Performance Modeling Framework

## Overview
This document gives a brief overview into the stucture and installation of the Eiger 
performance modeling framework. More details can be found in the documentation folder.

## Folder Layout
- **./api** - The Eiger C++ API
- **./database**    - sqlite3 schema for Eiger database
- **./documentation**   - Documentation describing the implementation of Eiger
- **./eiger**   - Eiger performance modeling framework
- **./examples**    - Examples of basic modeling functionality
- **./tests**   - Simple tests ensuring correct installation of Eiger 

Please review the requirements specification in the ./documentation folder for all
required software packages.

## Usage
All metric collection, model generation, polling, serialization, and reporting
functionality can be installed with the following command:

```bash
python setup.py install
```

They can be controlled by the
`Eiger.py` script; more information can be found by entering the following 
command after installing:

```bash
Eiger.py -h
```

As well, the Eiger module can be imported and used to issue any of the same
commands used by the command line script. Examples of basic modeling functions
can be found in the ./examples folder.

## Database Setup
The Eiger modeling framework relies upon a sqlite3 relational database to manage
metric storage, model construction, and model serialization. All data used by
sqlite3 is contained in a single database file. If a filename is provided which
has not been used for storing Eiger data, it will be converted into one.

## API Installation
The API is built using the standard Autotools workflow:
```bash
    ./bootstrap.sh
    ./configure
    make
    make install
```
Run `./configure --help` for more information.

## Example Data
Several sets of example data reside in the ./examples subdirectory. This data was used 
for different publications and may or may not retain their functionality as Eiger
is developed.

## `lwperf`
Lwperf is a lightweight C/C++/Fortran profiling tool that allows for dumping of
output files either to CSV, Eiger database, or serialization format for later
injection into an Eiger database. Please see the documentation in the lwperf
project for more information.

