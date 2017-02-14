/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/core-module.h"
#include "ns3/tmix-helper.h"
#include "ns3/tmix-ns2-style-trace-helper.h"
#include "ns3/netanim-module.h"
#include "ns3/global-route-manager.h"

using namespace ns3;

void
AddCvecsToPairs (Ptr<TmixTopology> tmix, bool twosided,
                 double portion, int numPairs, int cvecsPerPair)
{
  int i;
  std::vector<TmixTopology::TmixNodePair> leftNodes;
  std::vector<TmixTopology::TmixNodePair> rightNodes;

  Tmix::ConnectionVector cvec;

  Ptr<UniformRandomVariable> rnd = CreateObject<UniformRandomVariable> ();

  char fileNameIn[] = "scratch/inbound.ns";
  char fileNameOut[] = "scratch/outbound.ns";
  std::ifstream cvecFileIn;
  std::ifstream cvecFileOut;

  cvecFileIn.open (fileNameIn);
  if (!cvecFileIn)
    {
      std::exit (0);
    }

  if (twosided)
    {
      cvecFileOut.open (fileNameOut);
      if (!cvecFileOut)
        {
          std::exit (0);
        }
    }

  for (i = 0; i < numPairs; i++)
    {
      leftNodes.push_back (tmix->NewPair (TmixTopology::LEFT, Seconds(2), Seconds(2)));
    }
  if (twosided)
    {
      for (i = 0; i < numPairs; i++)
        {
          rightNodes.push_back (tmix->NewPair (TmixTopology::RIGHT, Seconds(2), Seconds(2)));
        }
    }

  for (i = 0; i < numPairs * cvecsPerPair; i += numPairs)
    {
      for (std::vector<TmixTopology::TmixNodePair>::const_iterator itr = leftNodes.begin ();
           itr != leftNodes.end (); ++itr)
        {
          if (Tmix::ParseConnectionVector (cvecFileIn, cvec) && (rnd->GetValue () < portion))
            {
              (itr->helper)->AddConnectionVector (cvec);
            }
        }
    }
  if (twosided)
    {
      for (i = 0; i < numPairs * cvecsPerPair; i += numPairs)
        {
          for (std::vector<TmixTopology::TmixNodePair>::const_iterator itr = rightNodes.begin ();
               itr != rightNodes.end (); ++itr)
            {
              if (Tmix::ParseConnectionVector (cvecFileIn, cvec) && (rnd->GetValue () < portion))
                {
                  (itr->helper)->AddConnectionVector (cvec);
                }
            }
        }
    }
}

int
main (int argc, char *argv[])
{
  bool twosided = false;
  double cvecPortion = 1.0;
  int numPairs = 3;
  int cvecsPerPair = 100;
  bool useAnimation = true;

  CommandLine cmd;

  cmd.AddValue ("two-sided", "Two sided traffic or one sided traffic", twosided);
  cmd.AddValue ("portion", "Portion of file to be used", cvecPortion);
  cmd.AddValue ("numPairs", "Number of pairs to be used", numPairs);
  cmd.AddValue ("cvecsPerPair", "Number of Conn vectors per pair", cvecsPerPair);
  cmd.AddValue ("animate", "Animation Required?", useAnimation);
  cmd.Parse (argc, argv);

  char animFile[] = "tmix-animation.xml";

  InternetStackHelper internet;
  Ptr<TmixToplogyParameters> ttp = Create<TmixToplogyParameters>();
  Ptr<TmixTopology> tmix = Create<TmixTopology> (internet,ttp);

  AddCvecsToPairs (tmix, twosided, cvecPortion, numPairs,cvecsPerPair);

  std::ostream& ns2StyleTraceFile = std::cout;
  Tmix::Ns2StyleTraceHelper ost (tmix, ns2StyleTraceFile);
  ost.Install ();

  GlobalRouteManager::BuildGlobalRoutingDatabase ();
  GlobalRouteManager::InitializeRoutes ();

  AnimationInterface anim (animFile);

  if (useAnimation == true)
    {
      Ptr<Node> r1 = tmix->GetLeftRouter ();
      Ptr<Node> r2 = tmix->GetRightRouter ();

      int pos = 1;
      std::vector<TmixTopology::TmixNodePair> leftNodes = tmix->LeftPairs ();

      for (std::vector<TmixTopology::TmixNodePair>::const_iterator i = leftNodes.begin ();
           i != leftNodes.end (); ++i, ++pos)
        {
          anim.SetConstantPosition (i->initiator, 0, 20 + pos * 15);
          anim.SetConstantPosition (i->acceptor, 175, 20 + pos * 15);
        }

      pos = 1;
      std::vector<TmixTopology::TmixNodePair> rightNodes = tmix->RightPairs ();
      for (std::vector<TmixTopology::TmixNodePair>::const_iterator i = rightNodes.begin ();
           i != rightNodes.end (); ++i, ++pos)
        {
          anim.SetConstantPosition (i->initiator, 185, 20 + pos * 15);
          anim.SetConstantPosition (i->acceptor, 10, 20 + pos * 15);
        }

      anim.SetConstantPosition (r1, 50, 50);
      anim.SetConstantPosition (r2, 150, 50);
      anim.SetStartTime (Seconds (0.25));
    }

  Simulator::Stop (Seconds (10));
  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}

