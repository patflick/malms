#!/bin/bash
# Bash Script to Test/Time MCSTL vs MALMS during cpu hotplugging


# ------------------------------------------------------- #
#                Settings for the Script
# ------------------------------------------------------- #

# The Size of the Input for the sorting Algorithms
MIN_INPUT_SIZE=100
MAX_INPUT_SIZE=100000000

# The Number of Threads used by the Algorithms
MIN_CORES=8
MAX_CORES=8

# Core that are being blocked before the sorting program is started
MIN_BLOCKED_CORES=0
MAX_BLOCKED_CORES=7

# The Type of the Input, according to the inputgeneration
# Programm
INPUT_TYPE=U

# Number of Workpakets (MALMS) / Threads (MCSTL) to use
WP=48
if [ -n "$1" ]; then
	WP=$1
fi

# Outputfile for the timing data
OUTPUTNAME=loadedcores_threads$WP.csv

# Number of Repetitions of the Tests
REPEAT=50

# ------------------------------------------------------- #
#                    Internal Settings
# ------------------------------------------------------- #

UTILS_DIR=../utils
DATA_DIR=./data
OUTPUT=$DATA_DIR/$OUTPUTNAME

# ------------------------------------------------------- #
#                 Prepare Output File
# ------------------------------------------------------- #
echo -n "" > $OUTPUT
 echo "Blocked.Cores;Cores;Input Size;Time;Time MCSTL;Workpakets" >> $OUTPUT

# ------------------------------------------------------- #
#                  Begin of Script
# ------------------------------------------------------- #

for ((size=$MIN_INPUT_SIZE; size<=$MAX_INPUT_SIZE; size*=10))
do
	echo "=== Input Size $size ==="


	# Generate Sorting input
	$UTILS_DIR/generatesortinput -n $size -t $INPUT_TYPE input.data
	
	for ((blockedcores=$MIN_BLOCKED_CORES; blockedcores<=$MAX_BLOCKED_CORES; blockedcores++))
	do
		echo " ==> $blockedcores Blocked Cores <=="
		# block core using while(true)
		if [ "$blockedcores" > "0" ]; then
			$UTILS_DIR/loadcore $blockedcores &
			PID_OF_LOADCORE=$!
		fi
		for ((cores=$MIN_CORES; cores<=$MAX_CORES; cores++))
		do
			echo -n " --> $cores Cores: "

			for ((i=0; i<$REPEAT; i++))
			do
				# Preparing new csv row
				echo -n "$blockedcores;$cores;$size;" >> $OUTPUT
				for algo in malms mcstl
				do
					malmscores=`expr $MAX_CORES - $blockedcores`
				
					# Start Sorting Process
					if [ "$algo" = "mcstl" ]; then
						./timesortfile -k $WP -a $algo input.data >> $OUTPUT &
					else
						./timesortfile -k $WP -c $malmscores -a $algo input.data >> $OUTPUT &
					fi
					PID_OF_SORT=$!


					# Wait for sorting process to finish
					wait $PID_OF_SORT
		
					echo -n ";" >> $OUTPUT
				done
				echo -e -n "$WP\n" >> $OUTPUT
				echo -n "."	

			done
			echo -e "\n"
		done
		if [ "$blockedcores" > "0" ]; then
			killall loadcore
		fi
	done
done


# ------------------------------------------------------- #
#                  Clean up
# ------------------------------------------------------- #

rm -f input.data
