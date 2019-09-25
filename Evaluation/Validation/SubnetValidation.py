#! /usr/bin/env python
# -*- coding: utf-8 -*-

import os
import sys
import math
import numpy as np
from matplotlib import pyplot as plt

def toInt(IP):
    split = IP.split(".")
    asInt = int(split[0]) * 256 * 256 * 256
    asInt += int(split[1]) * 256 * 256
    asInt += int(split[2]) * 256
    asInt += int(split[3])
    return asInt

def getBoundaries(subnet):
    boundaries = []
    prefixSplit = subnet.split('/')
    prefixLen = int(prefixSplit[1])
    lowerBoundary = toInt(prefixSplit[0])
    upperBoundary = lowerBoundary + pow(2, 32 - prefixLen)
    return [lowerBoundary, upperBoundary]

def encompass(subnet1, subnet2):
    bounds1 = getBoundaries(subnet1)
    bounds2 = getBoundaries(subnet2)
    if bounds1[0] <= bounds2[0] and bounds1[1] >= bounds2[1]:
        return True
    return False

def diffPrefix(subnet1, subnet2):
    prefixSplit1 = subnet1.split('/')
    lenPrefix1 = int(prefixSplit1[1])
    prefixSplit2 = subnet2.split('/')
    lenPrefix2 = int(prefixSplit2[1])
    return lenPrefix1 - lenPrefix2

def compareSets(groundtruth, measurement):
    results = []
    for i in range(0, 23):
        results.append(0)
    zeroIndex = 11
    
    otherSubnets = []
    for subnet in groundtruth:
        if subnet in measurement:
            results[zeroIndex] += 1
        else:
            otherSubnets.append(subnet)
    
    for i in range(0, len(otherSubnets)):
        subnet1 = otherSubnets[i]
        for subnet2 in measurement:
            if encompass(subnet1, subnet2):
                diff = diffPrefix(subnet1, subnet2)
                results[zeroIndex - diff] += 1
            elif encompass(subnet2, subnet1):
                diff = diffPrefix(subnet2, subnet1)
                results[zeroIndex + diff] += 1
    
    print(results)
    
    # Normalizing
    for i in range(0, 23):
        normalized = (float(results[i]) / float(len(groundtruth))) * 100
        results[i] = normalized
    #sumArr = 0
    #for i in range(0, 23):
    #    sumArr += results[i]
    #print(sumArr)
    return results

