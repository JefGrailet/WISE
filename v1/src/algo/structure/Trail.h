/*
 * Trail.h
 *
 *  Created on: Aug 30, 2018
 *      Author: jefgrailet
 *
 * This class models a "trail", i.e., the end of the route towards some IP which consists of the 
 * last valid IP observed before reaching the target plus an amount of anomalies. An anomaly is:
 * -an anonymous hop (there was no reply to the probe), 
 * -or a cycle.
 * A valid IP is any other IP. If there's a cycle, the last valid IP consists of the first 
 * occurrence of the cycling IP.
 * 
 * A trail is used to both identify individual subnets (IPs that are close in the IPv4 address 
 * space, which share the same distance TTL-wise and have the same trail should be on the same 
 * subnet) and gather subnets into neighborhoods. This class is meant to ease handling of such 
 * a concept in the rest of the code.
 */

#ifndef TRAIL_H_
#define TRAIL_H_

#include <iostream>
using std::ostream;

#include "../../common/inet/InetAddress.h"

class Trail
{
public:

    friend ostream &operator<<(ostream &out, const Trail &t)
	{
		out << "[" << t.getLastValidIP();
		unsigned short nbAnomalies = t.getNbAnomalies();
		if(nbAnomalies > 0)
		    out << " | " << nbAnomalies;
		out << "]";
		return out;
	}
    
    Trail(InetAddress lastValidIP, unsigned short nbAnomalies);
    Trail(InetAddress lastValidIP); // No anomaly
    Trail(unsigned short routeLength); // Exceptional situation where there's only anonymous hops
    ~Trail();
    
    inline InetAddress getLastValidIP() const { return lastValidIP; }
    inline unsigned short getNbAnomalies() const { return nbAnomalies; }
    
    inline bool isWarping() { return warping; }
    inline bool isFlickering() { return flickering; }
    inline bool isEchoing() { return echoing; }
    
    inline void setAsWarping() { warping = true; }
    inline void setAsFlickering() { flickering = true; }
    inline void setAsEchoing() { echoing = true; }
    
    bool equals(Trail *other);

private:
    
    InetAddress lastValidIP;
    unsigned short nbAnomalies;
	
	bool warping, flickering, echoing;
	
};

#endif /* TRAIL_H_ */
