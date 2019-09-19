#! /usr/bin/env python
# -*- coding: utf-8 -*-

# 18/09/2019: updated to take account of minor changes brought by WISE 1.1, mainly: 
# -taking account of adjusted prefixes, 
# -taking account of alternative contra-pivot definition (trail-based).

import os
import sys
import numpy as np
import math
from matplotlib import pyplot as plt

# Rules to evaluate the soundness of a subnet

# Rule 1: contra-pivot rule

def contrapivotRule(subnet):
    for i in range(1, len(subnet) - 1):
        intType = subnet[i][3] # For "interface type"
        if intType == 'Contra-pivot' or intType == 'Contra-pivot (alternative definition)':
            return True
    return False

# Rule 2: spread rule

def spreadRule(subnet):
    nbContrapivots = 0
    nbInterfaces = len(subnet) - 1
    for i in range(1, len(subnet) - 1):
        intType = subnet[i][3] # Same as above
        if intType == 'Contra-pivot' or intType == 'Contra-pivot (alternative definition)':
            nbContrapivots += 1
    if nbContrapivots == 1:
        return True
    elif nbContrapivots == 0:
        return False
    nbOthers = nbInterfaces - nbContrapivots
    
    splitCIDR = subnet[0].split("/")
    prefixLen = int(splitCIDR[1])
    if prefixLen >= 29:
        if nbOthers - nbContrapivots >= 0:
            return True
    elif prefixLen >= 27:
        ratio = float(nbContrapivots) / nbInterfaces
        if ratio < 0.2:
            return True
    else:
        ratio = float(nbContrapivots) / nbInterfaces
        if ratio < 0.1:
            return True
    return False

# Rule 3: outlier rule

def outlierRule(subnet):
    for i in range(1, len(subnet) - 1):
        if subnet[i][3] == 'Outlier':
            return False
    return True

# Checks whether the subnet consists of one unique TTL distance for all interfaces

def singleDistance(subnet):
    uniqueTTL = subnet[1][1]
    for i in range(2, len(subnet) - 1):
        if subnet[i][1] != uniqueTTL:
            return False
    return True

# Turns an IPv4 interface into its integer equivalent

def toInt(IP):
    if IP == "This computer":
        return 0
    
    split = IP.split(".")
    asInt = int(split[0]) * 256 * 256 * 256
    asInt += int(split[1]) * 256 * 256
    asInt += int(split[2]) * 256
    asInt += int(split[3])
    return asInt

# Checks if a subnet overlaps another one (overlaps can still occur)

def overlapTest(subnet1, subnet2):
    CIDR1 = subnet1[0].split('/')
    prefix1 = toInt(CIDR1[0])
    prefixLen1 = int(CIDR1[1])
    upperBorder1 = prefix1 + pow(2, 32 - prefixLen1) - 1

    CIDR2 = subnet2[0].split('/')
    lowerBorder2 = toInt(CIDR2[0])
    
    if upperBorder1 > lowerBorder2:
        return True
    return False

