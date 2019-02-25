/*
 * IPLookUpTable.cpp
 *
 *  Created on: Sep 29, 2015
 *      Author: jefgrailet
 *
 * Implements the class defined in IPLookUpTable.h (see this file to learn further about the goals 
 * of such a class).
 */
 
#include <fstream>
using std::ofstream;
#include <sys/stat.h> // For CHMOD edition

#include "IPLookUpTable.h"

IPLookUpTable::IPLookUpTable()
{
    this->haystack = new list<IPTableEntry*>[SIZE_TABLE];
}

IPLookUpTable::~IPLookUpTable()
{
    for(unsigned long i = 0; i < SIZE_TABLE; i++)
    {
        list<IPTableEntry*> IPList = this->haystack[i];
        for(list<IPTableEntry*>::iterator j = IPList.begin(); j != IPList.end(); ++j)
            delete (*j);
        haystack[i].clear();
    }
    delete[] haystack;
}

bool IPLookUpTable::isEmpty()
{
    for(unsigned long i = 0; i < SIZE_TABLE; i++)
    {
        list<IPTableEntry*> IPList = this->haystack[i];
        if(IPList.size() > 0)
            return true;
    }
    return false;
}

bool IPLookUpTable::hasAliasResolutionData()
{
    for(unsigned long i = 0; i < SIZE_TABLE; i++)
    {
        list<IPTableEntry*> IPList = this->haystack[i];
        if(IPList.size() > 0)
            for(list<IPTableEntry*>::iterator j = IPList.begin(); j != IPList.end(); ++j)
                if((*j)->getNbHints() > 0)
                    return true;
    }
    return false;
}

unsigned int IPLookUpTable::getTotalIPs()
{
    unsigned int total = 0;
    for(unsigned long i = 0; i < SIZE_TABLE; i++)
    {
        list<IPTableEntry*> IPList = this->haystack[i];
        total += IPList.size();
    }
    return total;
}

IPTableEntry *IPLookUpTable::create(InetAddress needle)
{
    unsigned long index = (needle.getULongAddress() >> 12);
    list<IPTableEntry*> *IPList = &(this->haystack[index]);
    
    for(list<IPTableEntry*>::iterator i = IPList->begin(); i != IPList->end(); ++i)
        if((*i)->getULongAddress() == needle.getULongAddress())
            return NULL;
    
    IPTableEntry *newEntry = new IPTableEntry(needle);
    IPList->push_back(newEntry);
    IPList->sort(IPTableEntry::compare);
    return newEntry;
}

IPTableEntry *IPLookUpTable::lookUp(InetAddress needle)
{
    unsigned long index = (needle.getULongAddress() >> 12);
    list<IPTableEntry*> *IPList = &(this->haystack[index]);
    
    for(list<IPTableEntry*>::iterator i = IPList->begin(); i != IPList->end(); ++i)
        if((*i)->getULongAddress() == needle.getULongAddress())
            return (*i);
    
    return NULL;
}

list<IPTableEntry*> IPLookUpTable::listTargetEntries()
{
    list<IPTableEntry*> res;
    for(unsigned long i = 0; i < SIZE_TABLE; i++)
    {
        list<IPTableEntry*> IPList = this->haystack[i];
        for(list<IPTableEntry*>::iterator j = IPList.begin(); j != IPList.end(); ++j)
        {
            IPTableEntry *cur = (*j);
            unsigned short type = cur->getType();
            if(type == IPTableEntry::SEEN_WITH_TRACEROUTE || type == IPTableEntry::SEEN_IN_TRAIL)
                continue;
            res.push_back(cur);
        }
    }
    return res;
}

list<IPTableEntry*> IPLookUpTable::listFlickeringIPs()
{
    list<IPTableEntry*> res;
    for(unsigned long i = 0; i < SIZE_TABLE; i++)
    {
        list<IPTableEntry*> IPList = this->haystack[i];
        for(list<IPTableEntry*>::iterator j = IPList.begin(); j != IPList.end(); ++j)
        {
            IPTableEntry *cur = (*j);
            if(cur->isFlickering())
                res.push_back(cur);
        }
    }
    return res;
}

list<IPTableEntry*> IPLookUpTable::listScannedIPs()
{
    list<IPTableEntry*> res;
    for(unsigned long i = 0; i < SIZE_TABLE; i++)
    {
        list<IPTableEntry*> IPList = this->haystack[i];
        for(list<IPTableEntry*>::iterator j = IPList.begin(); j != IPList.end(); ++j)
        {
            IPTableEntry *cur = (*j);
            if(cur->getType() == IPTableEntry::SUCCESSFULLY_SCANNED)
                res.push_back(cur);
        }
    }
    return res;
}

