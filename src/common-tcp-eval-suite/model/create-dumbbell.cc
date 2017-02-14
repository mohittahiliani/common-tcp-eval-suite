/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "create-dumbbell.h"
#include <iostream>
#include <fstream>
#include <string>


#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/error-model.h"

#include "ns3/ipv4-global-routing-helper.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("CreateDumbbell");


CreateDumbbell::CreateDumbbell(uint32_t num_of_nodes, std::string bot_delay, std::string bot_bw, std::string edge_bw, std::string edge_delay[],double bufferLimit)
{
	NS_LOG_INFO ("Create nodes");

	leftNodes.Create(num_of_nodes/2);
	rightNodes.Create(num_of_nodes/2);
	routerNodes.Create(2);
	num_nodes = num_of_nodes;

        NS_LOG_INFO ("Install internet stack on all nodes.");
        InternetStackHelper internet;
        internet.Install (leftNodes);
        internet.Install (rightNodes);
        internet.Install (routerNodes);
  
	NS_LOG_INFO ("Create router channels");
    
   PointToPointHelper p2p;
   tchBot.SetRootQueueDisc ("ns3::PfifoFastQueueDisc", "Limit", UintegerValue (bufferLimit));
   p2p.SetDeviceAttribute ("DataRate", StringValue (bot_bw));
   p2p.SetChannelAttribute ("Delay", StringValue(bot_delay));
   NodeContainer R1R2 = NodeContainer (routerNodes.Get(0),routerNodes.Get(1));
   routerDevices = p2p.Install(R1R2);
   Qdiscs = tchBot.Install (routerDevices);


   NS_LOG_INFO ("Create left leaf channels");

   for(uint32_t i=0 ; i< num_of_nodes/2;i++)
   {
   	p2p.SetDeviceAttribute ("DataRate", StringValue (edge_bw));
    	p2p.SetChannelAttribute ("Delay", StringValue(edge_delay[i]));
    	NetDeviceContainer c = p2p.Install(routerNodes.Get(0), leftNodes.Get(i));
    	leftDevices.Add(c.Get(1));
    	leftRouterDevices.Add(c.Get(0));
   }

   NS_LOG_INFO ("Create right leaf channels");

   for(uint32_t i=0 ; i< num_of_nodes/2;i++)
   {
   		p2p.SetDeviceAttribute ("DataRate", StringValue (edge_bw));
    	p2p.SetChannelAttribute ("Delay", StringValue(edge_delay[i+num_of_nodes/2]));
    	NetDeviceContainer c = p2p.Install(routerNodes.Get(0), rightNodes.Get(i));
    	rightDevices.Add(c.Get(1));
    	rightRouterDevices.Add(c.Get(0));
   }
}

CreateDumbbell::~CreateDumbbell()
{

}

void CreateDumbbell::InstallStack(InternetStackHelper stack)
{
	stack.Install(routerNodes);
	stack.Install(leftNodes);
	stack.Install(rightNodes);
}

void CreateDumbbell::AssignIpv4Address(Ipv4AddressHelper leftIp, Ipv4AddressHelper rightIp, Ipv4AddressHelper routerIp)
{
	routerInterfaces = routerIp.Assign(routerDevices);

	for(uint32_t i=0;i<num_nodes/2;i++)
	{
		NetDeviceContainer ndc;
      	ndc.Add (leftDevices.Get (i));
      	ndc.Add (leftRouterDevices.Get (i));
      	Ipv4InterfaceContainer ifc = leftIp.Assign (ndc);
    	  leftLeafInterfaces.Add (ifc.Get (0));
      	leftRouterInterfaces.Add (ifc.Get (1));
      	leftIp.NewNetwork ();
    }
  // Assign to right side
  for (uint32_t i = 0; i < num_nodes/2; ++i)
    {
      NetDeviceContainer ndc;
      ndc.Add (rightDevices.Get (i));
      ndc.Add (rightRouterDevices.Get (i));
      Ipv4InterfaceContainer ifc = rightIp.Assign (ndc);
      rightLeafInterfaces.Add (ifc.Get (0));
      rightRouterInterfaces.Add (ifc.Get (1));
      rightIp.NewNetwork ();
    }
}

Ptr<Node> CreateDumbbell::GetLeftRouter () const
{ // Get the left side bottleneck router
  return routerNodes.Get (0);
}

Ptr<Node> CreateDumbbell::GetLeft (uint32_t i) const
{ // Get the i'th left side leaf
  return leftNodes.Get (i);
}

Ptr<Node> CreateDumbbell::GetRight () const
{ // Get the right side bottleneck router
  return routerNodes.Get (1);
}

Ptr<Node> CreateDumbbell::GetRight (uint32_t i) const
{ // Get the i'th right side leaf
  return rightNodes.Get (i);
}

Ipv4Address CreateDumbbell::GetLeftIpv4Address (uint32_t i) const
{
  return leftLeafInterfaces.GetAddress (i);
}

Ipv4Address CreateDumbbell::GetRightIpv4Address (uint32_t i) const
{
  return rightLeafInterfaces.GetAddress (i);
}

}


