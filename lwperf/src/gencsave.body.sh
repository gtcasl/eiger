#!/bin/bash
#test args: "sub2" "IP(nx)" "DP(dp)" "IP(n)" "IP(nx)"

site=$1
shift
args=`echo $* | sed -e 's/IR(/ int /g' -e 's/DR(/double /g' -e 's/ID(/ int /g' -e 's/DD(/double /g' -e 's/IN(/ int /g' -e 's/DN(/double /g' -e 's/)/,/g' -e 's/,$//g'`
puts=`echo $* | sed -e 's/IR(\(\w*\))/ log->put(\1);\n/g' -e 's/ID(\(\w*\))/ log->put(\1);\n/g' -e 's/IN(\(\w*\))/ log->put(\1);\n/g' \
 -e 's/DR(\(\w*\))/ log->put(\1);\n/g' -e 's/DD(\(\w*\))/ log->put(\1);\n/g' -e 's/DN(\(\w*\))/ log->put(\1);\n/g'`

echo "void lwperf_save_$site($args) {
 PERFFORMATTER *log = PERF::Log($site, \"$site\");
 $puts
 log->nextrow();
}
"

