/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/tmix.h"
#include "ns3/tmix-topology.h"
#include "ns3/tmix-helper.h"
#include "ns3/tmix-ns2-style-trace-helper.h"
#include "ns3/tmix-topology-parameter.h"

#include "ns3/global-route-manager.h"

#include "ns3/test.h"

using namespace ns3;

// These two things are global because when TmixTopology c'tor is called, it assigns IP addresses in a given range without checking whether they have been used up or not.
// It causes problems since two TestCase c'tors call the TmixTopology c'tor (when creating object).
// So the creation of the TmixTopology object is done only in the first testcase.
// FIXME : Fix the above problem. Find a way to unassign the IP adresses or create the object outside the test cases.

Ptr<TmixTopology> tmix;
InternetStackHelper internet;

class TmixTopologyTest : public TestCase
{
public:
  TmixTopologyTest ();
  virtual ~TmixTopologyTest ();
private:
  virtual void DoRun (void);
  void AddNodes (Ptr<TmixTopology>& tmix, int nLeftPairs, int nRightPairs);
};

TmixTopologyTest::TmixTopologyTest ()
  : TestCase ("Creates nodes and checks if they have been created")
{
  Ptr<TmixToplogyParameters> ttp = Create<TmixToplogyParameters>();
  tmix = Create<TmixTopology> (internet,ttp);
}

TmixTopologyTest::~TmixTopologyTest ()
{
}

void TmixTopologyTest::AddNodes (Ptr<TmixTopology>& tmix,int nLeftPairs, int nRightPairs)
{
  int i;
  for (i = 0; i < nLeftPairs; i++)
    {
      tmix->NewPair (TmixTopology::LEFT);
    }
  for (i = 0; i < nRightPairs; i++)
    {
      tmix->NewPair (TmixTopology::RIGHT);
    }
}

void
TmixTopologyTest::DoRun (void)
{
  uint32_t num_left = 3, num_right = 4;
  AddNodes (tmix, num_left, num_right);
  NS_TEST_ASSERT_MSG_EQ ((tmix->LeftPairs ()).size (), num_left, "No of nodes being created not equal to the specified number at LEFT");
  NS_TEST_ASSERT_MSG_EQ ((tmix->RightPairs ()).size (), num_right, "No of nodes being created not equal to the specified number at RIGHT");
}

//***************************************//


class TmixCvecParseTest : public TestCase
{
public:
  TmixCvecParseTest ();
  virtual ~TmixCvecParseTest ();
private:
  virtual void DoRun (void);

  Tmix::ADU adu;
  Tmix::ConnectionVector cvec;
  bool parse_success;
};

TmixCvecParseTest::TmixCvecParseTest ()
  : TestCase ("Tries to parse a Cvec file")
{
}

TmixCvecParseTest::~TmixCvecParseTest ()
{
}