void IPLookUpTable::reviewScannedIPs()
{
    for(unsigned long i = 0; i < SIZE_TABLE; i++)
    {
        list<IPTableEntry*> IPList = this->haystack[i];
        for(list<IPTableEntry*>::iterator j = IPList.begin(); j != IPList.end(); ++j)
        {
            IPTableEntry *cur = (*j);
            unsigned char TTL = cur->getTTL();
            if(TTL == IPTableEntry::NO_KNOWN_TTL || (TTL > 1 && cur->getTrail() == NULL))
                cur->setType(IPTableEntry::UNSUCCESSFULLY_SCANNED);
            else
                cur->setType(IPTableEntry::SUCCESSFULLY_SCANNED);
        }
    }
}

unsigned short IPLookUpTable::isFlickering(IPTableEntry *cur, 
                                           IPTableEntry *prev, 
                                           IPTableEntry *prevPrev)
{
    // TTLs must be the same
    if(cur->getTTL() != prev->getTTL() || cur->getTTL() != prevPrev->getTTL())
        return 0;
    
    // All IPs must have trails
    if(cur->getTrail() == NULL || prev->getTrail() == NULL || prevPrev->getTrail() == NULL)
        return 0;
    
    Trail *t1 = cur->getTrail();
    Trail *t2 = prev->getTrail();
    Trail *t3 = prevPrev->getTrail();
    
    // Flickering analysis only makes sense with trails without anomalies
    if(t1->getNbAnomalies() > 0 || t2->getNbAnomalies() > 0 || t3->getNbAnomalies() > 0)
        return 0;
    
    // To continue, t1 IP must be the same as t3 while differing from t2
    InetAddress refIP = t1->getLastValidIP();
    if(refIP != t3->getLastValidIP() || refIP == t2->getLastValidIP())
        return 0;
    
    unsigned long ulIP1 = ((InetAddress) (*cur)).getULongAddress();
    unsigned long ulIP2 = ((InetAddress) (*prev)).getULongAddress();
    unsigned long ulIP3 = ((InetAddress) (*prevPrev)).getULongAddress();
    
    unsigned short delta = (unsigned short) ((ulIP1 - ulIP2) + (ulIP2 - ulIP3));
    return delta;
}

void IPLookUpTable::reviewSpecialIPs(unsigned short maxDelta)
{
    list<IPTableEntry*> filtTargets = this->listTargetEntries();
    for(list<IPTableEntry*>::iterator i = filtTargets.begin(); i != filtTargets.end(); ++i)
    {
        IPTableEntry *target = (*i);
        if(target->getType() != IPTableEntry::SUCCESSFULLY_SCANNED)
            continue;
        
        // Creates trail IP in dictionary (if needed) then marks it as such
        Trail *trail = target->getTrail();
        if(trail != NULL && trail->getLastValidIP() != InetAddress(0))
        {
            InetAddress trailIP = trail->getLastValidIP();
            IPTableEntry *assocEntry = this->lookUp(trailIP);
            // If already in dictionary: checks TTL, records it if different (and not an echo trail)
            if(assocEntry != NULL)
            {
                unsigned short trailAnomalies = trail->getNbAnomalies();
                if(trailAnomalies != 0 || trailIP != (InetAddress) (*target))
                {
                    unsigned char trailIPTTL = target->getTTL() - 1 - (unsigned char) trailAnomalies;
                    if(!assocEntry->hasTTL(trailIPTTL))
                    {
                        assocEntry->recordTTL(trailIPTTL);
                        unsigned short type = assocEntry->getType();
                        if(type == IPTableEntry::SEEN_IN_TRAIL || type == IPTableEntry::SEEN_WITH_TRACEROUTE)
                            assocEntry->pickMinimumTTL();
                    }
                }
            }
            // Otherwise, creates the dictionary entry
            else
            {
                unsigned short trailAnomalies = trail->getNbAnomalies();
                unsigned char trailIPTTL = target->getTTL() - 1 - (unsigned char) trailAnomalies;
                
                assocEntry = this->create(trailIP);
                assocEntry->setAsTrailIP();
                assocEntry->setTTL(trailIPTTL);
                assocEntry->setType(IPTableEntry::SEEN_IN_TRAIL);
            }
        }
    }
    
    // Second visit to mark warping/echoing IPs and trails, and detect flickering
    IPTableEntry *prevPrev = NULL, *prev = NULL;
    for(list<IPTableEntry*>::iterator i = filtTargets.begin(); i != filtTargets.end(); ++i)
    {
        IPTableEntry *target = (*i);
        if(target->getType() != IPTableEntry::SUCCESSFULLY_SCANNED)
            continue;
        
        // Marks trail IP
        Trail *trail = target->getTrail();
        if(trail != NULL && trail->getLastValidIP() != InetAddress(0))
        {
            InetAddress trailIP = trail->getLastValidIP();
            IPTableEntry *assocEntry = this->lookUp(trailIP);
            if(assocEntry == NULL) // Shouldn't occur due to previous loop
                continue;
            assocEntry->setAsTrailIP();
            
            // Warping
            if(assocEntry->getNbTTLs() > 0)
            {
                assocEntry->setAsWarping();
                trail->setAsWarping();
            }
            
            // Echoing
            if(trail->getNbAnomalies() == 0 && trailIP == (InetAddress) (*target))
            {
                assocEntry->setAsEchoing();
                trail->setAsEchoing();
            }
            
            // Flickering
            if(prev != NULL && prevPrev != NULL)
            {
                unsigned short delta = IPLookUpTable::isFlickering(target, prev, prevPrev);
                if(delta > 0 && delta <= maxDelta) // Yup, it's flickering
                {
                    // Pointers should be OK because they were checked in isFlickering()
                    Trail *otherTrail = prev->getTrail();
                    InetAddress otherTrailIP = otherTrail->getLastValidIP();
                    
                    IPTableEntry *otherAssocEntry = this->lookUp(otherTrailIP);
                    if(otherAssocEntry == NULL) // Should not occur (just in case)
                        continue;
                    
                    assocEntry->setAsFlickering();
                    assocEntry->addFlickeringPeer(otherTrailIP);
                    otherAssocEntry->setAsFlickering();
                    otherAssocEntry->addFlickeringPeer(trailIP);
                }
            }
        }
        
        prevPrev = prev;
        prev = target;
    }
    
    // Third and last visit to mark flickering trails
    for(list<IPTableEntry*>::iterator i = filtTargets.begin(); i != filtTargets.end(); ++i)
    {
        IPTableEntry *target = (*i);
        if(target->getType() != IPTableEntry::SUCCESSFULLY_SCANNED)
            continue;
        
        Trail *trail = target->getTrail();
        if(trail != NULL)
        {
            InetAddress trailIP = trail->getLastValidIP();
            if(trailIP != InetAddress(0) && trail->getNbAnomalies() == 0)
            {
                IPTableEntry *assocEntry = this->lookUp(trailIP);
                if(assocEntry == NULL) // Shouldn't occur due to first loop
                    continue;
                if(assocEntry->isFlickering())
                    trail->setAsFlickering();
            }
        }
    }
}

