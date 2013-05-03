#!/bin/bash
#test args: "sub2" "IP(nx)" "DP(dp)" "IP(n)" "IP(nx)"

site=$1
shift
args=`echo $* | sed -e 's/IP(/ int /g' -e 's/DP(/double /g' -e 's/)/,/g' -e 's/,$//g'`
puts=`echo $* | sed -e 's/IP(\(\w*\))/ log->put(\1);\n/g' \
 -e 's/DP(\(\w*\))/ log->put(\1);\n/g'`

echo "void lwperf_save_$site($args) {
 PERFFORMATTER *log = PERF::Log($site, \"$site\",_USE_LS);
 $puts
 log->nextrow();
}
"

