/* -*- Mode:C++; c-file-style:''gnu''; indent-tabs-mode:nil; -*- */

/*
 * Copyright 2012, Old Dominion University
 * Copyright 2012, University of North Carolina at Chapel Hill
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *    1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *    2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *    3. The name of the author may not be used to endorse or promote
 * products derived from this software without specific prior written
 * permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * M.C. Weigle, P. Adurthi, F. Hernandez-Campos, K. Jeffay, and F.D. Smith,
 * Tmix: A Tool for Generating Realistic Application Workloads in ns-2,
 * ACM Computer Communication Review, July 2006, Vol 36, No 3, pp. 67-76.
 *
 * Contact: Michele Weigle (mweigle@cs.odu.edu)
 * http://www.cs.odu.edu/inets/Tmix
 */

#ifndef DELAYBOXNETDEVICE_H_
#define DELAYBOXNETDEVICE_H_

#include "ns3/point-to-point-channel.h"
#include "ns3/point-to-point-net-device.h"
#include "ns3/log.h"
#include "ns3/node.h"
#include "ns3/error-model.h"
#include "ns3/packet.h"
#include "ns3/pointer.h"
#include "ns3/net-device.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/queue.h"
#include "ns3/core-module.h"
#include "delaybox.h"

namespace ns3 {

/**
 * \ingroup devices
 * \defgroup delaybox DelayBox
 *
 * \brief a net device that delays packets according to per-flow random variables
 *
 * DelayBox is a NetDevice that should be placed on a node in between
 * the source and destination nodes.  With Delaybox, packets from a
 * TCP flow can be delayed, dropped, and/or forced through a
 * bottleneck link before being passed on to the next node. A
 * RandomVariable distribution can be used to specify delay, loss,
 * and/or bottleneck link speed for a source-destination pair. Each
 * flow between that source-destination pair draws from the
 * distribution to determine its characteristics.  Delays in DelayBox
 * are randomized per-flow, rather than per-packet.
 *
 * The DelayBoxNetDeviceT template class allows adding DelayBox
 * behavior to any existing implementation of
 * NetDevice. DelayBoxPointToPointNetDevice and
 * DelayBoxSimpleNetDevice have already been defined.
 */

/**
 * \class DelayBoxNetDeviceT
 * \brief Delays sending packets according to DelayBox rules. Abstract base class/mixin.
 *
 * When installed on a node and pointed to an instance of DelayBox,
 * this NetDevice delays sending packets according to the DelayBox's
 * rules and flow table. This is in addition to any further delays,
 * bottleneck link speeds, or loss rates that may be imposed
 * afterwards by the underlying NetDevice and/or underlying Channel.
 *
 * This template class makes it simple to apply the DelayBox behavior
 * to any implementation of NetDevice. DelayBoxPointToPointNetDevice
 * and DelayBoxSimpleNetDevice are already defined (for
 * PointToPointNetDevice and SimpleNetDevice, respectively) as
 * typedefs in the delaybox-net-device.h header file, and instantiated
 * in delaybox-net-device.cc.  When creating a new typedef of
 * DelayBoxNetDeviceT<>, don't forget to also use the
 * NS_OBJECT_ENSURE_REGISTERED macro on it.
 *
 * \tparam ND Any concrete implementation of NetDevice:
 * PointToPointNetDevice, SimpleNetDevice, etc.
 * \see DelayBox
 * \see DelayBoxSimpleNetDevice
 * \see DelayBoxPointToPointNetDevice
 */
template<class ND>
class DelayBoxNetDeviceT : public ND
{
public:
  static TypeId
  GetTypeId ();

  /**
   * Create a new DelayBoxNetDevice without an attached instance of
   * DelayBox.  Until this DelayBoxNetDevice is attached to a
   * configured DelayBox with SetDelayBox(), all packets will be
   * sent without delay.
   */
  DelayBoxNetDeviceT ();


  virtual
  ~DelayBoxNetDeviceT ();

  /**
   * Send a packet, delaying it according to the rules set up in the given DelayBox.
   *
   * \param packet A packet with IPv4 and TCP headers on top,
   * suitable for examination by DelayBox.
   * \param dest mac address of the destination (already resolved)
   * \return If there is an instance of DelayBox attached, true if
   *         the packet was scheduled to be enqueued by the
   *         underlying NetDevice and false if it was dropped due to
   *         DelayBox rules.  If there is no instance of DelayBox
   *         attached, acts like PointToPointNetDevice::Send.
   */
  virtual bool
  Send (Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber);
  uint32_t m_inwaiting;
  uint32_t m_inwaitingp;
  /**
   * \return Instance of DelayBox used to classify and delay packets.
   */
  Ptr<DelayBox>
  GetDelayBox ()
  {
    return m_delayBox;
  }

  /**
   * \param delayBox Instance of DelayBox to use to classify and
   * delay packets, or null to detach.
   */
  void
  SetDelayBox (Ptr<DelayBox> delayBox)
  {
    m_delayBox = delayBox;
  }

private:
  void
  SendWithoutDelay (Ptr<Packet> packet, const Address& dest,
                    uint16_t protocolNumber)
  {
    m_inwaiting -= packet->GetSize ();
    m_inwaitingp --;
      Ptr<NetDeviceQueue> txq;
      if (this->ND::GetQueueInterface ())
        {
          txq = this->ND::GetQueueInterface ()->GetTxQueue (0);
          txq -> Start ();
        }
    this->ND::Send (packet, dest, protocolNumber);
  }

private:
  Ptr<DelayBox> m_delayBox;
  TracedCallback<Ptr<const Packet> > m_delayBoxEnqueueTrace;
  TracedCallback<Ptr<const Packet> > m_delayBoxDropTrace;
  Ptr<NetDeviceQueueInterface> m_queueInterface;   //!< NetDevice queue interface
};


/**
 * \brief PointToPointNetDevice which additionally delays packets
 * according to DelayBox rules.
 *
 * Note that PointToPointNetDevice has a DataRate attribute which is
 * set to a rather low value by default, and may impose unexpected
 * additional delays. It is also necessary to use
 * PointToPointNetDevice::SetQueue() to specify the type of packet
 * queue to use (for example DropTailQueue).
 *
 */
typedef DelayBoxNetDeviceT<PointToPointNetDevice> DelayBoxPointToPointNetDevice;


}

#endif /* DELAYBOXNETDEVICE_H_ */
