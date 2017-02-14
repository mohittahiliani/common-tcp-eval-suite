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

#ifndef DELAYBOX_H_
#define DELAYBOX_H_

#include "ns3/object.h"
#include "ns3/random-variable-stream.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"
#include "ns3/inet-socket-address.h"
#include "ns3/traffic-control-module.h"
#include "ns3/internet-module.h"
#include "tcp-flow-classifier.h"
#include "ns3/double.h"                                 //HJB

#include <deque>

namespace ns3 {

// TODO: Maybe hide the implementation so we can get rid of these
// too-visible "for internal use only" classes.

/**
 * \internal
 * \brief For internal use.
 *
 * Provides equality and a canonical sort order for DelayBoxRule lookups.
 */
class DelayBoxRuleKey
{
public:
  DelayBoxRuleKey (Ipv4Address src, uint16_t srcPort, Ipv4Address dst,
                   uint16_t dstPort, bool symmetric);

  bool
  operator< (const DelayBoxRuleKey& rhs) const;
  bool
  operator== (const DelayBoxRuleKey& rhs) const;

private:
  InetSocketAddress m_src;
  InetSocketAddress m_dst;
};

/**
 * \brief Delay, loss rate, and link speed.
 *
 * Contains three random variables, all of which are optional and default to zero:
 *  - delay (in seconds),
 *  - loss rate (between 0.0 and 1.0, defaults to 0),
 *  - bottleneck link speed (in bits per second; the default of 0 means
 *    there is no limit).
 */
class DelayBoxRule
{
public:
  /**
   * \param delay Length of time to delay packets, in seconds. Default 0.
   * \param lossRate Probability of DelayBox dropping a packet, between 0 and 1. Default 0.
   * \param linkSpeed Bottleneck link speed, in bits per second. A
   * value of zero indicates unlimited speed. Default 0.
   */
  DelayBoxRule (const double delay = 0.0,                                //HJB
                const double lossRate = 0.0,
                const double linkSpeed = 0.0)
  {
    m_delayRV = CreateObject<ConstantRandomVariable> ();
    m_lossRateRV = CreateObject<ConstantRandomVariable> ();
    m_linkSpeedRV = CreateObject<ConstantRandomVariable> ();
    m_delayRV->SetAttribute ("Constant",DoubleValue (delay));
    m_lossRateRV->SetAttribute ("Constant",DoubleValue (lossRate));
    m_linkSpeedRV->SetAttribute ("Constant",DoubleValue (linkSpeed));
  }

  const Ptr<ConstantRandomVariable>&
  GetDelay () const
  {
    return m_delayRV;
  }

  const Ptr<ConstantRandomVariable>&
  GetLossRate () const
  {
    return m_lossRateRV;
  }

  const Ptr<ConstantRandomVariable>&
  GetLinkSpeed () const
  {
    return m_linkSpeedRV;
  }

private:
  Ptr<ConstantRandomVariable> m_delayRV;
  Ptr<ConstantRandomVariable> m_lossRateRV;
  Ptr<ConstantRandomVariable> m_linkSpeedRV;
};

/**
 * \internal
 * \brief For internal use.
 *
 * Matches a packet to a DelayBoxRule based on the source and destination.
 */
class DelayBoxRuleTable
{
public:
  void
  Add (DelayBoxRuleKey key, const DelayBoxRule& rule);

  bool
  Lookup (Ipv4Address src, uint16_t srcPort, Ipv4Address dst, uint16_t dstPort,
          bool symmetric, DelayBoxRule& out) const;

private:
  std::map<DelayBoxRuleKey, DelayBoxRule> m_rules;
};

/**
 * \internal
 * \brief For internal use, or if you wish to manage flows manually.
 */
class DelayBoxFlow
{
public:
  DelayBoxFlow (const DelayBoxRule&);

  Ptr<UniformRandomVariable> rnd;               //HJB
  /**
   * Schedules sending this packet for some future time (or
   * immediately), depending on the parameters of this flow.
   *
   * \param send A callback which sends the packet when called.
   *
   * \return true if the packet was enqueued, false if it was dropped
   * due to the flow's loss rate.
   */
  bool
  Enqueue (Callback<void> send, uint32_t packetSize, const TcpHeader& tcpHeader);

