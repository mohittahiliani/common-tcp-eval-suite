#include "ns3/random-variable-stream.h"
#include <vector>
#include <string>

#define gluedir(a,b) ((b)==0 ? a##I : a##A)
#define glue(a,b) (a##(b))

#ifndef TMIX_SHUFFLE_TRACES_H
#define TMIX_SHUFFLE_TRACES_H

namespace ns3 {

class TmixShuffle
{
public:
  TmixShuffle ();
  ~TmixShuffle ();

  enum direction
  {
    INITIATOR,
    ACCEPTOR,
    BOTH
  };

  void SetBaseSeed (int bss);

  void SetStartStream (long int rss);

  int GetBaseSeed ();

  void AssignStreams ();

  void NextStream ();

  std::vector<int> FisherYatesShuffle (std::vector<int> binlist, int reqlen);

  double AddBurstStats (double data, double overhead, direction dir);

  void ProcessBurst (double data, direction dir, int first);

  std::vector<std::string> ShuffleTraces (double& scale, double& simtime, double binsecs, std::vector<std::string> tmixBaseCVName, bool findstats, bool findtarget, double prefillT, double bps, int ccTmixSrcs, int maxrtt, double targetload, direction targetdirection, double longflowthresh, int mss, int pktoh, double balancetol, double loadtol);

private:
  //Ptr<RandomVariableStream> m_srng;
  std::vector <double> m_binConnDataListI;
  std::vector <double> m_binConnDataListA;
  double m_burstTEstI;
  double m_burstTEstA;
  int m_numSbinsI;
  int m_numSbinsA;
  double m_binSizeus;
  double m_connDataI;
  double m_connDataA;
  double m_preConnDataI;
  double m_preConnDataA;
  double m_OverheadEstI;
  double m_OverheadEstA;
  double m_preConnOverheadI;
  double m_preConnOverheadA;
  double m_lastidleI;
  double m_lastidleA;
  double m_prefillus;
  double m_bpus;
  char m_CVtype;
  int m_mssI;
  int m_mssA;
  int m_pktoh;
  long int m_rStartStream;
  double m_connArrRate;
  double m_bl;
  double m_sumPrefillValI;
  double m_sumPrefillValA;
};

}
#endif
