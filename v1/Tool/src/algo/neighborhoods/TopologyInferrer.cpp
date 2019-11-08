/*
 * TopologyInferrer.cpp
 *
 *  Created on: Aug 22, 2019
 *      Author: jefgrailet
 *
 * Implements the class defined in TopologyInferrer.h (see this file to learn further about the 
 * goals of such a class).
 */

#include <sys/stat.h> // For CHMOD edition

#include "TopologyInferrer.h"
#include "IPClusterer.h" // Also grants access to std::map

TopologyInferrer::TopologyInferrer(Environment &e): env(e)
{
}

TopologyInferrer::~TopologyInferrer()
{
    for(list<Aggregate*>::iterator i = neighborhoods.begin(); i != neighborhoods.end(); ++i)
        delete (*i);
    neighborhoods.clear();
}

void TopologyInferrer::infer()
{
    IPLookUpTable *dict = env.getIPDictionary();
    AliasSet *aliases = env.getLattestAliases();

    /*
     * Step 1: subnet aggregation
     * --------------------------
     * The code first aggregates subnet that have an identical trail, excepts for subnets built 
     * around rule 3 (echoing trails) which are put away in a different structure to be processed 
     * later (see below). Aggregates (i.e., simplest form of neighborhoods) are then built based 
     * on these lists, taking in account the possibility that a trail might be flickering, and 
     * thus, we have to merge into a single neighborhood lists of subnets that have trails 
     * flickering with each other.
     */
    
    list<Subnet*> *initialList = env.getSubnets();
    list<Subnet*> toProcess(*initialList); // Copies the list
    
    map<string, list<Subnet*>* > subnetsByTrail;
    list<Subnet*> homeSubnets; // Subnets with no trail (subnets close to this computer)
    list<Subnet*> rule3Subnets; // Subnets
    while(toProcess.size() > 0)
    {
        Subnet *sub = toProcess.front();
        toProcess.pop_front();
        
        Trail &subTrail = sub->getTrail();
        if(subTrail.isVoid())
        {
            homeSubnets.push_back(sub);
            continue;
        }
        else if(subTrail.isEchoing())
        {
            rule3Subnets.push_back(sub);
            continue;
        }
        string trailStr = subTrail.toString();
        
        // Looks for existing list in the map; creates it if not found
        map<string, list<Subnet*>* >::iterator res = subnetsByTrail.find(trailStr);
        if(res != subnetsByTrail.end())
        {
            list<Subnet*> *existingList = res->second;
            existingList->push_back(sub);
        }
        else
        {
            list<Subnet*> *newList = new list<Subnet*>();
            newList->push_back(sub);
            
            subnetsByTrail.insert(pair<string, list<Subnet*>* >(trailStr, newList));
        }
    }
    
    if(homeSubnets.size() > 0)
    {
        Aggregate *newAgg = new Aggregate(homeSubnets);
        neighborhoods.push_back(newAgg);
    }
    
    // Creates the (first) aggregates
    while(subnetsByTrail.size() > 0)
    {
        map<string, list<Subnet*>* >::iterator it = subnetsByTrail.begin();
        list<Subnet*> *toAggregate = it->second;
        toAggregate->sort(Subnet::compare);
        Trail &refTrail = toAggregate->front()->getTrail();
        
        // Creates the aggregate from this first list
        Aggregate *newAgg = new Aggregate((*toAggregate));
        subnetsByTrail.erase(it);
        delete toAggregate;
        
        // Do we have flickering trails ? If yes, we need to look for associated lists
        if(refTrail.isFlickering() && refTrail.getNbAnomalies() == 0)
        {
            Alias *assocAlias = aliases->findAlias(refTrail.getLastValidIP());
            if(assocAlias != NULL)
            {
                list<AliasedInterface> *peers = assocAlias->getInterfacesList();
                for(list<AliasedInterface>::iterator j = peers->begin(); j != peers->end(); ++j)
                {
                    InetAddress peer = (InetAddress) (*(j->ip));
                    if(peer == refTrail.getLastValidIP())
                        continue; // Just in case
                    
                    stringstream madeUpTrail;
                    madeUpTrail << "[" << peer << "]";
                    string oTrail = madeUpTrail.str();
                    
                    map<string, list<Subnet*>* >::iterator res = subnetsByTrail.find(oTrail);
                    if(res != subnetsByTrail.end()) // Other list with flickering peer exists
                    {
                        list<Subnet*> *additionalSubnets = res->second;
                        newAgg->addSubnets((*additionalSubnets));
                        subnetsByTrail.erase(res);
                        delete additionalSubnets;
                    }
                }
            }
        }
        
        neighborhoods.push_back(newAgg);
    }
    
    // Deals with the special case of "rule 3" subnets
    if(rule3Subnets.size() > 0)
    {
        // 1) Finds pre-echoing IPs
        for(list<Subnet*>::iterator i = rule3Subnets.begin(); i != rule3Subnets.end(); ++i)
            (*i)->findPreTrailIPs();
        
        // 2) Further discriminates the subnets by using TTL distance and offset of pre-trail IPs
        map<string, list<Subnet*>* > sortedSubs;
        while(rule3Subnets.size() > 0)
        {
            Subnet *sub = rule3Subnets.front();
            rule3Subnets.pop_front();
            stringstream ss;
            ss << sub->getPivotTTL() << "," << sub->getPreTrailOffset();
            string classifyingLabel = ss.str();
            
            map<string, list<Subnet*>* >::iterator res = sortedSubs.find(classifyingLabel);
            if(res != sortedSubs.end())
            {
                list<Subnet*> *existingList = res->second;
                existingList->push_back(sub);
            }
            else
            {
                list<Subnet*> *newList = new list<Subnet*>();
                newList->push_back(sub);
                
                sortedSubs.insert(pair<string, list<Subnet*>* >(classifyingLabel, newList));
            }
        }
        
        // 3) For each sub-group of subnets...
        while(sortedSubs.size() > 0)
        {
            map<string, list<Subnet*>* >::iterator it = sortedSubs.begin();
            list<Subnet*> *subGroup = it->second;
            sortedSubs.erase(it);
            if(subGroup == NULL || subGroup->size() == 0)
            {
                if(subGroup != NULL)
                    delete subGroup;
                continue;
            }
            
            // 3.1) Clusters pre-trail IPs to have exhaustive neighborhoods
            IPClusterer *clusterer = new IPClusterer();
            for(list<Subnet*>::iterator i = subGroup->begin(); i != subGroup->end(); ++i)
                clusterer->update((*i)->getPreTrailIPs());
            
            // 3.2) Just like during initial aggregation, builds a map string -> list of subnets
            map<string, list<Subnet*>* > subnetsByPretrail;
            while(subGroup->size() > 0)
            {
                Subnet *sub = subGroup->front();
                InetAddress needleSub = sub->getPreTrailIPs().front();
                subGroup->pop_front();
                string pretrailStr = clusterer->getClusterString(needleSub);
                
                map<string, list<Subnet*>* >::iterator res = subnetsByPretrail.find(pretrailStr);
                if(res != subnetsByPretrail.end())
                {
                    list<Subnet*> *existingList = res->second;
                    existingList->push_back(sub);
                }
                else
                {
                    list<Subnet*> *newList = new list<Subnet*>();
                    newList->push_back(sub);
                    
                    subnetsByPretrail.insert(pair<string, list<Subnet*>* >(pretrailStr, newList));
                }
            }
            delete subGroup;
            
            // 3.3) Creates the aggregates
            while(subnetsByPretrail.size() > 0)
            {
                map<string, list<Subnet*>* >::iterator itBis = subnetsByPretrail.begin();
                list<Subnet*> *toAggregate = itBis->second;
                if(toAggregate->size() == 0) // Just in case
                {
                    subnetsByPretrail.erase(itBis);
                    delete toAggregate;
                    continue;
                }
                toAggregate->sort(Subnet::compare);
                
                // Gets an IP to get the cluster of pre-echoing IPs
                InetAddress needle(0);
                unsigned short preEchoingOffset = 0;
                for(list<Subnet*>::iterator i = toAggregate->begin(); i != toAggregate->end(); ++i)
                {
                    list<InetAddress> curPreEcho = (*i)->getPreTrailIPs();
                    if(curPreEcho.size() > 0)
                    {
                        needle = curPreEcho.front();
                        preEchoingOffset = (*i)->getPreTrailOffset();
                        break;
                    }
                }
                if(needle == InetAddress(0)) // Artefact (normally, it shouldn't occur)
                {
                    subnetsByPretrail.erase(itBis);
                    delete toAggregate;
                    continue;
                }
                list<InetAddress> preEchoing = clusterer->getCluster(needle);
                
                // Creates the aggregate
                Aggregate *newAgg = new Aggregate((*toAggregate));
                newAgg->setPreEchoing(preEchoing);
                newAgg->setPreEchoingOffset(preEchoingOffset);
                neighborhoods.push_back(newAgg);
                subnetsByPretrail.erase(itBis);
                delete toAggregate;
            }
            
            delete clusterer;
        }
    }
    
    // Finalizes the aggregates (early detection of peers)
    neighborhoods.sort(Aggregate::compare);
    for(list<Aggregate*>::iterator i = neighborhoods.begin(); i != neighborhoods.end(); ++i)
        (*i)->finalize(dict);
    
    // Stops here for now. Next steps (e.g. identifying clusters of aggregates) are for SAGE v2.0.
}

void TopologyInferrer::outputNeighborhoods(string filename)
{
    string output = "";
    for(list<Aggregate*>::iterator i = neighborhoods.begin(); i != neighborhoods.end(); ++i)
        output += (*i)->toString() + "\n";
    
    ofstream newFile;
    newFile.open(filename.c_str());
    newFile << output;
    newFile.close();
    
    string path = "./" + filename;
    chmod(path.c_str(), 0766);
}
