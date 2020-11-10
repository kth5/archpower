#!/bin/bash 

where=/tmp/mozgcda.tar

# all on one line yo
cd obj-powerpc64le-unknown-linux-gnu/instrumented || exit
tar cvf $where `find . -name '*.gcda' -print`
cd ..
tar xvf $where
rm -f $where 
