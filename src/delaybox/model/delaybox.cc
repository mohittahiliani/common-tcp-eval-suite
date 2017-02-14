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

#include "delaybox.h"

#include "ns3/log.h"
#include "ns3/tcp-header.h"
#include "ns3/ppp-header.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("DelayBox");
NS_OBJECT_ENSURE_REGISTERED (DelayBox);

DelayBoxRuleKey::DelayBoxRuleKey (Ipv4Address src, uint16_t srcPort,
                                  Ipv4Address dst, uint16_t dstPort, bool symmetric)
  : m_src (src, srcPort),
    m_dst (dst, dstPort)
{
  if (symmetric)
    {
      // This will cause both directions to map to the same canonical key.
      if (m_dst < m_src)
        {
          std::swap (m_dst, m_src);
        }
    }
}

bool
DelayBoxRuleKey::operator< (const DelayBoxRuleKey& rhs) const
{
  if (m_src < rhs.m_src)
    {
      return true;
    }
  else if (m_src == rhs.m_src)
    {
      return m_dst < rhs.m_dst;
    }
  else
    {
      return false;
    }
}

bool
DelayBoxRuleKey::operator== (const DelayBoxRuleKey& rhs) const
{
  return m_src == rhs.m_src && m_dst == rhs.m_dst;
}

TypeId
DelayBox::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::DelayBox") //
    .SetParent<Object> ()   //
    .AddConstructor<DelayBox> ()   //
  ;
  return tid;
}

DelayBox::DelayBox ()
  : m_symmetric (true)
{
}

void
DelayBox::SetSymmetric (bool symmetric)
{
  m_symmetric = symmetric;
}

bool
DelayBox::Classify (const Ipv4Header& ipHeader, Ptr<const Packet> ipPayload,
                    uint32_t* flowId, uint32_t* packetId)
{
  if (m_symmetric)
    {
      return m_symmetricClassifier.Classify (ipHeader, ipPayload, flowId,
                                             packetId);
    }
  else
    {
      return m_asymmetricClassifier.Classify (ipHeader, ipPayload, flowId,
                                              packetId);
    }
}

/*
 * Old logic from the ns-2 implementation:
 *      1. If forward flow isn't found...
 *              1a. If symmetric and it's a SYN+ACK, add reverse flow
 *              1b. Else if it's a SYN, add forward flow
 *              1c. Else it's a rogue packet or something, foward it immediately.
 *      To add the foward flow:
 *              2a. Look up the forward rule.
 *              2b. If not found and symmetric, look up the reverse rule.
 *              2c. If still not found, forward immediately.
 *              2d. Go ahead and add the flow.
 *      To add the reverse flow:
 *              3a. Look up the reverse flow.
 *              3b. If not found, forward immediately.
 *              3c. Add this packet's direction to the flow table by
 *              copying the reverse flow's parameters.
 *
 */
bool
DelayBox::Delay (Callback<void> send, Ptr<const Packet> packet)
{
  // TODO: instead of digging through headers here, we should take
  // them as parameters to this function.
  Ptr<Packet> ipPayload = packet->Copy ();
  Ipv4Header ipHeader;
  ipPayload->RemoveHeader (ipHeader);
  TcpHeader tcpHeader;
  if (packet->GetSize()<36)
    {
      send ();
      return true;
    }
  ipPayload->PeekHeader (tcpHeader);

  NS_LOG_LOGIC ("Considering packet: " << ipHeader.GetSource () << ":" << tcpHeader.GetSourcePort () << " -> " << ipHeader.GetDestination () << ":" << tcpHeader.GetDestinationPort ());

  uint32_t flowId;
  uint32_t packetId;
  if (Classify (ipHeader, ipPayload, &flowId, &packetId))
    {
      NS_LOG_LOGIC ("Packet classified: flow " << flowId << ", packet " << packetId);
    }
  else
    {
      NS_LOG_WARN ("Couldn't classify packet.");
      send ();
      return true;
    }

  DelayBoxRule rule;
  if (m_ruleTable.Lookup (ipHeader.GetSource (), tcpHeader.GetSourcePort (),
                          ipHeader.GetDestination (), tcpHeader.GetDestinationPort (), m_symmetric,
                          rule))
    {
      return m_flowTable.Enqueue (flowId, rule, send, packet->GetSize (),
                                  tcpHeader);
    }
  else
    {
      NS_LOG_DEBUG ("Packet didn't match a rule; sending immediately");
      send ();
      return true;
    }
}

