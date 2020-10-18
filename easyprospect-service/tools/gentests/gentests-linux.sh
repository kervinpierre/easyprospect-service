#!/bin/bash -xv

FILES=../../data/test/dataset0001/*
TESTDIRRELATIVEFROMWORKDIR=../../../data/test/dataset0001/
INSRC=../../tests/EPCT_gentests-tmpl.cpp
OUTSRC=../../tests/EPCT_gentests.cpp
CURR_TESTCASE=1
FCOUNT=0

cd "$(dirname "$0")"

cp -f $INSRC $OUTSRC

echo "/*********************************************************************" >> $OUTSRC 
echo "**" >> $OUTSRC 
echo "**  WARNING: This file is generated.  DO NOT MODIFY" >> $OUTSRC 
echo "**" >> $OUTSRC 
echo "**********************************************************************/" >> $OUTSRC 
echo "" >> $OUTSRC 

for fpath in `ls -1 $FILES | sort`
do
  echo "Processing '$fpath' file..."
  
  file="$(basename $fpath)"

  FCOUNT=$(expr ${FCOUNT} +1)

  if [[ $file =~ ^TC([0-9]{4})_T([0-9]{4})_([^_]+)_(.*)\.js$ ]]; 
  then
      tcId=$(expr ${BASH_REMATCH[1]} + 0)
      tId=$(expr ${BASH_REMATCH[2]} + 0)
	  nextId=$(expr ${CURR_TESTCASE} + 1)
	  if [[ ($tcId -eq $nextId || $tcId -eq 1) && $tId -eq 1 ]]; 
	  then
	      
		  if [[ $tcId -gt 1 ]];
		  then
		      echo "}\n" >> $OUTSRC
		  fi

          CURR_TESTCASE=$tcId
		  
		  CURR_NS="${BASH_REMATCH[3]//-/_}"
		  echo -e "BOOST_AUTO_TEST_CASE(${CURR_NS}${BASH_REMATCH[4]})\n{" >> $OUTSRC
	  fi
	  codeFile="${TESTDIRRELATIVEFROMWORKDIR}${file}"
	  echo "    BOOST_TEST(EPCT::EPCT_Utils::ContextSingleUseFromFile(*setup_fixture::ep, \"$codeFile\"));" >> $OUTSRC
  else 
    echo "'$file' doesn't name match."; 
  fi

  if [[ $FCOUNT -gt 0 ]];
  then
	echo -e "}\n" >> $OUTSRC
  fi
done