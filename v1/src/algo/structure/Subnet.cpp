/*
 * Subnet.cpp
 *
 *  Created on: Nov 27, 2018
 *      Author: jefgrailet
 *
 * Implements the class defined in Subnet.h (see this file to learn further about the goals of 
 * such a class).
 */

#include "Subnet.h"

Subnet::Subnet(IPTableEntry *pivot)
{
    initialPivot = (InetAddress) (*pivot);
    prefix = (InetAddress) (*pivot);
    prefixLength = 32;
    interfaces.push_back(new SubnetInterface(pivot, SubnetInterface::SELECTED_PIVOT));
    
    stopDescription = "";
    toPostProcess = false;
}

Subnet::Subnet(list<Subnet*> subnets)
{
    stringstream ss1;
    
    // Creates the final list of SubnetInterface objects while listing, in a string, the CIDR's
    bool guardian = true;
    while(subnets.size() > 0)
    {
        Subnet *curSub = subnets.front();
        subnets.pop_front();
        
        if(!guardian)
            ss1 << ", ";
        else
            guardian = false;
        ss1 << curSub->getCIDR();
        
        list<SubnetInterface*> *curIPs = curSub->getInterfacesList();
        while(curIPs->size() > 0)
        {
            interfaces.push_back(curIPs->front());
            curIPs->pop_front();
        }
        delete curSub;
    }
    interfaces.sort(SubnetInterface::smaller); // By design, the list will not be empty
    
    // Finds the prefix that accomodates all interfaces
    SubnetInterface *first = interfaces.front();
    SubnetInterface *last = interfaces.back();
    
    InetAddress firstIP = (InetAddress) (*(first->ip));
    InetAddress lastIP = (InetAddress) (*(last->ip));
    
    prefixLength = 32;
    unsigned long uintPrefix = firstIP.getULongAddress();
    unsigned long size = 1;
    while(uintPrefix + size < lastIP.getULongAddress())
    {
        prefixLength--;
        size *= 2;
        uintPrefix = uintPrefix >> (32 - prefixLength);
        uintPrefix = uintPrefix << (32 - prefixLength);
    }
    prefix = InetAddress(uintPrefix);
    
    // Gets the first selected pivot to act as the initial one (just to have a set value)
    for(list<SubnetInterface*>::iterator i = interfaces.begin(); i != interfaces.end(); ++i)
    {
        SubnetInterface *curIP = (*i);
        if(curIP->status == SubnetInterface::SELECTED_PIVOT)
        {
            initialPivot = (InetAddress) (*(curIP->ip));
            break;
        }
    }
    
    // Writes the description telling how the subnet was grown
    stringstream ss2;
    ss2 << "merging of undergrown subnets " << ss1.str() << " (similar pivot).";
    stopDescription = ss2.str();
    toPostProcess = false;
}

