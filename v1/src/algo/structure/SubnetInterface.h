/*
 * SubnetInterface.h
 *
 *  Created on: Nov 27, 2018
 *      Author: jefgrailet
 *
 * A simple class to represent a single interface that is part of a subnet.
 * 
 * This class has no connection with the SubnetSiteNode class used in TreeNET/SAGE v1.0.
 */

#ifndef SUBNETINTERFACE_H_
#define SUBNETINTERFACE_H_

#include "IPTableEntry.h"

class SubnetInterface
{
public:

    // Status/rules through which this interface has been added to the subnet
    enum InterfaceStatus
    {
        SELECTED_PIVOT, 
        CONTRAPIVOT, 
        RULE_1_TRAIL, // Same trail as the pivot interface, therefore same subnet
        RULE_2_TTL_TIMEOUT, // Same distance, but one trail got anomalies (= timeout)
        RULE_3_ECHOES, // Trails are echoing target IPs and interfaces are at the same distance
        RULE_4_FLICKERING, // Flickering trails which IPs were aliased together
        RULE_5_ALIAS, // Different trail than the pivot, but IPs were aliased (flickering/warping)
        OUTLIER
    };

    SubnetInterface(IPTableEntry *ip, unsigned short status);
    ~SubnetInterface();
    
    IPTableEntry *ip;
    unsigned short status;
    
    // Comparison method
    static bool smaller(SubnetInterface *i1, SubnetInterface *i2);
};

#endif /* SUBNETINTERFACE_H_ */
