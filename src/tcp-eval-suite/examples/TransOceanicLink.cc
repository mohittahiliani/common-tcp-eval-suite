#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
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

int
main (int argc, char *argv[])
{

  Time::SetResolution (Time::NS);
  LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);

  // Main Container of all nodes in the topology
  NS_LOG_INFO ("Create nodes");
  NodeContainer TransOceanicLink;

  TransOceanicLink.Create (8);
  Names::Add ( "N1", TransOceanicLink.Get (0));
  Names::Add ( "N2", TransOceanicLink.Get (1));
  Names::Add ( "N3", TransOceanicLink.Get (2));
  Names::Add ( "R1", TransOceanicLink.Get (3));
  Names::Add ( "R2", TransOceanicLink.Get (4));
  Names::Add ( "N4", TransOceanicLink.Get (5));
  Names::Add ( "N5", TransOceanicLink.Get (6));
  Names::Add ( "N6", TransOceanicLink.Get (7));


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

  UdpEchoServerHelper echoServer (9);

  ApplicationContainer serverApps = echoServer.Install (TransOceanicLink.Get (5));
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (10.0));

  UdpEchoClientHelper echoClient (Ir2n4.GetAddress (1), 9);
  echoClient.SetAttribute ("MaxPackets", UintegerValue (1));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

  ApplicationContainer clientApps = echoClient.Install (TransOceanicLink.Get (0));
  clientApps.Start (Seconds (2.0));
  clientApps.Stop (Seconds (10.0));

  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}
