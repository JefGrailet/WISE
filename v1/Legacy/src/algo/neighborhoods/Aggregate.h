/*
 * Aggregate.h
 *
 *  Created on: Aug 22, 2019
 *      Author: jefgrailet
 *
 * This class models an aggregate, which is the most basic form of neighborhood, i.e., a group of 
 * subnets which are located at at most one hop away from each other. Aggregates are inferred on 
 * the basis of trails; i.e., if the last hop(s) before reaching a set of subnets are identical, 
 * then the last router crossed before reaching these subnets is likely the same or part of a 
 * mesh, and therefore subnets are in the vicinity of each other. The data collected along subnets 
 * can later be used to dig deeper into the topology of the target domain.
 *
 * Subnets are of course typically aggregated when they have the same trail, but due to echoing 
 * and flickering trails, there are some situations where aggregating on another basis is 
 * preferable. However, the Aggregate class is only a data structure; it's up to other classes 
 * handling its instances to ensure they are consistent.
 *
 * Aggregate class still, however, provides a method to detect the peer(s) (i.e., IPs that 
 * appear in trails of other detected aggregates/subnets) once the aggregate has been fully 
 * constituted.
 */

#ifndef AGGREGATE_H_
#define AGGREGATE_H_

#include "../structure/Subnet.h"
#include "../structure/IPLookUpTable.h"

class Aggregate
{
public:

    // Constructors/destructor
    Aggregate(Subnet *first);
    Aggregate(list<Subnet*> subnets);
    ~Aggregate();
    
    /*
     * N.B.: next methods don't check the subnets are in the same neighborhood. It's up to the 
     * calling code to verify this; next methods only inserts the subnets and updates the list of 
     * trails accordingly. It is also assumed, for addSubnets(), that the provided subnets all 
     * have the same trail.
     */
    
    void addSubnet(Subnet *newSubnet);
    void addSubnets(list<Subnet*> newSubnets);
    
    /*
     * To call once the aggregate has been fully built, finalize() is responsible for listing all 
     * potential peers (as IPs) and sorting the list of subnets, using the IP dictionary to check 
     * a potential peer is indeed used to identify a neighborhood (an IP appearing in a trail does 
     * not always end up to denote a neighborhood, this is notably the case of contra-pivot IPs).
     */
    
    void finalize(IPLookUpTable *dictionary);
    
    // Accessers
    inline list<Trail*> *getTrails() { return &trails; }
    inline list<Subnet*> *getSubnets() { return &subnets; }
    inline list<InetAddress> *getPeers() { return &peers; }
    inline unsigned short getPeersOffset() { return peersOffset; }
    
    // Setter
    inline void setPreEchoingOffset(unsigned short offset) { preEchoingOffset = offset; }
    inline void setPreEchoing(list<InetAddress> preEchoing) { this->preEchoing = preEchoing; }
    
    // Methods providing metrics, to be expanded later
    inline unsigned short getNbSubnets() { return (unsigned short) subnets.size(); }
    inline unsigned short getNbPeers() { return (unsigned short) peers.size(); }
    
    // Comparison method for sorting
    static bool compare(Aggregate *a1, Aggregate *a2);
    
    // String methods
    string getLabel();
    string toString();

private:
    
    // Main fields
    list<Subnet*> subnets;
    
    /*
     * "trails" stores the trail(s) of the aggregated subnets. It can have multiple trails due to 
     * the existence of flickering trails, but most of the time the list will contain only one 
     * entry. In the case of an aggregate of subnets based on the 3rd rule of inference (echo 
     * rule), "trails" is left empty since there's no common trail, and the "label" of the 
     * aggregate (i.e. string returned by the getLabel() method) will be rather based on the 
     * the TTL of the pivot IPs (always the same in this case) and "pre-echoing" IPs, i.e., the 
     * first non-anonymous IPs appearing before the echoing trails in the partial routes (which 
     * can also be, but are not always, peers).
     */
    
    list<Trail*> trails;
    list<InetAddress> preEchoing;
    unsigned short preEchoingOffset;
    
    // Peer stuff
    list<InetAddress> peers;
    unsigned short peersOffset; // Difference in TTL between trail and first peer(s) (ideally 0)
    
    // Private method(s)
    void updateTrails(Trail *newTrail);

};

#endif /* AGGREGATE_H_ */
