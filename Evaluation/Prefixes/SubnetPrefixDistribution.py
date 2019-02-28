#! /usr/bin/env python
# -*- coding: utf-8 -*-

import os
import sys
import numpy as np
import math
from matplotlib import pyplot as plt

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
        print("Use this command: python SubnetPrefixDistribution.py [AS name] [Dates] [[Show dump]]")
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
    for i in range(0, len(dates)):
        date = dates[i]
        datasetPath = datasetPrefix + date[2] + "/" + date[1] + "/" + date[0] + "/"
        VPPath = datasetPath + "VP.txt"
        datasetPath += ASNumber + "_" + date[0] + "-" + date[1] + ".subnets"
        
        if not os.path.isfile(datasetPath):
            if showDump == "yes":
                print(datasetPath + " does not exist.\n")
            continue
        
        # Code to ignore specific vantage points (for which too few data could be collected)
        if not os.path.isfile(datasetPath):
            if showDump == "yes":
                print(VPPath + " does not exist.\n")
            continue
        
        with open(VPPath) as f:
            vantagePoint = f.read().splitlines()
        
        if vantagePoint[0] == "planet3.cs.huji.ac.il" or vantagePoint[0] == "planet4.cs.huji.ac.il":
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
            continue
        
        totalSubnets = float(len(subnets))
        
        # Computes the amount of subnets for each prefix length (/20 to /32)
        prefixesLength = np.zeros((13,)).tolist()
        for j in range(0, len(subnets)):
            CIDR = subnets[j][0].split('/')
            prefixLen = int(CIDR[1])
            prefixIndex = prefixLen - 20
            if prefixIndex < 0:
                continue # Artifact
            prefixesLength[prefixIndex] += 1
        
        # Re-computes them as a ratio
        ratios = []
        for j in range(0, 13):
            ratios.append(float(prefixesLength[j]) / totalSubnets * 100)
        
        # Display results in console
        if showDump == "yes":
            if i > 0:
                prefixesDump +="\n"
            prefixesDump = "Summary for " + date[0] + "/" + date[1] + "/" + date[2] + "\n"
            prefixesDump += "----------------------\n"
            for j in range(0, 13):
                prefixesDump += "/" + str(j + 20) + ": " + str(prefixesLength[j])
                prefixesDump += " (" + str('%.2f' % ratios[j]) + "%)\n"
            print(prefixesDump)
        
        # Plots a figure
        
        # Fonts for the plot
        hfont = {'fontname':'serif',
                 'fontweight':'bold',
                 'fontsize':28}

        hfont2 = {'fontname':'serif',
                 'fontsize':26}
        
        # Bar charts utilities
        ind = np.arange(13)
        width = 0.8
        padding = 0.1
        
        # Stats plotting
        plt.figure(figsize=(13,10))
        plt.bar(ind, ratios, width, color='#A0A0A0', edgecolor='#000000')
        
        maxRatio = 0
        for j in range(0, len(ratios)):
            if ratios[j] > maxRatio:
                maxRatio = ratios[j]
        
        yLimit = 100
        while yLimit - 10 > maxRatio:
            yLimit -= 10
        
        # Limits and ticks
        plt.ylim([0, yLimit])
        plt.xlim([-0.5, 12.5])
        
        yTicks = range(0, yLimit + 1, 10)
        yTicks.insert(0, 0)
        
        xTicks = ["/20", "/21", "/22", "/23", 
                  "/24", "/25", "/26", "/27", 
                  "/28", "/29", "/30", "/31", "/32"]
        
        plt.yticks(yTicks, **hfont2)
        plt.xticks(ind, xTicks, **hfont2)
        
        # Adds grid to Y axis
        axis = plt.gca()
        yaxis = axis.get_yaxis()
        yticks = yaxis.get_major_ticks()
        yticks[0].label1.set_visible(False)
        yaxis.grid(linestyle='--', color='#000000')
        
        # Labels
        plt.ylabel('Ratio of subnets (total: ' + str(int(totalSubnets)) + ')', **hfont2)
        plt.xlabel('Prefix length', **hfont)
        
        figureFileName = ASNumber + "-" + "-".join(date) + ".pdf"
        plt.savefig(figureFileName)
        plt.clf()
        print("New figure saved in " + figureFileName + ".")
