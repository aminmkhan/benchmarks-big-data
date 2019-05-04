#!/bin/bash
#
# Executes different benchmarks with Saddlebag
# Pass `bash -x` or `bash -v` to see the commands executed

# Iterations of each experiment
ITER=10
PROGRAMS=$HOME/husky/release

# function to display commands
exe() { echo "______________________________________________________" ; echo [$(date +"%x %r")]; echo "\$ $@" ; "$@" ; }

CONFIG_FILE="local-1-node-8-worker.ini"

for ((n=0;n<$ITER;n++))
do

	echo "______________________________________________________"
	echo [$(date +"%x %r")];
	echo Running iteration $n ...

	# ---------

	kill $(ps aux | grep 'Master --' | awk '{print $2}') 
	$PROGRAMS/Master --conf local-1-node-8-worker.ini &> /dev/null &


	echo -e "\n\n---------------------------\n${PROGRAMS}/BenchPageRank --conf local-1-node-8-worker.ini --input nfs:///work/users/akhan/data/pagerank/web-uk-2005-all.compressed.mtx --iter 5  >> output.log 2>&1 &" >> output.log


	$PROGRAMS/BenchPageRank --conf local-1-node-8-worker.ini --input nfs:///work/users/akhan/data/pagerank/web-uk-2005-all.compressed.mtx --iter 5  >> output.log 2>&1

	# ---------

	kill $(ps aux | grep 'Master --' | awk '{print $2}') 
	$PROGRAMS/Master --conf local-1-node-8-worker.ini &> /dev/null &


	echo -e "\n\n---------------------------\n${PROGRAMS}/BenchPageRank --conf local-1-node-8-worker.ini --input nfs:///work/users/akhan/data/pagerank/web-uk-2002-all.compressed.mtx --iter 5  >> output.log 2>&1 &" >> output.log


	$PROGRAMS/BenchPageRank --conf local-1-node-8-worker.ini --input nfs:///work/users/akhan/data/pagerank/web-uk-2002-all.compressed.mtx --iter 5  >> output.log 2>&1

	# ---------

	kill $(ps aux | grep 'Master --' | awk '{print $2}') 
	$PROGRAMS/Master --conf local-1-node-8-worker.ini &> /dev/null &


	echo -e "\n\n---------------------------\n${PROGRAMS}/BenchPageRank --conf local-1-node-8-worker.ini --input nfs:///work/users/akhan/data/pagerank/web-indochina-2004-all.compressed.mtx --iter 5  >> output.log 2>&1 &" >> output.log


	$PROGRAMS/BenchPageRank --conf local-1-node-8-worker.ini --input nfs:///work/users/akhan/data/pagerank/web-indochina-2004-all.compressed.mtx --iter 5  >> output.log 2>&1


	sleep 1
done
