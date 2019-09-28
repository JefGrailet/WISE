#! /usr/bin/env python
# -*- coding: utf-8 -*-

import os
import sys
import numpy as np

def toInt(IP):
    """
    Turns an IP in string format into an integer.
    
    Parameters
    ----------
    IP : string
        The string to convert.
    
    Returns
    -------
    result : int
        The same IP as an integer (32-bit).
    """
    if IP == "This computer":
        return 0
    split = IP.split(".")
    asInt = int(split[0]) * 256 * 256 * 256
    asInt += int(split[1]) * 256 * 256
    asInt += int(split[2]) * 256
    asInt += int(split[3])
    return asInt

def toIP(integer):
    """
    Turns a 32-bit integer corresponding to an IP into the usual string format.
    
    Parameters
    ----------
    integer : int
        The integer to convert.
    
    Returns
    -------
    result : string
        The same IP in string format.
    """
    firstByte = (integer & 0xFF000000) >> 24
    secondByte = (integer & 0x00FF0000) >> 16
    thirdByte = (integer & 0x0000FF00) >> 8
    fourthByte = integer & 0x000000FF
    return str(firstByte) + "." + str(secondByte) + "." + str(thirdByte) + "." + str(fourthByte)

def checkDictionary(dico):
    """
    Checks a dictionary of prefix mappings (prefix -> set of prefixes found in the same 
    neighborhood) and returns False if this dictionary is incomplete (e.g., for any pair of 
    prefixes, the "opposite" pair isn't recorded).
    
    Parameters
    ----------
    dico: dict
        The dictionary to check.
    
    Returns
    -------
    result: boolean
        True if the dictionary is complete (i.e., for any pair, the "opposite" exists).
    """
    for prefix in dico:
        assoc = dico[prefix]
        for secondPrefix in assoc:
            if prefix not in dico[secondPrefix]:
                return False
    return True

def hasPairBeenTested(ip1, ip2, pairRecords):
    """
    Checks if a pair of prefixes has been previously tested.
    
    Parameters
    ----------
    ip1 : string
        First IP of the pair.
    
    ip2 : string
        Second IP of the pair.
    
    pairRecords : set
        The structure used to keep track of the pairs previously considered.
    
    Returns
    -------
    result : boolean
        True if the pair has been already tested. When it returns False, it 
        also records the pair in pairRecords to avoid a duplicate test.
    """
    
    ip1Int = toInt(ip1)
    ip2Int = toInt(ip2)
    
    formattedPair = ""
    if ip1Int < ip2Int:
        formattedPair = ip1 + " - " + ip2
    else:
        formattedPair = ip2 + " - " + ip1
    
    if formattedPair in pairRecords:
        return True
    pairRecords.add(formattedPair)
    return False

