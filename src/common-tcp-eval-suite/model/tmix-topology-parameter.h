#ifndef TMIX_TOPOLOGY_PARAM_H_
#define TMIX_TOPOLOGY_PARAM_H_

#include "ns3/object.h"
#include "ns3/data-rate.h"
#include "ns3/core-module.h"

namespace ns3 {

class TmixToplogyParameters : public Object
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  TmixToplogyParameters (void);
  ~TmixToplogyParameters (void);

  void SetNodeToRouterDelay (Time del);
  Time GetNodeToRouterDelay (void) const;

  void SetCenterChannelDelay (Time del);
  Time GetCenterChannelDelay (void) const;

  void SetTmixDeviceRate (DataRate rate);
  DataRate GetTmixDeviceRate (void) const;

  void SetRouterDeviceInRate (DataRate rate);
  DataRate GetRouterDeviceInRate (void) const;

  void SetRouterDeviceOutRate (DataRate rate);
  DataRate GetRouterDeviceOutRate (void) const;

  void SetTmixDeviceQueueLimit (uint32_t lim);
  uint32_t GetTmixDeviceQueueLimit (void) const;

  void SetRouterInQueueLimit (uint32_t lim);
  uint32_t GetRouterInQueueLimit (void) const;

  void SetRouterOutQueueLimit (uint32_t lim);
  uint32_t GetRouterOutQueueLimit (void) const;

  void SetAqmUsage (bool val);
  bool IsAqmUsed (void) const;

private:
  Time m_nodeToRouterDelay, m_centerChannelDelay;
  DataRate m_tmixDeviceRate, m_routerDeviceInRate, m_routerDeviceOutRate;
  uint32_t m_tmixDeviceQueueLimit, m_routerInQueueLimit, m_routerOutQueueLimit;
  bool m_aqmUsed;
};
}
#endif
