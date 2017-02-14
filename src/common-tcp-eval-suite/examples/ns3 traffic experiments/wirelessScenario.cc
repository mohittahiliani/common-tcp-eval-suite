#include <iostream>
#include <fstream>
#include <string>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/error-model.h"
#include "ns3/wireless-point-to-point-dumbbell.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/traffic-control-module.h"

using namespace ns3;


/*
         DUMBELL TOPOLOGY

      N1            N4
       \            /
        \          /
  N2-----R1-------R2-----N5
        /          \
       /            \
      N3            N6

*/

NS_LOG_COMPONENT_DEFINE ("WirelessLan");

Ptr<PacketSink> sink;                         /* Pointer to the packet sink application */
uint64_t lastTotalRx = 0;                     /* The value of the last total received bytes */
std::string fileName = "";
std::ofstream fp;

void
CalculateThroughput ()
{
  Time now = Simulator::Now ();                                         /* Return the simulator's virtual time. */
  double cur = (sink->GetTotalRx() - lastTotalRx) * (double) 8/1e5;     /* Convert Application RX Packets to MBits. */
  fp << now.GetSeconds () << "s: \t" << cur << " Mbit/s" << std::endl;
  lastTotalRx = sink->GetTotalRx ();
  Simulator::Schedule (MilliSeconds (100), &CalculateThroughput);
}

int
main (int argc, char *argv[])
{ 
   CommandLine cmd;
   cmd.Parse (argc,argv);

  Time::SetResolution (Time::NS);

    

   uint32_t payloadSize = 1500;                       /* Transport layer payload size in bytes. */
   std::string dataRate = "100Mbps"; 
   double simulationTime = 10; 
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
    
    fileName = "WirelessLan"+transport_prot[i]+".txt";
    fp.open(fileName);

  NS_LOG_INFO ("Create nodes");
   // Create the point-to-point link helpers
  PointToPointHelper pointToPointRouter;
  pointToPointRouter.SetDeviceAttribute  ("DataRate", StringValue ("100Mbps"));
  pointToPointRouter.SetChannelAttribute ("Delay", StringValue ("2ms"));
  PointToPointHelper pointToPointLeaf;
  pointToPointLeaf.SetDeviceAttribute    ("DataRate", StringValue ("100Mbps"));
  pointToPointLeaf.SetChannelAttribute   ("Delay", StringValue ("2ms"));

  uint32_t    nLeftLeaf = 3;
  uint32_t    nRightLeaf = 3;

  std::string mobilityModel = "ns3::RandomWalk2dMobilityModel";
  //PointToPointHelper pointToPointLeafl;
  ns3::WirelessPointToPointDumbbellHelper d (nLeftLeaf, nRightLeaf, pointToPointLeaf, pointToPointRouter, mobilityModel);
  
  // Install Stack
  InternetStackHelper stack;
  d.InstallStack (stack);

  // Assign IP Addresses
  d.AssignIpv4Addresses (Ipv4AddressHelper ("10.1.1.0", "255.255.255.0"),
                         Ipv4AddressHelper ("10.2.1.0", "255.255.255.0"),
                         Ipv4AddressHelper ("10.3.1.0", "255.255.255.0"));
  

  // Set up the routing
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();;

    Ptr<Node> Traffic_source = d.GetLeft(0);
   Ptr<Node> Traffic_sink = d.GetRight(2);
  
   uint16_t port = 9;  // well-known echo port number
  /* Install TCP Receiver on the access point */
  PacketSinkHelper sinkHelper ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), port));
  ApplicationContainer sinkApp = sinkHelper.Install (Traffic_sink);
  sink = StaticCast<PacketSink> (sinkApp.Get (0));

  /* Install TCP/UDP Transmitter on the station */
  OnOffHelper server ("ns3::TcpSocketFactory", (InetSocketAddress (d.GetRightIpv4Address(2), port)));
  server.SetAttribute ("PacketSize", UintegerValue (payloadSize));
  server.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
  server.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
  server.SetAttribute ("DataRate", DataRateValue (DataRate (dataRate)));
  ApplicationContainer serverApp = server.Install (Traffic_source);

  
  sinkApp.Start (Seconds(0));
  serverApp.Start (Seconds(1.0));
  Simulator::Schedule (Seconds(1.1), &CalculateThroughput);
  serverApp.Stop(Seconds(simulationTime+1));
  Simulator::Stop (Seconds (simulationTime + 1));

  Simulator::Run ();
  Simulator::Destroy ();
  double averageThroughput = ((sink->GetTotalRx() * 8) / (1e6  * simulationTime));
  //if (averageThroughput < 50)
  //{
  //NS_LOG_ERROR ("Obtained throughput is not in the expected boundaries!");
  //exit (1);
  //}
  fp << "\nAverage throughtput " << transport_prot[i] << " : " << averageThroughput << " Mbit/s" << std::endl;
  fp.close();
  lastTotalRx=0;
}
  return 0;
}
