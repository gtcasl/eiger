#!/bin/sh 
# this differs from the c++ scanning version in that
# f90 doesn't use ; normally and we need extra files.
#
ffiles=`ls *.F90 | sed '
s/fperf.F90//g
s/flocations.F90//'`
echo "Finding Fortran profiling sites by scanning $ffiles"
ftagline="!!! generated from $ffiles"
ctagline="///  from $ffiles"

# build C++ calls from fortran source

echo "   updating CSVInitFuncs.h"
grep -h PERFLOG $ffiles | \
sed -e 's/[ \t]//g' \
 -e 's/;$//' \
 -e 's/.*PERFLOG[0-9](\(\w*\)[ ,]*/\tvoid init\1(csvformatter *c){/g' \
 -e 's/DR(\(\w*\)),*/ c->addCol("\1",DR);/g'  \
 -e 's/DD(\(\w*\)),*/ c->addCol("\1",DD);/g' \
 -e 's/DN(\(\w*\)),*/ c->addCol("\1",DN);/g' \
 -e 's/IR(\(\w*\)),*/ c->addCol("\1",IR);/g' \
 -e 's/ID(\(\w*\)),*/ c->addCol("\1",ID);/g' \
 -e 's/IN(\(\w*\)),*/ c->addCol("\1",IN);/g' \
 -e 's/)$/ }/' > CSVInitFuncs.h
echo $ctagline >> CSVInitFuncs.h

echo "   updating EigerInitFuncs.h"
grep -h PERFLOG $ffiles | \
sed -e 's/[ \t]//g' \
 -e 's/;$//' \
 -e 's/.*PERFLOG[0-9](\(\w*\)[ ,]*/\tvoid init\1(eigerformatter *c){/g' \
 -e 's/DR(\(\w*\)),*/ c->addCol("\1",DR);/g'  \
 -e 's/DD(\(\w*\)),*/ c->addCol("\1",DD);/g' \
 -e 's/DN(\(\w*\)),*/ c->addCol("\1",DN);/g' \
 -e 's/IR(\(\w*\)),*/ c->addCol("\1",IR);/g' \
 -e 's/ID(\(\w*\)),*/ c->addCol("\1",ID);/g' \
 -e 's/IN(\(\w*\)),*/ c->addCol("\1",IN);/g' \
 -e 's/)$/ }/' > EigerInitFuncs.h
echo $ctagline >> EigerInitFuncs.h

echo "   updating InitSwitchElements.h"
grep -h PERFLOG $ffiles |sed -e 's/.*PERFLOG[0-9](\(\w*\).*/\t\t\tcase \1: init\1(ncf); break;/g' > InitSwitchElements.h
echo $ctagline >> InitSwitchElements.h

# build C interface for fortran bind c
echo "   updating cperf _log"
grep -h PERFLOG $ffiles |sed -e 's/.*PERFLOG[0-9](\(\w*\).*/case ENUMXXX: { PERF::Log(\1, "\1", _USE_LS)->start(); } break; /g' |awk '
BEGIN { count=1 }
{
sub("ENUMXXX",count)
count++
print
}' > cperf._log.h
echo $ctagline >> cperf._log.h

echo "   updating cperf _stop"
grep -h PERFLOG $ffiles |sed -e 's/.*PERFLOG[0-9](\(\w*\).*/case ENUMXXX: { PERF::Log(\1, "\1", _USE_LS)->stop(); } break; /g' |awk '
BEGIN { count=1 }
{
sub("ENUMXXX",count)
count++
print
}' > cperf._stop.h
echo $ctagline >> cperf._stop.h

echo "   updating cperf _save"
grep -h PERFLOG $ffiles |sed -e 's/(/("/' -e 's/,/","/g' -e 's/))/)"/g' -e 's/,/ /g' -e 's/PERFLOG.(//' \
| ./looper.sh ./gencsave.body.sh > cperf._save.body.h
echo $ctagline >> cperf._save.body.h
grep -h PERFLOG $ffiles |sed -e 's/(/("/' -e 's/,/","/g' -e 's/))/)"/g' -e 's/,/ /g' -e 's/PERFLOG.(//' \
| ./looper.sh ./gencsave.sh > cperf._save.h
echo $ctagline >> cperf._save.h

echo "   updating fperf _save"
grep -h PERFLOG $ffiles |sed -e 's/(/("/' -e 's/,/","/g' -e 's/))/)"/g' -e 's/,/ /g' -e 's/PERFLOG.(//' \
| ./looper.sh ./genfsave.sh > fperf._save.h
echo $ftagline >> fperf._save.h


# build enum for c and fortran equivalent

echo "   updating flocations.h"
echo $ftagline > flocations.h
grep -h PERFLOG $ffiles |sed -e 's/.*PERFLOG[0-9]*(\(\w*\).*/ INTEGER(c_int) lwperf_\1 ; PARAMETER (lwperf_\1=ENUMXXX)/g' -e 's/);/=/g' | awk '
BEGIN { count=1 }
{
sub("ENUMXXX",count)
count++
print
}' >> flocations.h

echo "   updating LocationElements.h"
grep -h PERFLOG $ffiles |sed -e 's/.*PERFLOG[0-9](\(\w*\).*/\1 = ENUMXXX,/g' -e 's/);/,/g' |awk '
BEGIN { count=1 }
{
sub("ENUMXXX",count)
count++
print
}' > LocationElements.h
echo "#define LocationElements_h_seen" >> LocationElements.h
echo $ctagline >> LocationElements.h

