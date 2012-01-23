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


# The Type of the Input, according to the inputgeneration
# Programm
INPUT_TYPE=U

# Number of Workpakets (MALMS) / Threads (MCSTL) to use
WP=48
if [ -n "$1" ]; then
	WP=$1
fi

BLOCK_CYCLE_MICROSEC=100
if [ -n "$2" ]; then
	BLOCK_CYCLE_MICROSEC=$2
fi

# Outputfile for the timing data
OUTPUTNAME=dynload_new_$BLOCK_CYCLE_MICROSECµs_wp$WP\_info.csv

# Number of Repitions of the Tests
REPEAT=10



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
 echo "Cores;Input.Size;Time.MALMS.Info;Loops.MALMS.Info;Time.MALMS.NoInfo;Loops.MALMS.NoInfo;Time.MCSTL;Loops.MCSTL;Workpakets" >> $OUTPUT

# ------------------------------------------------------- #
#                  Begin of Script
# ------------------------------------------------------- #

for ((size=$MIN_INPUT_SIZE; size<=$MAX_INPUT_SIZE; size*=10))
do
	echo  "=== Input Size $size ==="


	# Generate Sorting input
	$UTILS_DIR/generatesortinput -n $size -t $INPUT_TYPE input.data
	

	for ((cores=$MIN_CORES; cores<=$MAX_CORES; cores++))
	do
		echo -n " --> $cores Cores: "

		for ((i=0; i<$REPEAT; i++))
		do
			# Preparing new csv row
			echo -n "$cores;$size;" >> $OUTPUT
			for algo in malmsinfo malmsnoinfo mcstl
			do
				# Starting Process that waits for the Signal from the Sort Process
				$UTILS_DIR/waitforsignal &
				PID_OF_WAIT=$!
				
				malmscores=$cores
				BlockNanoS=$((1000*$BLOCK_CYCLE_MICROSEC))
				
				# Start Sorting Process
				if [ "$algo" = "mcstl" ]; then
					./dynloadcores noinfo $BlockNanoS ./timesortfile -k $cores -a $algo input.data >> $OUTPUT &
				elif [ "$algo" = "malmsinfo" ]; then
					./dynloadcores info $BlockNanoS ./timesortfile -k $WP -c $malmscores -a $algo input.data >> $OUTPUT &
				elif [ "$algo" = "malmsnoinfo" ]; then
					./dynloadcores noinfo $BlockNanoS ./timesortfile -k $WP -c $malmscores -a $algo input.data >> $OUTPUT &
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
done


# ------------------------------------------------------- #
#                  Clean up
# ------------------------------------------------------- #

rm -f input.data
