#!/bin/bash
# Bash Script to Test/Time MCSTL vs MALMS during cpu hotplugging


# ------------------------------------------------------- #
#                Settings for the Script
# ------------------------------------------------------- #

# The Size of the Input for the sorting Algorithms
MIN_INPUT_SIZE=100
MAX_INPUT_SIZE=10000000

# The Number of Threads used by the Algorithms
MIN_CORES=8
MAX_CORES=8

# The Number of Workpakets 
WP_FROM=8
WP_TO=800
WP_INC=8

# The Type of the Input, according to the inputgeneration
# Programm
INPUT_TYPE=U

# Outputfile for the timing data
OUTPUTNAME=growing_k.csv

# Number of Repitions of the Tests
REPEAT=50



# ------------------------------------------------------- #
#                    Internal Settings
# ------------------------------------------------------- #

UTILS_DIR=../utils
DATA_DIR=./data
OUTPUT=$DATA_DIR/$OUTPUTNAME

# ------------------------------------------------------- #
#               Compile with Phase Timing
# ------------------------------------------------------- #

make timesortfile_phases

# ------------------------------------------------------- #
#                 Prepare Output File
# ------------------------------------------------------- #
echo -n "" > $OUTPUT
 echo "Cores;Input.Size;Time.Sorting;Time.Splitting;Time.Merging;Time.MALMS;Workpakets" >> $OUTPUT

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
		for ((WP=$WP_FROM; WP<=$WP_TO; WP+=WP_INC))
		do
			echo -n " --> $WP Workpackets: "
			for ((i=0; i<$REPEAT; i++))
			do
				# Preparing new csv row
				echo -n "$cores;$size;" >> $OUTPUT				

				# Start Sorting Process
				./timesortfile -k $WP -c $cores -a malms input.data >> $OUTPUT &
				PID_OF_SORT=$!

				# Wait for sorting process to finish
				wait $PID_OF_SORT

				echo -e -n ";$WP\n" >> $OUTPUT
				echo -n "."	

			done
			echo ""
		done

	done
	echo -e "\n"
done


# ------------------------------------------------------- #
#                  Clean up
# ------------------------------------------------------- #

rm -f input.data