  /**
   * Remove and drop all remaining packets in this flow's queue.
   * After a call to Cancel(), it is an error to call any functions except Cancelled().
   */
  void
  Cancel ();

  /**
   * True if Cancel() has been called.
   */
  bool
  Cancelled ()
  {
    return m_cancelled;
  }

protected:
  void
  Dequeue (Callback<void> send, const TcpHeader& tcpHeader);

private:
  Time m_delay;
  double m_lossRate;
  double m_linkSpeed;
  /**
   * The time that the last bit of the last packet in the "queue"
   * should be done transmitting.
   */
  Time m_tailPacketEnd;
  std::deque<EventId> m_eventQueue;
  bool m_cancelled;
};

/**
 * \internal
 * \brief For internal use.
 */
class DelayBoxFlowTable
{
public:
  DelayBoxFlowTable ()                                   //HJB
  {
    m_rng = CreateObject<UniformRandomVariable> ();
  }
  /**
   * \return true if the packet was enqueued or sent, false if it was
   * dropped due to loss rate.
   */
  bool
  Enqueue (uint32_t flowId, const DelayBoxRule& rule, Callback<void> send,
           uint32_t packetSize, const TcpHeader& tcpHeader);
private:
  std::map<uint32_t, DelayBoxFlow> m_flows;
  Ptr<UniformRandomVariable> m_rng;                     //HJB
};

/**
 * \brief Per-flow random delay, loss rate, and bottleneck link speed.
 *
 * Use this with one or more DelayBoxNetDevices. It's usually most
 * useful to have many DelayBoxNetDevices sharing one DelayBox
 * configured with all of the rules.
 *
 * See the manual section on DelayBox for detailed usage information.
 *
 * \see DelayBoxNetDeviceTm
 */
class DelayBox : public Object
{
public:
  static TypeId
  GetTypeId ();

  /**
   * Create an instance of DelayBox with no rules configured.
   */
  DelayBox ();

  /**
   * Calls send() at a later time, according to the packet's
   * classification and the configured rules.  If the packet does not
   * match any rules, or if it cannot be classified, send() is called
   * immediately.
   *
   * \param send A bound callback which should send the packet on. It
   * will be scheduled to execute at the proper time.
   * \param packet A packet with IPv4 and TCP headers (no PPP header).
   * \return true if the packet was enqueued successfully; false if it
   * was dropped due to the matching DelayBoxRule's loss rate.
   */
  bool
  Delay (Callback<void> send, Ptr<const Packet> packet);

  //bool
  //Delay (Callback<void> send, Ptr<const QueueDiscItem> item);

  /**
   * Use this to set up the delay parameters for your flows.
   */
  void
  AddRule (Ipv4Address src, Ipv4Address dst, const DelayBoxRule& rule);

  /**
   * Use this to set up more specific delay parameters, limited to
   * matching certain TCP ports.  A port number of zero means to match
   * any port on that side.
   */
  void
  AddRule (Ipv4Address src, uint16_t srcPort, Ipv4Address dst, uint16_t dstPort,
           const DelayBoxRule& rule);

  /**
   * \brief Enable or disable symmetric mode.
   *
   * Symmetric mode is enabled by default. Switching modes after some
   * packets have already been processed may have undesired results.
   *
   * In symmetric mode, both sides of a TCP conversation are treated
   * as part of the same flow, and each rule matches packets going
   * both from "src" to "dst" and from "dst" to "src". Thus the
   * arguments to AddRule may be given in either order.
   *
   * In non-symmetric mode, rules match packets more strictly and each
   * side of a TCP conversation is a separate flow. To apply a delay
   * to both directions of a conversation in this mode, you must
   * manually add a rule in the reverse direction; even then, the
   * reverse direction will still have its own sampling of random
   * variables and its own bottleneck-link-speed packet queue.
   *
   * \param symmetric True to enable symmetric mode. False to disable it.
   */
  void
  SetSymmetric (bool symmetric);

private:
  bool
  Classify (const Ipv4Header& ipHeader, Ptr<const Packet> ipPayload,
            uint32_t* flowId, uint32_t* packetId);

  bool m_symmetric;
  TcpFlowClassifier m_symmetricClassifier;
  Ipv4FlowClassifier m_asymmetricClassifier;
  DelayBoxRuleTable m_ruleTable;
  DelayBoxFlowTable m_flowTable;
};

}

#endif /* DELAYBOX_H_ */
