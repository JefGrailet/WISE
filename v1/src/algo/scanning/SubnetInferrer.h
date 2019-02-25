/*
 * SubnetInferrer.h
 *
 *  Created on: Nov 23, 2018
 *      Author: jefgrailet
 *
 * This class post-processes the IP dictionary (involving the alias resolution in the process) and 
 * uses the produced data to infer subnets in the target domain.
 */

#ifndef SUBNETINFERRER_H_
#define SUBNETINFERRER_H_

#include "../Environment.h"
#include "../structure/Subnet.h"

class SubnetInferrer
{
public:
    
    const static unsigned short MINIMUM_PREFIX_LENGTH = 20;
    const static unsigned short MAXIMUM_NB_CONTRAPIVOTS = 5;
    
    // Constructor, destructor
    SubnetInferrer(Environment *env);
    ~SubnetInferrer();
    
    // Post-processes IP dictionary and resolve potential aliases among flickering IPs
    void prepare();
    
    // Methods to perform the actual subnet inference (no probing)
    void infer();
    void postProcess();
    
    // Method to output subnets (will later be moved in env)
    inline unsigned int getNbSubnets() { return (unsigned int) subnets.size(); }
    void outputSubnets(string filename);
    
private:

    // Pointer to the environment singleton
    Environment *env;
    
    // Inferred subnets (will later be moved in env)
    list<Subnet*> subnets;
    
    // Builds alleged aliases using flickering IPs listed in the dictionary.
    static void addFlickeringPeers(IPTableEntry *IP, 
                                   list<IPTableEntry*> *alias, 
                                   list<IPTableEntry*> *flickIPs);
    
    /*
     * Private methods checking subnet inference rules. Note how some methods present additional 
     * parameters.
     * -Rule 2 can lead to the discovery of a better pivot interface, which is passed by the 
     *  newPivot double pointer when a better pivot is discovered.
     * -Rule 4 and 5 need the current alias set in order to check whether the IPs of the trails 
     *  for the pivot IP and the candidate IP are aliases of each other.
     */
    
    static bool subnetRule1(IPTableEntry *pivot, IPTableEntry *candidate);
    static bool subnetRule2(IPTableEntry *pivot, IPTableEntry *candidate, IPTableEntry **newPivot);
    static bool subnetRule3(IPTableEntry *pivot, IPTableEntry *candidate);
    static bool subnetRule4(IPTableEntry *pivot, IPTableEntry *candidate, AliasSet *aliases);
    static bool subnetRule5(IPTableEntry *pivot, IPTableEntry *candidate, AliasSet *aliases);
    
    /*
     * Private method to shrink a subnet. The reason why this method exists is because the 
     * shrink() of Subnet class only performs a part of the operations to carry out; in this 
     * context, one also has to restore IPs for subnet inference that were removed from the 
     * subnet at the shrinkage (plus contra-pivots interfaces) and restore the pivot of the 
     * last successful expansion (if pivot changed). Moreover, the whole process is called 
     * several times, hence why it was turned into a single method.
     */
    
    static void fullShrinkage(Subnet *sn, 
                              list<IPTableEntry*> *IPs, 
                              IPTableEntry *oldPivot, 
                              list<IPTableEntry*> *contrapivots = NULL);
    
}; 

#endif /* SUBNETINFERRER_H_ */
