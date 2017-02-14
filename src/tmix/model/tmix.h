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

#ifndef TMIX_H_
#define TMIX_H_

#include "ns3/nstime.h"
#include "ns3/application.h"
#include "ns3/delaybox.h"

#include <iostream>
#include <vector>

namespace ns3 {
namespace Tmix {

/**
 * \brief Application Data Unit
 *
 * Each ADU represents data to be sent over the network by either the initiator or the
 * acceptor of the TCP connection.  ADUs also specify when data should be
 * sent relative to either the reception of an ADU or the completed
 * sending of a previous ADU.
 */
struct ADU
{
  typedef enum
  {
    INITIATOR, ACCEPTOR
  } Side;

  /// INITIATOR or ACCEPTOR.
  Side side;
  /// The amount of time the initiator/acceptor has to wait before
  /// sending this ADU after sending its previous ADU.
  Time sendWaitTime;
  /// The amount of time the initiator/acceptor has to wait before
  /// sending this ADU after receiving an ADU from the other side.
  Time recvWaitTime;
  /// Number of bytes to send.
  unsigned size;

  /// Print a human readable representation of this ADU to the specified output stream.
  void
  DebugPrint (std::ostream& out) const;

  bool
  operator== (const ADU& rhs) const;
  bool
  operator!= (const ADU& rhs) const;
};

/**
 * Overloaded stream operator to pretty-print ADUs.
 */
std::ostream&
operator<< (std::ostream& lhs, const ADU& rhs);

/**
 * A connection vector is a representation of an observed TCP connection.
 * Each connection vector has several fields which specify the type of
 * connection (sequential or concurrent), connection start time, loss
 * rate, window size, etc.  Each connection vector also contains an
 * arbitrary number of application data units (ADUs).
 *
 * \see ADU
 */
struct ConnectionVector
{
  typedef enum
  {
    SEQUENTIAL, CONCURRENT
  } Type;

  /// SEQUENTIAL or CONCURRENT.
  Type type;
  /// The absolute time at which this connection vector is scheduled to begin.
  Time startTime;
  /// Reserved.
  unsigned id1;
  /// Reserved.
  unsigned id2;
  /// Initiator's window size.
  unsigned windowSizeInitiator;
  /// Acceptor's window size.
  unsigned windowSizeAcceptor;
  /// Minimum round-trip time (i.e. twice the minimum one-way delay).
  Time minRTT;
  /// Loss rate from the initiator to the acceptor.
  double lossRateItoA;
  /// Loss rate from the acceptor to the initiator.
  double lossRateAtoI;

  uint32_t mssInitiator;

  uint32_t mssAcceptor;

  /// Sequential list of ADUs.
  std::vector<ADU> adus;

  /// Print a human readable representation of this connection vector
  /// to the specified output stream.
  void
  DebugPrint (std::ostream&) const;
};

/**
 * Overloaded stream operator to pretty-print connection vectors.
 */
std::ostream&
operator<< (std::ostream& lhs, const ConnectionVector& rhs);

/**
 * Read a single connection vector in the ``new'' (ns-2) format from an input stream.
 * Call repeatedly to parse the whole file.
 *
 * \param in Stream to read from.
 * \param out ConnectionVector to write to.
 * \return true if successful, false if parsing failed or end of file was reached.
 */
bool
ParseConnectionVector (std::istream &in, ConnectionVector& out);

/**
 * \brief Implements the Tmix initiator and acceptor applications.
 *
 * It is difficult and probably unnecessary to use this class
 * directly; see TmixHelper or TmixTopology instead.
 *
 * \see TmixHelper
 * \see TmixTopology
 */
class Application : public ns3::Application
{
public:
  static TypeId
  GetTypeId (void);

  /**
   * \param delayBox An instance of DelayBox with no rules
   * configured. It must also be shared between the initiator and
   * acceptor application, otherwise Bad Things will happen.
   * \param side INITIATOR or ACCEPTOR.
   * \param localAddress Address of this application.
   * \param peerAddress Address of the corresponding
   * initiator (if this is an acceptor) or acceptor (if this is an
   * initiator).
   */
  Application (Ptr<DelayBox> delayBox, ADU::Side side,
               const Ipv4Address& localAddress, const Ipv4Address& peerAddress);
  ~Application ();

  /**
   * Schedule a connection vector for immediate execution (ignoring
   * its startTime).  You must specify the TCP port it will connect to
   * / listen on; make sure it won't be in use.
   * \param cvec Connection vector to execute immediately.
   * \param port TCP port to listen on or connect to, depending on
   * which side this application is on.
   * \param notifyCvecComplete Will be called when the connection
   * vector is nominally complete, but the port may still be in the
   * process of shutting down for some time.
   * \param deallocatePort Will be called when the connection vector
   * is complete and the port is no longer needed.
   */
  void
  StartConnectionVector (const ConnectionVector& cvec, uint16_t port,
                         Callback<void, ConnectionVector> notifyCvecComplete,
                         Callback<void> deallocatePort);

  /**
   * Maximum size of packets to send. Larger ADUs will be broken up
   * into chunks of no more than this many bytes.
   *
   * Default: 1460 bytes.
   */
  void
  SetPacketSize (unsigned packetSize)
  {
    m_packetSize = packetSize;
  }

private:
  /**
   * SetStartTime doesn't do anything; the application runs whenever
   * StartConnectionVector is called.
   */
  virtual void
  SetStartTime (Time)
  {
  }
  /**
   * SetStopTime doesn't do anything; the application stops whenever
   * it runs out of connection vectors to execute.
   */
  virtual void
  SetStopTime (Time)
  {
  }

private:
  /// Whether this app is handling the INITIATOR or the ACCEPTOR side.
  ADU::Side m_side;
  /// Maximum size of packets to send.
  unsigned m_packetSize;
  /// Address where this Tmix application is running.
  Ipv4Address m_localAddress;
  /// Address where the corresponding Tmix application is running.
  Ipv4Address m_peerAddress;
  /// DelayBox instance to be used for RTT and loss rate enforcement
  Ptr<DelayBox> m_delayBox;
};

}
}

#endif
