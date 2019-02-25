/*
 * DirectTCPProber.h
 *
 *  Created on: Dec 13, 2009
 *      Author: root
 *
 * Modifications brought by J.-F. Grailet since ExploreNET v2.1:
 * -September 2015: slight edit to improve coding style.
 * -March 4, 2016: slightly edited to comply to the (slightly) extended ProbeRecord class.
 * -August 2016: removed an old test method and added the new debug mechanics of TreeNET v3.0. 
 *  Also removed the loose source and record route options, because they are unused by both 
 *  TreeNET and ExploreNEt v2.1 and because the IETF (see RFC 7126) reports that packets featuring 
 *  these options are widely dropped, and that the default policy of a router receiving such 
 *  packets should be to drop them anyway due to security concerns.
 * -August 2017: drastically updated the class. SYN+ACK probe with payload is outdated. The 
 *  DirectTCPProber class now uses a SYN probe without payload.
 * -August 2018: updated the code to remove deprecated C++ features such as auto_ptr<> which 
 *  prevent the compilation without warnings on more recent systems.
 *
 * N.B.: as of August 2017, the dest port for TCP probes is always set to 80 by DirectProber 
 * (see getAvailableDstPortICMPseq(), singleProbe() and doubleProbe() methods). The port variables 
 * in the constructor could therefore be removed but are left there just in case. Same remark goes 
 * for attention message, which is unused in DirectTCPProber. Indeed, if these variables became 
 * relevant again in the future, this would spare the effort of rewriting the call of the 
 * DirectTCPProber constructor in all classes where TCP probing can be used.
 */

#ifndef DIRECTTCPPROBER_H_
#define DIRECTTCPPROBER_H_

#define DEFAULT_DIRECT_TCP_PROBER_BUFFER_SIZE 512
#define DEFAULT_TCP_PSEUDO_HEADER_LENGTH 512

#include <cstdio>
#include <cstring>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <cstdlib>

#include "../DirectProber.h"
#include "../../common/inet/InetAddress.h"
#include "../exception/SocketSendException.h"
#include "../exception/SocketReceiveException.h"
#include "../exception/SocketException.h"
#include "../structure/ProbeRecord.h"
#include "../../common/date/TimeVal.h"

class DirectTCPProber : public DirectProber
{
public:

    static const unsigned short DEFAULT_LOWER_TCP_SRC_PORT;
    static const unsigned short DEFAULT_UPPER_TCP_SRC_PORT;
    static const unsigned short DEFAULT_LOWER_TCP_DST_PORT;
    static const unsigned short DEFAULT_UPPER_TCP_DST_PORT;
    
    DirectTCPProber(string &attentionMessage, 
                    int tcpUdpRoundRobinSocketCount, 
                    const TimeVal &timeoutPeriod = DirectProber::DEFAULT_TIMEOUT_PERIOD, 
                    const TimeVal &probeRegulatorPausePeriod = DirectProber::DEFAULT_PROBE_REGULATOR_PAUSE_PERIOD, 
                    unsigned short lowerBoundTCPsrcPort = DirectTCPProber::DEFAULT_LOWER_TCP_SRC_PORT, 
                    unsigned short upperBoundTCPsrcPort = DirectTCPProber::DEFAULT_UPPER_TCP_SRC_PORT, 
                    unsigned short lowerBoundTCPdstPort = DirectTCPProber::DEFAULT_LOWER_TCP_DST_PORT, 
                    unsigned short upperBoundTCPdstPort = DirectTCPProber::DEFAULT_UPPER_TCP_DST_PORT, 
                    bool verbose = false);
    virtual ~DirectTCPProber();
    
    /**
     * Returns ICMP type DirectProber::PSEUDO_TCP_RESET_ICMP_TYPE, code 
     * DirectProber::PSEUDO_TCP_RESET_ICMP_CODE in case a TCP RESET is obtained from the 
     * destination. Otherwise returns ICMP type and codes declared by IANA.
     */
    virtual ProbeRecord *basic_probe(const InetAddress &src, 
                                     const InetAddress &dst, 
                                     unsigned short IPIdentifier, 
                                     unsigned char TTL, 
                                     bool usingFixedFlowID, 
                                     unsigned short srcPort, 
                                     unsigned short dstPort);

protected:

    ProbeRecord *buildProbeRecord(TimeVal reqTime, 
                                  const InetAddress &dstAddress, 
                                  const InetAddress &rplyAddress, 
                                  unsigned char reqTTL, 
                                  unsigned char rplyTTL, 
                                  unsigned char replyType, 
                                  unsigned char rplyCode, 
                                  unsigned short srcIPidentifier, 
                                  unsigned short rplyIPidentifier,  
                                  unsigned char payloadTTL, 
                                  unsigned short payloadLength, 
                                  int probingCost, 
                                  bool usingFixedFlowID);
    
    uint8_t buffer[DEFAULT_DIRECT_TCP_PROBER_BUFFER_SIZE];
    uint8_t pseudoBuffer[DEFAULT_TCP_PSEUDO_HEADER_LENGTH];

};

#endif /* DIRECTTCPPROBER_H_ */
