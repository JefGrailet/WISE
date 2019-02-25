/*
 * SubnetInterface.cpp
 *
 *  Created on: Nov 27, 2018
 *      Author: jefgrailet
 *
 * Implements the class defined in SubnetInterface.h (see this file to learn further about the 
 * goals of such a class).
 */

#include "SubnetInterface.h"

SubnetInterface::SubnetInterface(IPTableEntry *ip, unsigned short status)
{
    this->ip = ip;
    this->status = status;
}

SubnetInterface::~SubnetInterface()
{
}

bool SubnetInterface::smaller(SubnetInterface *i1, SubnetInterface *i2)
{
    return IPTableEntry::compare(i1->ip, i2->ip);
}
