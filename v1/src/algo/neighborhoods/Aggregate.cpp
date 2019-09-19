/*
 * Aggregate.cpp
 *
 *  Created on: Aug 22, 2019
 *      Author: jefgrailet
 *
 * Implements the class defined in Aggregate.h (see this file to learn further about the goals of 
 * such a class).
 */

#include "Aggregate.h"

Aggregate::Aggregate(Subnet *first)
{
    preEchoingOffset = 0;
    peersOffset = 0;
    this->addSubnet(first);
}

Aggregate::Aggregate(list<Subnet*> newSubnets)
{
    preEchoingOffset = 0;
    peersOffset = 0;
    this->addSubnets(newSubnets);
}

Aggregate::~Aggregate()
{
    // Nothing deleted here; every pointed object is destroyed elsewhere (e.g. Environment class)
}

void Aggregate::updateTrails(Trail *newTrail)
{
    if(newTrail == NULL) // "Home" subnet
        return;
    if(newTrail->isEchoing())
        return;

    /*
     * Does the new subnet have a flickering trail, and if yes, is it different than what we 
     * already know ? Because in that case, we need to update "trails" accordingly.
     */

    if(newTrail->isFlickering())
    {
        for(list<Trail*>::iterator i = trails.begin(); i != trails.end(); ++i)
        {
            Trail *curTrail = (*i);
            if(newTrail->equals(curTrail))
                return;
        }
        trails.push_back(newTrail);
        trails.sort(Trail::compare);
    }
    else if(trails.size() == 0)
    {
        trails.push_back(newTrail);
    }
    
    /*
     * N.B.: in the case of a subnet built around rule 1 & rule 4, 5 (flickering IPs), the code 
     * above doesn't exhaustively list all trails seen in a same subnet. The reason is that, as 
     * long as flickering trails appear individually as the main trails towards disjoint subsets 
     * of subnets, we will end up with an aggregate tying all subsets together and very likely 
     * labelled with all flickering trails. There is still a possibility one or few flickering IPs 
     * could be missing from the label of the aggregate, but a future post-processing could 
     * complete the list (for now, it doesn't have any negative effect on neighborhood inference).
     */
}

void Aggregate::addSubnet(Subnet *newSubnet)
{
    if(newSubnet == NULL)
        return;
    subnets.push_back(newSubnet);
    this->updateTrails(newSubnet->getTrail());
}

void Aggregate::addSubnets(list<Subnet*> newSubnets)
{
    if(newSubnets.size() < 1)
        return;
    this->updateTrails(newSubnets.front()->getTrail()); // Because splice() will empty newSubnets
    subnets.splice(subnets.end(), newSubnets);
}

void Aggregate::finalize(IPLookUpTable *dictionary)
{
    // 1) Lists subnet interfaces with a (partial) route
    list<SubnetInterface*> withPeers;
    for(list<Subnet*>::iterator i = subnets.begin(); i != subnets.end(); ++i)
    {
        list<SubnetInterface*> *IPs = (*i)->getInterfacesList();
        for(list<SubnetInterface*>::iterator j = IPs->begin(); j != IPs->end(); ++j)
        {
            SubnetInterface *cur = (*j);
            if(cur->partialRouteLength > 0 && cur->partialRoute != NULL)
                withPeers.push_back(cur);
        }
    }
    
    // No possible peers (gate)
    if(withPeers.size() == 0)
        return;
    
    // 2) Looks for peers within routes, if not found yet
    unsigned short offset = 0;
    while(peers.size() == 0 && withPeers.size() > 0)
    {
        for(list<SubnetInterface*>::iterator i = withPeers.begin(); i != withPeers.end(); ++i)
        {
            unsigned short routeLen = (*i)->partialRouteLength;
            if(offset >= routeLen)
                withPeers.erase(i--); // Can't be used anymore
            
            RouteHop *route = (*i)->partialRoute;
            short currentOffset = routeLen - 1 - offset;
            if(currentOffset >= 0 && route[currentOffset].isValidHop())
            {
                IPTableEntry *assocEntry = dictionary->lookUp(route[currentOffset].ip);
                if(assocEntry != NULL && assocEntry->denotesNeighborhood())
                {
                    peers.push_back(route[currentOffset].ip);
                    withPeers.erase(i--); // We won't use it anymore
                }
            }
        }
        offset++;
    }
    
    // 3) Peers were found: "peers" list is cleaned (removal of duplicata) and offset is set
    if(peers.size() > 0)
    {
        peersOffset = offset - 1; // "- 1" because of the final incrementation
        peers.sort(InetAddress::smaller);
        InetAddress prev(0);
        for(list<InetAddress>::iterator i = peers.begin(); i != peers.end(); ++i)
        {
            InetAddress cur = (*i);
            if(cur == prev)
            {
                peers.erase(i--);
                continue;
            }
            prev = cur;
        }
    }
}

