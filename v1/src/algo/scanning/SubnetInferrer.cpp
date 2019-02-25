/*
 * SubnetInferrer.cpp
 *
 *  Created on: Nov 23, 2018
 *      Author: jefgrailet
 *
 * Implements the class defined in SubnetInferrer.h (see this file to learn further about the 
 * goals of such a class).
 */

#include <fstream>
using std::ofstream;
#include <sys/stat.h> // For CHMOD edition

#include "SubnetInferrer.h"
#include "../aliasresolution/AliasHintsCollector.h"
#include "../aliasresolution/AliasResolver.h"

SubnetInferrer::SubnetInferrer(Environment *env)
{
    this->env = env;
}

SubnetInferrer::~SubnetInferrer()
{
    for(list<Subnet*>::iterator i = subnets.begin(); i != subnets.end(); i++)
        delete (*i);
    subnets.clear();
}

void SubnetInferrer::addFlickeringPeers(IPTableEntry *IP, 
                                        list<IPTableEntry*> *alias, 
                                        list<IPTableEntry*> *flickIPs)
{
    list<InetAddress> *peers = IP->getFlickeringPeers();
    for(list<InetAddress>::iterator i = peers->begin(); i != peers->end(); ++i)
    {
        InetAddress curIP = (*i);
        
        // Finds corresponding entry in flickIPs, does a recursive call and erases it from flickIPs
        for(list<IPTableEntry*>::iterator j = flickIPs->begin(); j != flickIPs->end(); ++j)
        {
            IPTableEntry *curEntry = (*j);
            if((InetAddress) (*curEntry) == curIP)
            {
                alias->push_back(curEntry);
                flickIPs->erase(j);
                addFlickeringPeers(curEntry, alias, flickIPs);
                break;
            }
        }
    }
}

void SubnetInferrer::prepare()
{
    IPLookUpTable *dict = env->getIPDictionary();
    
    /*
     * We now build alleged aliases, using a list of flickering IPs. For each item, we will:
     * 1) init a new list curAlias with the IP itself, 
     * 2) then, for each flickering peer:
     *    -add the peer to curAlias, 
     *    -remove it from the flickIPs list, 
     *    -perform the same operation recursively on its own peers;
     * 3) after, the final list is sorted, duplicate IPs are removed and curAlias is appended to 
     *    the list of alleged aliases.
     */
    
    list<list<IPTableEntry*> > aliases; // Only alleged, at this point
    list<IPTableEntry*> flickIPs = dict->listFlickeringIPs();
    while(flickIPs.size() > 0)
    {
        // 1) Inits the alias
        list<IPTableEntry*> curAlias;
        IPTableEntry *first = flickIPs.front();
        curAlias.push_back(first);
        flickIPs.pop_front();
        
        // 2) Adds flickering peers to the alias
        addFlickeringPeers(first, &curAlias, &flickIPs);
        
        // 3) Remove duplicate IPs and append to the alleged aliases
        curAlias.sort(IPTableEntry::compare);
        IPTableEntry *prev = NULL;
        for(list<IPTableEntry*>::iterator i = curAlias.begin(); i != curAlias.end(); ++i)
        {
            if(i == curAlias.begin())
            {
                prev = (*i);
                continue;
            }
            if((*i) == prev)
                curAlias.erase(i--);
            else
                prev = (*i);
        }
        aliases.push_back(curAlias);
    }
    
    if(aliases.size() == 0)
        return;
    
    ostream *out = env->getOutputStream();
    (*out) << "\nThere are flickering IPs that could lead to the discovery of ";
    if(aliases.size() > 1)
        (*out) << aliases.size() << " aliases.\n";
    else
        (*out) << "one alias.\n";
    (*out) << "Now starting alias hints collection...\n";
    
    /*
     * An AliasHintsCollector object is created to collect alias hints on all alleged aliases, 
     * in order to check the odds that flickering IPs indeed belong to a same device (or not).
     */
    
    AliasHintsCollector *ahc = new AliasHintsCollector(env);
    for(list<list<IPTableEntry*> >::iterator i = aliases.begin(); i != aliases.end(); ++i)
    {
        list<IPTableEntry*> hypothesis = (*i);
        (*out) << "Collecting hints for ";
        for(list<IPTableEntry*>::iterator j = hypothesis.begin(); j != hypothesis.end(); ++j)
        {
            if(j != hypothesis.begin())
                (*out) << ", ";
            (*out) << *(*j);
        }
        (*out) << "... " << std::flush;
        ahc->setIPsToProbe(hypothesis);
        ahc->collect();
    }
    delete ahc;
    
    // Post-processes newly collected hints
    dict->postProcessHints(env->getARVelocityMaxRollovers(), env->getARVelocityMaxError());
    
    // Builds the alias set, using the AliasResolver class.
    (*out) << "Resolving... " << std::flush;
    AliasSet *set = new AliasSet(AliasSet::SUBNET_DISCOVERY);
    AliasResolver *ar = new AliasResolver(env);
    for(list<list<IPTableEntry*> >::iterator i = aliases.begin(); i != aliases.end(); ++i)
    {
        list<Alias*> newAliases = ar->resolve((*i));
        set->addAliases(newAliases, true); // Only actual aliases (#IPs >= 2) should appear
    }
    env->addAliasSet(set);
    delete ar;
    (*out) << "Done." << endl;
}