if __name__ == "__main__":

    if len(sys.argv) < 3 or len(sys.argv) > 4:
        print("Use this command: python NeighborhoodsValidation.py [groundtruth.txt] [.neighborhoods] [[routers_meshes.txt]]")
        sys.exit()
    
    groundtruthFilePath = str(sys.argv[1])
    neighborhoodsFilePath = str(sys.argv[2])
    routersFilePath = ""
    withGroups = False
    if len(sys.argv) == 4:
        routersFilePath = str(sys.argv[3])
        withGroups = True
    
    # Checks existence of each file
    if not os.path.isfile(groundtruthFilePath):
        print(groundtruthFilePath + " does not exist.")
        sys.exit()
    
    if not os.path.isfile(neighborhoodsFilePath):
        print(neighborhoodsFilePath + " does not exist.")
        sys.exit()
    
    # Gets them raw
    with open(groundtruthFilePath) as f:
        groundtruthRaw = f.read().splitlines()
    
    with open(neighborhoodsFilePath) as f:
        neighborhoodsRaw = f.read().splitlines()
   
    # Parses the groups of routers
    groupsDict = dict()
    groupsSize = dict()
    if withGroups:
        if not os.path.isfile(routersFilePath):
            print(routersFilePath + " does not exist.")
            sys.exit()
        with open(routersFilePath) as f:
            groupsRaw = f.read().splitlines()
        for i in range(0, len(groupsRaw)):
            lineSplit = groupsRaw[i].split(' ')
            for i in range(0, len(lineSplit)):
                ID = lineSplit[i]
                if ID not in groupsDict:
                    groupsDict[ID] = '_'.join(lineSplit)
                    groupsSize[groupsDict[ID]] = len(lineSplit)
    
    # Parses the groundtruth (format: subnet|\t|router_ID), adjusting prefixes in the process
    deviceIDs = dict()
    for i in range(0, len(groundtruthRaw)):
        splitLine = groundtruthRaw[i].split("\t")
        deviceID = splitLine[1]
        # Gets a "merged" ID if deviceID appears in groupsDict
        if withGroups:
            if deviceID in groupsDict:
                deviceID = groupsDict[deviceID]
        prefix = splitLine[0]
        # Adjusts prefix (e.g. X.Y.Z.1/24 => prefix should be X.Y.Z.0)
        truePrefixStr = prefix # No "/" ~ loopback interface (/32)
        splitPrefix = prefix.split("/")
        if len(splitPrefix) == 2:
            intPrefix = toInt(splitPrefix[0])
            prefixLen = int(splitPrefix[1])
            truePrefixInt = (intPrefix >> (32 - prefixLen)) << (32 - prefixLen)
            truePrefixStr = toIP(truePrefixInt)
        if deviceID not in deviceIDs:
            deviceIDs[deviceID] = set()
        if truePrefixStr not in deviceIDs[deviceID]:
            deviceIDs[deviceID].add(truePrefixStr)
    
    # Parses the neighborhoods (only the subnets; peers etc. are irrelevant for this script)
    curNeighborhood = []
    neighborhoods = dict()
    for i in range(0, len(neighborhoodsRaw)):
        if not neighborhoodsRaw[i]:
            if len(curNeighborhood) > 0:
                nID = curNeighborhood[0]
                if nID in neighborhoods:
                    print("Warning: duplicate neighborhood {" + nID + "} !")
                    continue
                neighborhoods[nID] = curNeighborhood[1]
                curNeighborhood = []
            continue
        # Subnet
        if len(curNeighborhood) >= 1 and "/" in neighborhoodsRaw[i]:
            subnet = neighborhoodsRaw[i]
            if " (" in subnet:
                lineSplit = subnet.split(" (")
                subnet = lineSplit[0]
            # In WISE data, the given prefix is always the first IP on the subnet
            splitCIDR = subnet.split("/")
            if len(curNeighborhood) == 1:
                curNeighborhood.append([])
            curNeighborhood[1].append(splitCIDR[0])
        # Neighborhood ID
        elif neighborhoodsRaw[i].startswith("Neighborhood"):
            neighborhoodID = neighborhoodsRaw[i][18:-2]
            curNeighborhood.append(neighborhoodID)
    
    # Reverse dictionary for groundtruth (1st version)
    reverseDevices = dict()
    for ID in deviceIDs:
        prefixes = deviceIDs[ID]
        for prefix in prefixes:
            if prefix not in reverseDevices:
                reverseDevices[prefix] = set()
            if ID not in reverseDevices[prefix]:
                reverseDevices[prefix].add(ID)
    
    # Computes reverse dictionnary of neighborhoods
    reverseNeighborhoods = dict()
    for ID in neighborhoods:
        prefixes = neighborhoods[ID]
        for i in range(0, len(prefixes)):
            prefix = prefixes[i]
            if prefix in reverseNeighborhoods:
                continue
            reverseNeighborhoods[prefix] = ID
    
    # Makes a census of all prefixes appearing in both parts
    commonPrefixes = set()
    for ID in deviceIDs:
        prefixes = deviceIDs[ID]
        for prefix in prefixes:
            if prefix in reverseNeighborhoods:
                if prefix not in commonPrefixes:
                    commonPrefixes.add(prefix)
    
    # Due to the possibility of having a similar or same prefix appearing multiple times in our 
    # groundtruth files, it's possible there are multiple router IDs for a same prefix. This is 
    # of course an issue for validation, because a prefix can appear only in one neighborhood in 
    # a measurement, and due to the way of computing the dictionary used for validation (see 
    # below), this would increase artificially the amount of false negatives. To remove the 
    # ambiguity, we compute a second reverse dictionary for the router IDs such that only the 
    # router ID that matches what we measured will appear in the reverse dictionary (format: 
    # prefix -> router ID).
    
    reverseRouterIDs = dict()
    for prefix in reverseDevices:
        if len(reverseDevices[prefix]) == 1:
            soleItem = list(reverseDevices[prefix])
            reverseRouterIDs[prefix] = soleItem[0]
        elif len(reverseDevices[prefix]) > 1:
            possibleIDs = list(reverseDevices[prefix])
            if prefix not in commonPrefixes:
                continue
            assocPrefixes = neighborhoods[reverseNeighborhoods[prefix]] # Can be only one neighb.
            # Counts occurrences of each possible ID, when present in the neighborhood
            occurrences = []
            dictIDs = dict()
            for i in range(0, len(possibleIDs)):
                occurrences.append(0)
                dictIDs[possibleIDs[i]] = i
            for i in range(0, len(assocPrefixes)):
                if assocPrefixes[i] not in commonPrefixes:
                    continue
                assocID = reverseDevices[assocPrefixes[i]]
                if len(assocID) > 1:
                    continue
                assocIDStr = list(assocID)
                assocIDStr = assocIDStr[0]
                if assocIDStr not in dictIDs:
                    continue
                occurrences[dictIDs[assocIDStr]] += 1
            # Picks the most common router ID, if multiple
            pickedID = possibleIDs[0]
            maxOcc = occurrences[0]
            for i in range(1, len(occurrences)):
                if occurrences[i] > maxOcc:
                    pickedID = possibleIDs[i]
                    maxOcc = occurrences[i]
            reverseRouterIDs[prefix] = pickedID
    
    # Recomputes dictionnaries with pairs of prefixes appearing around the same ID/neighborhood
    groundtruth = dict()
    matchedIDs = set()
    for ID in deviceIDs:
        prefixes = deviceIDs[ID]
        matched = False
        for prefix in prefixes:
            if prefix not in commonPrefixes:
                continue
            matched = True
            if prefix not in groundtruth:
                groundtruth[prefix] = set()
            for oPrefix in prefixes:
                if oPrefix not in commonPrefixes:
                    continue
                if reverseRouterIDs[prefix] != reverseRouterIDs[oPrefix]:
                    # print(reverseRouterIDs[prefix] + " - " + reverseRouterIDs[oPrefix] + ": " + prefix + " & " + oPrefix)
                    continue
                if oPrefix != prefix and oPrefix not in groundtruth[prefix]:
                    groundtruth[prefix].add(oPrefix)
        if matched and ID not in matchedIDs:
            matchedIDs.add(ID)
    
    pairsToTest = dict()
    matchedNeighborhoods = set()
    for ID in neighborhoods:
        prefixes = neighborhoods[ID]
        matched = False
        for i in range(0, len(prefixes)):
            prefix = prefixes[i]
            if prefix not in commonPrefixes:
                continue
            matched = True
            if prefix not in pairsToTest:
                pairsToTest[prefix] = set()
            for j in range(0, len(prefixes)):
                if j == i or prefixes[j] not in commonPrefixes:
                    continue
                if prefixes[j] not in pairsToTest[prefix]:
                    pairsToTest[prefix].add(prefixes[j])
        if matched and ID not in matchedNeighborhoods:
            matchedNeighborhoods.add(ID)
    
    # Checks the dictionaries are solid
    if not checkDictionary(groundtruth):
        print("Groundtruth dictionary is incomplete.")
        sys.exit()
    
    if not checkDictionary(pairsToTest):
        print("Pairs to test dictionary is incomplete.")
        sys.exit()
    
    # Checks now the neighborhoods w.r.t. the groundtruth
    testedPairs = set() # To avoid counting twice a same pair among positives
    testedNonPairs = set() # To avoid counting twice a same pair among negatives
    truePositives = 0
    trueNegatives = 0
    falseNegatives = 0
    falsePositives = 0
    for prefix in commonPrefixes:
        curSet = pairsToTest[prefix]
        if len(curSet) == 0:
            continue
        
        # Counts true and false positives
        for secondPrefix in curSet:
            if hasPairBeenTested(prefix, secondPrefix, testedPairs):
                continue
            if secondPrefix in groundtruth[prefix]: # Same neighborhood
                truePositives += 1
            else: # Not in the same neighborhood
                falsePositives += 1
        
        # Counts true and false negatives
        for secondPrefix in commonPrefixes:
            if secondPrefix == prefix or secondPrefix in curSet:
                continue
            if hasPairBeenTested(prefix, secondPrefix, testedNonPairs):
                continue
            if secondPrefix in groundtruth[prefix]: # Under the same neighborhood but not same ID
                falseNegatives += 1
            else: # Indeed in distinct neighborhoods
                trueNegatives += 1
    
    # For formulas, see https://en.wikipedia.org/wiki/Sensitivity_and_specificity
    truePositiveRate = float(truePositives) / (float(truePositives) + float(falseNegatives)) * 100
    trueNegativeRate = float(trueNegatives) / (float(trueNegatives) + float(falsePositives)) * 100
    falsePositiveRate = float(falsePositives) / (float(trueNegatives) + float(falsePositives)) * 100
    falseNegativeRate = float(falseNegatives) / (float(truePositives) + float(falseNegatives)) * 100
    
    precision = float(truePositives) / (float(truePositives) + float(falsePositives)) * 100
    negativePredictiveValue = float(trueNegatives) / (float(trueNegatives) + float(falseNegatives)) * 100
    falseDiscoveryRate = float(falsePositives) / (float(truePositives) + float(falsePositives)) * 100
    
    accuracy = float(truePositives) + float(trueNegatives)
    accuracy /= (float(truePositives) + float(falsePositives) + float(trueNegatives) + float(falseNegatives))
    accuracy *= 100
    
    # Computes some additional details
    bestEffortNeighborhoods = 0
    for ID in matchedNeighborhoods:
        if "|" in ID or "Echo" in ID:
            bestEffortNeighborhoods += 1
    ratioBestEffort = (float(bestEffortNeighborhoods) / float(len(matchedNeighborhoods))) * 100
    
    nbMatchedIDs = 0
    nbExactMatches = 0
    for ID in matchedIDs:
        prefixes = deviceIDs[ID]
        if ID not in groupsSize:
            nbMatchedIDs += 1
        else:
            nbMatchedIDs += groupsSize[ID]
        prevNID = ""
        fullMatch = True
        for prefix in prefixes:
            if prefix not in commonPrefixes:
                continue
            if not prevNID:
                prevNID = reverseNeighborhoods[prefix]
            elif reverseNeighborhoods[prefix] != prevNID:
                fullMatch = False
                break
        if fullMatch:
            nbExactMatches += 1
    ratioExact = float(nbExactMatches) / float(len(matchedIDs)) * 100
    
    # Prints out everything
    print("Prefixes common to both datasets: " + str(len(commonPrefixes)))
    print("Router IDs that could be matched: " + str(nbMatchedIDs))
    print("Matched neighborhoods: " + str(len(matchedNeighborhoods)))
    print("Exact matches: " + str(nbExactMatches) + " (" + ('%.2f' % ratioExact) + "%)")
    print("\"Best effort\" (matched) neighborhoods: " + str(bestEffortNeighborhoods) + " (" + str('%.2f' % ratioBestEffort) + "%)")
    
    print("\nTrue positive rate: " + str("%.3f" % truePositiveRate) + "%")
    print("True negative rate: " + str("%.3f" % trueNegativeRate) + "%")
    print("False positive rate: " + str("%.3f" % falsePositiveRate) + "%")
    print("False negative rate: " + str("%.3f" % falseNegativeRate) + "%")
    
    print("\nPrecision (positive predictive value): " + str("%.3f" % precision) + "%")
    print("Negative predictive value: " + str("%.3f" % negativePredictiveValue) + "%")
    print("False discovery rate: " + str("%.3f" % falseDiscoveryRate) + "%")
    print("Accuracy: " + str("%.3f" % accuracy) + "%")
