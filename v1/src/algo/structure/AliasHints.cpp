/*
 * AliasHints.cpp
 *
 *  Created on: Nov 16, 2018
 *      Author: jefgrailet
 *
 * Implements the class defined in AliasHints.h (see this file to learn further about the goals of 
 * such a class).
 */

#include <cmath> // ceil(), floor()

#include "AliasHints.h"

unsigned short AliasHints::probingStage = AliasHints::DURING_SUBNET_DISCOVERY; // Initial stage

AliasHints::AliasHints(unsigned short nbIPIDs)
{
    when = probingStage;
    
    this->nbIPIDs = nbIPIDs;
    probeTokens = NULL;
    IPIdentifiers = NULL;
	echoMask = NULL;
	delays = NULL;
    
    echoInitialTTL = 0;
    hostName = "";
    replyingToTSRequest = false;
    portUnreachableSrcIP = InetAddress(0);
    UDPSecondary = false;
    
    velocityLowerBound = 0.0;
    velocityUpperBound = 0.0;
    IPIDCounterType = NO_IDEA;
}

AliasHints::~AliasHints()
{
    delete[] probeTokens;
    delete[] IPIdentifiers;
    delete[] echoMask;
    delete[] delays;
}

void AliasHints::prepareIPIDData()
{
    if(nbIPIDs == 0)
        return;

    probeTokens = new unsigned long[nbIPIDs];
    IPIdentifiers = new unsigned short[nbIPIDs];
    echoMask = new bool[nbIPIDs];
    delays = new unsigned long[nbIPIDs - 1];
    
    for(unsigned short i = 0; i < nbIPIDs; i++)
    {
        probeTokens[i] = 0;
        IPIdentifiers[i] = 0;
        echoMask[i] = false;
    }
    
    for(unsigned short i = 0; i < nbIPIDs - 1; i++)
        delays[i] = 0;
}

