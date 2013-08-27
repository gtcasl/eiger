#!/bin/bash
touch test.model

../eiger/Eiger.py \
--database-name=temp \
--database-user=root \
--database-password=root \
--database-location=localhost \
--rotate-application-pcs \
--clusters=1 \
--show-prediction-statistics \
--regression-type=linear \
--threshold=0.000001 \
--metric-ids 8 11 12 13 14 \
--training-datacollection-id=1 \
--experiment-datacollection-id=1 \
--output=test.model #&> /dev/null

#if diff gold.model test.model &>/dev/null ; then
if diff gold.model test.model ; then
	echo "Pass!"
else
	echo "Fail!"
fi

rm test.model &> /dev/null

