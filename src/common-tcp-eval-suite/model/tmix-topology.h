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

#ifndef TMIX_TOPOLOGY_H_
#define TMIX_TOPOLOGY_H_

#include <stdio.h>
#include <stdlib.h>
#include "ns3/log.h"
#include "ns3/simulator.h"

#include "ns3/simple-ref-count.h"
#include "ns3/tmix-helper.h"
#include "ns3/simple-channel.h"
#include "ns3/delaybox-net-device.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/tmix-topology-parameter.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"

#include "ns3/traffic-control-module.h"
#include "ns3/assert.h"
#include <map>

namespace ns3 {

/**
 * \brief Construct and manage the standard Tmix topology.
 *
 * This class will automagically set up the nodes, net devices,
 * channels, and so on needed to support the standard Tmix testbed
 * configuration, shown in the diagram below.
 *
 * \code
 * LEFT_INITIATOR(s) --\                   /-- RIGHT_ACCEPTOR(s)
 *                   ROUTER1 ---LINK--- ROUTER2
 * LEFT_ACCEPTOR(s)  --/                   \-- RIGHT_INITIATOR(s)
 * \endcode
 *
 * When a TmixTopology is instantiated it only contains two nodes: the
 * two routers, connected by a PointToPointChannel. Each time the
 * TmixTopology::NewPair() method is called, a new initiator-acceptor
 * pair of nodes is created (using TmixHelper) and inserted. The
 * initiator and acceptor of a pair are always separated by the two
 * routers and the center channel; the argument to NewPair determines
 * which side the initiator is placed on.
 *
 * While a single initiator-acceptor pair is sufficient for small
 * experiments, it is limited by the 16-bit TCP port number to a few
 * tens of thousands of simultaneously active connection vectors.  By
 * creating many such pairs and assigning a limited number of cvecs to
 * each one, this limitation is bypassed.
 *
 * There are only two DelayBoxPointToPointNetDevices in this topology;
 * they are connected to the router nodes, facing each other across
 * the center PointToPointChannel. All other NetDevices are unadorned
 * PointToPointNetDevices.
 *
 * Be sure to set up Ipv4 routing (usually using
 * Ipv4GlobalRoutingHelper::PopulateRoutingTables()) after creating
 * this topology.
 */
class TmixTopology : public SimpleRefCount<TmixTopology>
{
public:
  enum NodeType
  {
    INVALID = -1,
    LEFT_INITIATOR = 0,
    LEFT_ACCEPTOR = 2,
    RIGHT_ACCEPTOR = 1,
    RIGHT_INITIATOR = 3,
    LEFT_ROUTER = 4,
    RIGHT_ROUTER = 5
  };

  enum InitiatorSide
  {
    LEFT = 0, RIGHT = 1
  };

  /**
   * Structure describing one initiator-acceptor pair within the topology.
   */
  struct TmixNodePair
  {
    /// The instance of TmixHelper managing this pair.
    Ptr<TmixHelper> helper;

    /// The initiator node.
    Ptr<Node> initiator;
    /// The public address of the initiator node.
    Ipv4Address initiatorAddress;
    /// The NetDevice of the initiator node.
    Ptr<PointToPointNetDevice> initiatorDevice;
    /// The NetDevice on the router which is connected to the initiator node.
    Ptr<PointToPointNetDevice> routerInitiatorDevice;

    /// The acceptor node.
    Ptr<Node> acceptor;
    /// The public address of the acceptor node.
    Ipv4Address acceptorAddress;
    /// The NetDevice of the acceptor node.
    Ptr<PointToPointNetDevice> acceptorDevice;
    /// The NetDevice on the router which is connected to the initiator node.
    Ptr<PointToPointNetDevice> routerAcceptorDevice;
  };

public:
  /**
   * Creates a topology containing just the two routers. Use NewPair() to create tmix nodes.
   *
   * \param internet The InternetStackHelper instance used to install
   *    Ipv4 on any new nodes created in this topology.  A useful
   *    parameter is the choice of routing (with
   *    InternetStackHelper::SetRoutingHelper).
   */
  TmixTopology (const InternetStackHelper& internet, Ptr<TmixToplogyParameters> topParam, std::string ScenarioName, std::string TcpName, uint32_t expt_num );

  /**
   * Look up the type of a node by its address.
   * \return The NodeType, or -1 if the given address is not part of this topology.
   */
  NodeType
  NodeTypeByAddress (Ipv4Address address);

  /**
   * Create a new pair of nodes: one initiator and one acceptor,
   * connected via the two routers, with an instance of TmixHelper
   * watching over them.  \param side If LEFT, the initiator is on the
   * left side and the acceptor on the right. If RIGHT, their
   * locations are reversed.  \return A reference to the TmixNodePair
   * structure describing the newly created pair of nodes.
   */
  TmixNodePair&

  NewPair (InitiatorSide side, Time leftDelay, Time rightDelay, uint32_t left, uint32_t right);

  void AssignNodes (uint32_t left, uint32_t right);

  void Summary (std::string ScenarioName, std::string TcpName, uint32_t expt_num);

  Ptr<Node>
  GetLeftRouter ()
  {
    return m_leftRouter;
  }

  Ptr<Node>
  GetRightRouter ()
  {
    return m_rightRouter;
  }

  /**
   * \return The DelayBoxPointToPointNetDevice of the left router. It
   * is connected via a PointToPointChannel to the corresponding
   * device on the right router.
   */
  Ptr<DelayBoxPointToPointNetDevice>
  GetLeftRouterDevice ()
  {
    return m_leftRouterDevice;
  }
  /**
   * \return The DelayBoxPointToPointNetDevice of the right router. It
   * is connected via a PointToPointChannel to the corresponding
   * device on the left router.
   */
  Ptr<DelayBoxPointToPointNetDevice>
  GetRightRouterDevice ()
  {
    return m_rightRouterDevice;
  }