void AliasHints::postProcessIPIDData(unsigned short maxRollovers, double maxError)
{
    if(IPIDCounterType != NO_IDEA)
        return; // Not needed (most probably hints from a previous probing stage)
    
    // Checks if there's valid data to work with begin with
    if(probeTokens == NULL || probeTokens[0] == 0)
        return;
    
    // Checks if this is an IP that just echoes the ID provided in probes
    unsigned short nbEchoes = 0;
    for(unsigned short i = 0; i < nbIPIDs; i++)
        if(echoMask[i])
            nbEchoes++;
    
    if(nbEchoes == nbIPIDs)
    {
        IPIDCounterType = ECHO_COUNTER;
        return;
    }
    
    // Looks at the amount of negative deltas
    unsigned short negativeDeltas = 0;
    for(unsigned short i = 0; i < nbIPIDs - 1; i++)
        if(IPIdentifiers[i] > IPIdentifiers[i + 1])
            negativeDeltas++;
    
    // If one or zero negative deltas, straightforward evaluation
    if(negativeDeltas < 2)
    {
        double v[nbIPIDs - 1];
        
        // Velocity computation (just in case)
        for(unsigned short i = 0; i < nbIPIDs - 1; i++)
        {
            double b_i = (double) IPIdentifiers[i];
            double b_i_plus_1 = (double) IPIdentifiers[i + 1];
            double d_i = delays[i];
            
            if(b_i_plus_1 > b_i)
                v[i] = (b_i_plus_1 - b_i) / d_i;
            else
                v[i] = (b_i_plus_1 + (65535 - b_i)) / d_i;
        }
        
        // Finds minimum/maximum velocity
        double maxV = v[0], minV = v[0];
        for(unsigned short i = 0; i < nbIPIDs - 1; i++)
        {
            if(v[i] > maxV)
                maxV = v[i];
            if(v[i] < minV)
                minV = v[i];
        }
        
        velocityLowerBound = minV;
        velocityUpperBound = maxV;
        IPIDCounterType = HEALTHY_COUNTER;
        return;
    }
    
    // Otherwise, solving equations becomes necessary.
    double d0 = (double) delays[0];
    double b0 = (double) IPIdentifiers[0];
    double b1 = (double) IPIdentifiers[1];
    
    double x = 0.0;
    double v[nbIPIDs - 1];
    
    bool success = false;
    for(unsigned short i = 0; i < maxRollovers; i++)
    {
        // Computing speed for x
        if(b1 > b0)
            v[0] = (b1 - b0 + 65535 * x) / d0;
        else
            v[0] = (b1 + (65535 - b0) + 65535 * x) / d0;
        
        success = true;
        for(unsigned short j = 1; j < nbIPIDs - 1; j++)
        {
            // Computing speed for cur
            double b_j = (double) IPIdentifiers[j];
            double b_j_plus_1 = (double) IPIdentifiers[j + 1];
            double d_j = (double) delays[j];
            double cur = 0.0;
            
            cur += (d_j / d0) * x;
            
            if(b_j_plus_1 > b_j)
                cur -= ((b_j_plus_1 - b_j) / 65535);
            else
                cur -= ((b_j_plus_1 + (65535 - b_j)) / 65535);
            
            if(b1 > b0)
                cur += (((d_j * b1) - (d_j * b0)) / (65535 * d0));
            else
                cur += (((d_j * b1) + (d_j * (65535 - b0))) / (65535 * d0));
            
            // Flooring/ceiling cur
            double floorCur = floor(cur);
            double ceilCur = ceil(cur);
            
            double selectedCur = 0.0;
            double gap = 0.0;
            
            if((cur - floorCur) > (ceilCur - cur))
            {
                selectedCur = ceilCur;
                gap = ceilCur - cur;
            }
            else
            {
                selectedCur = floorCur;
                gap = cur - floorCur;
            }
            
            // Storing speed of current time interval
            if(selectedCur > 0.0 && gap <= maxError)
            {
                if(b_j_plus_1 > b_j)
                    v[j] = (b_j_plus_1 - b_j + 65535 * selectedCur) / d_j;
                else
                    v[j] = (b_j_plus_1 + (65535 - b_j) + 65535 * selectedCur) / d_j;
            }
            else
            {
                success = false;
                break;
            }
        }
        
        if(success)
            break;
        
        x += 1.0;
    }
    
    if(success)
    {
        double maxV = v[0], minV = v[0];
        for(unsigned short i = 0; i < nbIPIDs - 1; i++)
        {
            if(v[i] > maxV)
                maxV = v[i];
            if(v[i] < minV)
                minV = v[i];
        }
        
        velocityLowerBound = minV;
        velocityUpperBound = maxV;
        IPIDCounterType = FAST_COUNTER;
    }
    else
    {
        // "Infinite" velocity: [0.0, 65535.0]; interpreted as a random counter
        velocityLowerBound = 0.0;
        velocityUpperBound = 65535.0;
        IPIDCounterType = RANDOM_COUNTER;
    }
}
    
bool AliasHints::fingerprintSimilarTo(AliasHints *other)
{
    unsigned char otherInitialTTL = other->getEchoInitialTTL();
    unsigned short otherType = other->getIPIDCounterType();
    if(echoInitialTTL == otherInitialTTL && IPIDCounterType == otherType)
    {
        if(UDPSecondary || other->isUDPSecondary())
            return true;
        else if(portUnreachableSrcIP == other->getPortUnreachableSrcIP())
            return true;
    }
    return false;
}

bool AliasHints::toGroupByDefault()
{
    switch(IPIDCounterType)
    {
        case RANDOM_COUNTER:
        case ECHO_COUNTER:
            return true;
        default:
            break;
    }
    return false;
}