/*bool
DelayBox::Delay (Callback<void> send, Ptr<const QueueDiscItem> item)
{
  // TODO: instead of digging through headers here, we should take
  // them as parameters to this function.
  Ptr<Packet> packet = item->GetPacket ();
  Ptr<Packet> ipPayload = packet->Copy ();
  Ipv4Header ipHeader;
  Ptr<const Ipv4QueueDiscItem> iqd = Ptr<const Ipv4QueueDiscItem> (dynamic_cast<const Ipv4QueueDiscItem *> (PeekPointer (item)));
  ipHeader = iqd->GetHeader ();
  TcpHeader tcpHeader;
  if (packet->GetSize()<36)
    {
      send ();
      return true;
    }
  NS_LOG_LOGIC ("Before Peek Header");
  ipPayload->PeekHeader (tcpHeader);
  NS_LOG_LOGIC ("After Peek Header");

  NS_LOG_LOGIC ("Considering packet: " << ipHeader.GetSource () << ":" << tcpHeader.GetSourcePort () << " -> " << ipHeader.GetDestination () << ":" << tcpHeader.GetDestinationPort ());

  uint32_t flowId;
  uint32_t packetId;
  if (Classify (ipHeader, ipPayload, &flowId, &packetId))
    {
      NS_LOG_LOGIC ("Packet classified: flow " << flowId << ", packet " << packetId);
    }
  else
    {
      NS_LOG_WARN ("Couldn't classify packet.");
      send ();
      return true;
    }

  DelayBoxRule rule;
  if (m_ruleTable.Lookup (ipHeader.GetSource (), tcpHeader.GetSourcePort (),
                          ipHeader.GetDestination (), tcpHeader.GetDestinationPort (), m_symmetric,
                          rule))
    {
      return m_flowTable.Enqueue (flowId, rule, send, packet->GetSize (),
                                  tcpHeader);
    }
  else
    {
      NS_LOG_DEBUG ("Packet didn't match a rule; sending immediately");
      send ();
      return true;
    }
}*/

bool
DelayBoxFlowTable::Enqueue (uint32_t flowId, const DelayBoxRule& rule, Callback<
                              void> send, uint32_t packetSize, const TcpHeader& tcpHeader)
{
  std::map<uint32_t, DelayBoxFlow>::iterator flow = m_flows.find (flowId);
  if (flow == m_flows.end ())
    {
      // If the flow isn't found, create one, sampling from the rule's
      // random variables, and add it to the flow table.
      flow = m_flows.insert (std::map<uint32_t, DelayBoxFlow>::value_type (
                               flowId, DelayBoxFlow (rule))).first;
    }
  else if (flow->second.Cancelled ())
    {
      // If the flow has already been cancelled, we're not interested
      // in delaying any more packets that arrive on it.
      NS_LOG_DEBUG ("Packet arrived on cancelled flow; sending immediately.");
      send ();
      return true;
    }
  else
    {
      // The flow already exists and is active.
    }

  // Prune cancelled flows from the table every once in a while to
  // save on memory during long runs.  I'm not completely sure of the
  // reasoning here but checking the table of N flows about once every
  // N packets seems... reasonable.
  if (m_flows.size () > 100 && m_rng->GetInteger (1, m_flows.size ()) == 1)                //HJB
    {
      unsigned n = 0;
      for (std::map<uint32_t, DelayBoxFlow>::iterator it = m_flows.begin (); it
           != m_flows.end (); )
        {
          if (it->second.Cancelled ())
            {
              m_flows.erase (it++);
              ++n;
            }
          else
            {
              ++it;
            }
        }
      NS_LOG_LOGIC ("Pruned " << n << " cancelled flows. " << m_flows.size () << " remain.");
    }

  return flow->second.Enqueue (send, packetSize, tcpHeader);
}

void
DelayBoxRuleTable::Add (DelayBoxRuleKey key, const DelayBoxRule& rule)
{
  m_rules.insert (std::pair<DelayBoxRuleKey, DelayBoxRule> (key, rule));
}

bool
DelayBoxRuleTable::Lookup (Ipv4Address src, uint16_t srcPort, Ipv4Address dst,
                           uint16_t dstPort, bool sym, DelayBoxRule& out) const
{
  std::map<DelayBoxRuleKey, DelayBoxRule>::const_iterator rule;
  /*
   * The auto-formatter makes a mess of this, but what this does is
   * simple: try looking up each of the four combinations
   * (src,srcPort,dst,dstPort), (src,0,dst,srcPort),
   * (src,srcPort,dst,0), (src,0,dst,0), in that order, and return the
   * first one that's found (relying on short circuit evaluation).
   * Since port 0 means to match any port, we cover all possibilities
   * for how the user might have specified the rule.  The most
   * specific rule is tried first.
   */
  if ((rule = m_rules.find (DelayBoxRuleKey (src, srcPort, dst, dstPort, sym)))
      != m_rules.end () // match both ports
      || (rule = m_rules.find (DelayBoxRuleKey (src, 0, dst, dstPort, sym)))
      != m_rules.end ()    // any source port, match destination port
      || (rule = m_rules.find (DelayBoxRuleKey (src, srcPort, dst, 0, sym)))
      != m_rules.end ()    // match source port, any destination port
      || (rule = m_rules.find (DelayBoxRuleKey (src, 0, dst, 0, sym)))
      != m_rules.end ()    // any source port, any destination port
      )
    {
      out = rule->second;
      return true;
    }
  else
    {
      return false;
    }
}