// Trail rule

bool SubnetInferrer::subnetRule1(IPTableEntry *pivot, IPTableEntry *candidate)
{
    Trail *t1 = pivot->getTrail();
    Trail *t2 = candidate->getTrail();
    if(t1 != NULL && t2 != NULL && t1->equals(t2))
        return true;
    return false;
}

// TTL + timeout rule

bool SubnetInferrer::subnetRule2(IPTableEntry *pivot, IPTableEntry *candidate, IPTableEntry **newPivot)
{
    if(pivot->getTTL() != candidate->getTTL())
        return false;
    
    Trail *t1 = pivot->getTrail();
    Trail *t2 = candidate->getTrail();
    if(t1 != NULL && t2 != NULL && t1->getNbAnomalies() != t2->getNbAnomalies())
    {
        // Updates pivot if candidate IP has less anomalies
        if(t2->getNbAnomalies() < t1->getNbAnomalies() && newPivot != NULL)
            (*newPivot) = candidate;
        return true;
    }
    return false;
}

// Echoes rule

bool SubnetInferrer::subnetRule3(IPTableEntry *pivot, IPTableEntry *candidate)
{
    if(pivot->getTTL() != candidate->getTTL())
        return false;
    
    Trail *t1 = pivot->getTrail();
    Trail *t2 = candidate->getTrail();
    if(t1 != NULL && t2 != NULL && t1->isEchoing() && t2->isEchoing())
        return true;
    return false;
}

// Aliased flickering IPs rule

bool SubnetInferrer::subnetRule4(IPTableEntry *pivot, IPTableEntry *candidate, AliasSet *aliases)
{
    if(pivot->getTTL() != candidate->getTTL() || aliases == NULL)
        return false;
    
    Trail *t1 = pivot->getTrail();
    Trail *t2 = candidate->getTrail();
    if(t1 != NULL && t2 != NULL && t1->isFlickering() && t2->isFlickering())
    {
        InetAddress t1IP = t1->getLastValidIP();
        InetAddress t2IP = t2->getLastValidIP();
        
        Alias *toCheck = aliases->findAlias(t1IP);
        if(toCheck != NULL && toCheck->hasInterface(t2IP))
            return true;
    }
    return false;
}

// Aliased IPs rule (generalization of rule 4; applies to warping AND flickering IPs)

