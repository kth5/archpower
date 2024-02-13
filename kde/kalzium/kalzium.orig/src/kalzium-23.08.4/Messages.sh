#! /bin/sh
$EXTRACTRC `find src -name \*.ui` src/*.rc src/*.kcfg >> rc.cpp || exit 11
$EXTRACTRC --tag-group=none --tag=name --tag=desc --tag=refitem data/knowledge.xml >> rc.cpp || exit 14
$EXTRACTRC --tag-group=none --tag=name --tag=desc data/tools.xml >> rc.cpp || exit 15

# extracting a reduced version of the elements.xml, with only the data we want
echo "<?xml version=\"1.0\" ?><tmp>" > element_tiny.xml
cat libscience/data/elements.xml | grep 'bo:name' >> element_tiny.xml
echo "</tmp>" >> element_tiny.xml
$EXTRACTATTR --attr=label,value element_tiny.xml >> rc.cpp
$EXTRACTRC --tag-group=none --tag=scalar element_tiny.xml >> rc.cpp

$XGETTEXT rc.cpp compoundviewer/*.cpp libscience/*.cpp `find src -name \*.cpp | grep -v 'src/conversion'` -o $podir/kalzium.pot

rm -f element_tiny.xml