DelayBoxFlow::DelayBoxFlow (const DelayBoxRule& rule)
{
  rnd = CreateObject<UniformRandomVariable> ();                          //HJB
  rnd->SetAttribute ("Min",DoubleValue (0));
  rnd->SetAttribute ("Max",DoubleValue (1));
  m_delay = Seconds (rule.GetDelay ()->GetValue ());

  if (!m_delay.IsPositive ())
    {
      NS_FATAL_ERROR ("");
    }
  m_lossRate = rule.GetLossRate ()->GetValue ();                          //HJB
  m_linkSpeed = rule.GetLinkSpeed ()->GetValue ();
  m_tailPacketEnd = Seconds (0);
  m_cancelled = false;
  NS_LOG_DEBUG ("New flow, delay=" << m_delay << ", lossRate=" << m_lossRate << ", linkSpeed=" << m_linkSpeed);
}

/*
 * Packet timeline:
 *      A   B    C
 * --------------------
 *      *            Enqueue time
 *      |---|        Delay duration
 *          |----|   Transfer duration
 *               *   Reception (actual call to the "send" callback)
 *
 * At time A a packet is sent, routed to this flow by the classifier,
 * and this function Enqueue is called.  The packet is delayed for the
 * interval A->B, determined by the sample m_delay from the associated
 * rule's random variable.  If m_linkSpeed is nonzero, we also model
 * the time it would take to transfer this packet across a link with
 * the given speed, and delay it by the additional interval
 * B->C. Finally at time C the packet will have finished arriving and
 * can be delivered to the destination by calling send();.
 */
bool
DelayBoxFlow::Enqueue (Callback<void> send, uint32_t packetSize,
                       const TcpHeader& tcpHeader)
{
  NS_ASSERT (!m_cancelled);
  if (rnd->GetValue () < m_lossRate)                                     //HJB
    {
      NS_LOG_DEBUG ("Packet dropped due to flow loss rate " << m_lossRate);
      return false;
    }
  Time now = Simulator::Now ();
  Time transferDuration = Seconds (0);
  if (m_linkSpeed != 0)
    {
      transferDuration = Seconds (8.0 * ((double) packetSize) / m_linkSpeed);
      NS_LOG_DEBUG ("Packet transfer duration: " << transferDuration.GetSeconds () << "s, size: " << packetSize);
    }
  if (now > m_tailPacketEnd)
    {
      Time start = now + m_delay;
      Time end = now + m_delay + transferDuration;
      m_eventQueue.push_back (Simulator::Schedule (m_delay + transferDuration,
                                                   &DelayBoxFlow::Dequeue, this, send, tcpHeader));
      m_tailPacketEnd = end;
      NS_LOG_DEBUG ("No packets in queue. Start = " << start.GetSeconds () << "s, End = " << end.GetSeconds () << "s");
    }
  else
    {
      Time start = Max (now + m_delay, m_tailPacketEnd);
      Time end = start + transferDuration;
      // Low-priority TODO: Catastrophic floating-point cancellation
      // here (end-now) ? If only there was a way to schedule an event
      // for an absolute time. Or work with relative time throughout
      // the previous calculations instead.
      m_eventQueue.push_back (Simulator::Schedule (end - now,
                                                   &DelayBoxFlow::Dequeue, this, send, tcpHeader));
      m_tailPacketEnd = end;
      NS_LOG_DEBUG ("Packet queued behind " << m_eventQueue.size () - 1 << " others. Start = " << start.GetSeconds () << "s, End = " << end.GetSeconds () << "s");
    }
  return true;
}

void
DelayBoxFlow::Dequeue (Callback<void> send, const TcpHeader& tcpHeader)
{
  NS_ASSERT (!m_cancelled);
  m_eventQueue.pop_front ();
  send ();
  /**
   * When a packet with the FIN flag set arrives, we can basically
   * forget about that connection.  DelayBoxFlow::Cancel() will drop
   * all remaining packets in that queue, and mark itself as cancelled
   * so that any subsequent packets (including the FIN+ACK) will not
   * be delayed. This mimics the behavior of DelayBox in ns-2.
   */
  if (tcpHeader.GetFlags () & TcpHeader::FIN)
    {
      Cancel ();
      NS_LOG_DEBUG ("Flow cancelled by FIN.");
    }
}

void
DelayBoxFlow::Cancel ()
{
  // Prevent the simulator from executing any events that this flow
  // has scheduled which are still pending.
  for (std::deque<EventId>::iterator it = m_eventQueue.begin (); it
       != m_eventQueue.end (); ++it)
    {
      Simulator::Cancel (*it);
    }
  m_eventQueue.clear ();
  m_tailPacketEnd = Seconds (0);
  m_cancelled = true;
}

void
DelayBox::AddRule (Ipv4Address src, Ipv4Address dst, const DelayBoxRule& rule)
{
  AddRule (src, 0, dst, 0, rule);
}

void
DelayBox::AddRule (Ipv4Address src, uint16_t srcPort, Ipv4Address dst,
                   uint16_t dstPort, const DelayBoxRule& rule)
{
  m_ruleTable.Add (DelayBoxRuleKey (src, srcPort, dst, dstPort, m_symmetric),
                   rule);
  NS_LOG_LOGIC ("Rule added: " << src << ":" << srcPort << " -> " << dst << ":" << dstPort);
}

}