bool SubnetInferrer::subnetRule5(IPTableEntry *pivot, IPTableEntry *candidate, AliasSet *aliases)
{
    if(aliases == NULL)
        return false;
    
    Trail *t1 = pivot->getTrail();
    Trail *t2 = candidate->getTrail();
    if(t1 == NULL || t2 == NULL || t1->getNbAnomalies() > 0 || t2->getNbAnomalies() > 0)
        return false;
        
    InetAddress t1IP = t1->getLastValidIP();
    InetAddress t2IP = t2->getLastValidIP();
    
    Alias *toCheck = aliases->findAlias(t1IP);
    if(toCheck != NULL && toCheck->hasInterface(t2IP))
        return true;
    return false;
}

void SubnetInferrer::fullShrinkage(Subnet *sn, 
                                   list<IPTableEntry*> *IPs, 
                                   IPTableEntry *oldPivot, 
                                   list<IPTableEntry*> *contrapivots)
{
    /*
     * In a specific scenario, the "old" pivot, i.e. the pivot interface of the last successful 
     * expansion, might have been alleged to be a contra-pivot after all and is therefore not 
     * listed in the Subnet object anymore and rather in the list of contra-pivots. To undo this, 
     * we simply check if the old pivot indeed appears in the list, and if yes, we remove it and 
     * restore it to the Subnet object as the selected pivot. If the "old" pivot is not part of 
     * the contra-pivots list, it is simply updated in the subnet to be labelled again as the 
     * selected pivot interface.
     */
    
    if(contrapivots != NULL && oldPivot != NULL)
    {
        bool found = false;
        for(list<IPTableEntry*>::iterator i = contrapivots->begin(); i != contrapivots->end(); ++i)
        {
            if((*i) == oldPivot)
            {
                contrapivots->erase(i--);
                sn->addInterface(oldPivot, SubnetInterface::SELECTED_PIVOT);
                found = true;
                break;
            }
        }
        if(!found)
            sn->updateInterface(oldPivot, SubnetInterface::SELECTED_PIVOT);
    }
    else if(oldPivot != NULL)
    {
        sn->updateInterface(oldPivot, SubnetInterface::SELECTED_PIVOT);
    }
    
    // Retrieves and restores IPs to the list of IPs to consider for subnet inference
    list<IPTableEntry*> restore;
    sn->shrink(&restore);
    for(list<IPTableEntry*>::iterator i = restore.begin(); i != restore.end(); ++i)
        IPs->push_back((*i));
    if(contrapivots != NULL)
    {
        for(list<IPTableEntry*>::iterator i = contrapivots->begin(); i != contrapivots->end(); ++i)
            IPs->push_back((*i));
        contrapivots->clear();
    }
    IPs->sort(IPTableEntry::compare);
}

