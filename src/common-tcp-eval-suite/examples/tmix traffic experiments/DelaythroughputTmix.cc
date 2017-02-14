/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

//review

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
  uint32_t bottleneck_delay = 2000; //in microseconds
  double edgedelay[] = {0,0.012,0.025, 0.002,0.037,0.075};
  uint32_t buffer_length[] = {102,102};
  uint32_t core_delay = 2;  //milliseconds
  float BottleneckCapacity = 100;
  double BuffPercnt[] = {0.10,0.20,0.50,1,2};

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

  double targetload = 0.85;
  double testtime= 829.0;
  double warmup= 179.0;
  double prefillT = 52.02;
  double scale= 3.812;


  double shufbalancetol = 0.05;
  double shufloadtol= 0.02;
  int32_t mss = 1460;
  int32_t pktoh = 40;
  bool findstats = true;
  bool findtarget = false;
  float bufferLimit = 850;

  tmixScenario Delaythroughput;
  Delaythroughput.setexptParameters(twosided,cvecPortion,numPairs,cvecsPerPair,bottleneck_bw,bottleneck_delay);
  Delaythroughput.settmixParameters(scale, testtime, warmup, tmixBaseCVName, findstats, findtarget, prefillT, BottleneckCapacity, maxrtt, targetload, targetdirection, mss, pktoh, shufbalancetol, shufloadtol);
  
  for(uint32_t i=0;i<5;i++)
  {
    Delaythroughput.runScenario("Delaythroughput",edgedelay,bufferLimit*BuffPercnt[i],i);
  }

  return 0;
}