Subnet::Subnet(Subnet *s1, Subnet *s2)
{
    // Gets CIDR of base fragment (the one with the contra-pivot interface(s)) for the description
    string baseCIDR = "";
    if(s1->hasContrapivots())
        baseCIDR = s1->getCIDR();
    else
        baseCIDR = s2->getCIDR();

    // Recopies interfaces from both subnets
    list<SubnetInterface*> *IPs1 = s1->getInterfacesList();
    list<SubnetInterface*> *IPs2 = s2->getInterfacesList();
    while(IPs1->size() > 0)
    {
        interfaces.push_back(IPs1->front());
        IPs1->pop_front();
    }
    while(IPs2->size() > 0)
    {
        interfaces.push_back(IPs2->front());
        IPs2->pop_front();
    }
    interfaces.sort(SubnetInterface::smaller); // By design, the list will not be empty
    
    // Clears subnets
    delete s1;
    delete s2;
    
    // Finds the prefix that accomodates all interfaces
    SubnetInterface *first = interfaces.front();
    SubnetInterface *last = interfaces.back();
    
    InetAddress firstIP = (InetAddress) (*(first->ip));
    InetAddress lastIP = (InetAddress) (*(last->ip));
    
    prefixLength = 32;
    unsigned long uintPrefix = firstIP.getULongAddress();
    unsigned long size = 1;
    while(uintPrefix + size < lastIP.getULongAddress())
    {
        prefixLength--;
        size *= 2;
        uintPrefix = uintPrefix >> (32 - prefixLength);
        uintPrefix = uintPrefix << (32 - prefixLength);
    }
    prefix = InetAddress(uintPrefix);
    
    // Gets the first selected pivot to act as the initial one (just to have a set value)
    for(list<SubnetInterface*>::iterator i = interfaces.begin(); i != interfaces.end(); ++i)
    {
        SubnetInterface *curIP = (*i);
        if(curIP->status == SubnetInterface::SELECTED_PIVOT)
        {
            initialPivot = (InetAddress) (*(curIP->ip));
            break;
        }
    }
    
    // Writes the description telling how the subnet was grown
    stringstream ss;
    ss << "fragmented subnet re-built during post-processing, using ";
    ss << baseCIDR << " as the base fragment.";
    stopDescription = ss.str();
    toPostProcess = false;
}

Subnet::~Subnet()
{
    this->clearInterfaces();
}

void Subnet::addInterface(IPTableEntry *interface, unsigned short subnetRule)
{
    SubnetInterface *newInterface = new SubnetInterface(interface, subnetRule);
    interfaces.push_back(newInterface);
    interfaces.sort(SubnetInterface::smaller);
}

void Subnet::updateInterface(IPTableEntry *interface, unsigned short subnetRule)
{
    for(list<SubnetInterface*>::iterator i = interfaces.begin(); i != interfaces.end(); ++i)
        if((*i)->ip == interface)
            (*i)->status = subnetRule;
}

IPTableEntry* Subnet::getSelectedPivot()
{
    for(list<SubnetInterface*>::iterator i = interfaces.begin(); i != interfaces.end(); ++i)
        if((*i)->status == SubnetInterface::SELECTED_PIVOT)
            return (*i)->ip;
    return NULL; // To please the compiler. In practice, this should never occur.
}

bool Subnet::hasContrapivots()
{
    for(list<SubnetInterface*>::iterator i = interfaces.begin(); i != interfaces.end(); ++i)
        if((*i)->status == SubnetInterface::CONTRAPIVOT)
            return true;
    return false;
}

unsigned short Subnet::getNbContrapivots()
{
    unsigned short count = 0;
    for(list<SubnetInterface*>::iterator i = interfaces.begin(); i != interfaces.end(); ++i)
        if((*i)->status == SubnetInterface::CONTRAPIVOT)
            count++;
    return count;
}

void Subnet::clearInterfaces()
{
    for(list<SubnetInterface*>::iterator i = interfaces.begin(); i != interfaces.end(); ++i)
        delete (*i);
    interfaces.clear();
}

InetAddress Subnet::getLowerBorder(bool withLimits)
{
    if(prefixLength == 32)
        return initialPivot;

    if(!withLimits)
        return prefix + 1;
    return prefix;
}

InetAddress Subnet::getUpperBorder(bool withLimits)
{
    if(prefixLength == 32)
        return initialPivot;

    // N.B.: method to produce the upper border is identical to common/init/NetworkAddress
    unsigned long mask = ~0;
	mask = mask >> prefixLength;
	InetAddress upperBorderIP = InetAddress(prefix.getULongAddress() ^ mask);
	if(!withLimits)
	    return upperBorderIP - 1;
	return upperBorderIP;
}

bool Subnet::contains(InetAddress interface)
{
    InetAddress lowerBound = prefix;
    InetAddress upperBound = this->getUpperBorder();
    if(interface >= lowerBound && interface <= upperBound)
        return true;
    return false;
}