void SubnetInferrer::infer()
{
    list<IPTableEntry*> IPs = env->getIPDictionary()->listScannedIPs();
    AliasSet *aliases = env->getLattestAliases();
    while(IPs.size() > 0)
    {
        InetAddress prevLowBorder("255.255.255.255");
        if(subnets.size() > 0)
            prevLowBorder = subnets.front()->getLowerBorder();
        
        IPTableEntry *curPivot = IPs.back();
        IPTableEntry *prevPivot = NULL; // Pivot at the last successful expansion, see below
        Subnet *curSubnet = new Subnet(curPivot);
        IPs.pop_back();
        
        while(curSubnet->getPrefixLength() > MINIMUM_PREFIX_LENGTH)
        {
            // If previous subnet creation/expansion emptied the list of IPs, breaks out of loop
            if(IPs.size() == 0)
                break;
            
            // 1) Checks the subnet doesn't overlap the last subnet we inserted
            curSubnet->expand();
            InetAddress newUpBorder = curSubnet->getUpperBorder();
            if(newUpBorder >= prevLowBorder)
            {
                curSubnet->shrink(); // This will be enough, no need for fullShrinkage()
                string stopReason = "next prefix length overlapped previously inferred subnet.";
                curSubnet->setStopDescription(stopReason);
                curSubnet->setToPostProcess(true);
                break;
            }
            
            /*
             * N.B.: in some situations, merging consecutive subnets should be considered. This 
             * operation takes place during post-processing.
             */
            
            // 2) Picks IPs in the full list that are within the new boundaries
            InetAddress newLowBorder = curSubnet->getLowerBorder();
            list<IPTableEntry*> candidates;
            IPTableEntry *candidate = IPs.back();
            InetAddress candiAsIP = (InetAddress) (*candidate);
            while(candiAsIP >= newLowBorder && candiAsIP <= newUpBorder)
            {
                IPs.pop_back();
                candidates.push_back(candidate);
                if(IPs.size() == 0)
                    break;
                
                candidate = IPs.back();
                candiAsIP = (InetAddress) (*candidate);
            }
            
            /*
             * Some remarks for the next lines of code:
             * -rules from 1 to 4 all do the additional verifications that might be needed, such 
             *  as curPivot and curCandi having the same TTL (rules 2 to 4).
             * -there are two additional pointers for IPTableEntry objects modeling the pivot 
             *  interface:
             *  1) betterPivot is used in rule 2 to refresh the pivot if the candidate IP 
             *     featured a trail with less (or zero) anomalies because it will improve the 
             *     accuracy of the result subnet. This pointer stays to NULL if the original pivot 
             *     can be kept.
             *  2) prevPivot (declared previously) is used to keep track of the pivot interface 
             *     used during the last successful expansion. Indeed, if an expansion during which 
             *     a pivot change occurred is unsuccessful and leads to a shrinkage, then the old 
             *     pivot should be restored.
             * -while they're not all essential, outside brackets while checking subnet rules are 
             *  kept for readability.
             */
            
            // 3) Reviews candidate IPs to check if they belong to the subnet
            IPTableEntry *betterPivot = NULL;
            list<IPTableEntry*> contrapivots;
            unsigned short nbNotOnSubnet = 0;
            unsigned short IPsBeforeExpansion = curSubnet->getNbInterfaces();
            while(candidates.size() > 0)
            {
                IPTableEntry *curCandi = candidates.front();
                candidates.pop_front();
                
                if(subnetRule1(curPivot, curCandi))
                {
                    curSubnet->addInterface(curCandi, SubnetInterface::RULE_1_TRAIL);
                }
                else if(subnetRule2(curPivot, curCandi, &betterPivot))
                {
                    if(betterPivot != NULL)
                    {
                        curSubnet->addInterface(curCandi, SubnetInterface::SELECTED_PIVOT);
                        curSubnet->updateInterface(curPivot, SubnetInterface::RULE_2_TTL_TIMEOUT);
                        curPivot = betterPivot;
                        betterPivot = NULL;
                    }
                    else
                        curSubnet->addInterface(curCandi, SubnetInterface::RULE_2_TTL_TIMEOUT);
                }
                else if(subnetRule3(curPivot, curCandi))
                {
                    curSubnet->addInterface(curCandi, SubnetInterface::RULE_3_ECHOES);
                }
                else if(subnetRule4(curPivot, curCandi, aliases))
                {
                    curSubnet->addInterface(curCandi, SubnetInterface::RULE_4_FLICKERING);
                }
                else if(curPivot->getTTL() > curCandi->getTTL())
                {
                    if(subnetRule5(curPivot, curCandi, aliases))
                        curSubnet->addInterface(curCandi, SubnetInterface::RULE_5_ALIAS);
                    else
                        contrapivots.push_back(curCandi);
                }
                else if(curPivot->getTTL() < curCandi->getTTL())
                {
                    // curPivot could very well be a contra-pivot (critical for small subnets)
                    unsigned short nbIPs = curSubnet->getNbInterfaces();
                    if(nbIPs == 1 && (curCandi->getTTL() - curPivot->getTTL()) == 1)
                    {
                        curSubnet->clearInterfaces();
                        curSubnet->addInterface(curCandi, SubnetInterface::SELECTED_PIVOT);
                        contrapivots.push_back(curPivot);
                        curPivot = curCandi;
                    }
                    else
                    {
                        curSubnet->addInterface(curCandi, SubnetInterface::OUTLIER);
                        nbNotOnSubnet++;
                    }
                }
                else
                {
                    curSubnet->addInterface(curCandi, SubnetInterface::OUTLIER);
                    nbNotOnSubnet++;
                }
            }
            
            // 4) Diagnoses the subnet as a whole (for each shrinkage, a motivation is written)
            unsigned short totalPivots = (unsigned short) curSubnet->getNbInterfaces();
            unsigned short totalNew = totalPivots - IPsBeforeExpansion;
            
            // Potential contra-pivots discovered: subnet stops growing or shrinks
            if(contrapivots.size() > 0)
            {
                unsigned short nbCpivots = (unsigned short) contrapivots.size();
                unsigned short minority = (totalNew + nbCpivots) / 3;
                
                // More than 1/3 of new IPs aren't on the same subnet
                if(totalNew > contrapivots.size() && nbNotOnSubnet > minority)
                {
                    IPTableEntry *oldPivot = (prevPivot != curPivot) ? prevPivot : NULL;
                    fullShrinkage(curSubnet, &IPs, oldPivot, &contrapivots);
                    
                    stringstream ss;
                    ss << "next prefix length ";
                    if(totalNew > nbNotOnSubnet)
                    {
                        ss << "encompassed too many outliers (" << nbNotOnSubnet;
                        ss << " out of " << totalNew << " IPs).";
                    }
                    else
                        ss << "contained no IP appearing to be on the same subnet.";
                    curSubnet->setStopDescription(ss.str());
                    break;
                }
                // Too many contra-pivots or more contra-pivots than pivots
                else if(nbCpivots > MAXIMUM_NB_CONTRAPIVOTS || nbCpivots > totalPivots)
                {
                    IPTableEntry *oldPivot = (prevPivot != curPivot) ? prevPivot : NULL;
                    fullShrinkage(curSubnet, &IPs, oldPivot, &contrapivots);
                    
                    stringstream ss;
                    if(nbCpivots > 5)
                    {
                        ss << "next prefix length had too many hypothetical contra-pivot IPs (";
                        ss << nbCpivots << ").";
                    }
                    else
                    {
                        ss << "next prefix length had more contra-pivot IPs (";
                        ss << nbCpivots << ") than pivot IPs.";
                    }
                    curSubnet->setStopDescription(ss.str());
                    break;
                }
                
                // Checks whether contra-pivots (if several) have the same TTL or not
                bool sameTTL = true;
                unsigned char refTTL = contrapivots.front()->getTTL();
                list<IPTableEntry*>::iterator i = contrapivots.begin();
                for(++i; i != contrapivots.end(); ++i)
                {
                    if((*i)->getTTL() != refTTL)
                    {
                        sameTTL = false;
                        break;
                    }
                }
                
                if(sameTTL)
                {
                    // Checks if contra-pivots have a similar trail
                    bool similarTrail = true;
                    
                    if(refTTL > 1 && contrapivots.size() > 1)
                    {
                        // Finds a trail with the smallest amount of anomalies
                        IPTableEntry *refForTrail = NULL;
                        Trail *refTrail = NULL;
                        unsigned short minAnomalies = 255;
                        list<IPTableEntry*>::iterator i;
                        for(i = contrapivots.begin(); i != contrapivots.end(); ++i)
                        {
                            Trail *curTrail = (*i)->getTrail();
                            if(curTrail != NULL && curTrail->getNbAnomalies() < minAnomalies)
                            {
                                minAnomalies = curTrail->getNbAnomalies();
                                refTrail = curTrail;
                                refForTrail = (*i);
                            }
                        }
                        
                        // Compares trails
                        if(refTrail != NULL)
                        {
                            for(i = contrapivots.begin(); i != contrapivots.end(); ++i)
                            {
                                Trail *curTrail = (*i)->getTrail();
                                if(curTrail == NULL)
                                    continue;
                                
                                unsigned short curAnomalies = curTrail->getNbAnomalies();
                                if(curAnomalies == minAnomalies && !refTrail->equals(curTrail))
                                {
                                    // Re-using subnetRule5() to ensure IPs aren't aliases
                                    if(!subnetRule5(refForTrail, (*i), aliases))
                                    {
                                        similarTrail = false;
                                        break;
                                    }
                                }
                            }
                        }
                    }
                    
                    if(similarTrail)
                    {
                        stringstream ss;
                        ss << "discovered ";
                        if(nbCpivots > 1)
                            ss << nbCpivots << " sound contra-pivot IPs.";
                        else
                            ss << "a sound contra-pivot IP.";
                        curSubnet->setStopDescription(ss.str());
                        
                        // Finally adds the contra-pivots to the subnet
                        while(contrapivots.size() > 0)
                        {
                            IPTableEntry *toAdd = contrapivots.front();
                            contrapivots.pop_front();
                            curSubnet->addInterface(toAdd, SubnetInterface::CONTRAPIVOT);
                        }
                    }
                    else
                    {
                        IPTableEntry *oldPivot = (prevPivot != curPivot) ? prevPivot : NULL;
                        fullShrinkage(curSubnet, &IPs, oldPivot, &contrapivots);
                    
                        stringstream ss;
                        ss << "next prefix length had several contra-pivot candidates (";
                        ss << nbCpivots << ") with differing paths.";
                        curSubnet->setStopDescription(ss.str());
                    }
                }
                else
                {
                    IPTableEntry *oldPivot = (prevPivot != curPivot) ? prevPivot : NULL;
                    fullShrinkage(curSubnet, &IPs, oldPivot, &contrapivots);
                    
                    stringstream ss;
                    ss << "next prefix length had several contra-pivot candidates (";
                    ss << nbCpivots << ") located at different distances.";
                    curSubnet->setStopDescription(ss.str());
                }
                break;
            }
            // Only pivots or outliers were found during this expansion
            else
            {
                unsigned short minority = totalNew / 3;
                
                // IPs appearing as outliers make up more than 1/3 of new IPs: subnet shrinks
                if(nbNotOnSubnet > minority)
                {
                    fullShrinkage(curSubnet, &IPs, prevPivot != curPivot ? prevPivot : NULL);
                    
                    stringstream ss;
                    ss << "next prefix length ";
                    if(totalNew > nbNotOnSubnet)
                    {
                        ss << "encompassed too many outliers (" << nbNotOnSubnet;
                        ss << " out of " << totalNew << " IPs).";
                    }
                    else
                        ss << "contained no IP appearing to be on the same subnet.";
                    curSubnet->setStopDescription(ss.str());
                    break;
                }
            }
            
            prevPivot = curPivot;
        }
        
        if(curSubnet->getPrefixLength() == MINIMUM_PREFIX_LENGTH)
            curSubnet->setStopDescription("reached the maximum size allowed for a subnet.");
        else if(IPs.size() == 0 && curSubnet->getStopDescription().empty())
        {
            curSubnet->setStopDescription("no more IPs to expand the subnet further.");
            curSubnet->setToPostProcess(true); // Just in case
        }
        
        subnets.push_front(curSubnet);
    }
}

