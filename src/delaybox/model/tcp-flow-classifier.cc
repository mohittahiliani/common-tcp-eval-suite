/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

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

#include "tcp-flow-classifier.h"

#include "ns3/tcp-header.h"
#include "ns3/packet.h"

namespace ns3 {

const uint8_t TCP_PROT_NUMBER = 6;

TcpFlowClassifier::TcpFlowClassifier ()
{
}

bool
TcpFlowClassifier::Classify (const Ipv4Header& ipHeader,
                             Ptr<const Packet> ipPayload, uint32_t *out_flowId, uint32_t *out_packetId)
{
  if (ipHeader.GetProtocol () != TCP_PROT_NUMBER)
    {
      return false;
    }
  TcpHeader tcpHeader;
  ipPayload->PeekHeader (tcpHeader);
  return Classify (ipHeader, tcpHeader, out_flowId, out_packetId);
}

bool
TcpFlowClassifier::Classify (const Ipv4Header& ipHeader,
                             const TcpHeader& tcpHeader, uint32_t *out_flowId, uint32_t *out_packetId)
{
  if (ipHeader.GetProtocol () != TCP_PROT_NUMBER)
    {
      return false;
    }
  if (ipHeader.GetDestination () == Ipv4Address::GetBroadcast ())
    {
      // we are not prepared to handle broadcast
      return false;
    }

  TwoTuple node1;
  TwoTuple node2;
  node1.first = ipHeader.GetSource ();
  node1.second = tcpHeader.GetSourcePort ();
  node2.first = ipHeader.GetDestination ();
  node2.second = tcpHeader.GetDestinationPort ();
  // sort the tuple into some canonical order so that both directions
  // map to the same flow id
  if (node2 < node1)
    {
      std::swap (node1, node2);
    }
  FourTuple tuple;
  tuple.first = node1;
  tuple.second = node2;

  // try to insert the tuple, but check if it already exists
  std::pair<std::map<FourTuple, FlowId>::iterator, bool> insert =
    m_flowMap.insert (std::pair<FourTuple, FlowId> (tuple, 0));

  // if the insertion succeeded, we need to assign this tuple a new flow identifier
  if (insert.second)
    {
      insert.first->second = GetNewFlowId ();
    }

  if (out_flowId)
    {
      *out_flowId = insert.first->second;
    }
  if (out_packetId)
    {
      *out_packetId = ipHeader.GetIdentification ();
    }

  return true;
}

bool
TcpFlowClassifier::FindFlow (FlowId flowId, FourTuple* out) const
{
  for (std::map<FourTuple, FlowId>::const_iterator i = m_flowMap.begin (); i
       != m_flowMap.end (); ++i)
    {
      if (i->second == flowId)
        {
          if (out)
            {
              *out = i->first;
            }
          return true;
        }
    }
  return false;
}

void
TcpFlowClassifier::SerializeToXmlStream (std::ostream &os, int indent) const
{
  // FIXME: Not implemented.
  NS_ASSERT_MSG (false, "TcpFlowClassifier::SerializeToXmlStream not implemented");
}

}