if __name__ == "__main__":

    if len(sys.argv) < 3:
        print("Use this command: python WeaklyPersistentSubnets.py [AS name] [Dates]")
        sys.exit()
    
    ASNumber = str(sys.argv[1])
    datesFilePath = str(sys.argv[2])

    # Checks existence of the file providing dates
    if not os.path.isfile(datesFilePath):
        print(datesFilePath + " does not exist.")
        sys.exit()
    
    # Parses the dates file
    with open(datesFilePath) as f:
        datesRaw = f.read().splitlines()
    dates = []
    for i in range(0, len(datesRaw)):
        splitDate = datesRaw[i].split('/')
        dates.append(splitDate)
    
    # TODO: change datasetPrefix so this fits your computer.
    datasetPrefix = "/home/jefgrailet/PhD/Campaigns/WISE/" + ASNumber + "/"
    perDateSets = []
    perDateData = []
    validDatasets = []
    ignoredDates = []
    for i in range(0, len(dates)):
        date = dates[i]
        datasetPath = datasetPrefix + date[2] + "/" + date[1] + "/" + date[0] + "/"
        VPPath = datasetPath + "VP.txt"
        datasetPath += ASNumber + "_" + date[0] + "-" + date[1] + ".subnets"
        
        if not os.path.isfile(datasetPath):
            if showDump == "yes":
                print(datasetPath + " does not exist.\n")
            ignoredDates.append(date)
            continue
        
        # Code to ignore specific vantage points (for which very few data could be collected)
        if not os.path.isfile(datasetPath):
            if showDump == "yes":
                print(VPPath + " does not exist.\n")
            ignoredDates.append(date)
            continue
        
        with open(VPPath) as f:
            vantagePoint = f.read().splitlines()
        
        # N.B.: only useful for processing early campaigns (February 2019)
        if vantagePoint[0] == "planet3.cs.huji.ac.il" or vantagePoint[0] == "planet4.cs.huji.ac.il":
            ignoredDates.append(date)
            continue
        
        with open(datasetPath) as f:
            subnetsRaw = f.read().splitlines()
        
        # Subnet parsing
        curSubnet = []
        parsedSubnets = []
        for j in range(0, len(subnetsRaw)):
            if not subnetsRaw[j]:
                if len(curSubnet) > 0:
                    parsedSubnets.append(curSubnet)
                    curSubnet = []
                continue
            # Subnet interface
            if len(curSubnet) >= 1:
                line = subnetsRaw[j]
                if "Stop" in line:
                    curSubnet.append(line)
                    continue
                splitLine = line.split(" - ")
                # Parts of the interface
                IP = splitLine[1]
                TTL = int(splitLine[0])
                trail = splitLine[2]
                interfaceType = splitLine[3]
                curSubnet.append([IP, TTL, trail, interfaceType])
            # Subnet CIDR notation
            else:
                CIDR = subnetsRaw[j]
                if "adjusted from" in CIDR:
                    splitCIDR = CIDR.split(" (adjusted from")
                    CIDR = splitCIDR[0]
                curSubnet.append(CIDR)
        
        # Removes subnets that are overlapped
        subnets = []
        for j in range(0, len(parsedSubnets)):
            if len(subnets) > 0 and overlapTest(subnets[len(subnets) - 1], parsedSubnets[j]):
                continue
            else:
                subnets.append(parsedSubnets[j])
        
        if len(subnets) == 0:
            if showDump == "yes":
                print("No subnet could be parsed.")
                ignoredDates.append(date)
            continue
        
        validDatasets.append(date)
        
        newSet = set()
        fullData = dict()
        for j in range(0, len(subnets)):
            newSet.add(subnets[j][0])
            fullData[subnets[j][0]] = subnets[j]
        perDateSets.append(newSet)
        perDateData.append(fullData)
    
    # Comparing each dataset to the reference dataset
    refSet = perDateSets[0]
    maxSubnets = 0
    totalSubnets = []
    totalStrictlyPersistent = []
    totalPersistent = []
    for i in range(1, len(perDateSets)):
        differing = []
        for subnet in refSet:
            data = perDateData[0][subnet]
            ruleMask = [contrapivotRule(data), spreadRule(data), outlierRule(data)]
            if subnet in perDateSets[i]:
                otherData = perDateData[i][subnet]
                otherMask = [contrapivotRule(otherData), spreadRule(otherData), 
                             outlierRule(otherData)]
                nbSameRules = 0
                for j in range(0, 3):
                    if ruleMask[j] == otherMask[j]:
                        nbSameRules += 1
                if nbSameRules != 3:
                    differing.append(subnet)
        
        # If we got weakly persistent subnets
        if len(differing) > 0:
            date = validDatasets[i]
        
            # Sorting subnet prefixes
            sortedPrefixes = []
            prefAsInt = []
            intToPref = dict()
            for j in range(0, len(differing)):
                CIDR = differing[j].split('/')
                prefInt = toInt(CIDR[0])
                prefAsInt.append(prefInt)
                intToPref[str(prefInt)] = differing[j]
            prefAsInt.sort()
            for j in range(0, len(prefAsInt)):
                sortedPrefixes.append(intToPref[str(prefAsInt[j])])
            
            # Printing the weakly persistent subnets
            if i > 1:
                print("")
            print("Weakly persistent subnets on " + '/'.join(date) + ":")
            for j in range(0, len(sortedPrefixes)):
                print(sortedPrefixes[j])
