/*
 * ReverseDNSUnit.h
 *
 *  Created on: Mar 4, 2015
 *      Author: jefgrailet
 *
 * This class, inheriting Runnable, performs reverse DNS on a single IP to get its host name, if 
 * such name exists.
 *
 * As its name suggests, ReverseDNSUnit is used by AliasHintCollector for its probing. However, 
 * unlike IPIDUnit, it does no require a pointer to the calling object as there is no token 
 * mechanism here (the host name is constant). Also, no prober object is involved neither because 
 * the code simply uses the getHostName() from InetAddress which itself relies on the C function 
 * gethostbyaddr() (cf. http://beej.us/guide/bgnet/output/html/multipage/gethostbynameman.html).
 *
 * While one could argue that reverse DNS could be done sequentially, this class helps to 
 * run this part of the program in parallel, to speed up the whole procedure.
 */

#ifndef REVERSEDNSUNIT_H_
#define REVERSEDNSUNIT_H_

#include "../Environment.h"
#include "../../common/thread/Runnable.h"

class ReverseDNSUnit : public Runnable
{
public:

    // Constructor
    ReverseDNSUnit(Environment *env, IPTableEntry *IPToProbe);
    
    // Destructor and run method
    ~ReverseDNSUnit();
    void run();
    
private:
    
    // Private fields
    Environment *env;
    IPTableEntry *IPToProbe;

};

#endif /* REVERSEDNSUNIT_H_ */