bool Subnet::overlaps(Subnet *other)
{
    InetAddress lowerBound = prefix;
    InetAddress upperBound = this->getUpperBorder();
    
    InetAddress oLowerBound = other->getLowerBorder();
    InetAddress oUpperBound = other->getUpperBorder();
    
    if(lowerBound <= oUpperBound && oLowerBound <= upperBound)
        return true;
    return false;
}

void Subnet::resetPrefix()
{
    unsigned long mask = ~0;
	mask = mask >> (32 - prefixLength);
	mask = mask << (32 - prefixLength);
    prefix = InetAddress(initialPivot.getULongAddress() & mask);
}

void Subnet::expand()
{
    prefixLength--;
    this->resetPrefix();
}

void Subnet::shrink(list<IPTableEntry*> *out)
{
    prefixLength++;
    this->resetPrefix();
    
    InetAddress newLowBorder = this->getLowerBorder(), newUpBorder = this->getUpperBorder();
    for(list<SubnetInterface*>::iterator i = interfaces.begin(); i != interfaces.end(); ++i)
    {
        SubnetInterface *cur = (*i);
        InetAddress correspondingIP = (InetAddress) *(cur->ip);
        if(correspondingIP < newLowBorder || correspondingIP > newUpBorder)
        {
            if(out != NULL)
                out->push_back(cur->ip);
            delete cur;
            interfaces.erase(i--);
        }
    }
}

void Subnet::adjustSize()
{
    if(prefixLength == 32)
        return;

    // Shrinks up to the point where any listed interface is no longer encompassed
    bool coversEverything = true;
    while(coversEverything && prefixLength < 32)
    {
        prefixLength++;
        this->resetPrefix();
        
        InetAddress newLowBorder = this->getLowerBorder(), newUpBorder = this->getUpperBorder();
        for(list<SubnetInterface*>::iterator i = interfaces.begin(); i != interfaces.end(); ++i)
        {
            InetAddress curIP = (InetAddress) *((*i)->ip);
            if(curIP < newLowBorder || curIP > newUpBorder)
            {
                coversEverything = false;
                break;
            }
        }
    }
    
    // Expands once to undo the last shrinkage
    prefixLength--;
    this->resetPrefix();
}

string Subnet::getCIDR()
{
    stringstream ss;
    ss << prefix << "/" << prefixLength;
    return ss.str();
}

string Subnet::toString()
{
    stringstream ss;
    
    ss << this->getCIDR() << "\n";
    for(list<SubnetInterface*>::iterator i = interfaces.begin(); i != interfaces.end(); ++i)
    {
        SubnetInterface *cur = (*i);
        IPTableEntry *curIP = cur->ip;
        unsigned short dispTTL = (unsigned short) curIP->getTTL();
        ss << dispTTL << " - " << (InetAddress) (*curIP);
        if(dispTTL > 1 && curIP->getTrail() != NULL)
            ss << " - " << (*curIP->getTrail());
        else if(dispTTL == 1)
            ss << " - [This computer]";
        ss << " - ";
        switch(cur->status)
        {
            case SubnetInterface::SELECTED_PIVOT:
                ss << "Selected pivot";
                break;
            case SubnetInterface::CONTRAPIVOT:
                ss << "Contra-pivot";
                break;
            case SubnetInterface::RULE_1_TRAIL:
                ss << "Rule 1";
                break;
            case SubnetInterface::RULE_2_TTL_TIMEOUT:
                ss << "Rule 2";
                break;
            case SubnetInterface::RULE_3_ECHOES:
                ss << "Rule 3";
                break;
            case SubnetInterface::RULE_4_FLICKERING:
                ss << "Rule 4";
                break;
            case SubnetInterface::RULE_5_ALIAS:
                ss << "Rule 5";
                break;
            default:
                ss << "Outlier";
                break;
        }
        ss << "\n";
    }
    if(!stopDescription.empty())
        ss << "Stop: " << stopDescription << "\n";
    
    return ss.str();
}
