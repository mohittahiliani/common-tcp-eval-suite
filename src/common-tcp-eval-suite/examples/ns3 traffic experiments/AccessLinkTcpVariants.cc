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
#include "ns3/scenario-helper.h"

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

NS_LOG_COMPONENT_DEFINE ("AccessLinks");

int
main (int argc, char *argv[])
{ 
   
   uint32_t payloadSize = 1500;                       /* Transport layer payload size in bytes. */
   double bufferLimit = 850;
   std::string dataRate = "100Mbps"; 
   double simulationTime = 10; 
   
   uint32_t num_of_nodes = 6;
   std::string bot_delay = "2ms";
   std::string bot_bw = "100Mbps";
   std::string edge_bw = "100Mbps";
   std::string edge_delay[] = {"0ms","12ms","25ms","2ms","37ms","75ms"};

   scenario AccessLink;

   AccessLink.settrafficparam(payloadSize,dataRate,simulationTime,0,2);
   AccessLink.settopologyparam(num_of_nodes,bot_delay,bot_bw,edge_bw);
   AccessLink.runscenario("AccessLink",edge_delay,bufferLimit);

  return 0;
}
