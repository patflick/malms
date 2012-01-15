#!/bin/bash
# Bash Script to Test/Time MCSTL vs MALMS during cpu hotplugging

# first parameter determines the amount of workpakets for MALMS

# ------------------------------------------------------- #
#                Settings for the Test
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


# Outputfilename for the timing data
OUTPUTNAME=hotpluggedcores_wp$WP.csv

# Number of Repetitions of each Tests
REPEAT=50


# ------------------------------------------------------- #
#                    Internal Settings
# ------------------------------------------------------- #

UTILS_DIR=../utils
DATA_DIR=./data
OUTPUT=$DATA_DIR/$OUTPUTNAME

# ------------------------------------------------------- #
#                  Blocking Functions
# ------------------------------------------------------- #


# Hotpluggs the CPU given as first parameter from the system
# i.e. Disables the given CPU
# Parameters:
#		$1	The CPU ID of the core to be disabled
function block {
	$UTILS_DIR/sendblockcore block -p $PID_OF_SORT -c $1
	/admin/cpu-blocking/block-cpu$1 > /dev/null
	#echo 0 > /sys/devices/system/cpu/cpu$1/online
}


# Hotpluggs the CPU given as first parameter into the system
# i.e. Enables the given CPU (under the assumption that the CPU has been
#      hotplugged from the system earlier)
# Parameters:
#		$1	The CPU ID of the core to be enabled
function unblock {
	#echo 1 > /sys/devices/system/cpu/cpu$1/online
	$UTILS_DIR/sendblockcore unblock -p $PID_OF_SORT -c $1
	/admin/cpu-blocking/unblock-cpu$1 > /dev/null
}


# Unblocks all Cores in the System. This should be called after each test,
# so that for each test, the testing circumstances are the same
function unblockall {
	/admin/cpu-blocking/unblock-all > /dev/null
	#for i in 1 2 3;
	#do
	#	echo 1 > /sys/devices/system/cpu/cpu$i/online;
	#done
}

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
	echo  "=== Input Size $size ==="



	# Generate Sorting input
	$UTILS_DIR/generatesortinput -n $size -t $INPUT_TYPE input.data


	for ((blockedcores=$MIN_BLOCKED_CORES; blockedcores<=$MAX_BLOCKED_CORES; blockedcores++))
	do
		blockbegin=`expr $MAX_CORES - $blockedcores`

		for ((blockcore=$blockbegin; blockcore<$MAX_CORES; blockcore++))
		do
			block $blockcore
		done
		echo " = Blocking $blockedcores Cores ="

		for ((cores=$MIN_CORES; cores<=$MAX_CORES; cores++))
		do
			echo -n "   -> $cores Cores:"



			for ((i=0; i<$REPEAT; i++))
			do
				# Preparing new csv row
				echo -n "$blockedcores;$cores;$size;" >> $OUTPUT
				for algo in malms mcstl
				do
					# Starting Process that waits for the Signal from the Sort Process
					$UTILS_DIR/waitforsignal &
					PID_OF_WAIT=$!

					malmscores=$blockbegin
					
					# Start Sorting Process
					if [ "$algo" = "mcstl" ]; then
						./timesortfile -k $cores -a $algo -p $PID_OF_WAIT input.data >> $OUTPUT &
					else
						./timesortfile -k $WP -c $malmscores -a $algo -p $PID_OF_WAIT input.data >> $OUTPUT &
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
		
		unblockall
	done
done


# ------------------------------------------------------- #
#                  Clean up
# ------------------------------------------------------- #

rm -f input.data
unblockall
