#! /usr/bin/env python
# -*- coding: utf-8 -*-

import os
import sys
import numpy as np
from matplotlib import pyplot as plt

if __name__ == "__main__":

    if len(sys.argv) < 3 or len(sys.argv) > 4:
        print("Use this command: python NeighborhoodsAnalysis.py [AS] [Dates] [[prefix output file]]")
        sys.exit()
    
    ASNumber = str(sys.argv[1])
    datesFilePath = str(sys.argv[2])
    labelOutput = "Neighborhoods"
    if len(sys.argv) == 4:
        labelOutput = str(sys.argv[3])

    # Checks existence of the required file
    if not os.path.isfile(datesFilePath):
        print(datesFilePath + " does not exist.")
        sys.exit()
    
    # Parses dates
    with open(datesFilePath) as f:
        datesRaw = f.read().splitlines()
    dates = []
    for i in range(0, len(datesRaw)):
        splitDate = datesRaw[i].split('/')
        dates.append(splitDate)
    
    # TODO: change datasetPrefix so it matches your own filesystem.
    datasetPrefix = "/home/jefgrailet/PhD/Campaigns/WISE/"
    fullList = [] # I.e., of neighborhoods
    maxOffset = 0 # Max offset for a peer, overall
    maxPeers = 0 # Max amount of peers for one neighborhood, overall
    maxPeersDate = ""
    maxPeersID = ""
    maxSubnets = 0
    maxSubnetsDate = ""
    maxSubnetsID = ""
    
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
    maxSubnetsStr += maxSubnetsID + "} on " + maxSubnetsDate + "."
    print(maxSubnetsStr)
    
    maxPeersStr = "Maximum amount of peers (" + str(maxPeers) + ") found for {"
    maxPeersStr += maxPeersID + "} on " + maxPeersDate + "."
    print(maxPeersStr)
    
    # Counts neighborhoods having peers and classifies them by offset for peers
    peersByDist = []
    peersByDistRatios = []
    for i in range(0, maxOffset + 1):
        peersByDist.append(0)
        peersByDistRatios.append(0.0)
    
    totalWeight = 0
    for i in range(0, len(fullList)):
        curNeighborhood = fullList[i]
        totalWeight += len(curNeighborhood[1])
        if len(curNeighborhood[2]) > 0:
            peersByDist[curNeighborhood[3]] += len(curNeighborhood[1])
    
    for i in range(0, len(peersByDist)):
        peersByDistRatios[i] = float(peersByDist[i]) / totalWeight
    
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
    
    hfont3 = {'fontname':'serif',
             'fontsize':16}
    
    hfont4 = {'fontname':'serif',
             'fontsize':12}

    plt.figure(figsize=(13,9))
    
    plt.ylim([0, 1])
    plt.xlim([0, maxOffset + 1])
    plt.plot(xAxis, CDF, color='#000000', linewidth=3)
    rangeOfValues = np.arange(0, maxOffset + 2, 1)
    if len(rangeOfValues) > 20:
        plt.xticks(rangeOfValues, **hfont4)
    else:
        plt.xticks(rangeOfValues, **hfont3)
    plt.yticks(np.arange(0, 1.1, 0.1), **hfont2)
    
    axis = plt.gca()
    yaxis = axis.get_yaxis()
    yticks = yaxis.get_major_ticks()
    yticks[0].label1.set_visible(False)
    
    plt.ylabel('CDF with weighed neighborhoods', **hfont)
    plt.xlabel('Distance in TTL between neighborhoods and peers', **hfont)
    plt.grid()

    plt.savefig(labelOutput + "_peers_distance.pdf")
    plt.clf()
    print("CDF of peers distance saved in " + labelOutput + "_peers_distance.pdf.")
    
    # Counts neighborhoods in function of their amount of peers and computes % per amount of peers
    totalByNbPeers = []
    totalByNbPeersRatios = []
    for i in range(0, maxPeers + 1):
        totalByNbPeers.append(0)
        totalByNbPeersRatios.append(0.0)
        
    for i in range(0, len(fullList)):
        curNbPeers = len(fullList[i][2])
        totalByNbPeers[curNbPeers] += len(fullList[i][1])
    
    for i in range(0, len(totalByNbPeers)):
        totalByNbPeersRatios[i] = float(totalByNbPeers[i]) / totalWeight
    
    # Computes a CDF with the ratios, omitting the neighborhoods without peers
    xAxis = np.arange(0, maxPeers + 1, 1) # Renewed axis
    CDFBis = []
    for i in range(0, len(totalByNbPeersRatios)):
        curValue = 0
        if len(CDFBis) > 1: # Omits [0], because it corresponds to neighborhoods without peers
            curValue = CDFBis[i - 1]
        curValue += totalByNbPeersRatios[i]
        CDFBis.append(curValue)
    
    # Starts plotting
    plt.figure(figsize=(13,9))
    
    nextPowerOfTen = 1
    while nextPowerOfTen < maxPeers:
        nextPowerOfTen *= 10
    
    plt.semilogx(xAxis, CDFBis, color='#000000', linewidth=3)
    plt.ylim([0, 1])
    plt.xlim([1, maxPeers])
    plt.yticks(np.arange(0, 1.1, 0.1), **hfont2)
    
    # Ticks for the X axis (log scale)
    tickValues = []
    tickDisplay = []
    tickValues.append(1)
    tickDisplay.append(1)
    if maxPeers > 10:
        power = 1
        exponent = 0
        while power < maxPeers:
            power *= 10
            exponent +=1
            if power < maxPeers:
                tickValues.append(power)
                tickDisplay.append(r"$10^{{ {:1d} }}$".format(exponent))
    else:
        for i in range(2, maxPeers + 1):
            tickValues.append(i)
            tickDisplay.append(i)
    plt.xticks(tickValues, tickDisplay, **hfont2)
    
    axis = plt.gca()
    yaxis = axis.get_yaxis()
    xaxis = axis.get_xaxis()
    
    # Nit: hides minor ticks that automatically appear (e.g. if maxPeers < 10)
    xticks = xaxis.get_minor_ticks()
    for i in range(0, len(xticks)):
        xticks[i].label1.set_visible(False)
    
    # Nit: hides the 0 from the ticks of y axis
    yticks = yaxis.get_major_ticks()
    yticks[0].label1.set_visible(False)
    
    plt.ylabel('PDF with weighed neighborhoods', **hfont)
    plt.xlabel('Amount of peers', **hfont)
    plt.grid()

    plt.savefig(labelOutput + "_peers_nb.pdf")
    plt.clf()
    print("CDF of amount of peers saved in " + labelOutput + "_peers_nb.pdf.")
