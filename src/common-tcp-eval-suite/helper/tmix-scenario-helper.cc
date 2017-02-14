#include <iostream>
#include <fstream>
#include <string>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "tmix-scenario-helper.h"
#include "ns3/config-store-module.h"

namespace ns3
{
NS_LOG_COMPONENT_DEFINE ("TmixScenarioHelper");

tmixScenario::tmixScenario()
{

}

tmixScenario::~tmixScenario()
{

}

void tmixScenario::AddCvecsToPairs (Ptr<TmixTopology> tmix, bool twosided, double portion, uint32_t numPairs, uint32_t cvecsPerPair,double edgedelay[], std::vector<std::string> tmixBaseCVName)
{
 uint32_t i,j, flowsPerNode=3;
  std::vector<TmixTopology::TmixNodePair> leftNodes;
  std::vector<TmixTopology::TmixNodePair> rightNodes;
 
  Tmix::ConnectionVector cvec1;

  Ptr<UniformRandomVariable> rnd = CreateObject<UniformRandomVariable> ();
  char fileNameOut[] = "scratch/outbound.ns";
  std::string fileNameIn[9];

  system("ls \"tmixtraces\" > files.txt");
  system("g++ utils/cvec-orig2alt.cpp -o cvec");
  system("awk 'BEGIN{i=0;} {if ($0 ~ /.*shuf/){var=substr($0,0,5); system(\"./cvec tmixtraces/\"$0\" tmixtraces/\"var\".ns\");i++;system(\"rm tmixtraces/\"$0);}}' files.txt ");
  

  std::ifstream cvecFileIn[9];
  std::ifstream cvecFileOut;
  for(i=0;i<9;i++)
  {
    fileNameIn[i]=(tmixBaseCVName[i]+".ns");
    cvecFileIn[i].open (fileNameIn[i]); 

    if (!cvecFileIn[i])
    {
      std::exit (0);
    }

  }

     if (twosided)
    {
      cvecFileOut.open (fileNameOut);
      if (!cvecFileOut)
        {  
          std::exit (0);
        }
    }

    tmix->AssignNodes(numPairs,numPairs);

    for(uint32_t i=0;i<numPairs;i++)
    {
      for(uint32_t j=0;j<numPairs;j++)
      {
        leftNodes.push_back (tmix->NewPair (TmixTopology::LEFT, Seconds(edgedelay[i]), Seconds(edgedelay[j+3]), i,j));
      }
    }

 
  if (twosided)
    {

      for(uint32_t i=0;i<numPairs;i++)
    {
      for(uint32_t j=0;j<numPairs;j++)
      {
        leftNodes.push_back (tmix->NewPair (TmixTopology::RIGHT, Seconds(edgedelay[j]), Seconds(edgedelay[i+3]), i,j));
      }
    }


    }  
  for (i = 0; i < numPairs * cvecsPerPair*flowsPerNode; i += numPairs)
    {
      j=0; 
      for (std::vector<TmixTopology::TmixNodePair>::const_iterator itr = leftNodes.begin ();
           itr != leftNodes.end (); ++itr ,++j)
        { 
          if (Tmix::ParseConnectionVector (cvecFileIn[j], cvec1) && (rnd->GetValue () < portion))
            {  
                (itr->helper)->AddConnectionVector (cvec1);
            }
        }
    }
 
  if (twosided)
    {
      for (i = 0; i < numPairs * cvecsPerPair*flowsPerNode; i += numPairs)
        {
          j=0; 
          for (std::vector<TmixTopology::TmixNodePair>::const_iterator itr = rightNodes.begin ();
               itr != rightNodes.end (); ++itr, ++j)
            {

              if (Tmix::ParseConnectionVector (cvecFileIn[j], cvec1) && (rnd->GetValue () < portion))

                { 
                  (itr->helper)->AddConnectionVector (cvec1);
                }
            }
        }
    }
}

void tmixScenario::setexptParameters(bool two_sided, double cvec_Portion, uint32_t num_Pairs, uint32_t cvecs_PerPair, std::string bot_bw, uint32_t bot_delay,std::string Edge_bw)
{
	twosided = two_sided;
	numPairs = num_Pairs;
        cvecsPerPair = cvecs_PerPair;
        cvecPortion = cvec_Portion;
        bottleneck_bw = bot_bw;
        bottleneck_delay = bot_delay;
        edge_bw = Edge_bw;
}

void tmixScenario::settmixParameters(double scale, double testtime, double warmup,std::vector<std::string> tmixBaseCVName, bool findstats, bool findtarget, double prefillT, double BottleneckCapacity, int32_t maxrtt, double targetload, ns3::TmixShuffle::direction targetdirection, int32_t mss, int32_t pktoh, double balancetol, double loadtol)
{
  Scale = scale;
  Binsecs = 500.0 / BottleneckCapacity;
  TmixBaseCVName = tmixBaseCVName;
  Findstats = findstats;
  Findtarget = findtarget;
  PrefillT = prefillT;
  Bps = BottleneckCapacity * 1.0 * pow (10,6);;
  CcTmixSrcs = tmixBaseCVName.size ();;
  Maxrtt = maxrtt;
  Targetload = targetload;
  Targetdirection = targetdirection;
  Mss = mss;
  Pktoh = pktoh;
  Balancetol = balancetol;
  Loadtol = loadtol;
  Maxtrace = 3000.0 * scale;
  Simtime = testtime + warmup + prefillT;
  if (Simtime > Maxtrace)
  {
      Simtime = Maxtrace;
  }
  Longflowthresh = 0.5 * prefillT;
}


void 
tmixScenario::DestroyTrace (Ptr<TmixTopology> tmix)
{
  tmix->DestroyConnection ();
}


void tmixScenario::runScenario(std:: string scenarioName, double edgedelay[],float bufferLimit,uint32_t expt_num)

{
  std::string fileName = "";

  ts.ShuffleTraces (Scale,Simtime,Binsecs,TmixBaseCVName,Findstats,Findtarget,PrefillT,Bps,CcTmixSrcs,Maxrtt,Targetload,Targetdirection,Longflowthresh,Mss,Pktoh,Balancetol,Loadtol);

  TypeId tid;
  std::string transport_prot[] = { "ns3::TcpNewReno", "ns3::TcpHybla","ns3::TcpHighSpeed","ns3::TcpVegas", "ns3::TcpScalable","ns3::TcpHtcp", "ns3::TcpVeno", "ns3::TcpBic", "ns3::TcpYeah", "ns3::TcpIllinois","ns3::TcpWestwood", "ns3::TcpWestwoodPlus"} ;
   
  
 // Main Container of all nodes in the topology
  for (uint32_t i = 0; i < 12; ++i)
  {
   //Select TCP variant       Use lookup by name

    if (i == 11)
    {
      std::cout<<"\nTCP WestwoodPlus is not supported by Tmix yet";
      //Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue (TcpWestwood::GetTypeId ()));
      //Config::SetDefault ("ns3::TcpWestwood::ProtocolType", EnumValue (TcpWestwood::WESTWOODPLUS));
    }
    else
    {
      tid = TypeId::LookupByName (transport_prot[i]);
      Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue (tid));
    }
    
    fileName = "tcp-eval-output/"+ scenarioName + "/EXPT-"+std::to_string(expt_num+1);

    char temp[200] ;
    system("if test tcp-eval-output ; then mkdir tcp-eval-output; fi");
    sprintf(temp,"if test tcp-eval-output/%s ; then mkdir tcp-eval-output/%s; fi",scenarioName.c_str(),scenarioName.c_str());
    system(temp);
    sprintf(temp,"if test %s ; then mkdir %s; fi",fileName.c_str(),fileName.c_str());
    system(temp);

  InternetStackHelper internet;
  Ptr<TmixToplogyParameters> ttp = Create<TmixToplogyParameters>();
  std::cout <<bottleneck_bw<<" "<<bottleneck_delay<<" "<<bufferLimit<<"\n";
  ttp->SetTmixDeviceRate(DataRate (edge_bw));
  ttp->SetRouterDeviceInRate(DataRate (bottleneck_bw));
  ttp->SetRouterDeviceOutRate(DataRate (bottleneck_bw));
  ttp->SetRouterOutQueueLimit(bufferLimit);
  ttp->SetCenterChannelDelay(MicroSeconds (bottleneck_delay));
  Ptr<TmixTopology> tmix = Create<TmixTopology> (internet,ttp, scenarioName, transport_prot[i], expt_num);

  AddCvecsToPairs (tmix, twosided, cvecPortion, numPairs,cvecsPerPair, edgedelay, TmixBaseCVName);
  
  GlobalRouteManager::BuildGlobalRoutingDatabase ();
  GlobalRouteManager::InitializeRoutes ();

  Simulator::Schedule (Seconds (Simtime), &tmixScenario::DestroyTrace, this, tmix);
  Simulator::Stop (Seconds (Simtime));
  Simulator::Run ();
  Simulator::Destroy ();


  tmix->Summary(scenarioName, transport_prot[i], expt_num);
}
for(uint32_t i=0;i<9;i++)
{
        std::string var = "rm "+TmixBaseCVName[i]+".ns";
	system(var.c_str());
}
}

}
