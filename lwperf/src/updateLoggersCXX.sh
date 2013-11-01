#!/bin/sh 
cfiles=`echo "$*" | tr ' ' '\n' |sed '
s/perf.cpp//g
s/csvbackend.cpp//g
s/eigerbackend.cpp//g
s/\n/ /g
s/  / /g'`
# -e 's/cperf.cpp//g' -e 's/csvformatter.cpp//g' -e 's/diffrusage.cpp//g' -e 's/eperf.cpp//g' -e 's/eigerformatter.cpp//g'`
echo "Finding CXX profiling sites by scanning $cfiles"
ctagline="/// generated from $cfiles "
echo "   updating InitFuncs.h"
grep -h PERFLOG $cfiles | \
sed -e 's/[ \t]//g' \
 -e 's/.*PERFLOG\(\|KEEP\)(\(\w*\)[ ,]*/\tvoid init\2(formatter<PERFBACKEND> *c){/g' \
 -e 's/DR(\([A-Za-z0-9._]*\)),*/ c->addCol("\1",RESULT);/g'  \
 -e 's/DD(\([A-Za-z0-9._]*\)),*/ c->addCol("\1",DETERMINISTIC);/g' \
 -e 's/DN(\([A-Za-z0-9._]*\)),*/ c->addCol("\1",NONDETERMINISTIC);/g' \
 -e 's/IR(\([A-Za-z0-9._]*\)),*/ c->addCol("\1",RESULT);/g' \
 -e 's/ID(\([A-Za-z0-9._]*\)),*/ c->addCol("\1",DETERMINISTIC);/g' \
 -e 's/IN(\([A-Za-z0-9._]*\)),*/ c->addCol("\1",NONDETERMINISTIC);/g' \
 -e 's/);$/ }/' > InitFuncs.h
echo $ctagline >> InitFuncs.h

echo "   updating InitSwitchElements.h"
grep -h PERFLOG $cfiles |sed -e 's/.*PERFLOG\(\|KEEP\)(\(\w*\).*/\t\t\tcase lwperf_\2: init\2(ncf); break;/g' > InitSwitchElements.h
echo $ctagline >> InitSwitchElements.h

echo "   updating LocationElements.h"
grep -h PERFLOG $cfiles |sed -e 's/.*PERFLOG\(\|KEEP\)(\(\w*\).*/lwperf_\2 = ENUMXXX,/g' -e 's/);/,/g' |awk '
BEGIN { count=1 }
{
sub("ENUMXXX",count)
count++
print
}' > LocationElements.h
echo "#define LocationElements_h_seen" >> LocationElements.h
echo $ctagline >> LocationElements.h

# poison unused bindings
echo '#error "perf interface last generated for c++"' >cperf._log.h
echo '#error "perf interface last generated for c++"' >cperf._stop.h
echo '#error "perf interface last generated for c++"' >cperf._save.h
