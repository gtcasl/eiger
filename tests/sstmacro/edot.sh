#!/bin/bash

Eiger.py --database-name ben --database-user root --database-password root --database-location localhost -t 1 -e 1 --show-prediction-statistics --plot-performance-scatter \
		--regression-type linear


Eiger.py --database-name ben --database-user root --database-password root --database-location localhost -t 1 -e 1 --show-prediction-statistics --plot-performance-scatter \
		--regression-type nearest --nearest-neighbors 5 
