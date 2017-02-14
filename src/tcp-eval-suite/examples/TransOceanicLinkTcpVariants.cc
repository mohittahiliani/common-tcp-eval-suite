#include <iostream>
#include <fstream>
#include <string>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/error-model.h"

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

NS_LOG_COMPONENT_DEFINE ("TransOceanicLink");

Ptr<PacketSink> sink;                         /* Pointer to the packet sink application */
uint64_t lastTotalRx = 0;                     /* The value of the last total received bytes */
std::string fileName = "";
std::ofstream fp;

NodeContainer N1R1;
NodeContainer N2R1;
NodeContainer N3R1;
NodeContainer R1R2;
NodeContainer R2N4;
NodeContainer R2N5;
NodeContainer R2N6;

Ipv4InterfaceContainer In1r1;
Ipv4InterfaceContainer In2r1;
Ipv4InterfaceContainer In3r1;
Ipv4InterfaceContainer Ir1r2;
Ipv4InterfaceContainer Ir2n4;
Ipv4InterfaceContainer Ir2n5;
Ipv4InterfaceContainer Ir2n6;

std::string filename = "WESTWOOD";
void
CalculateThroughput ()
{
  Time now = Simulator::Now ();                                         /* Return the simulator's virtual time. */
  double cur = (sink->GetTotalRx () - lastTotalRx) * (double) 8 / 1e5;     /* Convert Application RX Packets to MBits. */
  fp << now.GetSeconds () << "s: \t" << cur << " Mbit/s" << std::endl;
  lastTotalRx = sink->GetTotalRx ();
  Simulator::Schedule (MilliSeconds (100), &CalculateThroughput);
}

