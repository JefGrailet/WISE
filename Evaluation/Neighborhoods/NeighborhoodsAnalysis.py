#! /usr/bin/env python
# -*- coding: utf-8 -*-

import os
import sys
import numpy as np
from matplotlib import pyplot as plt

if __name__ == "__main__":

    if len(sys.argv) < 2:
        print("Use this command: python NeighborhoodsAnalysis.py [ASes] [Dates] [[prefix output file]]")
        sys.exit()
    
    ASesFilePath = str(sys.argv[1])
    datesFilePath = str(sys.argv[2])
    labelOutput = "Neighborhoods"
    if len(sys.argv) == 4:
        labelOutput = str(sys.argv[3])

    # Checks existence of each required file
    if not os.path.isfile(ASesFilePath):
        print(ASesFilePath + " does not exist.")
        sys.exit()
    
    if not os.path.isfile(datesFilePath):
        print(datesFilePath + " does not exist.")
        sys.exit()
    
    # Parses ASes and dates
    with open(ASesFilePath) as f:
        ASes = f.read().splitlines()
    
    with open(datesFilePath) as f:
        datesRaw = f.read().splitlines()
    dates = []
    for i in range(0, len(datesRaw)):
        splitDate = datesRaw[i].split('/')
        dates.append(splitDate)
    
    # TODO: change datasetPrefix so this fits your computer.
    datasetPrefix = "/home/jefgrailet/PhD/Campaigns/WISE/"
    fullList = [] # I.e., of neighborhoods
    maxOffset = 0 # Max offset for a peer, overall
    maxPeers = 0 # Max amount of peers for one neighborhood, overall
    maxPeersAS = ""
    maxPeersDate = ""
    maxPeersID = ""
    maxSubnets = 0
    maxSubnetsAS = ""
    maxSubnetsDate = ""
    maxSubnetsID = ""
    for indexAS in range(0, len(ASes)):
        ASNumber = ASes[indexAS]
        ASFolderPrefix = datasetPrefix + ASNumber + "/"
        for indexDate in range(0, len(dates)):
            date = dates[indexDate]
            datasetPath = ASFolderPrefix + date[2] + "/" + date[1] + "/" + date[0] + "/"
            datasetPath += ASNumber + "_" + date[0] + "-" + date[1] + ".neighborhoods"
            
            # Checks existence of the .neighborhoods file
            if not os.path.isfile(datasetPath):
                print(datasetPath + " does not exist.")
                sys.exit()
            
            # Parses the .neighborhoods file
            with open(datasetPath) as f:
                neighborhoodsRaw = f.read().splitlines()
            
            # Processes the lines of the file
            curNeighborhood = []
            neighborhoods = []
            nIDs = set()
            for i in range(0, len(neighborhoodsRaw)):
                if not neighborhoodsRaw[i]:
                    if len(curNeighborhood) > 0:
                        nbSubnets = len(curNeighborhood[1])
                        if nbSubnets > maxSubnets:
                            maxSubnets = nbSubnets
                            # Details about the neighborhood having max. amount of subnets
                            maxSubnetsAS = ASNumber
                            maxSubnetsDate = '/'.join(date)
                            maxSubnetsID = curNeighborhood[0]
                        neighborhoods.append(curNeighborhood)
                        curNeighborhood = []
                    continue
                # Subnet
                if len(curNeighborhood) >= 1 and "/" in neighborhoodsRaw[i]:
                    subnet = neighborhoodsRaw[i]
                    if " (" in subnet:
                        lineSplit = subnet.split(" (")
                        subnet = lineSplit[0]
                    if len(curNeighborhood) > 1:
                        curNeighborhood[1].append(subnet)
                    else:
                        curNeighborhood.append([subnet]) # [1] = list of aggregated subnets
                # Peers
                elif not neighborhoodsRaw[i].startswith("Neighborhood"):
                    curNeighborhood.append([]) # [2] = peers (empty list = no peers could be found)
                    curNeighborhood.append(0) # [3] = offset of peers (0 = direct peer)
                    if "offset" in neighborhoodsRaw[i]:
                        lineSplit = neighborhoodsRaw[i].split("(offset=")
                        if len(lineSplit) == 2: # Just in case
                            curNeighborhood[3] = int(lineSplit[1][:-1])
                            if curNeighborhood[3] > maxOffset:
                                maxOffset = curNeighborhood[3]
                    if neighborhoodsRaw[i].startswith("Peer: "):
                        curNeighborhood[2].append(neighborhoodsRaw[i][6:])
                    elif neighborhoodsRaw[i].startswith("Peers: "):
                        peersList = neighborhoodsRaw[i][7:].split(", ")
                        nbPeers = len(peersList)
                        if nbPeers > maxPeers:
                            maxPeers = nbPeers
                            # Details about the neighborhood having max. amount of peers
                            maxPeersAS = ASNumber
                            maxPeersDate = '/'.join(date)
                            maxPeersID = curNeighborhood[0]
                        curNeighborhood[2].extend(peersList)
                # Neighborhood {} notation
                else:
                    neighborhoodID = neighborhoodsRaw[i][18:-2]
                    curNeighborhood.append(neighborhoodID) # [0] = neighborhood ID
                    if neighborhoodID not in nIDs:
                        nIDs.add(neighborhoodID)
                    else:
                        print("Duplicate neighborhood: {" + neighborhoodID + "}")
            
            if len(neighborhoods) == 0:
                dateStr = '/'.join(date)
                print("No neighborhood could be parsed for " + ASNumber + " on " + dateStr + ".")
                continue
            else:
                fullList.extend(neighborhoods)
    
    maxSubnetsStr = "Maximum amount of subnets (" + str(maxSubnets) + ") found for {"
    maxSubnetsStr += maxSubnetsID + "} in " + maxSubnetsAS + " on " + maxSubnetsDate + "."
    print(maxSubnetsStr)
    
    maxPeersStr = "Maximum amount of peers (" + str(maxPeers) + ") found for {"
    maxPeersStr += maxPeersID + "} in " + maxPeersAS + " on " + maxPeersDate + "."
    print(maxPeersStr)
    
    # Counts neighborhoods having peers and classifies them by offset for peers
    peersByDist = []
    peersByDistRatios = []
    for i in range(0, maxOffset + 1):
        peersByDist.append(0)
        peersByDistRatios.append(0.0)
    
    for i in range(0, len(fullList)):
        curNeighborhood = fullList[i]
        if len(curNeighborhood[2]) > 0:
            peersByDist[curNeighborhood[3]] += 1
    
    totalNeighborhoods = float(len(fullList))
    for i in range(0, len(peersByDist)):
        peersByDistRatios[i] = float(peersByDist[i]) / totalNeighborhoods
    
    # Computes a CDF with that
    xAxis = np.arange(0, maxOffset + 2, 1) # +2 because +1 stops the range a [0 ... maxOffset]
    CDF = []
    CDF.append(0)
    for i in range(0, len(peersByDistRatios)):
        curValue = CDF[i]
        curValue += peersByDistRatios[i]
        CDF.append(curValue)
    
    # Plots result
    hfont = {'fontname':'serif',
             'fontweight':'bold',
             'fontsize':26}

    hfont2 = {'fontname':'serif',
             'fontsize':22}

    plt.figure(figsize=(13,9))
    
    plt.ylim([0, 1])
    plt.xlim([0, maxOffset + 1])
    plt.plot(xAxis, CDF, color='#000000', linewidth=3)
    plt.xticks(np.arange(0, maxOffset + 2, 1), **hfont2)
    plt.yticks(np.arange(0, 1.1, 0.1), **hfont2)
    
    axis = plt.gca()
    yaxis = axis.get_yaxis()
    yticks = yaxis.get_major_ticks()
    yticks[0].label1.set_visible(False)
    
    plt.ylabel('Cumulative distribution function (CDF)', **hfont)
    plt.xlabel('Distance in TTL between neighborhoods and peers', **hfont)
    plt.grid()

    plt.savefig(labelOutput + "_peers_distance.pdf")
    plt.clf()
    print("CDF of peers distance saved in " + labelOutput + "_peers_distance.pdf.")
    
    # Counts neighborhoods by how many peers they have to plot this as a PDF
    totalByNbPeers = []
    for i in range(0, maxPeers + 1):
        totalByNbPeers.append(0)
        
    for i in range(0, len(fullList)):
        curNbPeers = len(fullList[i][2])
        totalByNbPeers[curNbPeers] += 1
    
    maxNeighborhoods = 0
    for i in range(0, len(totalByNbPeers)):
        curAmount = totalByNbPeers[i]
        if curAmount > maxNeighborhoods:
            maxNeighborhoods = curAmount
    
    # Starts plotting
    xAxis = np.arange(0, maxPeers + 1, 1) # Renewed axis
    plt.figure(figsize=(13,9))
    
    nextPowerOfTenY = 1
    while nextPowerOfTenY < maxNeighborhoods:
        nextPowerOfTenY *= 10
    
    nextPowerOfTenX = 1
    while nextPowerOfTenX < maxPeers:
        nextPowerOfTenX *= 10
    
    plt.loglog(xAxis, totalByNbPeers, color='#000000', linewidth=3, linestyle=':')
    plt.ylim([1, nextPowerOfTenY])
    plt.xlim([1, nextPowerOfTenX])
    
    # Ticks for the X axis (log scale)
    tickValuesX = []
    tickDisplayX = []
    tickValuesX.append(1)
    tickDisplayX.append(1)
    power = 1
    exponent = 0
    while power < maxPeers:
        power *= 10
        exponent += 1
        tickValuesX.append(power)
        tickDisplayX.append(r"$10^{{ {:1d} }}$".format(exponent))
    
    tickValuesY = []
    tickDisplayY = []
    power = 1
    exponent = 0
    while power < maxNeighborhoods:
        power *= 10
        exponent += 1
        tickValuesY.append(power)
        tickDisplayY.append(r"$10^{{ {:1d} }}$".format(exponent))
    
    plt.yticks(tickValuesY, tickDisplayY, **hfont2)
    plt.xticks(tickValuesX, tickDisplayX, **hfont2)
    
    plt.ylabel('Probability density function (PDF)', **hfont)
    plt.xlabel('Amount of peers', **hfont)
    plt.grid()

    plt.savefig(labelOutput + "_peers_nb.pdf")
    plt.clf()
    print("PDF of amount of peers saved in " + labelOutput + "_peers_nb.pdf.")
    
    # Makes a census of neighborhoods by size (i.e., # aggregated subnets)
    totalByNbSubnets = []
    for i in range(0, maxSubnets + 1):
        totalByNbSubnets.append(0)
    
    for i in range(0, len(fullList)):
        curNbSubnets = len(fullList[i][1])
        totalByNbSubnets[curNbSubnets] += 1
    
    maxNeighborhoods = 0
    for i in range(0, len(totalByNbSubnets)):
        curAmount = totalByNbSubnets[i]
        if curAmount > maxNeighborhoods:
            maxNeighborhoods = curAmount
    
    # Plots this as PDF with log scale
    xAxis = np.arange(0, maxSubnets + 1, 1) # Renewed axis
    plt.figure(figsize=(13,9))
    
    nextPowerOfTenY = 1
    while nextPowerOfTenY < maxNeighborhoods:
        nextPowerOfTenY *= 10
    
    nextPowerOfTenX = 1
    while nextPowerOfTenX < maxSubnets:
        nextPowerOfTenX *= 10
    
    plt.loglog(xAxis, totalByNbSubnets, color='#000000', linewidth=3, linestyle=':')
    plt.ylim([1, nextPowerOfTenY])
    plt.xlim([1, nextPowerOfTenX])
    
    # Ticks for the X axis (log scale)
    tickValuesX = []
    tickDisplayX = []
    tickValuesX.append(1)
    tickDisplayX.append(1)
    power = 1
    exponent = 0
    while power < maxSubnets:
        power *= 10
        exponent +=1
        tickValuesX.append(power)
        tickDisplayX.append(r"$10^{{ {:1d} }}$".format(exponent))
    
    tickValuesY = []
    tickDisplayY = []
    power = 1
    exponent = 0
    while power < maxNeighborhoods:
        power *= 10
        exponent += 1
        tickValuesY.append(power)
        tickDisplayY.append(r"$10^{{ {:1d} }}$".format(exponent))
    
    plt.yticks(tickValuesY, tickDisplayY, **hfont2)
    plt.xticks(tickValuesX, tickDisplayX, **hfont2)
    
    plt.ylabel('Probability density function (PDF)', **hfont)
    plt.xlabel('Amount of aggregated subnets', **hfont)
    plt.grid()

    plt.savefig(labelOutput + "_size.pdf")
    plt.clf()
    print("PDF of amount of aggregated subnets saved in " + labelOutput + "_size.pdf.")
