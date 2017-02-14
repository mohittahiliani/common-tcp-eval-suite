/* -*- Mode:C++; c-file-style:''gnu''; indent-tabs-mode:nil; -*- */

/*
 * Copyright 2012, Old Dominion University
 * Copyright 2012, University of North Carolina at Chapel Hill
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *    1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *    2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *    3. The name of the author may not be used to endorse or promote
 * products derived from this software without specific prior written
 * permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * M.C. Weigle, P. Adurthi, F. Hernandez-Campos, K. Jeffay, and F.D. Smith,
 * Tmix: A Tool for Generating Realistic Application Workloads in ns-2,
 * ACM Computer Communication Review, July 2006, Vol 36, No 3, pp. 67-76.
 *
 * Contact: Michele Weigle (mweigle@cs.odu.edu)
 * http://www.cs.odu.edu/inets/Tmix
 */

#ifndef TCP_FLOW_CLASSIFIER_H_
#define TCP_FLOW_CLASSIFIER_H_

#include "ns3/tcp-header.h"
#include "ns3/ipv4-flow-classifier.h"

namespace ns3 {

// TODO: have this watch for FINs to make sure that we don't erroneously reuse a flow id in the event that a new conversation is started on the same ports

/**
 * \brief Like Ipv4FlowClassifier, but sorts both sides of a TCP conversation into a single flow ID.
 *
 * Similar to Ipv4FlowClassifier, a tuple (source-ip, destination-ip,
 * source-port, destination-port) is extracted from the packet
 * headers, and a unique flow identified is assigned to each
 * tuple. TcpFlowClassifier sorts this tuple so that packets traveling
 * in both directions are mapped to the same flow ID.
 *
 * \see Ipv4FlowClassifier
 */
class TcpFlowClassifier : public FlowClassifier
{
public:
  typedef std::pair<Ipv4Address, uint16_t> TwoTuple;
  typedef std::pair<TwoTuple, TwoTuple> FourTuple;

  TcpFlowClassifier ();

  /**
   * Try to classify the packet into a flow, creating a new flow if it
   * does not match any existing one.
   *
   * \param ipHeader The packet's IP header, already split off.
   * \param ipPayload A packet with a TCP header on top.
   * \param out_flowId If not null, the packet's flow id will be written here on success.
   * \param out_packetId If not null, the packet's id (unique within
   * the flow) will be written here on success.
   * \return true if the packet was successfully classified (either
   * into an existing flow or a new one), false if it could not be
   * classified for some reason.
   */
  bool
  Classify (const Ipv4Header& ipHeader, Ptr<const Packet> ipPayload,
            uint32_t *out_flowId, uint32_t *out_packetId);

  /**
   * Try to classify the packet into a flow, creating a new flow if it
   * does not match any existing one.
   *
   * \param ipHeader The packet's IP header.
   * \param tcpHeader The packet's TCP header.
   * \param out_flowId If not null, the packet's flow id will be written here on success.
   * \param out_packetId If not null, the packet's id (unique within
   * the flow) will be written here on success.
   * \return true if the packet was successfully classified (either
   * into an existing flow or a new one), false if it could not be
   * classified for some reason.
   */
  bool
  Classify (const Ipv4Header& ipHeader, const TcpHeader& tcpHeader,
            uint32_t *out_flowId, uint32_t *out_packetId);

  /**
   * Searches for the FourTuple corresponding to the given flowId.
   * \param flowId The flow ID to search for.
   * \param out If not null, the flow's information will be written
   * here if the flow is found.
   * \return true if the flow was found, otherwise false.
   */
  bool
  FindFlow (FlowId flowId, FourTuple* out) const;

  /**
   * FIXME: Not implemented.
   */
  virtual void
  SerializeToXmlStream (std::ostream &os, int indent) const;

private:
  std::map<FourTuple, FlowId> m_flowMap;

};

}

#endif