void SubnetInferrer::postProcess()
{
    AliasSet *aliases = env->getLattestAliases(); // For subnetRule5()
    list<Subnet*> finalSubnets;
    while(subnets.size() > 0)
    {
        Subnet *curSub = subnets.front();
        subnets.pop_front();
        if(!curSub->needsPostProcessing())
        {
            finalSubnets.push_back(curSub);
            continue;
        }
        
        // Evaluates merging with subsequent subnets
        list<Subnet*> candidates;
        candidates.push_back(curSub);
        IPTableEntry *refPivot = curSub->getSelectedPivot(); // Reference pivot
        unsigned short nbWithContrapivots = 0; // Maximum one merging candidate with contrapivots
        while(subnets.size() > 0)
        {
            Subnet *nextSub = subnets.front();
            if(nbWithContrapivots > 0 && nextSub->hasContrapivots())
                break;
            
            // Can nextSub and candidates be merged together ? We compare the pivots
            IPTableEntry *cmpPivot = nextSub->getSelectedPivot(), *betterPivot = NULL;
            bool mergeOK = false;
            if(subnetRule1(refPivot, cmpPivot)) // Same trail
                mergeOK = true;
            else if(subnetRule2(refPivot, cmpPivot, &betterPivot)) // Same TTL, but one got timeout
                mergeOK = true;
            else if(subnetRule3(refPivot, cmpPivot)) // Echoing IPs
                mergeOK = true;
            else if(subnetRule5(refPivot, cmpPivot, aliases)) // Aliased flickering/warping IPs
                mergeOK = true;
            
            // Breaks out of loop if merging doesn't make sense
            if(!mergeOK)
                break;
            
            // Updates reference pivot if subnetRule2() detected a better one
            if(betterPivot != NULL)
                refPivot = betterPivot;
            
            // Adds the subnet to the candidates for merging
            candidates.push_back(nextSub);
            if(nextSub->hasContrapivots())
                nbWithContrapivots++;
            subnets.pop_front();
        }
        
        // Merging is a possibility
        if(candidates.size() > 1)
        {
            // Computes the total amount of interfaces found in the candidates
            unsigned short compatibleIPs = 0;
            for(list<Subnet*>::iterator i = candidates.begin(); i != candidates.end(); ++i)
                compatibleIPs += (*i)->getNbInterfaces();
            
            // Predicts the borders of the subnet encompassing all candidates
            candidates.sort(Subnet::compare);
    
            InetAddress lowBorder = candidates.front()->getLowerBorder();
            InetAddress upBorder = candidates.back()->getUpperBorder();
            
            unsigned short prefixLength = 32;
            unsigned long uintBorder = lowBorder.getULongAddress();
            unsigned long size = 1;
            while((uintBorder + size - 1) < upBorder.getULongAddress())
            {
                prefixLength--;
                size *= 2;
                uintBorder = uintBorder >> (32 - prefixLength);
                uintBorder = uintBorder << (32 - prefixLength);
            }
            InetAddress finalLowBorder(uintBorder);
            InetAddress finalUpBorder = finalLowBorder + size - 1;
            
            /*
             * Here, merging can be aborted for two reasons:
             * -either the prefix length of the merged subnet is way too large (e.g. /19), 
             * -either the low border of the accomodating subnet overlaps a previous subnet. By 
             *  design, this situation should not occur since subnets marked for post-processing 
             *  were marked because they couldn't expand "to the right". By security, merging is 
             *  aborted in this specific case.
             */
            
            InetAddress predecessorUpBorder(0);
            if(finalSubnets.size() > 0)
                predecessorUpBorder = finalSubnets.back()->getUpperBorder();
            if(prefixLength < MINIMUM_PREFIX_LENGTH || finalLowBorder < predecessorUpBorder)
            {
                // First candidate is added to the final subnets
                finalSubnets.push_back(candidates.front());
                candidates.pop_front();
                
                // Others are put back for post-processing, in case a smaller merging is possible
                while(candidates.size() > 0)
                {
                    subnets.push_front(candidates.back());
                    candidates.pop_back();
                }
                continue;
            }
            
            // Lists the potentially overlapped subnets
            list<Subnet*> after;
            unsigned short artefactIPs = 0;
            while(subnets.size() > 0 && finalUpBorder >= subnets.front()->getUpperBorder())
            {
                Subnet *artefact = subnets.front();
                subnets.pop_front();
                
                after.push_back(artefact);
                artefactIPs += artefact->getNbInterfaces();
            }
            
            // Checks that the hypothetical merged subnet has a majority of compatible IPs
            unsigned short totalIPs = artefactIPs + compatibleIPs;
            double ratioCompatible = (double) compatibleIPs / (double) (totalIPs);
            if(ratioCompatible < 0.7)
            {
                // Overlapped subnets are put back for post-processing
                while(after.size() > 0)
                {
                    subnets.push_front(after.back());
                    after.pop_back();
                }
                
                // First candidate is added to the final subnets
                finalSubnets.push_back(candidates.front());
                candidates.pop_front();
                
                // Others are put back for post-processing, in case a smaller merging is possible
                while(candidates.size() > 0)
                {
                    subnets.push_front(candidates.back());
                    candidates.pop_back();
                }
                continue;
            }
            
            while(after.size() > 0)
            {
                candidates.push_back(after.front());
                after.pop_front();
            }
            
            // Creation of the merged subnet
            Subnet *merged = new Subnet(candidates);
            
            /*
             * Special merging case: in some instances, a subnet can be fragmented into several 
             * small prefixes because of a small prefix that could not be expanded due to a bogus 
             * TTL value or trail (i.e., an outlier), preventing it from being associated with 
             * other IPs. Depending on the positioning of this outlier, the subnet can be inferred 
             * wholy or in several chunks. For the latter, this occurs if the outlier is the last 
             * IP in the scope of the subnet. E.g., a /24 can be inferred as a .0/25, .128/26, 
             * .192/27, .224/28 and .254/31 because the .254/31 contained an outlier.
             *
             * The effect of post-processing is that, in this case, the 4 last subnets will be 
             * merged into a unique .128/25. But if this subnet has a similar pivot as the .0/25 
             * and has no contra-pivot while the .0/25 got one, then they should be merged into a 
             * .0/24. Of course, this particular case only occurs if just expanding the .0/25 once 
             * is enough to encompass all the merged subnets.
             */
            
            Subnet *predecessor = NULL;
            if(finalSubnets.size() > 0)
            {
                predecessor = finalSubnets.back();
                if(!merged->hasContrapivots() && predecessor->hasContrapivots())
                {
                    predecessor->expand();
                    if(predecessor->overlaps(merged))
                    {
                        predecessor->shrink();
                        
                        IPTableEntry *cmpPivot = predecessor->getSelectedPivot(), *betterPivot = NULL;
                        bool mergeOK = false;
                        if(subnetRule1(refPivot, cmpPivot)) // Same trail
                            mergeOK = true;
                        else if(subnetRule2(refPivot, cmpPivot, &betterPivot)) // Same TTL, but one got timeout
                            mergeOK = true;
                        else if(subnetRule3(refPivot, cmpPivot)) // Echoing IPs
                            mergeOK = true;
                        else if(subnetRule5(refPivot, cmpPivot, aliases)) // Aliased flickering/warping IPs
                            mergeOK = true;
                        
                        if(mergeOK)
                        {
                            finalSubnets.pop_back();
                            Subnet *tmp = new Subnet(predecessor, merged);
                            merged = tmp;
                        }
                    }
                    else
                    {
                        predecessor->shrink();
                    }
                }
            }
            
            finalSubnets.push_back(merged);
        }
        else
        {
            finalSubnets.push_back(candidates.front());
        }
    }
    
    // Re-copies the final subnets, adjusting their size in the process
    while(finalSubnets.size() > 0)
    {
        Subnet *curSub = finalSubnets.front();
        finalSubnets.pop_front();
        curSub->adjustSize();
        subnets.push_back(curSub);
    }
}

void SubnetInferrer::outputSubnets(string filename)
{
    string output = "";
    
    for(list<Subnet*>::iterator i = subnets.begin(); i != subnets.end(); i++)
        output += (*i)->toString() + "\n";
    
    ofstream newFile;
    newFile.open(filename.c_str());
    newFile << output;
    newFile.close();
    
    string path = "./" + filename;
    chmod(path.c_str(), 0766);
}
