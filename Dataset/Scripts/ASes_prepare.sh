#!/bin/bash
# ASes_prepare.sh: prepares the nodes for a measurement campaign.

if [ $# -ne 3 ] && [ $# -ne 2 ]; then
    echo "Usage: ./ASes_prepare.sh [file w/ target ASes] [file w/ PlanetLab nodes] [[rotation number]]"
    exit 1
fi

rotationNumber=0
if [ $# -eq 3 ]; then
    rotationNumber=${3}
fi

# Retrieves ASes and nodes
n_ASes=$( cat ${1} | wc -l )
IFS=$'\r\n' GLOBIGNORE='*' command eval  'targets=($(cat ${1}))'
n_nodes=$( cat ${2} | wc -l )
IFS=$'\r\n' GLOBIGNORE='*' command eval  'nodes=($(cat ${2}))'

# Checks the amount of ASes and PlanetLab nodes match
if [ $n_ASes -ne $n_nodes ]; then
    echo "Amount of target ASes and amount of PlanetLab nodes don't match."
    exit 1
fi

# Checks that the rotation number is between 0 and n_nodes (excluded)
if [ $rotationNumber -gt $n_nodes ] || [ $rotationNumber -lt 0 ]; then
    echo "Rotation number must be comprised in [0, #nodes[."
    exit 1
fi

echo "Preparing PlanetLab nodes for a campaign"

i=0
while [ $i -lt $n_nodes ]
do
    j=$((($i+$rotationNumber)%$n_nodes))
    echo "Preparation of ${nodes[$j]}"
    homeFolder=/home/jefgrailet/PhD # TODO: change this
    commands="mkdir -p WISE; cd WISE; rm AS*; "
    commandsNext="cd WISE; chmod 755 wise;"
    ssh ulgple_lisp@${nodes[$j]} -i ~/.ssh/id_rsa -T $commands # TODO: adapt login
    
    # TODO: change the paths to adapt this script to your file system
    scp -i ~/.ssh/id_rsa $homeFolder/Executables\ PlanetLab/i686/wise ulgple_lisp@${nodes[$j]}:/home/ulgple_lisp/WISE
    scp -i ~/.ssh/id_rsa $homeFolder/Campaigns/WISE/${targets[$i]}/${targets[$i]}.txt ulgple_lisp@${nodes[$j]}:/home/ulgple_lisp/WISE
    
    ssh ulgple_lisp@${nodes[$j]} -i ~/.ssh/id_rsa -T $commandsNext # TODO: adapt
    i=`expr $i + 1`
done

