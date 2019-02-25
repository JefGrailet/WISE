#! /usr/bin/env python
# -*- coding: utf-8 -*-

import os
import sys
import numpy as np
from matplotlib import pyplot as plt

if __name__ == "__main__":

    if len(sys.argv) < 4:
        print("Use this command: python GraphicalTrailAnalysis.py [AS] [Dates] [[Show dump ?]]")
        sys.exit()
    
    AS = str(sys.argv[1])
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
    datasetPrefix = "/home/jefgrailet/PhD/Campaigns/WISE"
    validDatasets = []
    ratiosWarping = []
    ratiosFlickering = []
    ratiosEchoes = []
    for d in range(0, len(dates)):
        curDate = dates[d]
        IPFilePath = datasetPrefix + "/" + AS + "/" + curDate[2] + "/" + curDate[1] + "/"
        IPFilePath += curDate[0] + "/" + AS + "_" + curDate[0] + "-" + curDate[1] + ".ips"
        
        # Checks existence of the .ips file
        if not os.path.isfile(IPFilePath):
            if showDump == "yes":
                print(IPFilePath + " does not exist.\n")
            continue
        
        # Parses the .ips file
        with open(IPFilePath) as f:
            IPsRaw = f.read().splitlines()
        
        # Sets to maintain unique trails of each kind (or all trails)
        trails = set()
        flickeringTrails = set()
        echoingTrails = set()
        warpingTrails = set()
        
        # Various dictionaries to get the occurrences of each trails + different TTLs for warping
        trailsOcc = dict()
        trailsDist = dict()
        trailsDistOcc = dict()
        for i in range(0, len(IPsRaw)):
            if "Scan failure" in IPsRaw[i] or "-" not in IPsRaw[i]:
                continue
            
            notATarget = False
            if "Part of a trail" in IPsRaw[i]:
                notATarget = True
            toSplit = IPsRaw[i]
            labels = ""
            if "Echoing" in toSplit or "Warping" in toSplit or "Flickering" in toSplit:
                zeroSplit = toSplit.rsplit(' | ', 1)
                toSplit = zeroSplit[0]
                labels = zeroSplit[1]
            firstSplit = toSplit.split(' - ')
            secondSplit = firstSplit[1].split(' ', 1)
            
            IP = firstSplit[0]
            TTL = secondSplit[0]
            
            # Adding the trail for this IP to dictionaries
            if not notATarget:
                trail = secondSplit[1]
                if trail not in trails:
                    trails.add(trail)
                    trailsDist[trail] = set()
                    trailsDistOcc[trail] = dict()
                    trailsOcc[trail] = 0
                trailsOcc[trail] += 1
                if TTL not in trailsDist[trail]:
                    trailsDist[trail].add(TTL)
                    trailsDistOcc[trail][TTL] = 0
                trailsDistOcc[trail][TTL] += 1
            
            # Checkings the labels
            if labels != "":
                IPAsTrail = "[" + IP + "]"
                if IPAsTrail not in trails:
                    trails.add(IPAsTrail)
                    trailsDist[IPAsTrail] = set()
                    trailsDistOcc[IPAsTrail] = dict()
                    trailsOcc[IPAsTrail] = 0
                if "Flickering" in labels and IPAsTrail not in flickeringTrails:
                    flickeringTrails.add(IPAsTrail)
                if "Echoing" in labels and IPAsTrail not in echoingTrails:
                    echoingTrails.add(IPAsTrail)
                if "Warping" in labels and IPAsTrail not in warpingTrails:
                    warpingTrails.add(IPAsTrail)
        
        # Having no trails is seen as the same as not having an .ips file
        if len(trails) == 0:
            if showDump == "yes":
                print(IPFilePath + " does not have any trail to analyze.\n")
            continue
        
        validDatasets.append(curDate)
        
        # Mean amount of differing TTLs for all warping IPs
        accTTLs = 0
        for trail in trails:
            if len(trailsDist[trail]) > 1:
                accTTLs += len(trailsDist[trail])
        
        meanTTLs = 1
        if len(warpingTrails) > 0:
            meanTTLs = float(accTTLs) / float(len(warpingTrails))
        
        # Stats
        totalWarping = 0
        totalFlickering = 0
        totalEchoes = 0
        for trail in trails:
            if trail in warpingTrails:
                totalWarping += trailsOcc[trail]
            if trail in flickeringTrails:
                totalFlickering += trailsOcc[trail]
            if trail in echoingTrails:
                totalEchoes += trailsOcc[trail]
        ratioWarping = (float(totalWarping) / float(len(IPsRaw))) * 100
        ratioFlickering = (float(totalFlickering) / float(len(IPsRaw))) * 100
        ratioEchoes = (float(totalEchoes) / float(len(IPsRaw))) * 100
        
        ratioWarpingUnique = (float(len(warpingTrails)) / float(len(trails))) * 100
        ratioFlickeringUnique = (float(len(flickeringTrails)) / float(len(trails))) * 100
        ratioEchoingUnique = (float(len(echoingTrails)) / float(len(trails))) * 100
        
        if showDump == "yes":
            dump = "On " + '/'.join(curDate) + ":\n"
            dump += "Total of unique trails: " + str(len(trails)) + "\n"
            dump += "Flickering trails: " + str(totalFlickering) + " (" + str('%.2f' % ratioFlickering) + "%)\n"
            dump += "Warping trails: " + str(totalWarping) + " (" +  str('%.2f' % ratioWarping) + "%)\n"
            dump += "Echoing trails: " + str(totalEchoes) + " (" + str('%.2f' % ratioEchoes) + "%)\n"
            dump += "Mean amount of different TTLs (warping IPs): " + str('%.2f' % meanTTLs) + "\n"
            dump += "Unique flickering IPs: " + str(len(flickeringTrails)) + " (" + str('%.2f' % ratioFlickeringUnique) + "%)\n"
            dump += "Unique warping IPs: " + str(len(warpingTrails)) + " (" + str('%.2f' % ratioWarpingUnique) + "%)\n"
            dump += "Unique echoing IPs: " + str(len(echoingTrails)) + " (" + str('%.2f' % ratioEchoingUnique) + "%)\n"
            print(dump)
        
        ratiosWarping.append(ratioWarping)
        ratiosFlickering.append(ratioFlickering)
        ratiosEchoes.append(ratioEchoes)
    
    if len(validDatasets) < 2:
        print("Not enough data to plot a figure.")
        sys.exit()
    
    # Plots result
    hfont = {'fontname':'serif',
             'fontweight':'bold',
             'fontsize':28}

    hfont2 = {'fontname':'serif',
             'fontsize':26}

    hfont3 = {'fontname':'serif',
             'fontsize':15}

    plt.figure(figsize=(13,9))
    
    xAxis = range(0, len(validDatasets), 1)
    plt.plot(xAxis, ratiosWarping, color='#000000', linewidth=3, label="Warping")
    plt.plot(xAxis, ratiosFlickering, color='#000000', linewidth=3, linestyle=':', label="Flickering")
    plt.plot(xAxis, ratiosEchoes, color='#000000', linewidth=3, linestyle='--', label="Echoing")
    plt.rcParams.update({'font.size': 20})
    
    plt.ylim([0, 100])
    plt.xlim([0, len(validDatasets) - 1])
    plt.yticks(np.arange(0, 110, 10), **hfont2)
    axis = plt.gca()
    yaxis = axis.get_yaxis()
    yticks = yaxis.get_major_ticks()
    yticks[0].label1.set_visible(False)
    
    ticksForX = []
    for i in range(0, len(validDatasets)):
        ticksForX.append(validDatasets[i][0] + "/" + validDatasets[i][1])
    plt.xticks(xAxis, ticksForX, rotation=30, **hfont3)
    
    plt.ylabel('Ratio among trails (%)', **hfont)
    plt.xlabel('Measurement', **hfont)
    
    plt.grid()
    
    plt.legend(bbox_to_anchor=(0, 1.02, 1.0, .102), 
               loc=3,
               ncol=4, 
               mode="expand", 
               borderaxespad=0.,
               fontsize=26)
    
    firstDataset = '-'.join(validDatasets[0])
    lastDataset = '-'.join(validDatasets[len(validDatasets) - 1])
    figureFileName = AS + ".pdf" # + "_" + firstDataset + "_to_" + lastDataset + ".pdf"
    plt.savefig(figureFileName)
    plt.clf()
    print("New figure saved in " + figureFileName + ".")
