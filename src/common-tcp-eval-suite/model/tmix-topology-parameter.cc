#include "tmix-topology-parameter.h"
#include "ns3/log.h"
#include "ns3/core-module.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("TmixToplogyParameters");

NS_OBJECT_ENSURE_REGISTERED (TmixToplogyParameters);

TypeId
TmixToplogyParameters::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::TmixToplogyParameters")
    .SetParent<Object> ()
    .AddAttribute ("aqmUsed", "Decide the usage of AQM",
                   BooleanValue (false),
                   MakeBooleanAccessor (&TmixToplogyParameters::m_aqmUsed),
                   MakeBooleanChecker ())
    .AddAttribute ("tmixDeviceQueueLimit", "Queue Limit of Tmix Device",
                   UintegerValue (200),
                   MakeUintegerAccessor (&TmixToplogyParameters::m_tmixDeviceQueueLimit),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("routerInQueueLimit", "Queue Limit of router Device connected to nodes",
                   UintegerValue (200),
                   MakeUintegerAccessor (&TmixToplogyParameters::m_routerInQueueLimit),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("routerOutQueueLimit", "Queue Limit of router Device connected in central channel",
                   UintegerValue (200),
                   MakeUintegerAccessor (&TmixToplogyParameters::m_routerOutQueueLimit),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("nodeToRouterDelay",
                   "Delay in periphary channels",
                   TimeValue (MicroSeconds (100)),
                   MakeTimeAccessor (&TmixToplogyParameters::m_nodeToRouterDelay),
                   MakeTimeChecker ())
    .AddAttribute ("centerChannelDelay",
                   "Delay in periphary channels",
                   TimeValue (MicroSeconds (100)),
                   MakeTimeAccessor (&TmixToplogyParameters::m_centerChannelDelay),
                   MakeTimeChecker ())
    .AddAttribute ("tmixDeviceRate",
                   "BW in periphary nodes",
                   DataRateValue (DataRate ("10Mbps")),
                   MakeDataRateAccessor (&TmixToplogyParameters::m_tmixDeviceRate),
                   MakeDataRateChecker ())
    .AddAttribute ("routerDeviceInRate",
                   "BW in periphary nodes to router channel",
                   DataRateValue (DataRate ("10Mbps")),
                   MakeDataRateAccessor (&TmixToplogyParameters::m_routerDeviceInRate),
                   MakeDataRateChecker ())
    .AddAttribute ("routerDeviceOutRate",
                   "Delay in central channel",
                   DataRateValue (DataRate ("1Mbps")),
                   MakeDataRateAccessor (&TmixToplogyParameters::m_routerDeviceOutRate),
                   MakeDataRateChecker ());
  return tid;
}

TmixToplogyParameters::TmixToplogyParameters (void)
{
  m_nodeToRouterDelay = MicroSeconds (100);
  m_centerChannelDelay = MicroSeconds (100);
  m_tmixDeviceRate = DataRate ("1Mbps");
  m_routerDeviceInRate = DataRate ("1Mbps");
  m_routerDeviceOutRate = DataRate ("10Mbps");
  m_tmixDeviceQueueLimit = 200;
  m_routerInQueueLimit = 200;
  m_routerOutQueueLimit = 200;
  m_aqmUsed = false;
}

TmixToplogyParameters::~TmixToplogyParameters (void)
{
}

void TmixToplogyParameters::SetNodeToRouterDelay (Time del)
{
  m_nodeToRouterDelay = del;
}

Time TmixToplogyParameters::GetNodeToRouterDelay (void) const
{
  return m_nodeToRouterDelay;
}


void TmixToplogyParameters::SetCenterChannelDelay (Time del)
{
  m_centerChannelDelay = del;
}
Time TmixToplogyParameters::GetCenterChannelDelay (void) const
{
  return m_centerChannelDelay;
}

void TmixToplogyParameters::SetTmixDeviceRate (DataRate rate)
{
  m_tmixDeviceRate = rate;
}
DataRate TmixToplogyParameters::GetTmixDeviceRate (void) const
{
  return m_tmixDeviceRate;
}

void TmixToplogyParameters::SetRouterDeviceInRate (DataRate rate)
{
  m_routerDeviceInRate = rate;
}
DataRate TmixToplogyParameters::GetRouterDeviceInRate (void) const
{
  return m_routerDeviceInRate;
}

void TmixToplogyParameters::SetRouterDeviceOutRate (DataRate rate)
{
  m_routerDeviceOutRate = rate;
}


DataRate TmixToplogyParameters::GetRouterDeviceOutRate (void) const
{
  return m_routerDeviceOutRate;
}

void TmixToplogyParameters::SetTmixDeviceQueueLimit (uint32_t lim)
{
  m_tmixDeviceQueueLimit = lim;
}

uint32_t TmixToplogyParameters::GetTmixDeviceQueueLimit (void) const
{
  return m_tmixDeviceQueueLimit;
}

void TmixToplogyParameters::SetRouterInQueueLimit (uint32_t lim)
{
  m_routerInQueueLimit = lim;
}

uint32_t TmixToplogyParameters::GetRouterInQueueLimit (void) const
{
  return m_routerInQueueLimit;
}

void TmixToplogyParameters::SetRouterOutQueueLimit (uint32_t lim)
{
  m_routerOutQueueLimit = lim;
}

uint32_t TmixToplogyParameters::GetRouterOutQueueLimit (void) const
{
  return m_routerOutQueueLimit;
}

void TmixToplogyParameters::SetAqmUsage (bool val)
{
  m_aqmUsed = val;
}

bool TmixToplogyParameters::IsAqmUsed (void) const
{
  return m_aqmUsed;
}

}
