#!/bin/sh 
# this differs from the c++ scanning version in that
# c doesn't have the same scoping rules, especially 
# in older versions.
#
cfiles=`ls *.c | sed '
s/Make.c//g'`
echo "Finding C profiling sites by scanning $cfiles"
ftagline="!!! generated from $cfiles"
ctagline="///  from $cfiles"

# build C++ calls from fortran source

echo "   updating CSVInitFuncs.h"
grep -h PERFLOG $cfiles | \
sed -e 's/[ \t]//g' \
 -e 's/.*PERFLOG(\(\w*\)[ ,]*/\tvoid init\1(csvformatter *c){/g' \
 -e 's/DR(\([A-Za-z0-9._]*\)),*/ c->addCol("\1",DR);/g'  \
 -e 's/DD(\([A-Za-z0-9._]*\)),*/ c->addCol("\1",DD);/g' \
 -e 's/DN(\([A-Za-z0-9._]*\)),*/ c->addCol("\1",DN);/g' \
 -e 's/IR(\([A-Za-z0-9._]*\)),*/ c->addCol("\1",IR);/g' \
 -e 's/ID(\([A-Za-z0-9._]*\)),*/ c->addCol("\1",ID);/g' \
 -e 's/IN(\([A-Za-z0-9._]*\)),*/ c->addCol("\1",IN);/g' \
 -e 's/);$/ }/' > CSVInitFuncs.h
echo $ctagline >> CSVInitFuncs.h

echo "   updating EigerInitFuncs.h"
grep -h PERFLOG $cfiles | \
sed -e 's/[ \t]//g' \
 -e 's/.*PERFLOG(\(\w*\)[ ,]*/\tvoid init\1(eigerformatter *c){/g' \
 -e 's/DR(\([A-Za-z0-9._]*\)),*/ c->addCol("\1",DR);/g'  \
 -e 's/DD(\([A-Za-z0-9._]*\)),*/ c->addCol("\1",DD);/g' \
 -e 's/DN(\([A-Za-z0-9._]*\)),*/ c->addCol("\1",DN);/g' \
 -e 's/IR(\([A-Za-z0-9._]*\)),*/ c->addCol("\1",IR);/g' \
 -e 's/ID(\([A-Za-z0-9._]*\)),*/ c->addCol("\1",ID);/g' \
 -e 's/IN(\([A-Za-z0-9._]*\)),*/ c->addCol("\1",IN);/g' \
 -e 's/);$/ }/' > EigerInitFuncs.h
echo $ctagline >> EigerInitFuncs.h

echo "   updating InitSwitchElements.h"
grep -h PERFLOG $cfiles |sed -e 's/.*PERFLOG(\(\w*\).*/\t\t\tcase \1: init\1(ncf); break;/g' > InitSwitchElements.h
echo $ctagline >> InitSwitchElements.h

# build C interface for fortran bind c
echo "   updating cperf _log"
grep -h PERFLOG $cfiles |sed -e 's/.*PERFLOG(\(\w*\).*/case ENUMXXX: { PERF::Log(\1, "\1", _USE_LS)->start(); } break; /g' |awk '
BEGIN { count=1 }
{
sub("ENUMXXX",count)
count++
print
}' > cperf._log.h
echo $ctagline >> cperf._log.h

echo "   updating cperf _stop"
grep -h PERFLOG $cfiles |sed -e 's/.*PERFLOG(\(\w*\).*/case ENUMXXX: { PERF::Log(\1, "\1", _USE_LS)->stop(); } break; /g' |awk '
BEGIN { count=1 }
{
sub("ENUMXXX",count)
count++
print
}' > cperf._stop.h
echo $ctagline >> cperf._stop.h

echo "   updating cperf _save"

grep -h PERFLOG $cfiles |sed \
-e 's/.*PERFLOG/PERFLOG/g' \
-e 's/(/("/' -e 's/,/","/g' -e 's/))/)"/g' -e 's/,/ /g' -e 's/PERFLOG(//' \
| ./looper.sh ./gencsave.sh > cperf._save.h
echo $ctagline >> cperf._save.body.h

grep -h PERFLOG $cfiles |sed \
-e 's/.*PERFLOG/PERFLOG/g' \
-e 's/(/("/' -e 's/,/","/g' -e 's/))/)"/g' -e 's/,/ /g' -e 's/PERFLOG(//' \
| ./looper.sh ./gencsave.body.sh > cperf._save.body.h
echo $ctagline >> cperf._save.body.h

# build enum for c and fortran equivalent

echo "   updating LocationElements.h"
grep -h PERFLOG $cfiles |sed -e 's/.*PERFLOG(\(\w*\).*/\1 = ENUMXXX,/g' -e 's/);/,/g' |awk '
BEGIN { count=1 }
{
sub("ENUMXXX",count)
count++
print
}' > LocationElements.h
echo "#define LocationElements_h_seen" >> LocationElements.h
echo $ctagline >> LocationElements.h

echo "   updating clocations.h"
echo $ctagline > clocations.h
grep -h PERFLOG $cfiles |sed -e 's/.*PERFLOG(\(\w*\).*/ lwperf_\1=ENUMXXX,/g' -e 's/);/=/g' | awk '
BEGIN { count=1 }
{
sub("ENUMXXX",count)
count++
print
}' >> clocations.h

