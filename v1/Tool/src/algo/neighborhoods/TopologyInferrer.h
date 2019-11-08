/*
 * TopologyInferrer.h
 *
 *  Created on: Aug 22, 2019
 *      Author: jefgrailet
 *
 * TopoloygInferrer is the main class implementing the algorithmical steps for discovering the 
 * neighborhoods of a target domain and infer how they are located w.r.t. each other, using the 
 * additional (partial) traceroute data collected by the PeerScanner class.
 *
 * It currently just infers neighborhoods, as "Aggregate" objects (see the Aggregate class), and 
 * provides a method to write them in an output file.
 */

#ifndef TOPOLOGYINFERRER_H_
#define TOPOLOGYINFERRER_H_

#include "../Environment.h"
#include "Aggregate.h"

class TopologyInferrer
{
public:

    // Constructor, destructor
    TopologyInferrer(Environment &env);
    ~TopologyInferrer();
    
    void infer();
    void outputNeighborhoods(string filename); // As aggregates, for now

private:

    // Pointer to the environment singleton
    Environment &env;
    
    // List of inferred neighborhoods (as aggregates first; to re-name later ?)
    list<Aggregate*> neighborhoods;

};

#endif /* TOPOLOGYINFERRER_H_ */
