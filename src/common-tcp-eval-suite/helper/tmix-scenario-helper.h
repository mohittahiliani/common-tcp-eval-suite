/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/core-module.h"
#include "ns3/tmix-helper.h"
#include "ns3/tmix-ns2-style-trace-helper.h"
#include "ns3/netanim-module.h"
#include "ns3/global-route-manager.h"
#include "ns3/tmix-shuffle.h"

namespace ns3
{
class tmixScenario
{
public:
        tmixScenario();

        ~tmixScenario();

        void AddCvecsToPairs (Ptr<TmixTopology> tmix, bool twosided, double portion, uint32_t numPairs, uint32_t cvecsPerPair,double edgedelay[], std::vector<std::string> tmixBaseCVName);

        void runScenario(std:: string scenarioName, double edgedelay[],float bufferLimit, uint32_t expt_num);
        void setexptParameters(bool two_sided, double cvec_Portion, uint32_t num_Pairs, uint32_t cvecs_PerPair, std::string bot_bw, uint32_t bot_delay,std::string Edge_bw);
        void settmixParameters(double scale, double testtime, double warmup, std::vector<std::string> tmixBaseCVName, bool findstats, bool findtarget, double prefillT, double BottleneckCapacity, int32_t maxrtt, double targetload, ns3::TmixShuffle::direction targetdirection, int32_t mss, int32_t pktoh, double balancetol, double loadtol);
        void DestroyTrace (Ptr<TmixTopology> tmix);

private:
        bool twosided; 
        uint32_t numPairs; 
        uint32_t cvecsPerPair;
        double cvecPortion;
        std::string bottleneck_bw;
        std::string edge_bw;
        uint32_t bottleneck_delay;

        double Scale;
        double Simtime;
        double Binsecs;
        std::vector<std::string> TmixBaseCVName;
        bool Findstats;
        bool Findtarget;
        double PrefillT;
        double Bps;
        int32_t CcTmixSrcs;
        float Maxrtt;
        double Targetload;
        ns3::TmixShuffle::direction Targetdirection;
        double Longflowthresh;
        int32_t Mss;
        int32_t Pktoh;
        double Balancetol;
        double Loadtol;
        double Maxtrace;
        TmixShuffle ts;
};
}
