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

#ifndef TMIX_HELPER_H_
#define TMIX_HELPER_H_

#include "tmix.h"

namespace ns3 {

/**
 * \brief Sets up a unidirectional pair of Tmix applications.
 *
 * This class allows you to more easily set up the Tmix application
 * correctly on both the initiator and the acceptor node.  Normally
 * you'll need to use TmixHelper twice, to set up both
 * initiator-acceptor pairs as in the following diagram (showing a
 * total of six nodes).
 *
 * \code
 * INITIATOR1 --\                  /-- ACCEPTOR1
 *           ROUTER1 ---LINK--- ROUTER2
 *  ACCEPTOR2 --/                  \-- INITIATOR2
 * \endcode
 *
 * Thus the INITIATOR1-ACCEPTOR1 pair will use one set of connection
 * vectors, and the ACCEPTOR2-INITIATOR2 pair will use a different
 * set.
 *
 * \see TmixTopology for an easier way to set this up.
 */
class TmixHelper : public SimpleRefCount<TmixHelper>
{
public:
  /**
   * If this constructor is used, you must ensure that both nodes are
   * using some DelayBoxNetDeviceT, with this same single instance of
   * DelayBox attached to both of them. Don't add any rules yourself;
   * this instance of DelayBox will be managed by the Tmix
   * applications.
   *
   * \param delayBox The instance of DelayBox shared among the Tmix applications.
   */
  TmixHelper (Ptr<DelayBox> delayBox, Ptr<Node> initiator,
              Ipv4Address initiatorAddress, Ptr<Node> acceptor,
              Ipv4Address acceptorAddress);

  /**
   * If set to true, the loss rate on all connection vectors processed
   * through this helper is set to zero before they are executed.
   *
   * The default value is false, meaning to honor the loss rate given
   * in the connection vector.
   */
  void
  SetLossless (bool lossless);

  /**
   * Add and schedule one connection vector for execution at its startTime.
   */
  void
  AddConnectionVector (const Tmix::ConnectionVector& cvec);

  /**
   * Parse all connection vectors from the given file and schedule them.
   * \return the number of connection vectors added.
   */
  unsigned
  AddConnectionVectors (const std::string& filename);

  /**
   * Parse all connection vectors from the given input stream and schedule them.
   * \return the number of connection vectors added.
   */
  unsigned
  AddConnectionVectors (std::istream& in);

  /**
   * Parse all connection vectors from the given input stream and schedule
   * a certain fraction of them, chosen at random. The ratio argument should
   * be between zero and one inclusive.
   * \return the number of connection vectors added.
   */
  unsigned
  AddConnectionVectors (std::istream& in, double ratio);

  /**
   * \attention Not yet implemented.
   *
   * Parse connection vectors one at a time, waiting for the simulation to catch up
   * after each one is scheduled. This function will schedule parsing of the first
   * connection vector for simulation time 0. It expects that connection vectors
   * will arrive at the input stream sorted in ascending order by start time and will
   * abort the simulation if they are not.
   *
   * Adding connection vectors in this way may help reduce memory usage and startup time
   * for exceptionally large sets of connection vectors.
   *
   * TODO: NOT YET IMPLEMENTED.
   */
  void
  AddConnectionVectorsLazy (std::istream& in)
  {
    NS_FATAL_ERROR ("Not implemented.");
  }

  void
  SetCvecCompleteCallback (const Callback<void, Tmix::ConnectionVector>& callback)
  {
    m_notifyCvecComplete = callback;
  }

private:
  void
  StartConnectionVector (const Tmix::ConnectionVector& cvec);

private:
  Ptr<UniformRandomVariable> m_rng;                     //HJB
  bool m_ignoreLossRate;

  Callback<void, Tmix::ConnectionVector> m_notifyCvecComplete;

  Ptr<Tmix::Application> m_initiator;
  Ptr<Tmix::Application> m_acceptor;

  class PortAllocator : public SimpleRefCount<PortAllocator>
  {
public:
    PortAllocator ();

    uint16_t
    AllocatePort ();

    void
    DeallocatePort (uint16_t port);

private:
    /// Min-heap of available ports. Starts out empty and grows as needed.
    std::vector<uint16_t> m_availablePorts;
    /// Next port number to add if m_availablePorts is empty.
    int m_topPort;
  };

  Ptr<PortAllocator> m_portAllocator;
};

}

#endif
