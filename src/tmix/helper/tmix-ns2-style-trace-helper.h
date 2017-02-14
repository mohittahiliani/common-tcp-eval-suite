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

#ifndef TMIXNS2STYLETRACEHELPER_H_
#define TMIXNS2STYLETRACEHELPER_H_

#include <iostream>
#include "ns3/delaybox.h"
#include "ns3/delaybox-net-device.h"
#include "ns3/tmix-topology.h"

namespace ns3 {
namespace Tmix {

/**
 * \brief Produces ns-2 style trace files for a TmixTopology.
 *
 * This class produces ns2-style Tmix trace files when its trace
 * accessors are connected to a standard Tmix setup, a la
 * TmixTopology.  This may be useful to extend the life of trace
 * analysis tools originally written for ns-2.
 *
 * After instantiating with an instance of TmixTopology, call
 * Install() only once to install the trace accessors. The ns-2 style
 * trace will then be written to the specified output stream while the
 * simulation is running.
 *
 * You must ensure that the instance of Ns2StyleTraceHelper is not
 * destroyed until the simulation is over.
 *
 *  - Initiator node IDs: 0, 3.
 *  - Acceptor node IDs: 1, 2.
 *  - Router node IDs: 4, 5.
 *
 * The output format is one line per event, with data columns
 * separated by whitespace.
 * - Column 1: Event. '+' means enqueued, '-' means dequeued, 'r'
 * means received, 'd' means dropped. Here enqueue and dequeue always
 * happen in pairs and at the same time; the DelayBox delay appears
 * between the dequeue time and the receive time.
 * - Column 2: The time in seconds when the event occurred.
 * - Column 3: Link-layer source node ID.
 * - Column 4: Link-layer destination node ID.
 * - Column 5: Packet name. Unused, always "ack" (in ns-2 one-way TCP
     mode it would be different).
 * - Column 6: Total packet size in bytes.
 * - Column 7: Flags. Unused, always "-------".
 * - Column 8: Integer flow ID. Currently unused and always set to zero. TODO: implement.
 * - Column 9: Source. Format is the node ID and the port number
 * separated by a period, e.g. "0.1". Warning: unlike in Tmix for
 * ns-2, the TCP source port no longer has any meaningful relationship
 * to anything.
 * - Column 10: Destination. Same format as source.
 * - Column 11: Packet sequence number.
 * - Column 12: Semi-unique packet ID. Warning: in ns-2, fragments of
 * the same packet would get separate IDs, but in ns-3 they no longer
 * do.
 * - Column 13: Ack number.
 * - Column 14: TCP flags as a hexadecimal value, e.g. "0xa" for PUSH|SYN.
 * - Column 15: Header length. Unused, always equal to 40.
 * - Column 16: Socket address length. Unused, always zero.
 */
class Ns2StyleTraceHelper
{
public:
  /**
   * \param tmix Instance of TmixTopology.
   * \param out Output stream to print the
   */
  Ns2StyleTraceHelper (Ptr<TmixTopology> tmix, std::ostream& out);

  /**
   * Writes a slightly informative block of comments to the output file.
   */
  void
  WriteInformativeHeader ();

  /**
   * Installs the traces on all node pairs in the topology.
   */
  void
  Install ();

private:
  enum TraceEvent
  {
    ENQUEUE, DROP, DEQUEUE, RECEIVE
  };

  Ptr<PointToPointNetDevice>
  GetDevice (TmixTopology::NodeType node);
  Ptr<PointToPointNetDevice>
  GetRouterDevice (TmixTopology::NodeType node);
  Ipv4Address
  GetAddress (TmixTopology::NodeType node);

  void
  Install (TmixTopology::NodeType start, TmixTopology::NodeType router1,
           TmixTopology::NodeType router2, TmixTopology::NodeType end);
  void
  Install (Ptr<PointToPointNetDevice> device, std::string trace,
           TraceEvent event, TmixTopology::NodeType llsrc,
           TmixTopology::NodeType lldst, Ipv4Address src, Ipv4Address dst);

  struct TraceSpec
  {
    TraceSpec (TraceEvent ev, TmixTopology::NodeType lls,
               TmixTopology::NodeType lld, Ipv4Address s, Ipv4Address d, uint16_t id)
      : event (ev),
        llsrc (lls),
        lldst (lld),
        src (s),
        dst (d),
        uniqid (id)
    {
    }

    TraceEvent event;
    TmixTopology::NodeType llsrc;
    TmixTopology::NodeType lldst;
    Ipv4Address src;
    Ipv4Address dst;
    uint16_t uniqid;

    bool
    operator!= (const TraceSpec& rhs) const
    {
      return event != rhs.event || llsrc != rhs.llsrc || lldst != rhs.lldst
             || src != rhs.src || dst != rhs.dst || uniqid != rhs.uniqid;
    }
  };

  void
  OutputTrace (TraceSpec ts, Ptr<const Packet> packet);

private:
  Ptr<TmixTopology> m_tmix;
  std::ostream& m_out;
  uint16_t m_uniqid;
  TmixTopology::TmixNodePair m_left;
  TmixTopology::TmixNodePair m_right;

  std::map<TmixTopology::NodeType, int> m_idOfNodeType;
};

}
}

#endif
