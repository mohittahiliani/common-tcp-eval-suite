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

#include "tmix-topology.h"
#include "ns3/eval-app.h"
#include "ns3/eval-ts.h" 

#include "ns3/node-container.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/drop-tail-queue.h"
#include "ns3/uinteger.h"
#include "ns3/log.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/traffic-control-module.h"
#include "ns3/assert.h"
#include "ns3/config-store-module.h"

NS_LOG_COMPONENT_DEFINE ("TmixTopology");

namespace ns3 {

/**
 * Returns the Ipv4 address assigned to the tmix node's net device.
 *
 * Out parameters: tmixDevice, routerDevice, address
 */

 void
TmixTopology::PacketEnqueueF (Ptr<const QueueItem> item)
{
  Ptr<Packet> p = item->GetPacket ();       
  EvalTimestampTag tag;    
  p->AddPacketTag (tag);
}

void
TmixTopology::PacketDequeueF (Ptr<const QueueItem> item)
{
  Ptr<Packet> p = item->GetPacket ();      
  EvalTimestampTag tag;   
  p->RemovePacketTag (tag);   
  Time delta = Simulator::Now () - tag.GetTxTime ();     
  if (m_lastQDrecordF == Time::Min () || Simulator::Now () - m_lastQDrecordF > MilliSeconds (10))
    {
      m_lastQDrecordF = Simulator::Now ();
      if (m_numQDrecordF > 0)
        {
            m_QDrecordTotalF += (m_QDrecordF*1.0)/(m_numQDrecordF*1.0);
            m_numQDrecordTotalF ++;
            *m_QDfileF->GetStream ()<< Simulator::Now ().GetSeconds () << " "<< (m_QDrecordF*1.0)/(m_numQDrecordF*1.0)<< "\n";
        }
      m_QDrecordF = 0;
      m_numQDrecordF = 0;
    }
  m_numQDrecordF++;
  m_QDrecordF += delta.GetMilliSeconds ();
}

void
TmixTopology::PacketSizeF (Ptr<const Packet> packet)
{

  if (m_lastTPrecordF == Time::Min () || Simulator::Now () - m_lastTPrecordF > MilliSeconds (10))
    {
      if (m_TPrecordF > 0)
        {
            *m_TPfileF->GetStream ()<< Simulator::Now ().GetSeconds () << " "<< (m_TPrecordF*1.0)/(Simulator::Now () - m_lastTPrecordF).GetSeconds ()<< "\n";
            m_TPrecordTotalF += (m_TPrecordF*1.0)/(Simulator::Now () - m_lastTPrecordF).GetSeconds ();
            m_TPTotalF++; 
            *m_PktDropfileF->GetStream ()<< Simulator::Now ().GetSeconds () << " "<< (queueF->GetTotalDroppedPackets ()-droppedPacketsF)<< "\n";
            TotaldroppedPacketsF = queueF->GetTotalDroppedPackets ();
            droppedPacketsF = queueF->GetTotalDroppedPackets ();
        }
      m_lastTPrecordF = Simulator::Now ();
      m_TPrecordF = 0;
    }
  Total_numdPktsF++;
  m_TPrecordF += packet->GetSize ();

}
void
TmixTopology::PacketEnqueueR (Ptr<const QueueItem> item)
{
  Ptr<Packet> p = item->GetPacket ();       
  EvalTimestampTag tag;    
  p->AddPacketTag (tag);
}

void
TmixTopology::PacketDequeueR (Ptr<const QueueItem> item)
{
  Ptr<Packet> p = item->GetPacket ();      
  EvalTimestampTag tag;   
  p->RemovePacketTag (tag);   
  Time delta = Simulator::Now () - tag.GetTxTime ();     
  if (m_lastQDrecordR == Time::Min () || Simulator::Now () - m_lastQDrecordR > MilliSeconds (10))
    {
      m_lastQDrecordR = Simulator::Now ();
      if (m_numQDrecordR > 0)
        {
            m_QDrecordTotalR += (m_QDrecordR*1.0)/(m_numQDrecordR*1.0);
            m_numQDrecordTotalR ++;
            *m_QDfileR->GetStream ()<< Simulator::Now ().GetSeconds () << " "<< (m_QDrecordR*1.0)/(m_numQDrecordR*1.0)<< "\n";
        }
      m_QDrecordR = 0;
      m_numQDrecordR = 0;
    }
  m_numQDrecordR++;
  m_QDrecordR += delta.GetMilliSeconds ();
}

void
TmixTopology::PacketSizeR (Ptr<const Packet> packet)
{

  if (m_lastTPrecordR == Time::Min () || Simulator::Now () - m_lastTPrecordR > MilliSeconds (10))
    {
      if (m_TPrecordR > 0)
        {
            *m_TPfileR->GetStream ()<< Simulator::Now ().GetSeconds () << " "<< (m_TPrecordR*1.0)/(Simulator::Now () - m_lastTPrecordR).GetSeconds ()<< "\n";
            m_TPrecordTotalR += (m_TPrecordR*1.0)/(Simulator::Now () - m_lastTPrecordR).GetSeconds ();
            m_TPTotalR++; 
            *m_PktDropfileR->GetStream ()<< Simulator::Now ().GetSeconds () << " "<< (queueR->GetTotalDroppedPackets ()-droppedPacketsR)<< "\n";
            TotaldroppedPacketsR = queueR->GetTotalDroppedPackets ();
            droppedPacketsR = queueR->GetTotalDroppedPackets ();
        }
      m_lastTPrecordR = Simulator::Now ();
      m_TPrecordR = 0;
    }
  Total_numdPktsR++;
  m_TPrecordR += packet->GetSize ();

}
void

TmixTopology::Summary (std::string ScenarioName, std::string TcpName, uint32_t expt_num)
{
  AsciiTraceHelper asciiQD;
  m_Avgfile = asciiQD.CreateFileStream (std::string("tcp-eval-output/"+ScenarioName+"/EXPT-"+std::to_string(expt_num+1)+"/"+TcpName+"_AverageData.dat").c_str());
  *m_Avgfile->GetStream ()<< "\nFORWARD:\nAverage Queue Delay:  "<< (m_QDrecordTotalF)/(m_numQDrecordTotalF)<< "\n";
  *m_Avgfile->GetStream ()<< "\nAverage ThroughPut:  "<< (m_TPrecordTotalF)/m_TPTotalF<< "\n";
  *m_Avgfile->GetStream ()<< "\nAverage PacketDrop:  "<< (TotaldroppedPacketsF)/Total_numdPktsF<< "\n";
  *m_Avgfile->GetStream ()<< "\nREVERSE:\nAverage Queue Delay:  "<< (m_QDrecordTotalR)/(m_numQDrecordTotalR)<< "\n";
  *m_Avgfile->GetStream ()<< "\nAverage ThroughPut:  "<< (m_TPrecordTotalR)/m_TPTotalR<< "\n";
  *m_Avgfile->GetStream ()<< "\nAverage PacketDrop:  "<< (TotaldroppedPacketsR)/Total_numdPktsR<< "\n";
}

void
TmixTopology::DestroyConnection ()
{
  queueF->TraceDisconnectWithoutContext ("Enqueue", MakeCallback (&TmixTopology::PacketEnqueueF, this));
  queueF->TraceDisconnectWithoutContext ("Dequeue", MakeCallback (&TmixTopology::PacketDequeueF, this));
  DeviceF->TraceDisconnectWithoutContext ("PhyTxBegin", MakeCallback (&TmixTopology::PacketSizeF, this));

  queueR->TraceDisconnectWithoutContext ("Enqueue", MakeCallback (&TmixTopology::PacketEnqueueR, this));
  queueR->TraceDisconnectWithoutContext ("Dequeue", MakeCallback (&TmixTopology::PacketDequeueR, this));
  DeviceR->TraceDisconnectWithoutContext ("PhyTxBegin", MakeCallback (&TmixTopology::PacketSizeR, this));
}


void
TmixTopology::ConnectNodeToRouter (Ptr<Node> routerNode, Ptr<Node> tmixNode, Ptr<
                       PointToPointNetDevice>& tmixDevice,
                     Ptr<PointToPointNetDevice>& routerDevice, Ipv4AddressHelper &addresses,
                     Ipv4Address& address, Time delay)
{
  Ptr<PointToPointChannel> channel = CreateObject<PointToPointChannel> ();

  routerDevice = CreateObject<PointToPointNetDevice> ();
  routerDevice->SetAddress (Mac48Address::Allocate ());
  routerDevice->Attach (channel);
  routerNode->AddDevice (routerDevice);
    TrafficControlHelper tchPfifo;
    uint16_t handle = tchPfifo.SetRootQueueDisc ("ns3::PfifoFastQueueDisc", "Limit", UintegerValue (m_routerOutQueueLimit));
    tchPfifo.AddInternalQueues (handle, 3, "ns3::DropTailQueue",  "MaxPackets", UintegerValue (m_routerOutQueueLimit));
    tchPfifo.Install(routerDevice);
  addresses.Assign (NetDeviceContainer (routerDevice));

  tmixDevice = CreateObject<PointToPointNetDevice> ();
  tmixDevice->SetAddress (Mac48Address::Allocate ());
  tmixDevice->Attach (channel);
  tmixNode->AddDevice (tmixDevice);
  address = addresses.Assign (NetDeviceContainer (tmixDevice)).GetAddress (0);

  // Increment the network number for the next node
  addresses.NewNetwork ();

  // Set additional topology delays, unrelated to DelayBox/Tmix
  // TODO: make these not hard-coded
  channel->SetAttribute ("Delay", TimeValue (delay));

  tmixDevice->SetAttribute ("DataRate", DataRateValue (m_tmixDeviceRate));
  {
    Ptr<DropTailQueue> queue = CreateObject<DropTailQueue> ();
    queue->SetMode (Queue::QUEUE_MODE_PACKETS);
    queue->SetAttribute ("MaxPackets", UintegerValue (200));
    tmixDevice->SetQueue (queue);
  }
  routerDevice->SetAttribute ("DataRate", DataRateValue (m_routerDeviceInRate));
  {
    Ptr<DropTailQueue> queue = CreateObject<DropTailQueue> ();
    queue->SetMode (Queue::QUEUE_MODE_PACKETS);
    queue->SetAttribute ("MaxPackets", UintegerValue (m_routerOutQueueLimit));
    routerDevice->SetQueue (queue);
  }
}


/**
 * Out parameters: device, routerAddress
 */
void
TmixTopology::ConnectRouter (Ptr<Node> router, Ptr<PointToPointChannel> channel,
               Ptr<DelayBox> delayBox, Ptr<DelayBoxPointToPointNetDevice>& device,
               Ipv4AddressHelper& addresses, Ipv4Address& routerAddress, Ptr<QueueDisc>& queuedisc, InitiatorSide side, std::string ScenarioName, std::string TcpName, uint32_t expt_num)
{
  device = CreateObject<DelayBoxPointToPointNetDevice> ();
  device->SetAddress (Mac48Address::Allocate ());
  device->SetDelayBox (delayBox);
  device->Attach (channel);
    Ptr<DropTailQueue> queuex = CreateObject<DropTailQueue> ();
    queuex->SetMode (Queue::QUEUE_MODE_PACKETS);
    queuex->SetAttribute ("MaxPackets", UintegerValue (m_routerOutQueueLimit));
    device->SetQueue (queuex);
  std::cout<<"device count before: "<<router->GetNDevices ();
  router->AddDevice (device);
  std::cout<<"device count after: "<<router->GetNDevices ();
    TrafficControlHelper tchPfifo;
    uint16_t handle = tchPfifo.SetRootQueueDisc ("ns3::PfifoFastQueueDisc", "Limit", UintegerValue (m_routerOutQueueLimit));
    tchPfifo.AddInternalQueues (handle, 3, "ns3::DropTailQueue",  "MaxPackets", UintegerValue (m_routerOutQueueLimit));
    queuedisc = tchPfifo.Install(device).Get(0);
      
    if( side == LEFT)
    {
      queuedisc->TraceConnectWithoutContext ("Enqueue", MakeCallback (&TmixTopology::PacketEnqueueF, this));
      queuedisc->TraceConnectWithoutContext ("Dequeue", MakeCallback (&TmixTopology::PacketDequeueF, this));
      device->TraceConnectWithoutContext ("PhyTxBegin", MakeCallback (&TmixTopology::PacketSizeF, this));
           
      queueF = queuedisc;
      DeviceF = device;
      m_QDrecordF = 0;
      m_numQDrecordF = 0;
      m_QDrecordTotalF = 0;
      m_numQDrecordTotalF =0;
      m_TPrecordTotalF =0;
      m_TPTotalF =0;
      m_lastQDrecordF = Time::Min ();
      AsciiTraceHelper asciiQD;
      m_QDfileF = asciiQD.CreateFileStream (std::string("tcp-eval-output/"+ScenarioName+"/EXPT-"+std::to_string(expt_num+1)+"/"+TcpName+"_qdelF.dat").c_str());
      m_TPrecordF = 0;
      m_lastTPrecordF = Time::Min ();
      AsciiTraceHelper asciiTP;
      m_TPfileF = asciiTP.CreateFileStream (std::string("tcp-eval-output/"+ScenarioName+"/EXPT-"+std::to_string(expt_num+1)+"/"+TcpName+"_throughputF.dat").c_str());
      AsciiTraceHelper asciiPD;
      m_PktDropfileF = asciiPD.CreateFileStream (std::string("tcp-eval-output/"+ScenarioName+"/EXPT-"+std::to_string(expt_num+1)+"/"+TcpName+"_packetdropF.dat").c_str());
      droppedPacketsF =0;
      TotaldroppedPacketsF =0;
      Total_numdPktsF =0;
    }
    else
    {
      queuedisc->TraceConnectWithoutContext ("Enqueue", MakeCallback (&TmixTopology::PacketEnqueueR, this));
      queuedisc->TraceConnectWithoutContext ("Dequeue", MakeCallback (&TmixTopology::PacketDequeueR, this));
      device->TraceConnectWithoutContext ("PhyTxBegin", MakeCallback (&TmixTopology::PacketSizeR, this));
      queueR = queuedisc;
      DeviceR = device;
      m_QDrecordR = 0;
      m_numQDrecordR = 0;
      m_QDrecordTotalR = 0;
      m_numQDrecordTotalR =0;
      m_TPrecordTotalR =0;
      m_TPTotalR =0;
      m_lastQDrecordR = Time::Min ();
      AsciiTraceHelper asciiQD;
      m_QDfileR = asciiQD.CreateFileStream (std::string("tcp-eval-output/"+ScenarioName+"/EXPT-"+std::to_string(expt_num+1)+"/"+TcpName+"_qdelR.dat").c_str());
      m_TPrecordR = 0;
      m_lastTPrecordR = Time::Min ();
      AsciiTraceHelper asciiTP;
      m_TPfileR = asciiTP.CreateFileStream (std::string("tcp-eval-output/"+ScenarioName+"/EXPT-"+std::to_string(expt_num+1)+"/"+TcpName+"_throughputR.dat").c_str());
      AsciiTraceHelper asciiPD;
      m_PktDropfileR = asciiPD.CreateFileStream (std::string("tcp-eval-output/"+ScenarioName+"/EXPT-"+std::to_string(expt_num+1)+"/"+TcpName+"_packetdropR.dat").c_str());
      droppedPacketsR =0;
      TotaldroppedPacketsR =0;
      Total_numdPktsR =0;
    }
    
    NS_ASSERT_MSG(queuedisc, "Queue disc creation failed");
  //}
  routerAddress = addresses.Assign (NetDeviceContainer (device)).GetAddress (0);

  // Set additional topology delays, unrelated to DelayBox
  // This is to more precisely duplicate the behavior of ns-2 Tmix

  device->SetAttribute ("DataRate", DataRateValue (m_routerDeviceOutRate));

}

void
TmixTopology::AssignNodes (uint32_t left, uint32_t right)
{
  leftNodes.Create(left);
  rightNodes.Create(right);
  m_internet.Install(leftNodes);
  m_internet.Install(rightNodes);
}

TmixTopology::TmixNodePair&
TmixTopology::NewPair (InitiatorSide side, Time leftDelay, Time rightDelay, uint32_t left, uint32_t right)
{
  TmixNodePair pair;
  
  if(side == LEFT)
  {
    pair.initiator = leftNodes.Get(left);
    pair.acceptor = rightNodes.Get(right);
  }
  else
  {
    pair.initiator = rightNodes.Get(right);
    pair.acceptor = leftNodes.Get(left);
  }

  ConnectNodeToRouter ((side == LEFT) ? m_leftRouter : m_rightRouter, pair.initiator, pair.initiatorDevice,
                       pair.routerInitiatorDevice, (side == LEFT) ? m_leftAddresses
                       : m_rightAddresses, pair.initiatorAddress, leftDelay);
    
  ConnectNodeToRouter ((side == RIGHT) ? m_leftRouter : m_rightRouter, pair.acceptor, pair.acceptorDevice,
                       pair.routerAcceptorDevice, (side == RIGHT) ? m_leftAddresses
                       : m_rightAddresses, pair.acceptorAddress, rightDelay);

  NS_LOG_LOGIC (pair.initiatorAddress << " assigned to "<< side << "INITIATOR");


  NS_LOG_LOGIC (pair.acceptorAddress << " assigned to "<< side << "ACCEPTOR");
  if (side == LEFT)
    {
      m_nodeTypeOfAddress[pair.initiatorAddress] = LEFT_INITIATOR;
      m_nodeTypeOfAddress[pair.acceptorAddress] = RIGHT_ACCEPTOR;
    }
  else
    {
      m_nodeTypeOfAddress[pair.initiatorAddress] = RIGHT_INITIATOR;
      m_nodeTypeOfAddress[pair.acceptorAddress] = LEFT_ACCEPTOR;
    }

  pair.helper = Create<TmixHelper> (m_delayBox, pair.initiator,
                                    pair.initiatorAddress, pair.acceptor, pair.acceptorAddress);

  if (side == LEFT)
    {
      m_leftPairs.push_back (pair);
      return m_leftPairs.back ();
    }
  else
    {
      m_rightPairs.push_back (pair);
      return m_rightPairs.back ();
    }
}

TmixTopology::NodeType
TmixTopology::NodeTypeByAddress (Ipv4Address address)
{
  std::map<Ipv4Address, NodeType>::const_iterator i = m_nodeTypeOfAddress.find (
      address);
  if (i == m_nodeTypeOfAddress.end ())
    {
      return INVALID;
    }
  else
    {
      return i->second;
    }
}

TmixTopology::TmixTopology (const InternetStackHelper& internet, Ptr<TmixToplogyParameters> topParam, std::string ScenarioName, std::string TcpName, uint32_t expt_num)
  : m_internet (internet)
{
  m_centerChannel = CreateObject<PointToPointChannel> ();

  m_centerChannelDelay = topParam->GetCenterChannelDelay();
  m_nodeToRouterDelay=topParam->GetNodeToRouterDelay();
  m_tmixDeviceRate=topParam->GetTmixDeviceRate();
  m_tmixDeviceQueueLimit=topParam->GetTmixDeviceQueueLimit();
  m_routerDeviceInRate=topParam->GetRouterDeviceInRate();
  m_routerInQueueLimit=topParam->GetRouterInQueueLimit();
  m_routerDeviceOutRate=topParam->GetRouterDeviceOutRate();
  m_routerOutQueueLimit=topParam->GetRouterOutQueueLimit();
  m_aqmUsed = topParam->IsAqmUsed();

  m_centerChannel->SetAttribute ("Delay", TimeValue (m_centerChannelDelay));

  m_delayBox = CreateObject<DelayBox> ();

  m_leftRouter = CreateObject<Node> ();
  m_rightRouter = CreateObject<Node> ();
  m_internet.Install (m_leftRouter);
  m_internet.Install (m_rightRouter);

  m_leftAddresses.SetBase ("10.1.0.0", "255.255.255.252");
  m_rightAddresses.SetBase ("10.2.0.0", "255.255.255.252");

  Ipv4AddressHelper routerAddresses;
  routerAddresses.SetBase ("10.0.0.0", "255.255.255.252");
  ConnectRouter (m_leftRouter, m_centerChannel, m_delayBox, m_leftRouterDevice,
                 routerAddresses, m_leftRouterAddress, m_leftRouterQueueDisc, LEFT,ScenarioName,TcpName, expt_num);
  NS_LOG_LOGIC (m_leftRouterAddress << " assigned to LEFT_ROUTER");
  NS_ASSERT_MSG(m_leftRouterQueueDisc, "Queue disc creation failed");
  ConnectRouter (m_rightRouter, m_centerChannel, m_delayBox,
                 m_rightRouterDevice, routerAddresses, m_rightRouterAddress,m_rightRouterQueueDisc, RIGHT,ScenarioName,TcpName, expt_num);
  NS_LOG_LOGIC (m_rightRouterAddress << " assigned to RIGHT_ROUTER");

  m_nodeTypeOfAddress[m_leftRouterAddress] = LEFT_ROUTER;
  m_nodeTypeOfAddress[m_rightRouterAddress] = RIGHT_ROUTER;
}

}