  /**
   * \return All node pairs where the initiator is on the left side.
   */
  std::vector<TmixNodePair>&
  LeftPairs ()
  {
    return m_leftPairs;
  }

  /**
   * \return All node pairs where the initiator is on the right side.
   */
  std::vector<TmixNodePair>&
  RightPairs ()
  {
    return m_rightPairs;
  }

  Ptr<QueueDisc> GetLeftRouterQueueDisc()
  {
    return m_leftRouterQueueDisc;
  }

  Ptr<QueueDisc> GetRightRouterQueueDisc()
  {
    return m_rightRouterQueueDisc;
  }

  void DestroyConnection ();
private:

  void
  ConnectNodeToRouter (Ptr<Node> routerNode, Ptr<Node> tmixNode, Ptr<
                       PointToPointNetDevice>& tmixDevice,
                       Ptr<PointToPointNetDevice>& routerDevice, Ipv4AddressHelper &addresses,
                       Ipv4Address& address, Time delay);

  void
  ConnectRouter (Ptr<Node> router, Ptr<PointToPointChannel> channel,
                 Ptr<DelayBox> delayBox, Ptr<DelayBoxPointToPointNetDevice>& device,
                 Ipv4AddressHelper& addresses, Ipv4Address& routerAddress, Ptr<QueueDisc>& queuedisc,InitiatorSide side, std::string ScenarioName, std::string TcpName, uint32_t expt_num);

  void PacketEnqueueF (Ptr<const QueueItem> item);
  void PacketDequeueF (Ptr<const QueueItem> item);
  void PacketSizeF (Ptr<const Packet> packet);
  void PacketEnqueueR (Ptr<const QueueItem> item);
  void PacketDequeueR (Ptr<const QueueItem> item);
  void PacketSizeR (Ptr<const Packet> packet);
  void PayloadSize (Ptr<const Packet> packet, const Address & address);
  void TCPinfo (Ptr<const Packet> packet, const TcpHeader& tcpheader, Ptr<const TcpSocketBase> tsb);

  Ptr<Node> m_leftRouter, m_rightRouter;
  Ptr<PointToPointChannel> m_centerChannel;

  /// The single instance of DelayBox shared among all nodes in this topology.
  Ptr<DelayBox> m_delayBox;

  Ipv4AddressHelper m_leftAddresses, m_rightAddresses;

  /// Pairs where the initiator is on the left side
  std::vector<TmixNodePair> m_leftPairs;
  /// Pairs where the initiator is on the right side
  std::vector<TmixNodePair> m_rightPairs;

  /// Left router's public interface (facing the right router)
  Ptr<DelayBoxPointToPointNetDevice> m_leftRouterDevice;
  /// Right router's public interface (facing the left router)
  Ptr<DelayBoxPointToPointNetDevice> m_rightRouterDevice;

  Ipv4Address m_leftRouterAddress, m_rightRouterAddress;

  InternetStackHelper m_internet;

  std::map<Ipv4Address, NodeType> m_nodeTypeOfAddress;

  Time m_nodeToRouterDelay, m_centerChannelDelay;
  DataRate m_tmixDeviceRate, m_routerDeviceInRate, m_routerDeviceOutRate;
  uint32_t m_tmixDeviceQueueLimit, m_routerInQueueLimit, m_routerOutQueueLimit;
  bool m_aqmUsed;
  
  Ptr<QueueDisc> m_leftRouterQueueDisc; 
  Ptr<QueueDisc> m_rightRouterQueueDisc;
  std::map<Ptr<const QueueItem>, Time> enqueueTime;
  NodeContainer leftNodes;
  NodeContainer rightNodes;
  Ptr<OutputStreamWrapper> m_Avgfile;

  Ptr<QueueDisc> queueF;
  Ptr<DelayBoxPointToPointNetDevice> DeviceF;
  double m_QDrecordF;
  double m_QDrecordTotalF;
  uint64_t m_numQDrecordF;
  uint64_t m_numQDrecordTotalF;
  Time m_lastQDrecordF;
  Ptr<OutputStreamWrapper> m_QDfileF;
  uint64_t m_TPrecordF;
  double m_TPrecordTotalF;
  uint64_t m_TPTotalF;
  Time m_lastTPrecordF;
  Ptr<OutputStreamWrapper> m_TPfileF;
  uint32_t droppedPacketsF;
  uint32_t TotaldroppedPacketsF;
  uint32_t Total_numdPktsF;
  Ptr<OutputStreamWrapper> m_PktDropfileF;

  Ptr<QueueDisc> queueR;
  Ptr<DelayBoxPointToPointNetDevice> DeviceR;
  double m_QDrecordR;
  double m_QDrecordTotalR;
  uint64_t m_numQDrecordR;
  uint64_t m_numQDrecordTotalR;
  Time m_lastQDrecordR;
  Ptr<OutputStreamWrapper> m_QDfileR;
  uint64_t m_TPrecordR;
  double m_TPrecordTotalR;
  uint64_t m_TPTotalR;
  Time m_lastTPrecordR;
  Ptr<OutputStreamWrapper> m_TPfileR;
  uint32_t droppedPacketsR;
  uint32_t TotaldroppedPacketsR;
  uint32_t Total_numdPktsR;
  Ptr<OutputStreamWrapper> m_PktDropfileR;
  


};

}

#endif /* TMIX_TOPOLOGY_H_ */
