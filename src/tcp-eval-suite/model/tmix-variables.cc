#include "tmix-variables.h"
#include "ns3/log.h"
#include "ns3/tmix.h"
#include "ns3/tmix-topology.h"
#include "ns3/tmix-helper.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("TmixVariables");

NS_OBJECT_ENSURE_REGISTERED (TmixVariables);

TypeId
TmixVariables::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::TmixVariables")
    .SetParent<Object> ()
    .SetGroupName ("TcpEvaluationSuite")
    .AddAttribute ("Bottlenecks",
                   "number of Bottleneck link",
                   UintegerValue (1),
                   MakeUintegerAccessor (&TmixVariables::m_nBottlenecks),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("twosided",
                   "Is the traffic two sided",
                   BooleanValue (false),
                   MakeBooleanAccessor (&TmixVariables::m_twosided),
                   MakeBooleanChecker ())
    .AddAttribute ("cvecPortion",
                   "fraction of cvec to be used",
                   DoubleValue (1.0),
                   MakeDoubleAccessor (&TmixVariables::m_cvecPortion),
                   MakeDoubleChecker<double> (0))
    .AddAttribute ("numPairs",
                   "number of source sink pair",
                   UintegerValue (3),
                   MakeUintegerAccessor (&TmixVariables::m_numPairs),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("cvecsPerPair",
                   "number of cvecs per pair",
                   UintegerValue (100),
                   MakeUintegerAccessor (&TmixVariables::m_cvecsPerPair),
                   MakeUintegerChecker<uint32_t> ())
  ;
  return tid;
}

TmixVariables::TmixVariables (void)
{
  m_nBottlenecks = 1;
  m_twosided = false;
  m_cvecPortion = 1.0;
  m_numPairs = 3;
  m_cvecsPerPair = 100000;
}

TmixVariables::~TmixVariables (void)
{
}

void TmixVariables::SetNBottlenecks(uint32_t nBottlenecks)
{
  m_nBottlenecks = nBottlenecks;
}

uint32_t TmixVariables::GetNBottlenecks(void)
{
  return m_nBottlenecks;
}

void TmixVariables::SetTwoSided(bool twoSided)
{
  m_twosided = twoSided;
}

void TmixVariables::SetCvecPortion(double cvecPortion)
{
  m_cvecPortion = cvecPortion;
}
  
void TmixVariables::SetNumPairs(int numPairs)
{
  m_numPairs = numPairs;
}
  
void TmixVariables::SetCvecsPerPair(uint32_t cvecsPerPair)
{
  m_cvecsPerPair = cvecsPerPair;
}

void TmixVariables::AddCvecsToPairs (Ptr<TmixTopology> tmix)
{
  uint32_t i;
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

  if (m_twosided)
    {
      cvecFileOut.open (fileNameOut);
      if (!cvecFileOut)
        {
          std::exit (0);
        }
    }

  for (i = 0; i < m_numPairs; i++)
    {
      leftNodes.push_back (tmix->NewPair (TmixTopology::LEFT, Seconds(2), Seconds(2),i,i));
    }
  if (m_twosided)
    {
      for (i = 0; i < m_numPairs; i++)
        {
          rightNodes.push_back (tmix->NewPair (TmixTopology::RIGHT, Seconds(2), Seconds(2),i,i));
        }
    }

  for (i = 0; i < m_numPairs * m_cvecsPerPair; i += m_numPairs)
    {
      for (std::vector<TmixTopology::TmixNodePair>::const_iterator itr = leftNodes.begin ();
           itr != leftNodes.end (); ++itr)
        {
          if (Tmix::ParseConnectionVector (cvecFileIn, cvec) && (rnd->GetValue () < m_cvecPortion))
            {
              (itr->helper)->AddConnectionVector (cvec);
            }
        }
    }
  if (m_twosided)
    {
      for (i = 0; i < m_numPairs * m_cvecsPerPair; i += m_numPairs)
        {
          for (std::vector<TmixTopology::TmixNodePair>::const_iterator itr = rightNodes.begin ();
               itr != rightNodes.end (); ++itr)
            {
              if (Tmix::ParseConnectionVector (cvecFileIn, cvec) && (rnd->GetValue () < m_cvecPortion))
                {
                  (itr->helper)->AddConnectionVector (cvec);
                }
            }
        }
    }
}

}
