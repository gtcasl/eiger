#!/bin/bash
#test args: 4 "sub2" "IP(nx)" "DP(dp)" "IP(n)" "IP(nx)"
site=$1
shift
args=`echo $* | sed -e 's/IR(//g' -e 's/DR(//g' -e 's/ID(//g' -e 's/DD(//g' -e 's/IN(//g' -e 's/DN(//g' -e 's/)/,/g' -e 's/,$//g'`
intents=`echo $* | sed -e 's/IR(\(\w*\))/  INTEGER(c_int),  value, INTENT(IN) :: \1\n/g' -e 's/ID(\(\w*\))/  INTEGER(c_int),  value, INTENT(IN) :: \1\n/g' -e 's/IN(\(\w*\))/  INTEGER(c_int),  value, INTENT(IN) :: \1\n/g' \
 -e 's/DR(\(\w*\))/  real(c_double), value, INTENT(IN) :: \1\n/g' -e 's/DD(\(\w*\))/  real(c_double), value, INTENT(IN) :: \1\n/g' -e 's/DN(\(\w*\))/  real(c_double), value, INTENT(IN) :: \1\n/g'`
echo " subroutine lwperf_save_$site($args) bind(C,name=\"lwperf_save_$site\")
 use iso_c_binding
$intents
 end subroutine
"

