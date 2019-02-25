/*
 * RouteHop.h
 *
 *  Created on: Sept 6, 2018
 *      Author: jefgrailet
 *
 * A simple class to represent a single hop in a route.
 *
 * N.B.: this class is equivalent to "RouteInterface" in SAGE and late TreeNET versions.
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
        LIMITED, // Same, but got something when retrying later (because of rate-limitation or firewall)
        REPAIRED_1, // Repaired at first offline fix
        REPAIRED_2, // Repaired offline after re-covering a "limited" IP in another route
        VIA_TRACEROUTE // Obtained through traceroute
    };
    
    // TODO: are limited/repaired still relevant for upcoming software ?

    RouteHop(); // Creates a "NOT_MEASURED" route hop
    RouteHop(InetAddress ip);
    ~RouteHop();
    
    void update(InetAddress ip); // For when the hop is initially "NOT_MEASURED"
    void anonymize(); // Same but for when there is a timeout (= anonymous router)
    void repair(InetAddress ip); // Always sets state to "REPAIRED_1"
    void repairBis(InetAddress ip); // Always sets state to "REPAIRED_2"
    void deanonymize(InetAddress ip); // Always sets state to "LIMITED"
    
    // Overriden equality operator
    RouteHop &operator=(const RouteHop &other);
    
    InetAddress ip;
    unsigned short state;

};

#endif /* ROUTEHOP_H_ */
