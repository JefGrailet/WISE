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

void RouteHop::update(InetAddress ip)
{
    this->ip = ip;
    if(ip == InetAddress(0))
        this->state = ANONYMOUS;
    else
        this->state = VIA_TRACEROUTE;
}

void RouteHop::anonymize()
{
    this->ip = InetAddress(0);
    this->state = ANONYMOUS;
}

void RouteHop::repair(InetAddress ip)
{
    this->ip = ip;
    this->state = REPAIRED_1;
}

void RouteHop::repairBis(InetAddress ip)
{
    this->ip = ip;
    this->state = REPAIRED_2;
}

void RouteHop::deanonymize(InetAddress ip)
{
    this->ip = ip;
    this->state = LIMITED;
}

RouteHop &RouteHop::operator=(const RouteHop &other)
{
    this->ip = other.ip;
    this->state = other.state;
    return *this;
}
