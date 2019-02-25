/*
 * Subnet.h
 *
 *  Created on: Nov 27, 2018
 *      Author: jefgrailet
 *
 * A simple class to model a subnet, as a list of SubnetInterface to which a prefix IP and a 
 * prefix length are associated. The class also provides various methods to evaluate it and output 
 * it. It will be later expanded to integrate it into SAGE v2.0.
 * 
 * This class has no connection with the SubnetSite class from TreeNET/SAGE v1.0, since subnet 
 * inference is re-built from scratch here.
 *
 * N.B.: this class presents similarities with Alias, as the interfaces list is handled in the 
 * same manner. However, since it only makes a up a small part of the code, there was no strong 
 * incentive to use inheritance (and for instance create classes to respectively model an 
 * interface and a list of interfaces).
 */

#ifndef SUBNET_H_
#define SUBNET_H_

#include "SubnetInterface.h"

class Subnet
{
public:
    
    /*
     * General remark on constructors: it's never checked whether the resulting subnet will be 
     * empty or not (e.g., if the submitted pivot is a NULL pointer, or if the list of subnets is 
     * empty for the second constructor). It's simply because the only class creating subnets for 
     * now (the SubnetInferrer class) already ensures these scenarii never occur, directly or 
     * indirectly. Otherwise, the proper way to handle these scenarii would be to throw an 
     * exception when they occur.
     */
    
    Subnet(IPTableEntry *pivot); // Initializes a /32 subnet
    Subnet(list<Subnet*> subnets); // Merge several subnets together
    Subnet(Subnet *s1, Subnet *s2); // Merge 2 subnets (special case, see SubnetInferrer class)
    ~Subnet();
    
    // Methods to handle interfaces in general
    void addInterface(IPTableEntry *interface, unsigned short subnetRule);
    void updateInterface(IPTableEntry *interface, unsigned short newRule);
    void clearInterfaces();
    IPTableEntry *getSelectedPivot();
    inline unsigned short getNbInterfaces() { return (unsigned short) interfaces.size(); }
    bool hasContrapivots();
    unsigned short getNbContrapivots();
    
    // Accessers
    inline list<SubnetInterface*> *getInterfacesList() { return &interfaces; }
    inline InetAddress getInitialPivot() { return initialPivot; }
    inline InetAddress getPrefix() { return prefix; }
    inline unsigned short getPrefixLength() { return prefixLength; }
    inline string getStopDescription() { return stopDescription; }
    inline bool needsPostProcessing() { return toPostProcess; }
    inline void setStopDescription(string desc) { stopDescription = desc; }
    inline void setToPostProcess(bool pp) { toPostProcess = pp; }
    
    // Methods to handle borders in the wide sense
    InetAddress getLowerBorder(bool withLimits = true); // Include network/broadcast addresses
    InetAddress getUpperBorder(bool withLimits = true);
    bool contains(InetAddress interface);
    bool overlaps(Subnet *other);
    
    // Shrinks/expands the subnet
    void expand();
    void shrink(list<IPTableEntry*> *out = NULL); // (out => IPs no longer within the borders)
    void adjustSize(); // Shrinks up to just before the prefix where listed IPs are out of subnet
    
    // Various outputs methods
    string getCIDR(); // CIDR notation = prefix_IP/prefix_length (e.g. 10.0.0.0/8)
    string toString();
    
    // Methods to sort and compare subnets
    static inline bool compare(Subnet *s1, Subnet *s2) { return s1->prefix < s2->prefix; }
    
private:

    // Initial pivot IP, prefix, prefix length and list of interfaces
    InetAddress initialPivot, prefix;
    unsigned short prefixLength;
    list<SubnetInterface*> interfaces;
    
    // String containing a short but detailed description of why subnet growth stopped.
    string stopDescription;
    
    /*
     * Special flag used for subnet post-processing. It signals that the growth stopped because 
     * the subnet started overlapping a previously inferred subnet, potentially compromising the 
     * accuracy of the latter. However, due to contra-pivot positioning, having an actually 
     * undergrown subnet remains a possibility and needs to be mitigated as much as possible.
     */
    
    bool toPostProcess;
    
    // Re-computes prefix (called after expanding/shrinking)
    void resetPrefix();

};

#endif /* SUBNET_H_ */