void IPLookUpTable::postProcessHints(unsigned short maxRollovers, double maxError)
{
    for(unsigned long i = 0; i < SIZE_TABLE; i++)
    {
        list<IPTableEntry*> IPList = this->haystack[i];
        for(list<IPTableEntry*>::iterator j = IPList.begin(); j != IPList.end(); ++j)
        {
            IPTableEntry *cur = (*j);
            AliasHints *lattestHints = cur->getLattestHints();
            if(lattestHints != NULL)
                lattestHints->postProcessIPIDData(maxRollovers, maxError);
        }
    }
}

void IPLookUpTable::outputDictionary(string filename)
{
    string output = "";
    
    for(unsigned long i = 0; i < SIZE_TABLE; i++)
    {
        list<IPTableEntry*> IPList = this->haystack[i];
        for(list<IPTableEntry*>::iterator j = IPList.begin(); j != IPList.end(); ++j)
            output += (*j)->toString() + "\n";
    }
    
    ofstream newFile;
    newFile.open(filename.c_str());
    newFile << output;
    newFile.close();
    
    string path = "./" + filename;
    chmod(path.c_str(), 0766); // File must be accessible to all
}

void IPLookUpTable::outputAliasHints(string filename)
{
    string output = "";
    
    for(unsigned long i = 0; i < SIZE_TABLE; i++)
    {
        list<IPTableEntry*> IPList = this->haystack[i];
        for(list<IPTableEntry*>::iterator j = IPList.begin(); j != IPList.end(); ++j)
            if((*j)->getNbHints() > 0)
                output += (*j)->hintsToString();
    }
    
    ofstream newFile;
    newFile.open(filename.c_str());
    newFile << output;
    newFile.close();
    
    string path = "./" + filename;
    chmod(path.c_str(), 0766);
}

void IPLookUpTable::outputFingerprints(string filename)
{
    string output = "";
    
    for(unsigned long i = 0; i < SIZE_TABLE; i++)
    {
        list<IPTableEntry*> IPList = this->haystack[i];
        for(list<IPTableEntry*>::iterator j = IPList.begin(); j != IPList.end(); ++j)
            if((*j)->getNbHints() > 0)
                output += (*j)->toStringSimple() + " - " + (*j)->getLattestHints()->fingerprintToString() + "\n";
    }
    
    /*
     * N.B.: only the last fingerprint is printed. Why ? Because the fingerprint isn't supposed to 
     * change over time, unlike alias resolution hints which should be renewed whenever we want to 
     * perform alias resolution, at the very least to get the right sequences of IP-IDs (and 
     * perhaps other hints, in the future, which also rely on the momentum to infer aliases).
     */
    
    ofstream newFile;
    newFile.open(filename.c_str());
    newFile << output;
    newFile.close();
    
    string path = "./" + filename;
    chmod(path.c_str(), 0766);
}
