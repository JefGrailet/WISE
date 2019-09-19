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
        print("Use this command: python SubnetAnalysis.py [AS name] [Dates] [[Show dump]]")
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
    
    # TODO: change datasetPrefix so this fits your computer.
    datasetPrefix = "/home/jefgrailet/PhD/Campaigns/WISE/" + ASNumber + "/"
    ratios1 = []
    ratios2 = []
    ratios3 = []
    total3Rules = []
    total2Rules = []
    total1Rule = []
    totalNoRule = []
    totalAmountSubnets = []
    maxSubnets = 0 # Max amount of inferred subnets for a given dataset
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
        
        # Code to ignore specific vantage points (for which too few data could be collected)
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
        
        # Subnet parsing starts here
        with open(datasetPath) as f:
            subnetsRaw = f.read().splitlines()
        
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
        
        totalSubnets = float(len(subnets))
        if totalSubnets > maxSubnets:
            maxSubnets = totalSubnets
        
        # Subnet rules fulfillment
        nbRule1 = 0
        nbRule2 = 0
        nbRule3 = 0
        nbSingleDist = 0
        nbAccurate = 0
        nbShadow = 0
        nb2Rules = 0
        nb1Rule = 0 # nbAccurate = 3 rules, nbShadow = no rule
        for j in range(0, len(subnets)):
            rule1 = contrapivotRule(subnets[j])
            rule2 = spreadRule(subnets[j])
            rule3 = outlierRule(subnets[j])
            if rule1:
                nbRule1 += 1
            if rule2:
                nbRule2 += 1
            if rule3:
                nbRule3 += 1
            if singleDistance(subnets[j]):
                nbSingleDist += 1
            if rule1 and rule2 and rule3:
                nbAccurate += 1
            elif not rule1 and not rule2 and not rule3:
                nbShadow += 1
            else:
                rules = [rule1, rule2, rule3]
                nbRules = 0
                for k in range(0, 3):
                    if rules[k]:
                        nbRules += 1
                if nbRules == 2:
                    nb2Rules += 1
                else:
                    nb1Rule += 1
        
        ratioRule1 = (float(nbRule1) / totalSubnets) * 100
        ratioRule2 = (float(nbRule2) / totalSubnets) * 100
        ratioRule3 = (float(nbRule3) / totalSubnets) * 100
        ratios1.append(ratioRule1)
        ratios2.append(ratioRule2)
        ratios3.append(ratioRule3)
        total3Rules.append(nbAccurate)
        total2Rules.append(nb2Rules)
        total1Rule.append(nb1Rule)
        totalNoRule.append(nbShadow)
        totalAmountSubnets.append(int(totalSubnets))
        ratioSingleDist = (float(nbSingleDist) / totalSubnets) * 100
        ratioAccurate = (float(nbAccurate) / totalSubnets) * 100
        ratioShadow = (float(nbShadow) / totalSubnets) * 100
        
        if showDump == "yes":
            fulfillmentDump = "Summary for " + date[0] + "/" + date[1] + "/" + date[2] + "\n"
            fulfillmentDump += "----------------------\n"
            fulfillmentDump += "Total amount of subnets: " + str(int(totalSubnets)) + "\n"
            fulfillmentDump += "Subnets fulfilling contra-pivot rule: " + str(nbRule1) + " (" + str('%.2f' % ratioRule1) + "%)\n"
            fulfillmentDump += "Subnets fulfilling spread rule: " + str(nbRule2) + " (" + str('%.2f' % ratioRule2) + "%)\n"
            fulfillmentDump += "Subnets fulfilling outlier rule: " + str(nbRule3) + " (" + str('%.2f' % ratioRule3) + "%)\n"
            fulfillmentDump += "Subnets with only one TTL distance: " + str(nbSingleDist) + " (" + str('%.2f' % ratioSingleDist) + "%)\n"
            fulfillmentDump += "Accurate subnets (all rules): " + str(nbAccurate) + " (" + str('%.2f' % ratioAccurate) + "%)\n"
            fulfillmentDump += "Shadow subnets (no rule): " + str(nbShadow) + " (" + str('%.2f' % ratioShadow) + "%)\n"
            print(fulfillmentDump)
    
    # Plot if enough data
    if len(validDatasets) < 2:
        print("Not enough data to plot a figure.")
        sys.exit()
    
    # Normalize the totals w.r.t. the total amount of subnets
    normalizedTotal1 = [] # 3 rules
    normalizedTotal2 = [] # 2 rules
    normalizedTotal3 = [] # 1 rule
    normalizedTotal4 = [] # No rule
    for i in range(0, len(totalAmountSubnets)):
        normalizedTotal1.append((float(total3Rules[i]) / float(totalAmountSubnets[i])) * 100)
        normalizedTotal2.append((float(total2Rules[i]) / float(totalAmountSubnets[i])) * 100)
        normalizedTotal3.append((float(total1Rule[i]) / float(totalAmountSubnets[i])) * 100)
        normalizedTotal4.append((float(totalNoRule[i]) / float(totalAmountSubnets[i])) * 100)
    
    # Fonts for the plot
    hfont = {'fontname':'serif',
             'fontweight':'bold',
             'fontsize':28}

    hfont2 = {'fontname':'serif',
             'fontsize':26}
    
    hfont3 = {'fontname':'serif',
             'fontsize':20}
    
    hfont4 = {'fontname':'serif',
             'fontsize':13}
    
    # Bar charts utilities
    ind = np.arange(len(validDatasets))
    widthUp = 1.0
    widthDown = 0.5
    paddingDown = 0.25
    
    # Stats plotting
    plt.figure(figsize=(13,10))
    
    # First subplot: ratio of subnets fulfilling a certain amount of rules
    plt.subplot(2, 1, 1)
    
    bottom1 = normalizedTotal1
    bottom2 = np.zeros((len(validDatasets), 1)).tolist()
    for i in range(0, len(validDatasets)):
        bottom2[i] = bottom1[i] + normalizedTotal2[i]
    bottom3 = np.zeros((len(validDatasets), 1)).tolist()
    for i in range(0, len(validDatasets)):
        bottom3[i] = bottom2[i] + normalizedTotal3[i]
    
    p1 = plt.bar(ind, normalizedTotal1, widthUp, color='#A0A0A0', edgecolor='#A0A0A0')
    p2 = plt.bar(ind, normalizedTotal2, widthUp, color='#757575', edgecolor='#757575', bottom=bottom1)
    p3 = plt.bar(ind, normalizedTotal3, widthUp, color='#404040', edgecolor='#404040', bottom=bottom2)
    p4 = plt.bar(ind, normalizedTotal4, widthUp, color='#000000', edgecolor='#000000', bottom=bottom3)
    
    # Limits and ticks
    plt.ylim([0, 100])
    plt.xlim([-0.5, len(validDatasets) - 0.5])
    
    # Dates of "valid" datasets
    xTicks = []
    for i in range(0, len(validDatasets)):
        xTicks.append(validDatasets[i][0] + "/" + validDatasets[i][1])
    
    yTicks = range(0, 101, 25)
    yTicks.insert(0, 0)
    
    plt.yticks(yTicks, **hfont2)
    plt.xticks(ind, xTicks, rotation=30, **hfont4)
    
    # Adds grid to Y axis
    axis = plt.gca()
    yaxis = axis.get_yaxis()
    yticks = yaxis.get_major_ticks()
    yticks[0].label1.set_visible(False)
    yaxis.grid(linestyle='--', color='#FFFFFF')
    
    # Labels
    plt.ylabel('% of subnets', **hfont)
    
    plt.rc('font', family='serif', size=15)
    plt.legend((p1[0], p2[0], p3[0], p4[0]), 
               ('3 rules', '2 rules', '1 rule', 'None'), 
               bbox_to_anchor=(0.05, 1.02, 0.90, .102), 
               loc=3,
               ncol=5, 
               mode="expand", 
               borderaxespad=0., 
               fontsize=18)
    
    # Second subplot: total of inferred subnets
    plt.subplot(2, 1, 2)
    plt.bar(ind + paddingDown, totalAmountSubnets, widthDown, color='#A0A0A0', edgecolor='#A0A0A0')
    
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
    yTicks2 = [0]
    while yTicks2[len(yTicks2) - 1] < maxSubnets:
        yTicks2.append(yTicks2[len(yTicks2) - 1] + step)
    
    # Limits and ticks
    plt.ylim([0, yLimit])
    plt.xlim([-0.5, len(validDatasets)])
    
    plt.yticks(yTicks2, **hfont3)
    plt.xticks(ind + paddingDown, xTicks, rotation=30, **hfont4)
    
    # Adds grid to Y axis
    axis = plt.gca()
    yaxis = axis.get_yaxis()
    yticks = yaxis.get_major_ticks()
    yticks[0].label1.set_visible(False)
    yaxis.grid(linestyle='--', color='#000000')
    
    # Labels
    plt.ylabel('# subnets', **hfont)
    plt.xlabel('Dataset', **hfont)
    
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
