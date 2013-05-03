#!/bin/bash
#test args: 4 "sub2" "IP(nx)" "DP(dp)" "IP(n)" "IP(nx)"
site=$1
shift
args=`echo $* | sed -e 's/IP(//g' -e 's/DP(//g' -e 's/)/,/g' -e 's/,$//g'`
intents=`echo $* | sed -e 's/IP(\(\w*\))/  INTEGER(c_int),  value, INTENT(IN) :: \1\n/g' \
 -e 's/DP(\(\w*\))/  real(c_double), value, INTENT(IN) :: \1\n/g'`
echo " subroutine lwperf_save_$site($args) bind(C,name=\"lwperf_save_$site\")
 use iso_c_binding
$intents
 end subroutine
"

