#!/bin/sh 
cfiles=`ls *.cpp |sed '
s/aperf.cpp//g
s/cperf.cpp//g
s/csvformatter.cpp//g
s/diffrusage.cpp//g
s/eperf.cpp//g
s/eigerformatter.cpp//g
s/\n/ /g
s/  / /g'`
# -e 's/cperf.cpp//g' -e 's/csvformatter.cpp//g' -e 's/diffrusage.cpp//g' -e 's/eperf.cpp//g' -e 's/eigerformatter.cpp//g'`
echo "Finding CXX profiling sites by scanning $cfiles"
ctagline="/// generated from $cfiles "
echo "   updating CSVInitFuncs.h"
grep -h PERFLOG $cfiles | \
sed -e 's/[ \t]//g' \
 -e 's/.*PERFLOG(\(\w*\)[ ,]*/\tvoid init\1(csvformatter *c){/g' \
 -e 's/DR(\([A-Za-z0-9.]*\)),*/ c->addCol("\1",DR);/g'  \
 -e 's/DD(\([A-Za-z0-9.]*\)),*/ c->addCol("\1",DD);/g' \
 -e 's/DN(\([A-Za-z0-9.]*\)),*/ c->addCol("\1",DN);/g' \
 -e 's/IR(\([A-Za-z0-9.]*\)),*/ c->addCol("\1",IR);/g' \
 -e 's/ID(\([A-Za-z0-9.]*\)),*/ c->addCol("\1",ID);/g' \
 -e 's/IN(\([A-Za-z0-9.]*\)),*/ c->addCol("\1",IN);/g' \
 -e 's/);$/ }/' > CSVInitFuncs.h
echo $ctagline >> CSVInitFuncs.h

echo "   updating EigerInitFuncs.h"
grep -h PERFLOG $cfiles | \
sed -e 's/[ \t]//g' \
 -e 's/.*PERFLOG(\(\w*\)[ ,]*/\tvoid init\1(eigerformatter *c){/g' \
 -e 's/DR(\([A-Za-z0-9.]*\)),*/ c->addCol("\1",DR);/g'  \
 -e 's/DD(\([A-Za-z0-9.]*\)),*/ c->addCol("\1",DD);/g' \
 -e 's/DN(\([A-Za-z0-9.]*\)),*/ c->addCol("\1",DN);/g' \
 -e 's/IR(\([A-Za-z0-9.]*\)),*/ c->addCol("\1",IR);/g' \
 -e 's/ID(\([A-Za-z0-9.]*\)),*/ c->addCol("\1",ID);/g' \
 -e 's/IN(\([A-Za-z0-9.]*\)),*/ c->addCol("\1",IN);/g' \
 -e 's/);$/ }/' > EigerInitFuncs.h
echo $ctagline >> EigerInitFuncs.h

echo "   updating InitSwitchElements.h"
grep -h PERFLOG $cfiles |sed -e 's/.*PERFLOG(\(\w*\).*/\t\t\tcase lwperf_\1: init\1(ncf); break;/g' > InitSwitchElements.h
echo $ctagline >> InitSwitchElements.h

echo "   updating LocationElements.h"
grep -h PERFLOG $cfiles |sed -e 's/.*PERFLOG(\(\w*\).*/lwperf_\1 = ENUMXXX,/g' -e 's/);/,/g' |awk '
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