bool AliasHints::compare(AliasHints *h1, AliasHints *h2)
{
    unsigned char initialTTL1 = h1->getEchoInitialTTL(), initialTTL2 = h2->getEchoInitialTTL();
    if(initialTTL1 == initialTTL2)
    {
        InetAddress portUnreachableSrcIP1 = h1->getPortUnreachableSrcIP();
        InetAddress portUnreachableSrcIP2 = h2->getPortUnreachableSrcIP();
        bool UDPIsSecondary = (h1->isUDPSecondary() || h2->isUDPSecondary());
        if(UDPIsSecondary || portUnreachableSrcIP1 == portUnreachableSrcIP2)
        {
            unsigned short IPIDCounterType1 = h1->getIPIDCounterType();
            unsigned short IPIDCounterType2 = h2->getIPIDCounterType();
            if(IPIDCounterType1 == IPIDCounterType2)
            {
                string hn1 = h1->getHostName(), hn2 = h2->getHostName();
                if((hn1.empty() && hn2.empty()) || (!hn1.empty() && !hn2.empty()))
                {
                    if(!h1->repliesToTSRequest() && h2->repliesToTSRequest())
                        return true;
                }
                else if(!hn1.empty() && hn2.empty())
                    return true;
            }
            else if(IPIDCounterType1 > IPIDCounterType2)
                return true;
        }
        else if(portUnreachableSrcIP1 > portUnreachableSrcIP2)
            return true;
    }
    else if(initialTTL1 > initialTTL2)
        return true;
    return false;
}

string AliasHints::fingerprintToString()
{
    stringstream ss;
    
    ss << "<";
    if(echoInitialTTL > 0)
        ss << (unsigned short) echoInitialTTL;
    else
        ss << "*";
    ss << ",";
    if(portUnreachableSrcIP != InetAddress("0.0.0.0"))
        ss << portUnreachableSrcIP;
    else
        ss << "*";
    ss << ",";
    switch(IPIDCounterType)
    {
        case HEALTHY_COUNTER:
            ss << "Healthy";
            break;
        case FAST_COUNTER:
            ss << "Fast";
            break;
        case RANDOM_COUNTER:
            ss << "Random";
            break;
        case ECHO_COUNTER:
            ss << "Echo";
            break;
        default:
            ss << "*";
            break;
    }
    ss << ",";
    if(!hostName.empty())
        ss << "Yes";
    else
        ss << "No";
    ss << ",";
    if(replyingToTSRequest)
        ss << "Yes";
    else
        ss << "No";
    ss << ">";
    return ss.str();
}

string AliasHints::toString()
{
    stringstream ss;
    
    // Probing stage
    switch(when)
    {
        case 1:
            ss << "Target scanning";
            break;
        case 2:
            ss << "Graph building";
            break;
        case 3:
            ss << "Full alias resolution";
            break;
        default:
            ss << "Unknown";
            break;
    }
    
    // : [Initial echo TTL] - ECHO,[Host name]
    if(IPIDCounterType == ECHO_COUNTER)
    {
        ss << ": ";
        
        unsigned short iTTL = (unsigned short) echoInitialTTL;
        if(iTTL > 0)
            ss << iTTL << " - ";
        
        ss << "ECHO";
        
        if(!hostName.empty())
            ss << "," << hostName;
    }
    // : [Initial echo TTL] - [IP-ID data],[Host name]
    else if(nbIPIDs > 0 && IPIdentifiers != NULL && probeTokens[0] != 0)
    {
        ss << ": ";
        
        unsigned short iTTL = (unsigned short) echoInitialTTL;
        if(iTTL > 0)
            ss << iTTL << " - ";
        
        for(unsigned short i = 0; i < nbIPIDs; i++)
        {
            if(i > 0)
                ss << "," << delays[i - 1] << ",";
            ss << probeTokens[i] << ";" << IPIdentifiers[i];
        }
        
        if(!hostName.empty())
            ss << "," << hostName;
    }
    // : [Host name]
    else if(!hostName.empty())
        ss << ": " << hostName;
    
    // ... | Yes,[Unreachable port reply IP]
    if(replyingToTSRequest)
    {
        ss << " | Yes";
        if(portUnreachableSrcIP != InetAddress(0))
            ss << "," << portUnreachableSrcIP;
    }
    // ... | [Unreachable port reply IP]
    else if(portUnreachableSrcIP != InetAddress(0))
        ss << " | " << portUnreachableSrcIP;
    
    return ss.str();
}
