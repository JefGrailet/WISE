/*
 * RouteHop.h
 *
 *  Created on: Sept 6, 2018
 *      Author: jefgrailet
 *
 * A simple class to represent a single hop in a route.
 *
 * N.B.: this class is equivalent to "RouteInterface" in SAGE v1.0 and late TreeNET versions, but 
 * with a few changes to be more adapted to the probing strategies of WISE/SAGE v2.0.
 */

#ifndef ROUTEHOP_H_
#define ROUTEHOP_H_

#include "../../common/inet/InetAddress.h"

class RouteHop
{
public:

    // Possible methods being used when this IP is associated to a router
    enum HopStates
    {
        NOT_MEASURED, // Not measured yet
        ANONYMOUS, // Tried to get it via traceroute, but only got timeout
        VIA_TRACEROUTE, // Intermediate hop obtained through traceroute
        PEERING_POINT // IP appears as the (direct) trail of some neighborhood, i.e. it's a peer
    };
    
    // Output method
    friend ostream &operator<<(ostream &out, const RouteHop &hop)
    {
        if(hop.state == ANONYMOUS)
            out << "Anonymous";
        else if(hop.state == PEERING_POINT)
            out << "[" << hop.ip << "]";
        else
            out << hop.ip;
        return out;
    }
    
    RouteHop(); // Creates a "NOT_MEASURED" route hop
    RouteHop(InetAddress ip);
    ~RouteHop();
    
    void reset();
    void update(InetAddress ip, bool peer = false); // Updates the state too (see implementation)
    
    // Short methods to avoid using "RouteHop::STATE" in other parts of the code
    inline bool isUnset() { return state == NOT_MEASURED; }
    inline bool isAnonymous() { return state == ANONYMOUS; }
    inline bool isValidHop() { return state == VIA_TRACEROUTE || state == PEERING_POINT; }
    inline bool isPeer() { return state == PEERING_POINT; }
    
    // Overriden equality operator
    RouteHop &operator=(const RouteHop &other);
    
    InetAddress ip;
    unsigned short state;

};

#endif /* ROUTEHOP_H_ */
