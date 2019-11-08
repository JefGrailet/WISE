#!/bin/bash

if [ $# -ne 3 ]; then
    echo "Usage: ./PlotFigures.sh [ASes] [Dates] [Output files prefix]"
    exit 1
fi

datesFile=${2}
outputPrefix=${3}

# Retrieves ASes
n_ASes=$( cat ${1} | wc -l )
IFS=$'\r\n' GLOBIGNORE='*' command eval  'ASes=($(cat ${1}))'

# Retrieves dates (for output directory)
n_dates=$( cat ${2} | wc -l )
IFS=$'\r\n' GLOBIGNORE='*' command eval  'dates=($(cat ${2}))'
lastIndex=`expr $n_dates - 1`
firstDataset=${dates[0]}
firstDataset=${firstDataset//\//-}
lastDataset=${dates[$lastIndex]}
lastDataset=${lastDataset//\//-}
outputDirName=$firstDataset" to "$lastDataset

i=0
while [ $i -lt $n_ASes ]
do
    thisPrefix=$outputPrefix"_"${ASes[$i]}
    echo "Building figures for ${ASes[$i]}..."
    mkdir -p ./Figures/${ASes[$i]}/"$outputDirName"
    python NeighborhoodsAnalysis.py ${ASes[$i]} ${2} $thisPrefix
    mv $thisPrefix* ./Figures/${ASes[$i]}/"$outputDirName"/
    i=`expr $i + 1`
done
