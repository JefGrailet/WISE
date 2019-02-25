/*
 * Trail.cpp
 *
 *  Created on: Aug 30, 2018
 *      Author: jefgrailet
 *
 * Implements the class defined in Trail.h (see this file to learn further about the goals of such 
 * a class).
 */

#include "Trail.h"

Trail::Trail(InetAddress lastValidIP, unsigned short nbAnomalies)
{
    this->lastValidIP = lastValidIP;
    this->nbAnomalies = nbAnomalies;
    
    warping = false;
    flickering = false;
    echoing = false;
}

Trail::Trail(InetAddress lastValidIP)
{
    this->lastValidIP = lastValidIP;
    nbAnomalies = 0;
    
    warping = false;
    flickering = false;
    echoing = false;
}

Trail::Trail(unsigned short routeLength)
{
    lastValidIP = InetAddress(0);
    nbAnomalies = routeLength;
    
    warping = false;
    flickering = false;
    echoing = false;
}

Trail::~Trail()
{
}

bool Trail::equals(Trail *other)
{
    if(lastValidIP == other->getLastValidIP() && nbAnomalies == other->getNbAnomalies())
        return true;
    return false;
}