int
main (int argc, char *argv[])
{

  uint32_t payloadSize = 1500;                        /* Transport layer payload size in bytes. */
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
      fileName = "TransOceanicLink"+transport_prot[i]+".txt";
      fp.open (fileName);
      // Main Container of all nodes in the topology
      NS_LOG_INFO ("Create nodes");
      NodeContainer TransOceanicLink;

      TransOceanicLink.Create (8);

      Ptr<Node> Traffic_source = TransOceanicLink.Get (0);
      Ptr<Node> Traffic_sink = TransOceanicLink.Get (7);

      N1R1 = NodeContainer (TransOceanicLink.Get (0), TransOceanicLink.Get (3));
      N2R1 = NodeContainer (TransOceanicLink.Get (1), TransOceanicLink.Get (3));
      N3R1 = NodeContainer (TransOceanicLink.Get (2), TransOceanicLink.Get (3));
      R1R2 = NodeContainer (TransOceanicLink.Get (3), TransOceanicLink.Get (4));
      R2N4 = NodeContainer (TransOceanicLink.Get (4), TransOceanicLink.Get (5));
      R2N5 = NodeContainer (TransOceanicLink.Get (4), TransOceanicLink.Get (6));
      R2N6 = NodeContainer (TransOceanicLink.Get (4), TransOceanicLink.Get (7));

      NS_LOG_INFO ("Install internet stack on all nodes.");
      InternetStackHelper internet;
      internet.Install (TransOceanicLink);

      // Setting the channel properties

      NS_LOG_INFO ("Create channels");
      PointToPointHelper p2p;

      TrafficControlHelper tchBottleneck;
      tchBottleneck.SetRootQueueDisc ("ns3::PfifoFastQueueDisc", "Limit", UintegerValue (13750));

      p2p.SetDeviceAttribute ("DataRate", StringValue ("1Gbps"));
      p2p.SetChannelAttribute ("Delay", StringValue ("0ms"));
      NetDeviceContainer devN1R1 = p2p.Install (N1R1);


      p2p.SetDeviceAttribute ("DataRate", StringValue ("1Gbps"));
      p2p.SetChannelAttribute ("Delay", StringValue ("12ms"));
      NetDeviceContainer devN2R1 = p2p.Install (N2R1);


      p2p.SetDeviceAttribute ("DataRate", StringValue ("1Gbps"));
      p2p.SetChannelAttribute ("Delay", StringValue ("25ms"));
      NetDeviceContainer devN3R1 = p2p.Install (N3R1);


      p2p.SetDeviceAttribute ("DataRate", StringValue ("1Gbps"));
      p2p.SetChannelAttribute ("Delay", StringValue ("65ms"));
      NetDeviceContainer devR1R2 = p2p.Install (R1R2);
      QueueDiscContainer qdiscs;
      qdiscs = tchBottleneck.Install (devR1R2);

      p2p.SetDeviceAttribute ("DataRate", StringValue ("1Gbps"));
      p2p.SetChannelAttribute ("Delay", StringValue ("2ms"));
      NetDeviceContainer devR2N4 = p2p.Install (R2N4);

      p2p.SetDeviceAttribute ("DataRate", StringValue ("1Gbps"));
      p2p.SetChannelAttribute ("Delay", StringValue ("37ms"));
      NetDeviceContainer devR2N5 = p2p.Install (R2N5);

      p2p.SetDeviceAttribute ("DataRate", StringValue ("1Gbps"));
      p2p.SetChannelAttribute ("Delay", StringValue ("75ms"));
      NetDeviceContainer devR2N6 = p2p.Install (R2N6);

      NS_LOG_INFO ("Assign IP Addresses");
      Ipv4AddressHelper ipv4;

      ipv4.SetBase ("10.1.1.0", "255.255.255.0");
      In1r1 = ipv4.Assign (devN1R1);

      ipv4.SetBase ("10.1.2.0", "255.255.255.0");
      In2r1 = ipv4.Assign (devN2R1);

      ipv4.SetBase ("10.1.3.0", "255.255.255.0");
      In3r1 = ipv4.Assign (devN3R1);

      ipv4.SetBase ("10.1.4.0", "255.255.255.0");
      Ir1r2 = ipv4.Assign (devR1R2);

      ipv4.SetBase ("10.1.5.0", "255.255.255.0");
      Ir2n4 = ipv4.Assign (devR2N4);

      ipv4.SetBase ("10.1.6.0", "255.255.255.0");
      Ir2n5 = ipv4.Assign (devR2N5);

      ipv4.SetBase ("10.1.7.0", "255.255.255.0");
      Ir2n6 = ipv4.Assign (devR2N6);

      // Set up the routing
      Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

      // Ping Application, Pinging from N1 (Node 0) to N4 (Node 5)

      uint16_t port = 9; // well-known echo port number
      /* Install TCP Receiver on the access point */
      PacketSinkHelper sinkHelper ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), port));
      ApplicationContainer sinkApp = sinkHelper.Install (Traffic_sink);
      sink = StaticCast<PacketSink> (sinkApp.Get (0));

      /* Install TCP/UDP Transmitter on the station */
      OnOffHelper server ("ns3::TcpSocketFactory", (InetSocketAddress (Ir2n6.GetAddress (1), port)));
      server.SetAttribute ("PacketSize", UintegerValue (payloadSize));
      server.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
      server.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
      server.SetAttribute ("DataRate", DataRateValue (DataRate (dataRate)));
      ApplicationContainer serverApp = server.Install (Traffic_source);


      sinkApp.Start (Seconds (0));
      serverApp.Start (Seconds (1.0));
      Simulator::Schedule (Seconds (1.1), &CalculateThroughput);
      serverApp.Stop (Seconds (simulationTime + 1));
      Simulator::Stop (Seconds (simulationTime + 1));

      Simulator::Run ();
      Simulator::Destroy ();
      double averageThroughput = ((sink->GetTotalRx () * 8) / (1e6  * simulationTime));
      //if (averageThroughput < 50)
      //{
      //NS_LOG_ERROR ("Obtained throughput is not in the expected boundaries!");
      //exit (1);
      //}
      fp << "\nAverage throughtput " << transport_prot[i] << " : " << averageThroughput << " Mbit/s" << std::endl;
      fp.close ();
      lastTotalRx = 0;
    }
  return 0;
}
