#! /usr/bin/env python
# -*- coding: utf-8 -*-

import os
import sys
import numpy as np
import math
from matplotlib import pyplot as plt

# Rules to evaluate the soundness of a subnet

# Rule 1: contra-pivot rule

def contrapivotRule(subnet):
    for i in range(1, len(subnet) - 1):
        if subnet[i][3] == 'Contra-pivot':
            return True
    return False

# Rule 2: spread rule

def spreadRule(subnet):
    nbContrapivots = 0
    nbInterfaces = len(subnet) - 1
    for i in range(1, len(subnet) - 1):
        if subnet[i][3] == 'Contra-pivot':
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
        print("Use this command: python SubnetPersistence.py [AS name] [Dates] [[Show dump]]")
        sys.exit()
    
    ASNumber = str(sys.argv[1])
    datesFilePath = str(sys.argv[2])
    showDump = ""
    if len(sys.argv) == 4:
        showDump = str(sys.argv[3]).lower()

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
    
    # TODO: change datasetPrefix to fit your file system
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
                curSubnet.append(subnetsRaw[j])
        
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
    
    refSet = perDateSets[0]
    maxSubnets = 0
    totalSubnets = []
    totalStrictlyPersistent = []
    totalPersistent = []
    for i in range(1, len(perDateSets)):
        strictCounter = 0
        distinctCounter = 0
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
                if nbSameRules == 3:
                    strictCounter += 1
                else:
                    distinctCounter += 1
        amountOfSubnets = len(perDateSets[i])
        if amountOfSubnets > maxSubnets:
            maxSubnets = amountOfSubnets
        totalSubnets.append(amountOfSubnets)
        totalStrictlyPersistent.append(strictCounter)
        totalPersistent.append(distinctCounter)
        if showDump == "yes":
            print("\nDataset n°" + str(i) + " has " + str(strictCounter) + " strictly persistent subnets w.r.t. the reference dataset.")
            print("Dataset n°" + str(i) + " has " + str(distinctCounter) + " differing persistent subnets w.r.t. the reference dataset.")
    
    # Plot if enough data
    if len(validDatasets) < 2:
        print("Not enough data to plot a figure.")
        sys.exit()
        
    hfont = {'fontname':'serif',
             'fontweight':'bold',
             'fontsize':28}

    hfont2 = {'fontname':'serif',
             'fontsize':22}
    
    hfont3 = {'fontname':'serif',
             'fontsize':16}
    
    # Stacked bar charts to show the amounts of (persistent) subnets
    lengthBarChart = len(totalPersistent)
    ind = np.arange(lengthBarChart)
    width = 0.8
    padding = 0.1
    
    remaining = []
    for i in range(0, lengthBarChart):
        remaining.append(totalSubnets[i] - totalPersistent[i] - totalStrictlyPersistent[i])
    
    bottom1 = totalStrictlyPersistent
    bottom2 = np.zeros((lengthBarChart, 1)).tolist()
    for i in range(0, lengthBarChart):
        bottom2[i] = bottom1[i] + totalPersistent[i]

    plt.figure(figsize=(13,10))

    p1 = plt.bar(ind + padding, totalStrictlyPersistent, width, color='#F0F0F0', edgecolor='#000000')
    p2 = plt.bar(ind + padding, totalPersistent, width, color='#D0D0D0', edgecolor='#000000', bottom=bottom1)
    p3 = plt.bar(ind + padding, remaining, width, color='#FFFFFF', linestyle='--', edgecolor='#000000', bottom=bottom2)
    
    # Get a power of ten (*1, *0.5 or *0.25) that's above the maximum amount of subnets
    power10 = 10
    while int(float(maxSubnets) / power10) > 0:
        power10 *= 10
    yLimit = power10
    if (power10 / 4) > maxSubnets:
        yLimit /= 4
    elif (power10 / 2) > maxSubnets:
        yLimit /= 2
    step = yLimit / 5
    while yLimit > maxSubnets:
        yLimit -= step
    yTicks = [0]
    while yTicks[len(yTicks) - 1] < maxSubnets:
        yTicks.append(yTicks[len(yTicks) - 1] + step)
    
    # Dates of valid datasets
    xTicks = []
    for i in range(1, len(validDatasets)):
        xTicks.append(validDatasets[i][0] + "/" + validDatasets[i][1])
    
    # Limits and ticks
    plt.xlim([-1, lengthBarChart])
    plt.ylim([0, yLimit])
    
    plt.xticks(ind + padding, xTicks, rotation=30, **hfont3)
    plt.yticks(yTicks, **hfont2)
    axis = plt.gca()
    yaxis = axis.get_yaxis()
    yticks = yaxis.get_major_ticks()
    yticks[0].label1.set_visible(False)
    yaxis.grid(linestyle='--', color='#000000')
    
    # Labels
    firstDate = validDatasets[0][0] + "/" + validDatasets[0][1]
    plt.ylabel('# subnets (w.r.t. ' + firstDate + ')', **hfont)
    plt.xlabel('Dataset', **hfont)
    
    plt.rc('font', family='serif', size=15)
    plt.legend((p1[0], p2[0], p3[0]), 
               ('Strictly persistent', 'Persistent', 'No equivalent'), 
               bbox_to_anchor=(0.05, 1.02, 0.90, .102), 
               loc=3,
               ncol=3, 
               mode="expand", 
               borderaxespad=0.,
               fontsize=20)
    
    # Saves everything
    figureFileName = ASNumber + ".pdf"
    plt.savefig(figureFileName)
    plt.clf()
    print("New figure saved in " + figureFileName + ".")
    
    if showDump == "yes" and len(ignoredDates) > 0:
        print("The following dates were ignored because there was little to no data:")
        for i in range(0, len(ignoredDates)):
            date = ignoredDates[i]
            print(date[0] + "/" + date[1] + "/" + date[2])
