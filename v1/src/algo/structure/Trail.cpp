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
    
    direct = true;
    if(nbAnomalies > 0)
        direct = false;
    
    warping = false;
    flickering = false;
    echoing = false;
}

Trail::Trail(InetAddress lastValidIP)
{
    this->lastValidIP = lastValidIP;
    nbAnomalies = 0;
    
    direct = true;
    warping = false;
    flickering = false;
    echoing = false;
}

Trail::Trail(unsigned short routeLength)
{
    lastValidIP = InetAddress(0);
    nbAnomalies = routeLength;
    
    direct = false;
    warping = false;
    flickering = false;
    echoing = false;
}

Trail::~Trail()
{
}

bool Trail::compare(Trail *t1, Trail *t2)
{
    if(t1->lastValidIP == t2->lastValidIP)
        return t1->nbAnomalies < t2->nbAnomalies;
    return t1->lastValidIP < t2->lastValidIP;
}

string Trail::toString()
{
    stringstream ss;
    ss << (*this);
    return ss.str();
}

bool Trail::equals(Trail *other)
{
    if(lastValidIP == other->getLastValidIP() && nbAnomalies == other->getNbAnomalies())
        return true;
    return false;
}
