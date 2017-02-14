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


#include "delaybox-net-device.h"

namespace ns3 {

template<class ND>
TypeId
DelayBoxNetDeviceT<ND>::GetTypeId ()
{
  static TypeId
    tid = TypeId (("ns3::DelayBoxNetDeviceT<" + ND::GetTypeId ().GetName ()
                   + ">").c_str ()) //
    .SetParent<ND> ()         //
    .AddAttribute ("DelayBox",
                   "The configured DelayBox instance used to delay packets",
                   PointerValue (), MakePointerAccessor (
                     &DelayBoxNetDeviceT<ND>::m_delayBox), MakePointerChecker<
                     DelayBox> ()) //
    .AddTraceSource (
      "DelayBoxEnqueue",
      "Trace source activated immediately when a packet is enqueued to be sent from this device (either this or DelayBoxDrop will be called for each packet sent)",
      MakeTraceSourceAccessor (
        &DelayBoxNetDeviceT<ND>::m_delayBoxEnqueueTrace))             //
    .AddTraceSource (
      "DelayBoxDrop",
      "Trace source indicating a packet has been dropped due to the DelayBox rule (either this or DelayBoxEnqueue will be called for each packet sent)",
      MakeTraceSourceAccessor (
        &DelayBoxNetDeviceT<ND>::m_delayBoxDropTrace))             //
  ;
  return tid;
}

template<class ND>
DelayBoxNetDeviceT<ND>::DelayBoxNetDeviceT ()
  : m_delayBox (CreateObject<DelayBox> ())
{
  m_inwaiting = 0;
}

template<class ND>
DelayBoxNetDeviceT<ND>::~DelayBoxNetDeviceT ()
{
}

template<class ND>
bool
DelayBoxNetDeviceT<ND>::Send (Ptr<Packet> packet, const Address& dest,
                              uint16_t protocolNumber)
{
  return this->ND::Send (packet, dest, protocolNumber);
  if (m_delayBox)
    {
      m_inwaiting += packet->GetSize ();
      m_inwaitingp ++;
      Ptr<Queue> qu = this->ND::GetQueue ();
      Ptr<NetDeviceQueue> txq;
      if (this->ND::GetQueueInterface ())
        {
          txq = this->ND::GetQueueInterface ()->GetTxQueue (0);
        }
      Callback<void> send = MakeCallback (
          &DelayBoxNetDeviceT<ND>::SendWithoutDelay, this).Bind (packet).Bind (
          dest).Bind (protocolNumber);
      if (m_delayBox->Delay (send, static_cast<Ptr<const Packet> > (packet)))
        {
          m_delayBoxEnqueueTrace (packet);
     if (txq)
        {
          if ((qu->GetMode () == Queue::QUEUE_MODE_PACKETS &&
               m_inwaitingp + qu->GetNPackets () >= qu->GetMaxPackets ()) ||
              (qu->GetMode () == Queue::QUEUE_MODE_BYTES &&
               m_inwaiting + qu->GetNBytes () + this->ND::GetMtu () > qu->GetMaxBytes ()))
            {
              txq->Stop ();
            }
        }
          return true;
        }
      else
        {
          m_delayBoxDropTrace (packet);
          return false;
        }
    }
  else
    {
      m_delayBoxEnqueueTrace (packet);
      return this->ND::Send (packet, dest, protocolNumber);
    }
}

/////////////////////////////////////////////////////////
//////////////   Instantiations below    ////////////////
/////////////////////////////////////////////////////////

NS_OBJECT_ENSURE_REGISTERED (DelayBoxPointToPointNetDevice);
NS_LOG_COMPONENT_DEFINE ("DelayBoxPointToPointNetDevice");
template class DelayBoxNetDeviceT<PointToPointNetDevice>;


}