void
TmixCvecParseTest::DoRun (void)
{
  // please use the inbound.ns file given by UNC, Chapel Hill
  // values are hard coded, based on the inbound.ns file

  std::ifstream cvecfile;
  cvecfile.open ("src/tmix/test//tmix-inbound.ns");
  parse_success = Tmix::ParseConnectionVector (cvecfile, cvec);
  cvecfile.close ();

  NS_TEST_ASSERT_MSG_EQ (parse_success, true, "Single Cvec parsed");

  NS_TEST_ASSERT_MSG_EQ (cvec.type, Tmix::ConnectionVector::SEQUENTIAL, "Type read wrong");
  NS_TEST_ASSERT_MSG_EQ (cvec.id1, 21217, "ID 1 read wrong");
  NS_TEST_ASSERT_MSG_EQ (cvec.id2, 555381, "ID 2 read wrong");
  NS_TEST_ASSERT_MSG_EQ (cvec.windowSizeInitiator, 64800, "Initiator window size read wrong");
  NS_TEST_ASSERT_MSG_EQ (cvec.windowSizeAcceptor, 6432, "Acceptor window size read wrong");
  NS_TEST_ASSERT_MSG_EQ (cvec.minRTT, MicroSeconds (1118156), "RTT read wrong");
  NS_TEST_ASSERT_MSG_EQ (cvec.lossRateItoA, 0.000000, "lossRateItoA read wrong");
  NS_TEST_ASSERT_MSG_EQ (cvec.lossRateAtoI, 0.000000, "lossRateAtoI read wrong");

  NS_TEST_ASSERT_MSG_EQ (cvec.adus.size (), 3, "ADUs number read wrong");

  int p = 0;
  Tmix::ADU::Side sidesList[] = {Tmix::ADU::INITIATOR,Tmix::ADU::ACCEPTOR,Tmix::ADU::ACCEPTOR};
  uint32_t sendWaitTimeList[] = {0, 0, 6308497};
  uint32_t recvWaitTimeList[] = {0, 123693, 0};
  uint32_t sizeList[] = {253, 510, 0};

  for (std::vector<Tmix::ADU>::const_iterator i = cvec.adus.begin (); i != cvec.adus.end (); ++i,++p)
    {
      NS_TEST_ASSERT_MSG_EQ (i->side, sidesList[p], "Something read wrong");
      NS_TEST_ASSERT_MSG_EQ (i->sendWaitTime, MicroSeconds (sendWaitTimeList[p]), "Something read wrong");
      NS_TEST_ASSERT_MSG_EQ (i->recvWaitTime, MicroSeconds (recvWaitTimeList[p]), "Something read wrong");
      NS_TEST_ASSERT_MSG_EQ (i->size, sizeList[p], "Something read wrong");
    }
}

//***************************************//


class TmixTrafficTest : public TestCase
{
public:
  TmixTrafficTest ();
  virtual ~TmixTrafficTest ();
private:
  virtual void DoRun (void);

  bool FileCompare (const char filename1[], const char filename2[]);
  Tmix::ConnectionVector cvec;
  TmixTopology::TmixNodePair pair;
  std::ifstream cvecfile;
  std::ofstream outputfile;
};

TmixTrafficTest::TmixTrafficTest ()
  : TestCase ("Creates traffic and checks if they have been created")
{
}

TmixTrafficTest::~TmixTrafficTest ()
{
}

bool TmixTrafficTest::FileCompare (const char filename1[], const char filename2[])
{
  std::ifstream file1 (filename1);
  std::ifstream file2 (filename2);
  std::string s1,s2;
  std::getline (file1,s1);
  getline (file2,s2);

  if (!(s1.compare (s2)))
    {
      return true;
    }
  else
    {
      return false;
    }
}

void
TmixTrafficTest::DoRun (void)
{
  pair = tmix->NewPair (TmixTopology::LEFT);

  cvecfile.open ("src/tmix/test/tmix-inbound.ns");
  outputfile.open ("scratch/out.txt");

  Tmix::ParseConnectionVector (cvecfile, cvec);
  pair.helper->AddConnectionVector (cvec);

  std::ofstream& ns2StyleTraceFile = outputfile;
  Tmix::Ns2StyleTraceHelper ost (tmix, ns2StyleTraceFile);
  ost.Install ();

  GlobalRouteManager::BuildGlobalRoutingDatabase ();
  GlobalRouteManager::InitializeRoutes ();

  Simulator::Stop (Seconds (10));
  Simulator::Run ();
  Simulator::Destroy ();

  outputfile.close ();

  NS_TEST_ASSERT_MSG_EQ (FileCompare ("scratch/out.txt","src/tmix/test/ref.txt"),true, "Files don't match");

}

//***********************************************************************//

class TmixTestSuite : public TestSuite
{
public:
  TmixTestSuite ();
};

TmixTestSuite::TmixTestSuite ()
  : TestSuite ("tmix", UNIT)
{
  AddTestCase (new TmixTopologyTest, TestCase::QUICK);
  AddTestCase (new TmixCvecParseTest, TestCase::QUICK);
  AddTestCase (new TmixTrafficTest, TestCase::QUICK);
}

static TmixTestSuite tmixTestSuite;