if __name__ == "__main__":

    if len(sys.argv) < 5:
        print("Use this command: python SubnetValidation.py [groundtruth .txt] [WISE .subnets] [SAGE .subnets] [SAGE .xnet] [[--adjusted]]")
        sys.exit()
    
    groundtruthPath = str(sys.argv[1])
    WISEFilePath = str(sys.argv[2])
    TreeNETFilePath = str(sys.argv[3])
    ExploreNETFilePath = str(sys.argv[4])
    adjustedPrefixes = False
    if len(sys.argv) == 6:
        if str(sys.argv[5]) == '--adjusted':
            adjustedPrefixes = True

    # Checks existence of the file providing prefixes of the groundtruth
    if not os.path.isfile(groundtruthPath):
        print(groundtruthPath + " does not exist.")
        sys.exit()
    
    # Parses the groundtruth file
    with open(groundtruthPath) as f:
        prefixesRaw = f.read().splitlines()
    groundtruth = set()
    for i in range(0, len(prefixesRaw)):
        if prefixesRaw[i] not in groundtruth:
            groundtruth.add(prefixesRaw[i])
    
    # Checks existence of the .subnets file produced by WISE
    if not os.path.isfile(WISEFilePath):
        print(WISEFilePath + " does not exist.")
        sys.exit()
    
    # Parses the .subnets file produced by WISE
    with open(WISEFilePath) as f:
        WISERaw = f.read().splitlines()
    WISE = set()
    for i in range(0, len(WISERaw)):
        if "merging" in WISERaw[i] or "fragment" in WISERaw[i]:
            continue
        if "aggregate" in WISERaw[i]:
            continue
        if "/" in WISERaw[i]:
            subnetPrefix = WISERaw[i]
            if "adjusted" in WISERaw[i]:
                prefixSplit = WISERaw[i].split(" (adjusted from ")
                prefix1 = prefixSplit[0]
                prefix2 = prefixSplit[1][:-1]
                if adjustedPrefixes:
                    subnetPrefix = prefix1
                else:
                    subnetPrefix = prefix2
            if subnetPrefix not in WISE:
                WISE.add(subnetPrefix)
            else:
                print("Warning: " + subnetPrefix + " is a duplicate in WISE dataset.")
    
    # Checks existence of the .subnets file produced by SAGE v1.0
    if not os.path.isfile(TreeNETFilePath):
        print(TreeNETFilePath + " does not exist.")
        sys.exit()
    
    # Parses the .subnets file produced by SAGE v1.0 (equivalent to what TreeNET would produce)
    with open(TreeNETFilePath) as f:
        TreeNETRaw = f.read().splitlines()
    TreeNET = set()
    for i in range(0, len(TreeNETRaw)):
        if "/" in TreeNETRaw[i]:
            if TreeNETRaw[i] not in TreeNET:
                TreeNET.add(TreeNETRaw[i])
            else:
                print("Warning: " + TreeNETRaw[i] + " is a duplicate in TreeNET dataset.")
    
    # Checks existence of the .xnet file produced by SAGE v1.0
    if not os.path.isfile(ExploreNETFilePath):
        print(ExploreNETFilePath + " does not exist.")
        sys.exit()
    
    # Parses the .xnet file produced by SAGE v1.0 (equivalent to what ExploreNET would produce)
    with open(ExploreNETFilePath) as f:
        ExploreNETRaw = f.read().splitlines()
    ExploreNET = set()
    for i in range(0, len(ExploreNETRaw)):
        lineSplit = ExploreNETRaw[i].split()
        if "/" in lineSplit[1]:
            if lineSplit[1] not in ExploreNET:
                ExploreNET.add(lineSplit[1])
            else:
                print("Warning: " + lineSplit[1] + " is a duplicate in ExploreNET dataset.")
   
    # Prints perfectly inferred subnets found by TreeNET but not by WISE
    perfectWISE = set()
    perfectTreeNET = set()
    for subnet in groundtruth:
        if subnet in WISE:
            perfectWISE.add(subnet)
        if subnet in TreeNET:
            perfectTreeNET.add(subnet)
    
    missed = []
    for subnet in perfectTreeNET:
        if subnet not in perfectWISE:
            missed.append(subnet)
    missed.sort()
    print("Missed by WISE:")
    for i in range(0, len(missed)):
        print(missed[i])
    
    compWISE = compareSets(groundtruth, WISE)
    compTreeNET = compareSets(groundtruth, TreeNET)
    compExploreNET = compareSets(groundtruth, ExploreNET)
    
    # Plots result
    hfont = {'fontname':'serif',
             'fontsize':24}
   
    hfont2 = {'fontname':'serif',
             'fontsize':22}
   
    hfont3 = {'fontname':'serif',
             'fontsize':18}

    plt.figure(figsize=(13,9))
    
    xAxis = range(0, len(compWISE), 1)
    plt.plot(xAxis, compWISE, color='#000000', linewidth=3, label="WISE")
    plt.plot(xAxis, compTreeNET, color='#000000', linewidth=3, linestyle='--', label="TreeNET")
    plt.plot(xAxis, compExploreNET, color='#000000', linewidth=2, linestyle=':', label="ExploreNET")
    plt.rcParams.update({'font.size': 20})
    plt.axvline(x=11, linewidth=2)
    
    highestPercent = 100
    maxValue = 0
    for i in range(0, len(compWISE)):
        if compWISE[i] > maxValue:
            maxValue = compWISE[i]
        if compTreeNET[i] > maxValue:
            maxValue = compTreeNET[i]   
        if compExploreNET[i] > maxValue:
            maxValue = compExploreNET[i]
    
    while highestPercent > maxValue:
        highestPercent -= 10
    highestPercent += 10
    
    plt.ylim([0, highestPercent])
    plt.xlim([0, len(compWISE) - 1])
    plt.yticks(np.arange(0, highestPercent + 1, 10), **hfont2)
    axis = plt.gca()
    yaxis = axis.get_yaxis()
    yticks = yaxis.get_major_ticks()
    yticks[0].label1.set_visible(False)
    
    differences = []
    for i in range(0, 11):
        differences.append(str(i - 11))
    differences.append(0)
    for i in range(0, 11):
        differences.append(str(1 + i))
    plt.xticks(xAxis, differences, **hfont3)
    
    plt.ylabel('Ratio of ground truth prefixes (%)', **hfont)
    plt.xlabel('Difference with ground truth prefixes', **hfont)
    
    plt.grid()
    
    plt.legend(bbox_to_anchor=(0, 1.02, 1.0, .102), 
               loc=3,
               ncol=4, 
               mode="expand", 
               borderaxespad=0.,
               fontsize=24)
    
    plt.savefig("Validation.pdf")
    plt.clf()
