/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/log.h"
#include "ns3/core-module.h"
#include "ns3/tmix-helper.h"
#include "ns3/tmix-ns2-style-trace-helper.h"
#include "ns3/netanim-module.h"
#include "ns3/global-route-manager.h"
#include "ns3/tmix-scenario-helper.h"

using namespace ns3;

int main (int argc, char *argv[])
{
  CommandLine cmd;
  cmd.Parse (argc,argv);
  bool twosided = false;
  double cvecPortion = 1.0;
  int numPairs = 3;
  int cvecsPerPair = 4000;
  std::string bottleneck_bw = "100Mbps";
  std::string edge_bw = "100Mbps";  
  uint32_t bottleneck_delay = 2000;
  double edgedelay[] = {0,0.012,0.025,0.002,0.037,0.075};
  uint32_t buffer_length[] = {102,102};
  uint32_t core_delay = 200;
  float BottleneckCapacity = 10;
  std::vector<std::string> tmixBaseCVName; 

  tmixBaseCVName.push_back ("tmixtraces/r4s1");
  tmixBaseCVName.push_back ("tmixtraces/r4s2");
  tmixBaseCVName.push_back ("tmixtraces/r4s3");
  tmixBaseCVName.push_back ("tmixtraces/r5s1");
  tmixBaseCVName.push_back ("tmixtraces/r5s2");
  tmixBaseCVName.push_back ("tmixtraces/r5s3");
  tmixBaseCVName.push_back ("tmixtraces/r6s1");
  tmixBaseCVName.push_back ("tmixtraces/r6s2");
  tmixBaseCVName.push_back ("tmixtraces/r6s3");

  int32_t maxrtt = ( buffer_length[0]+buffer_length[1] +  (100.0 + core_delay) * 2.0) / 1000.0;
  ns3::TmixShuffle::direction targetdirection = ns3::TmixShuffle::direction(2);

  double targetload[] = {0.60, 0.85, 1.1};
  double testtime[]= {479.0, 829.0, 1423.0};
  double warmup[]= {84.0, 179.0, 34.0};
  double prefillT[] = {36.72, 52.02, 67.32};
  double scale[]= {5.276, 3.812, 2.947};


  double shufbalancetol = 0.05;
  double shufloadtol= 0.02;
  int32_t mss = 1460;
  int32_t pktoh = 40;
  bool findstats = true;
  bool findtarget = false;
  float bufferLimit = 850;

  tmixScenario Accesslink;
  Accesslink.setexptParameters(twosided,cvecPortion,numPairs,cvecsPerPair,bottleneck_bw,bottleneck_delay,edge_bw);
  
  for(uint32_t i=0;i<3;i++)
  {
    Accesslink.settmixParameters(scale[i], testtime[i], warmup[i], tmixBaseCVName, findstats, findtarget, prefillT[i], BottleneckCapacity, maxrtt, targetload[i], targetdirection, mss, pktoh, shufbalancetol, shufloadtol);
    Accesslink.runScenario("Accesslink",edgedelay,bufferLimit,i);

  }

  return 0;
}

