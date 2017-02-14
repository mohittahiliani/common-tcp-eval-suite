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

#include "ns3/log.h"
#include "ns3/tmix-helper.h"

NS_LOG_COMPONENT_DEFINE ("TmixHelper");

#include <fstream>
#include <algorithm>                            //HJB

namespace ns3 {

TmixHelper::TmixHelper (Ptr<DelayBox> delayBox, Ptr<Node> initiatorNode,
                        Ipv4Address initiatorAddress, Ptr<Node> acceptorNode,
                        Ipv4Address acceptorAddress)
  : m_ignoreLossRate (false),
    m_notifyCvecComplete (MakeNullCallback<void,
                                           Tmix::ConnectionVector> ())
{
  NS_LOG_FUNCTION (this << delayBox << initiatorAddress << acceptorAddress);
  m_portAllocator = Create<PortAllocator> ();
  m_initiator = CreateObject<Tmix::Application> (delayBox,
                                                 Tmix::ADU::INITIATOR, initiatorAddress, acceptorAddress);
  m_acceptor = CreateObject<Tmix::Application> (delayBox, Tmix::ADU::ACCEPTOR,
                                                acceptorAddress, initiatorAddress);
  initiatorNode->AddApplication (m_initiator);
  acceptorNode->AddApplication (m_acceptor);

}

void
TmixHelper::SetLossless (bool lossless)
{
  m_ignoreLossRate = lossless;
}

void
TmixHelper::AddConnectionVector (const Tmix::ConnectionVector& cvec)
{ 
  NS_LOG_FUNCTION (cvec.startTime);
  Simulator::Schedule (cvec.startTime, &TmixHelper::StartConnectionVector, this,
                       cvec);
}

void
TmixHelper::StartConnectionVector (const Tmix::ConnectionVector& cvecLossy)
{
  NS_LOG_FUNCTION (cvecLossy.startTime);

  uint16_t port = m_portAllocator->AllocatePort ();
  Callback<void> deallocate = MakeCallback (&PortAllocator::DeallocatePort,
                                            m_portAllocator).Bind (port);

  Tmix::ConnectionVector cvec (cvecLossy);
  if (m_ignoreLossRate)
    {
      cvec.lossRateAtoI = 0;
      cvec.lossRateItoA = 0;
    }
  m_acceptor->StartConnectionVector (cvec, port, m_notifyCvecComplete,
                                     deallocate);
  m_initiator->StartConnectionVector (cvec, port, m_notifyCvecComplete,
                                      deallocate);
}

unsigned
TmixHelper::AddConnectionVectors (const std::string& filename)
{
  std::ifstream in (filename.c_str ());
  return AddConnectionVectors (in);
}

unsigned
TmixHelper::AddConnectionVectors (std::istream& in)
{
  unsigned n = 0;
  Tmix::ConnectionVector cvec;
  while (Tmix::ParseConnectionVector (in, cvec))
    {
      AddConnectionVector (cvec);
      ++n;
    }
  return n;
}

unsigned
TmixHelper::AddConnectionVectors (std::istream& in, double ratio)
{
  unsigned n = 0;
  Tmix::ConnectionVector cvec;
  while (Tmix::ParseConnectionVector (in, cvec))
    {
      if (m_rng->GetValue (0., 1.) < ratio)              //HJB
        {
          AddConnectionVector (cvec);
          ++n;
        }
    }
  return n;
}

TmixHelper::PortAllocator::PortAllocator ()
  : m_topPort (1024)
{
}

uint16_t
TmixHelper::PortAllocator::AllocatePort ()
{
  if (!m_availablePorts.empty ())
    {
      uint16_t port = m_availablePorts.front ();
      std::pop_heap (m_availablePorts.begin (), m_availablePorts.end (),
                     std::greater<uint16_t> ());
      m_availablePorts.pop_back ();
      NS_LOG_LOGIC ("Port " << port << " is being reused.");
      return port;
    }
  else
    {
      // Did we use too many ports simultaneously?
      if (m_topPort == 65535)
        {
          NS_FATAL_ERROR ("Ran out of port numbers to assign. Too many concurrent connections?");
        } //
      NS_LOG_LOGIC ("Port " << m_topPort << " is newly assigned.");
      return m_topPort++;
    }
}

void
TmixHelper::PortAllocator::DeallocatePort (uint16_t port)
{
  NS_ASSERT_MSG (std::find (m_availablePorts.begin (), m_availablePorts.end (), port) == m_availablePorts.end (), "Port " << port << " deallocated twice.");
  m_availablePorts.push_back (port);
  std::push_heap (m_availablePorts.begin (), m_availablePorts.end (),
                  std::greater<uint16_t> ());
  NS_LOG_LOGIC ("Port " << port << " deallocated.");
}

}
