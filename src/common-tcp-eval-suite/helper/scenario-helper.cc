/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "scenario-helper.h"
//#include <ns3/evaluation.h>
#include <iostream>
#include <fstream>
#include <string>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("ScenarioHelper");

scenario::scenario()
{

}

scenario::~scenario()
{
	
}

void scenario::settrafficparam(uint32_t payload, std::string Rate, double simTime, uint32_t source_node_num, uint32_t sink_node_num)
{
	payloadSize = payload;
	dataRate = Rate;
	simulationTime = simTime;
	source_node = source_node_num;
	sink_node = sink_node_num;
}

void scenario::settopologyparam(uint32_t num_of_nodes, std::string bot_delay, std::string bot_bw, std::string edge_bw)
{
	
	numnodes = num_of_nodes;
	bottleneck_delay = bot_delay;
	bottleneck_bw = bot_bw;
	edgelink_bw = edge_bw;
	//for(uint32_t i=0;i<numnodes;i++)
	//{
	//	std::strcpy(edgelink_delay[i],edge_delay[i]);
	//}
}

void scenario::runscenario(std::string scenarioName, std::string edge_delay[],double bufferLimit)
{
   TypeId tid;
   std::string transport_prot[] = { "ns3::TcpNewReno", "ns3::TcpHybla","ns3::TcpHighSpeed","ns3::TcpVegas", "ns3::TcpScalable","ns3::TcpHtcp", "ns3::TcpVeno", "ns3::TcpBic", "ns3::TcpYeah", "ns3::TcpIllinois","ns3::TcpWestwood", "ns3::TcpWestwoodPlus"} ;
   // Main Container of all nodes in the topology
  for (int i = 0; i < 12; ++i)
  {
  // Select TCP variant       Use lookup by name

    if (i == 11)
    {
      Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue (TcpWestwood::GetTypeId ()));
      Config::SetDefault ("ns3::TcpWestwood::ProtocolType", EnumValue (TcpWestwood::WESTWOODPLUS));
    }
    else

    {
      tid = TypeId::LookupByName (transport_prot[i]);
      Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue (tid));
    }
    
    fileName = "TCPEvalOutput/"+scenarioName+"_"+transport_prot[i]+".txt";
    fp.open(fileName);

   ns3::CreateDumbbell scenariotop (numnodes, bottleneck_delay, bottleneck_bw, edgelink_bw, edge_delay,bufferLimit);

   /*NS_LOG_INFO ("Install internet stack on all nodes.");
   InternetStackHelper internet;
   scenariotop.InstallStack (internet);*/

  
   NS_LOG_INFO ("Assign IP Addresses");
   scenariotop.AssignIpv4Address(Ipv4AddressHelper ("10.1.1.0", "255.255.255.0"),
                         Ipv4AddressHelper ("10.2.1.0", "255.255.255.0"),
                         Ipv4AddressHelper ("10.3.1.0", "255.255.255.0"));

   // Set up the routing
   Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

   // Ping Application, Pinging from N1 (Node 0) to N4 (Node 5)
  
  Ptr<Node> Traffic_source = scenariotop.GetLeft(source_node);
  Ptr<Node> Traffic_sink = scenariotop.GetRight(sink_node);
   uint16_t port = 9;  // well-known echo port number
  /* Install TCP Receiver on the access point */
  PacketSinkHelper sinkHelper ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), port));
  ApplicationContainer sinkApp = sinkHelper.Install (Traffic_sink);
  //evaluation Eval;
  //Eval.sink = StaticCast<PacketSink> (sinkApp.Get (0));

  /* Install TCP/UDP Transmitter on the station */
  OnOffHelper server ("ns3::TcpSocketFactory", (InetSocketAddress (scenariotop.GetRightIpv4Address (2), port)));
  server.SetAttribute ("PacketSize", UintegerValue (payloadSize));
  server.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
  server.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
  server.SetAttribute ("DataRate", DataRateValue (DataRate (dataRate)));
  ApplicationContainer serverApp = server.Install (Traffic_source);

  
  sinkApp.Start (Seconds(0));
  serverApp.Start (Seconds(1.0));
  //Simulator::Schedule (Seconds(1.1), &Eval.CalculateThroughput);
  serverApp.Stop(Seconds(simulationTime+1));
  Simulator::Stop (Seconds (simulationTime + 1));

  Simulator::Run ();
  Simulator::Destroy ();
  //double averageThroughput = ((sink->GetTotalRx() * 8) / (1e6  * simulationTime));
  //if (averageThroughput < 50)
  //{
  //NS_LOG_ERROR ("Obtained throughput is not in the expected boundaries!");
  //exit (1);
  //}
  //fp << "\nAverage throughtput " << transport_prot[i] << " : " << averageThroughput << " Mbit/s" << std::endl;
  fp << "\nTCP completed " << transport_prot[i] << std::endl;
  fp.close();
  //lastTotalRx=0;
}
}

}

