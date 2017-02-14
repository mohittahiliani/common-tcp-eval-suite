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

#include "tmix-ns2-style-trace-helper.h"

#include <iomanip>
#include "ns3/ipv4-interface.h"
#include "ns3/ppp-header.h"
#include "ns3/log.h"

NS_LOG_COMPONENT_DEFINE ("TmixNs2StyleTraceHelper");

namespace ns3 {
namespace Tmix {

Ns2StyleTraceHelper::Ns2StyleTraceHelper (Ptr<TmixTopology> tmix,
                                          std::ostream& out)
  : m_tmix (tmix),
    m_out (out)
{
  m_idOfNodeType[TmixTopology::LEFT_INITIATOR] = 0;
  m_idOfNodeType[TmixTopology::LEFT_ACCEPTOR] = 2;
  m_idOfNodeType[TmixTopology::RIGHT_ACCEPTOR] = 1;
  m_idOfNodeType[TmixTopology::RIGHT_INITIATOR] = 3;
  m_idOfNodeType[TmixTopology::LEFT_ROUTER] = 4;
  m_idOfNodeType[TmixTopology::RIGHT_ROUTER] = 5;
}

/**
 * Return the node's public-side device.
 */
Ptr<PointToPointNetDevice>
Ns2StyleTraceHelper::GetDevice (TmixTopology::NodeType node)
{
  switch (node)
    {
    case TmixTopology::LEFT_INITIATOR:
      return m_left.initiatorDevice;
    case TmixTopology::LEFT_ACCEPTOR:
      return m_right.acceptorDevice;
    case TmixTopology::RIGHT_INITIATOR:
      return m_right.initiatorDevice;
    case TmixTopology::RIGHT_ACCEPTOR:
      return m_left.acceptorDevice;
    case TmixTopology::LEFT_ROUTER:
      return m_tmix->GetLeftRouterDevice ();
    case TmixTopology::RIGHT_ROUTER:
      return m_tmix->GetRightRouterDevice ();
    default:
      NS_FATAL_ERROR ("Ns2StyleTraceHelper::GetDevice");
      return 0;
    }
}

Ipv4Address
Ns2StyleTraceHelper::GetAddress (TmixTopology::NodeType node)
{
  switch (node)
    {
    case TmixTopology::LEFT_INITIATOR:
      return m_left.initiatorAddress;
    case TmixTopology::LEFT_ACCEPTOR:
      return m_right.acceptorAddress;
    case TmixTopology::RIGHT_INITIATOR:
      return m_right.initiatorAddress;
    case TmixTopology::RIGHT_ACCEPTOR:
      return m_left.acceptorAddress;
    default:
      NS_FATAL_ERROR ("Ns2StyleTraceHelper::GetAddress");
      return 0;
    }
}

/**
 * Return the appropriate router's private-side net device connected to the given node.
 */
Ptr<PointToPointNetDevice>
Ns2StyleTraceHelper::GetRouterDevice (TmixTopology::NodeType node)
{
  switch (node)
    {
    case TmixTopology::LEFT_INITIATOR:
      return m_left.routerInitiatorDevice;
    case TmixTopology::LEFT_ACCEPTOR:
      return m_right.routerAcceptorDevice;
    case TmixTopology::RIGHT_INITIATOR:
      return m_right.routerInitiatorDevice;
    case TmixTopology::RIGHT_ACCEPTOR:
      return m_left.routerAcceptorDevice;
    case TmixTopology::LEFT_ROUTER:
      return m_tmix->GetLeftRouterDevice ();
    case TmixTopology::RIGHT_ROUTER:
      return m_tmix->GetRightRouterDevice ();
    default:
      NS_FATAL_ERROR ("Ns2StyleTraceHelper::GetRouterDevice");
      return 0;
    }
}

void
Ns2StyleTraceHelper::Install ()
{  
  // Ensure that there is an equal number of left and right pairs
  while (m_tmix->LeftPairs ().size () < m_tmix->RightPairs ().size ())
    {
      m_tmix->NewPair (TmixTopology::LEFT, Seconds(0), Seconds(0),3,3);
    }
  while (m_tmix->RightPairs ().size () < m_tmix->LeftPairs ().size ())
    {
      m_tmix->NewPair (TmixTopology::RIGHT, Seconds(0), Seconds(0),3,3);
    } //
  NS_ASSERT (m_tmix->LeftPairs ().size () == m_tmix->RightPairs ().size ());

  std::vector<TmixTopology::TmixNodePair>::iterator leftI, rightI;
  leftI = m_tmix->LeftPairs ().begin ();
  rightI = m_tmix->RightPairs ().begin ();
  m_uniqid = 0;

  while (leftI != m_tmix->LeftPairs ().end () && rightI
         != m_tmix->RightPairs ().end ())
    {
      m_left = *leftI;
      m_right = *rightI;
      // left initiator <-> right acceptor
      Install (TmixTopology::LEFT_INITIATOR, TmixTopology::LEFT_ROUTER,
               TmixTopology::RIGHT_ROUTER, TmixTopology::RIGHT_ACCEPTOR);
      Install (TmixTopology::RIGHT_ACCEPTOR, TmixTopology::RIGHT_ROUTER,
               TmixTopology::LEFT_ROUTER, TmixTopology::LEFT_INITIATOR);

      // right initiator <-> left acceptor
      Install (TmixTopology::RIGHT_INITIATOR, TmixTopology::RIGHT_ROUTER,
               TmixTopology::LEFT_ROUTER, TmixTopology::LEFT_ACCEPTOR);
      Install (TmixTopology::LEFT_ACCEPTOR, TmixTopology::LEFT_ROUTER,
               TmixTopology::RIGHT_ROUTER, TmixTopology::RIGHT_INITIATOR);

      leftI++;
      rightI++;
      m_uniqid++;
    }
}

void
Ns2StyleTraceHelper::Install (TmixTopology::NodeType start,
                              TmixTopology::NodeType router1, TmixTopology::NodeType router2,
                              TmixTopology::NodeType end)
{
  Ipv4Address src = GetAddress (start);
  Ipv4Address dst = GetAddress (end);
  // Enqueue from start
  Install (GetDevice (start), "MacTx", ENQUEUE, start, router1, src, dst);
  // Dequeue from start
  Install (GetDevice (start), "PhyTxBegin", DEQUEUE, start, router1, src, dst);
  // Receive at first router
  Install (GetRouterDevice (start), "PhyRxEnd", RECEIVE, start, router1, src, dst);

  /*Original Code: Does not show the traffic between routers for node 1 and node 3

  // Enqueue from first router
  Install(GetDevice(router1), "MacTx", ENQUEUE, router1, router2, src,
      dst);
  // Dequeue from first router
  Install(GetDevice(router1), "PhyTxBegin", DEQUEUE, router1, router2,
      src, dst);
  // Receive at second router
  Install(GetDevice(router2), "PhyRxEnd", RECEIVE, router1, router2, src,
      dst);
  */

  //This modified code is more of a hack than a fix. There appears to be an issue when
  //the router1 and router2 are used for node 1 and node 3
  if (start == TmixTopology::LEFT_INITIATOR || start == TmixTopology::RIGHT_ACCEPTOR)
    {
      // Enqueue from first router
      Install (GetDevice (router1), "MacTx", ENQUEUE, router1, router2, src,
               dst);
      // Dequeue from first router
      Install (GetDevice (router1), "PhyTxBegin", DEQUEUE, router1, router2,
               src, dst);
      // Receive at second router
      Install (GetDevice (router2), "PhyRxEnd", RECEIVE, router1, router2, src,
               dst);
    }
  else
    {
      // Enqueue from first router
      Install (GetDevice (router2), "MacTx", ENQUEUE, router1, router2, src,
               dst);
      // Dequeue from first router
      Install (GetDevice (router2), "PhyTxBegin", DEQUEUE, router1, router2,
               src, dst);
      // Receive at second router
      Install (GetDevice (router1), "PhyRxEnd", RECEIVE, router1, router2, src,
               dst);
    }


  // Enqueue from second router
  Install (GetRouterDevice (end), "MacTx", ENQUEUE, router2, end, src, dst);
  // Dequeue from second router
  Install (GetRouterDevice (end), "PhyTxBegin", DEQUEUE, router2, end, src, dst);
  // Receive at end
  Install (GetDevice (end), "PhyRxEnd", RECEIVE, router2, end, src, dst);
}

void
Ns2StyleTraceHelper::Install (Ptr<PointToPointNetDevice> device,
                              std::string trace, TraceEvent event, TmixTopology::NodeType llsrc,
                              TmixTopology::NodeType lldst, Ipv4Address src, Ipv4Address dst)
{
  device->TraceConnectWithoutContext (trace, MakeCallback (
                                        &Ns2StyleTraceHelper::OutputTrace, this).Bind (TraceSpec (event, llsrc,
                                                                                                  lldst, src, dst, m_uniqid)));
}

void
Ns2StyleTraceHelper::WriteInformativeHeader ()
{
  m_out
  << "# event time llsrc lldst name size flags flowid src dst seqno uniqid ackno tcpflags hdrlen sockaddrlen"
  << std::endl;
}

void
Ns2StyleTraceHelper::OutputTrace (TraceSpec ts, Ptr<const Packet> packet)
{
  PppHeader pppHeader;
  Ipv4Header ipHeader;
  TcpHeader tcpHeader;
  {
    Ptr<Packet> p = packet->Copy ();
    p->RemoveHeader (pppHeader);
    p->RemoveHeader (ipHeader);
    if (ipHeader.GetFragmentOffset () > 0)
      {
        // XXX need to be more robust to fragmentation and also IPv6
        // for now, skip peeking the TCP header that is not really there
        NS_LOG_WARN ("Tmix does not handle fragmentation; adjust MTU so fragmentation doesn't occur");
      }
    else
      {
        p->PeekHeader (tcpHeader);
      }
  }

  NS_LOG_FUNCTION (ts.event << ts.llsrc << ts.lldst << ipHeader.GetSource () << ipHeader.GetDestination () << packet->GetSize ());

  // Check that this packet belongs to the topology
  NS_ASSERT_MSG (m_tmix->NodeTypeByAddress (ipHeader.GetSource ()) != TmixTopology::INVALID, "Couldn't match packet's source with a Tmix node"); //
  NS_ASSERT_MSG (m_tmix->NodeTypeByAddress (ipHeader.GetDestination ()) != TmixTopology::INVALID, "Couldn't match packet's destination with a Tmix node");

  // Make sure this packet matches our src/dst filter
  if (ipHeader.GetSource () != ts.src || ipHeader.GetDestination () != ts.dst)
    {
      NS_LOG_LOGIC (ipHeader.GetSource () << " != " << ts.src << " || " << ipHeader.GetDestination () << " != " << ts.dst);
      return;
    }

  // Column 1: Event. '+' means enqueued, '-' means dequeued, 'r'
  // means received, 'd' means dropped.
  if (ts.event == ENQUEUE)
    {
      m_out << '+';
    }
  else if (ts.event == DEQUEUE)
    {
      m_out << '-';
    }
  else if (ts.event == RECEIVE)
    {
      m_out << 'r';
    }
  else if (ts.event == DROP)
    {
      m_out << 'd';
    }
  m_out << ' ';

  // Column 2: The time in seconds when the event occurred.
  m_out << std::setiosflags (std::ios::fixed) << std::setprecision (6) << Simulator::Now ().GetSeconds ();
  m_out << ' ';

  // Column 3: Link-layer source node ID.
  m_out << m_idOfNodeType[ts.llsrc];
  m_out << ' ';

  // Column 4: Link-layer destination node ID.
  m_out << m_idOfNodeType[ts.lldst];
  m_out << ' ';

  // Column 5: Packet name. Unused, always "ack" (in ns-2 one-way TCP
  // mode it would be different).
  m_out << "ack";
  m_out << ' ';

  // Column 6: Total packet size in bytes.
  // (we subtract the size of the PPP header)
  m_out << packet->GetSize () - 2;
  m_out << ' ';

  // Column 7: Flags. Unused, always "-------".
  m_out << "-------";
  m_out << ' ';

  // Column 8: Integer flow ID. Currently unused and always set to zero. TODO: implement.
  m_out << 0;
  m_out << ' ';

  // Column 9: Source. Format is the node ID and the port number
  // separated by a period, e.g. "0.1"
  m_out << m_idOfNodeType[m_tmix->NodeTypeByAddress (ipHeader.GetSource ())]
        << '.' << ((uint32_t (ts.uniqid) << 16) + tcpHeader.GetSourcePort ());
  m_out << ' ';

  // Column 10: Destination. Same format as source.
  m_out << m_idOfNodeType[m_tmix->NodeTypeByAddress (ipHeader.GetDestination ())]
        << '.' << ((uint32_t (ts.uniqid) << 16) + tcpHeader.GetDestinationPort ());
  m_out << ' ';

  // Column 11: Packet sequence number.
  m_out << tcpHeader.GetSequenceNumber ().GetValue ();
  m_out << ' ';

  // Column 12: Unique packet ID.
  // FIXME: using Packet::GetUid may not be correct as it might change
  // with fragmentation perhaps
  m_out << packet->GetUid ();
  m_out << ' ';

  // Column 13: Ack number.
  m_out << tcpHeader.GetAckNumber ();
  m_out << ' ';

  // Column 14: TCP flags as a hexadecimal value, e.g. "0xa" for PUSH|SYN.
  m_out << "0x" << std::hex << std::setfill ('0') << std::setw (2) << int(
    tcpHeader.GetFlags ());
  m_out << std::dec << std::setw (0) << ' ';

  // Column 15: Header length. Unused, always equal 40.
  m_out << 40;
  m_out << ' ';

  // Column 16: Socket address length. Unused, always zero.
  m_out << 0;
  m_out << ' ';

  // Done.
  m_out << std::endl;
}

}
}
