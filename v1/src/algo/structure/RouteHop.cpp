/*
 * RouteHop.cpp
 *
 *  Created on: Sept 6, 2018
 *      Author: jefgrailet
 *
 * Implements the class defined in RouteHop.h (see this file to learn further about the goals of 
 * such a class).
 */

#include "RouteHop.h"

RouteHop::RouteHop()
{
    this->ip = InetAddress(0);
    this->state = NOT_MEASURED;
}

RouteHop::RouteHop(InetAddress ip)
{
    this->ip = ip;
    if(ip == InetAddress(0))
        this->state = ANONYMOUS;
    else
        this->state = VIA_TRACEROUTE;
}

RouteHop::~RouteHop()
{
}

void RouteHop::reset()
{
    this->ip = InetAddress(0);
    this->state = NOT_MEASURED;
}

void RouteHop::update(InetAddress ip, bool peer)
{
    this->ip = ip;
    if(ip != InetAddress(0))
    {
        if(peer)
            this->state = PEERING_POINT;
        else
            this->state = VIA_TRACEROUTE;
    }   
    else
        this->state = ANONYMOUS;
}

RouteHop &RouteHop::operator=(const RouteHop &other)
{
    this->ip = other.ip;
    this->state = other.state;
    return *this;
}