bool Aggregate::compare(Aggregate *a1, Aggregate *a2)
{
    if(a1->trails.size() == 0 && a2->trails.size() == 0)
        return Subnet::compare(a1->subnets.front(), a2->subnets.front());
    else if(a2->trails.size() == 0)
        return false;
    else if(a1->trails.size() == 0)
        return true;
    Trail *firstTrail = a1->trails.front();
    Trail *secondTrail = a2->trails.front();
    return Trail::compare(firstTrail, secondTrail);
}

string Aggregate::getLabel()
{
    stringstream label;
    label << "{";
    
    if(trails.size() == 0 && subnets.front()->getPivotTTL() <= 2)
    {
        label << "This computer}";
        return label.str();
    }
    
    // Flickering trails
    if(trails.size() > 1)
    {
        for(list<Trail*>::iterator i = trails.begin(); i != trails.end(); ++i)
        {
            if(i != trails.begin())
                label << ", ";
            label << (*i)->getLastValidIP();
        }
    }
    // Echoing trails (best effort strategy used instead)
    else if(trails.size() == 0)
    {
        label << "Echo, TTL=" << (unsigned short) subnets.front()->getPivotTTL();
        label << ", Pre-echoing=";
        for(list<InetAddress>::iterator i = preEchoing.begin(); i != preEchoing.end(); ++i)
        {
            if(i != preEchoing.begin())
                label << ", ";
            label << (*i);
        }
        if(preEchoingOffset > 0)
            label << " (offset=" << preEchoingOffset << ")";
    }
    // Any other case (direct or incomplete trail common to all aggregated subnets)
    else
    {
        Trail *curTrail = trails.front();
        label << curTrail->getLastValidIP();
        if(curTrail->getNbAnomalies() > 0)
            label << " | " << curTrail->getNbAnomalies();
    }
    
    label << "}";
    return label.str();
}

string Aggregate::toString()
{
    stringstream ss;
    ss << "Neighborhood for " << this->getLabel() << ":\n";
    
    // Now listing the aggregated subnets.
    
    // Case of an aggregate with flickering trails: tells which trail(s) appear in which subnet(s)
    if(trails.size() > 1)
    {
        for(list<Subnet*>::iterator i = subnets.begin(); i != subnets.end(); ++i)
        {
            Subnet *cur = (*i);
            ss << cur->getAdjustedCIDR();
            list<InetAddress> cTrailIPs = cur->getDirectTrailIPs();
            if(cTrailIPs.size() > 1)
            {
                ss << " (trails: ";
                for(list<InetAddress>::iterator j = cTrailIPs.begin(); j != cTrailIPs.end(); ++j)
                {
                    if(j != cTrailIPs.begin())
                        ss << ", ";
                    ss << (*j);
                }
            }
            else
                ss << " (trail: " << cTrailIPs.front();
            ss << ")\n";
        }
    }
    // Case of an "Echo" aggregate with multiple pre-echoing IPs: details pre-echoing IPs
    else if(trails.size() == 0 && preEchoing.size() > 1)
    {
        for(list<Subnet*>::iterator i = subnets.begin(); i != subnets.end(); ++i)
        {
            Subnet *cur = (*i);
            ss << cur->getAdjustedCIDR() << " (pre-echoing: ";
            list<InetAddress> preEchoes = cur->getPreTrailIPs();
            for(list<InetAddress>::iterator j = preEchoes.begin(); j != preEchoes.end(); ++j)
            {
                if(j != preEchoes.begin())
                    ss << ", ";
                ss << (*j);
            }
            ss << ")\n";
        }
    }
    // Any other situation: just prints the (adjusted) CIDR's
    else
    {
        for(list<Subnet*>::iterator i = subnets.begin(); i != subnets.end(); ++i)
            ss << (*i)->getAdjustedCIDR() << "\n";
    }
    
    // Writes down the peer(s), otherwise that this neighborhood is a gate to the topology
    if(peers.size() > 0)
    {
        if(peers.size() > 1)
        {
            ss << "Peers: ";
            for(list<InetAddress>::iterator i = peers.begin(); i != peers.end(); ++i)
            {
                if(i != peers.begin())
                    ss << ", ";
                ss << (*i);
            }
            if(peersOffset > 0)
                ss << " (offset=" << peersOffset << ")";
            ss << "\n";
        }
        else
        {
            ss << "Peer: " << peers.front() << "\n";
        }
    }
    else
        ss << "No peers; gate to the discovered topology.\n";
    
    return ss.str();
}
