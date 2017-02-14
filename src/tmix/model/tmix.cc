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

#include "ns3/socket.h"
#include "ns3/tcp-socket-factory.h"
#include "ns3/uinteger.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/inet-socket-address.h"
#include "ns3/packet.h"
#include "ns3/delaybox-net-device.h"
#include "ns3/tcp-socket.h"

#include "tmix.h"

#include <limits>
#include <deque>

NS_LOG_COMPONENT_DEFINE ("Tmix");

namespace ns3 {
namespace Tmix {

Application::Application (Ptr<DelayBox> delayBox, ADU::Side side,
                          const Ipv4Address& localAddress, const Ipv4Address& peerAddress)
  : m_side (side),
    m_packetSize (1448),
    m_localAddress (localAddress),
    m_peerAddress (peerAddress),
    m_delayBox (delayBox)
{
  //std::cout << m_localAddress << "\t" << m_peerAddress << "\n";
}

Application::~Application ()
{
}

TypeId
Application::GetTypeId ()
{
  NS_LOG_FUNCTION_NOARGS();
  static TypeId tid = TypeId ("ns3::Tmix::Application").SetParent<
      ns3::Application> ();
  return tid;
}

bool
ADU::operator== (const ADU& rhs) const
{
  NS_LOG_FUNCTION_NOARGS();
  return (size == rhs.size) && (side == rhs.side) && (recvWaitTime
                                                      == rhs.recvWaitTime) && (sendWaitTime == rhs.sendWaitTime);
}
bool
ADU::operator!= (const ADU& rhs) const
{
  NS_LOG_FUNCTION_NOARGS();
  return !(*this == rhs);
}

/**
 * Abstract base class for Initiator and Acceptor which factors out the common logic for
 * keeping track of what bytes must be sent, what bytes must be received, and when.
 */
class Worker : public Object
{
public:
  virtual void
  Start () = 0;

  Worker (Ptr<Socket> socket, unsigned packetSize, const ConnectionVector& cvec,
          ADU::Side side, Callback<void, ConnectionVector> notifyCvecComplete)
    : m_cvec (cvec),
      m_socket (socket),
      m_packetSize (packetSize),
      m_lastSendTime (
        Seconds (-1)),
      m_lastRecvTime (Seconds (-1)),
      m_closed (false),
      m_notifyCvecComplete (notifyCvecComplete)
  {
    NS_LOG_FUNCTION_NOARGS();
    NS_LOG_FUNCTION (this << socket << cvec);
    for (std::vector<ADU>::const_iterator i = cvec.adus.begin (); i
         != cvec.adus.end (); ++i)
      {
        if (i->side == side)
          {
            m_adusToSend.push_back (*i);
          }
        else
          {
            m_adusToRecv.push_back (*i);
          }
      }
    // Check that the cvec was OK
    for (std::deque<ADU>::const_iterator i = m_adusToSend.begin (); i
         != m_adusToSend.end (); ++i)
      {
        if (i->size == 0)
          {
            if ((i + 1) != m_adusToSend.end ())
              {
                NS_LOG_WARN ("Bad cvec: there can be only one ADU with zero size per side and it must be the last one\n" << cvec);
              }
          }
      }
    for (std::deque<ADU>::const_iterator i = m_adusToRecv.begin (); i
         != m_adusToRecv.end (); ++i)
      {
        if (i->size == 0)
          {
            if ((i + 1) != m_adusToRecv.end ())
              {
                NS_LOG_WARN ("Bad cvec: there can be only one ADU with zero size per side and it must be the last one\n" << cvec);
              }
          }
      }
  }

  void
  Receive (Ptr<Socket> socket)
  {
    NS_LOG_FUNCTION_NOARGS();
    NS_LOG_FUNCTION (this << socket);
    Ptr<Packet> packet = socket->Recv ();
    if (packet == NULL)
      {
        // This means we have already half-closed our socket, and we
        // just received a FIN; therefore the connection is now fully
        // closed.  We shouldn't have any more ADUs to send.
        NS_ASSERT (m_adusToSend.empty ()); //
        // We should either be out of ADUs to receive, or we should
        // have just the explicit FIN ADU remaining.
        if (m_adusToRecv.empty () || (m_adusToRecv.size () == 1
                                      && m_adusToRecv.front ().size == 0))
          {
            NS_LOG_LOGIC ("Received FIN; Connection vector completed.");
            if (!m_notifyCvecComplete.IsNull ())
              {
                m_notifyCvecComplete (m_cvec);
              }
          }
        else
          {
            // This used to happen a lot (spuriously) due to a bug in
            // the old ns-3 TCP implementation. Happily that problem
            // has been solved, and we can now be sure that this
            // condition indicates an actual problem.
            NS_LOG_WARN ("Received FIN, but there are still ADUs to receive (connected to wrong application?). Next ADU to receive: " << m_adusToRecv.front ());
          }
        CloseIfDone (socket);
        NS_ASSERT (m_closed);
        return;
      }

    unsigned bytesToRecv = packet->GetSize ();
    // Slurp ADUs off of m_adusToRecv until all received bytes have been used up.
    while (bytesToRecv)
      {
        if (m_adusToRecv.empty () || m_adusToRecv.front ().size == 0)
          {
            NS_LOG_WARN ("Expected to receive FIN, but received " << bytesToRecv << " extra bytes instead. Cvec: " << m_cvec);
            break;
          }
        else if (m_adusToRecv.front ().size > bytesToRecv)
          {
            // Received less than a whole ADU. Reduce its size by the
            // number of bytes received.
            m_adusToRecv.front ().size -= bytesToRecv;
            NS_LOG_LOGIC ("Received " << bytesToRecv << " bytes of an ADU. " << m_adusToRecv.front ().size << " bytes to go.");
            break;
          }
       // else
         // {
            // front ADU size <= bytesToRecv: remove the whole thing and continue receiving
            bytesToRecv -= m_adusToRecv.front ().size;
            m_adusToRecv.pop_front ();
            NS_LOG_LOGIC ("Finished receiving an ADU. " << m_adusToRecv.size () << " to go.");
            m_lastRecvTime = Simulator::Now ();
            // Successful receipt of an ADU might trigger a send.
            if (!m_adusToSend.empty ()
                && m_adusToSend.front ().recvWaitTime.IsStrictlyPositive ())
              {
                ScheduleNextSend (socket);
              }
            continue;
          //}
      }
    // FIXME: Is it necessary to call CloseIfDone() here? Would it hurt?
  }

  /**
   * \return true if the socket was closed.
   */
  bool
  CloseIfDone (Ptr<Socket> socket)
  {
    NS_LOG_FUNCTION_NOARGS();
    if (m_adusToSend.empty () || (m_adusToSend.size () == 1
                                  && m_adusToSend.front ().size == 0))
      {
        // No more ADUs to send.
        if (m_adusToRecv.empty () || (m_adusToRecv.size () == 1
                                      && m_adusToRecv.front ().size == 0))
          {
            // No more ADUs to receive.
            NS_LOG_LOGIC ("Closing socket " << socket);
            m_closed = true;
            socket->Close ();
          }
        else
          {
            NS_LOG_LOGIC (m_adusToRecv.size () << " ADUs to receive.");
          }
      }
    else
      {
        NS_LOG_LOGIC (m_adusToSend.size () << " ADUs to send.");
      }
    NS_LOG_LOGIC ("CloseIfDone() is about to return");
    return m_closed;
  }

  virtual void
  Closed (Ptr<Socket> socket)
  {
    NS_LOG_FUNCTION_NOARGS();
    NS_LOG_FUNCTION (this << socket);

    if (!m_closed)
      {
        CloseIfDone (socket);
      }
  }

protected:
  void
  ScheduleNextSend (Ptr<Socket> socket)
  {
    NS_LOG_FUNCTION_NOARGS();
    NS_LOG_FUNCTION (this << socket);
    // It's expected that this won't be called if m_adusToSend is
    // empty, since the last ADU should have size==0, indicating FIN
    NS_ASSERT (!m_adusToSend.empty ());
    // Either sendWaitTime, or recvWaitTime, or both are always zero.
    if (m_adusToSend.front ().sendWaitTime.IsStrictlyPositive ())
      {
        if (m_lastSendTime < Seconds (0))
          {
            NS_LOG_WARN ("send_wait: haven't sent anything yet. connection vector corrupted or wrong?");
          }
        else
          {
            // (m_lastSendTime + adu.sendWaitTime) is the earliest
            // time at which the data may be sent.
            Time sendTime = m_lastSendTime + m_adusToSend.front ().sendWaitTime;
            if (sendTime >= Simulator::Now ())
              {
                NS_LOG_LOGIC ("send_wait: Scheduling DoSend at " << sendTime.GetSeconds () << "s");
                Simulator::Schedule (sendTime - Simulator::Now (),
                                     &Worker::DoSend, this, socket);
              }
            else
              {
                NS_LOG_LOGIC ("send_wait: Scheduling DoSend immediately (was overdue by " << (Simulator::Now () - sendTime).GetSeconds () << "s");
                Simulator::ScheduleNow (&Worker::DoSend, this, socket);
              }
          }
      }
    else if (m_adusToSend.front ().recvWaitTime.IsStrictlyPositive ())
      {
        if (m_lastRecvTime < Seconds (0))
          {
            NS_LOG_LOGIC ("recv_wait: haven't received yet; keep waiting.");
          }
        else
          {
            // (m_lastRecvTime + adu.recvWaitTime) is the earliest
            // time at which the data may be sent.
            Time sendTime = m_lastRecvTime + m_adusToSend.front ().recvWaitTime;
            if (sendTime >= Simulator::Now ())
              {
                NS_LOG_LOGIC ("recv_wait: Scheduling DoSend at " << sendTime.GetSeconds () << "s");
                Simulator::Schedule (sendTime - Simulator::Now (),
                                     &Worker::DoSend, this, socket);
              }
            else
              {
                NS_LOG_LOGIC ("recv_wait: Scheduling DoSend immediately (was overdue by " << (Simulator::Now () - sendTime) << ")");
                Simulator::ScheduleNow (&Worker::DoSend, this, socket);
              }
            // Very important: clear m_lastRecvTime now that the
            // response to this packet has been scheduled.  This
            // prevents us from skipping immediately all the way down
            // a chain of recv_wait ADUs.
            m_lastRecvTime = Seconds (-1);
          }
      }
    else
      {
        NS_LOG_LOGIC ("neither recv_wait nor send_wait: calling DoSend");
        // When both the recv_wait and send_wait times are zero, it
        // means to send those bytes immediately.
        DoSend (socket);
      }
  }

  /**
   * Performs the actual send of data through the socket.
   * May be scheduled or called directly from ScheduleNextSend.
   */
  void
  DoSend (Ptr<Socket> socket)
  {
    NS_LOG_FUNCTION_NOARGS();
    if (m_adusToSend.empty ())
      {
        // This happens sometimes when 2 ADUs are received in a row
        // and both trigger a DoSend scheduling via recv_wait.  It can
        // be safely ignored.
        return;
      }
    ADU adu = m_adusToSend.front ();
    m_adusToSend.pop_front ();
    NS_LOG_FUNCTION (this << socket << adu);

    if (adu.size == 0)
      {
        NS_ASSERT_MSG (m_adusToSend.empty (), "Tmix::Worker::DoSend(): Non-zero-length ADU follows zero-length ADU: " << '\n' << adu << '\n' << m_adusToSend.front () << '\n');
        // Zero size indicates it's time to send a FIN, closing the
        // connection from this side.  Can't do that if we still have
        // ADUs to receive, though, so don't close unless we're done.
        CloseIfDone (socket);
        NS_LOG_LOGIC ("Returned to DoSend");
        // In case the socket wasn't closed just now, it means that it
        // will be the other side's job to initiate the close after it
        // has finished sending its remaining ADUs to us.
      }
    else
      {
        // Stuff the whole ADU into the send buffer, one packet at a time.
        unsigned bytesToSend = adu.size;
        unsigned bytesQueued = 0;
        while (bytesToSend)
          {
            int bytesAccepted;
            if (bytesToSend > m_packetSize)
              {
                bytesAccepted = socket->Send (Create<Packet> (m_packetSize));
              }
            else
              {
                bytesAccepted = socket->Send (Create<Packet> (bytesToSend));
              }
            if (bytesAccepted > 0)
              {
                bytesToSend -= bytesAccepted;
                bytesQueued += bytesAccepted;
              }
            else if (socket->GetErrno () == TcpSocket::ERROR_MSGSIZE)
              {
                // TODO: instead of trying to load the whole thing
                // into the send buffer all at once, register a send
                // callback and finish the job asynchronously
                NS_LOG_WARN ("Send buffer filled while sending ADU " << adu << " (" << bytesQueued << " bytes already queued). Increase TcpSocket's SndBufSize.");
                break;
              }
            else if (socket->GetErrno () == TcpSocket::ERROR_NOTCONN)
              {
                // This means that the socket was already closed by us for sending, somehow.
                NS_LOG_WARN ("ERROR_NOTCONN while sending - this is a bug");
                break;
              }
            else
              {
                NS_LOG_WARN ("Error sending " << bytesToSend << " bytes of data (" << bytesQueued << " bytes already queued). TcpSocket errno == " << socket->GetErrno ());
                break;
              }
          }

        m_lastSendTime = Simulator::Now ();

        if (m_adusToSend.empty ())
          {
            // Technically, all connection vectors should be
            // terminated with a zero-length ADU indicating the time
            // at which the connection is closed.  However, in this
            // case it is missing and we must insert it, making that
            // assumption that the connection should be closed
            // immediately.
            NS_LOG_LOGIC ("No more ADUs to send and no explicit closing ADU (size=0). Queueing a close immediately.");

            ADU terminal;
            terminal.recvWaitTime = Seconds (0);
            terminal.sendWaitTime = Seconds (0);
            terminal.side = m_side;
            terminal.size = 0;

            m_adusToSend.push_back (terminal);
          }

        ScheduleNextSend (socket);

      } // end else if adu.size != 0
    NS_LOG_LOGIC ("Ending DoSend");
  }

protected:
  ADU::Side m_side;
  ConnectionVector m_cvec;
  Ptr<Socket> m_socket;
  unsigned m_packetSize;
  std::deque<ADU> m_adusToSend;
  std::deque<ADU> m_adusToRecv;
  Time m_lastSendTime;
  Time m_lastRecvTime;
  bool m_closed;
  Callback<void, ConnectionVector> m_notifyCvecComplete;
};

/**
 * Here Worker::m_socket is used to listen for and accept one incoming
 * connection only; once a connection is established the listen socket
 * is closed and control is handed to Worker.
 */
class Acceptor : public Worker
{
public:
  Acceptor (Ptr<Socket> socket, unsigned packetSize,
            const ConnectionVector& cvec, const InetSocketAddress& localAddress,
            Callback<void, ConnectionVector> notifyCvecComplete,
            Callback<void> deallocate)
    : Worker (socket, packetSize, cvec, ADU::ACCEPTOR, notifyCvecComplete),
      m_localAddress (
        localAddress),
      m_deallocate (deallocate)
  {
    NS_LOG_FUNCTION_NOARGS();
    NS_LOG_FUNCTION (this << socket);
    m_socket->SetAcceptCallback (
      MakeCallback (&Acceptor::ConnectionRequest, this), MakeCallback (
        &Acceptor::NewConnectionCreated, this));
    // Don't bother using SetConnectCallbacks -- we don't initiate connections.
  }

  virtual void
  Start ()
  {
    NS_LOG_FUNCTION_NOARGS();
    NS_LOG_FUNCTION (this << m_localAddress);
    NS_LOG_DEBUG ("In this start");
    if (m_socket->Bind (m_localAddress))
      {
        NS_LOG_WARN ("Acceptor::Start(): m_socket->Bind(" << m_localAddress << ") failed with errno " << m_socket->GetErrno ());
      }
    if (m_socket->Listen ())
      {
        NS_LOG_WARN ("Acceptor::Start(): m_socket->Listen() failed with errno " << m_socket->GetErrno ());
      }
  }

  virtual void
  Closed (Ptr<Socket> socket)
  {
    NS_LOG_FUNCTION_NOARGS();
    NS_LOG_FUNCTION (this << socket);

    Worker::Closed (socket);

    if (!m_closed)
      {
        CloseIfDone (socket);
        if (m_closed)
          {
            m_deallocate ();
          }
      }

    // Deallocate the port IF it has been fully closed by both sides,
    // and the socket's endpoint deallocated.  FIXME: Unfortunately
    // with the new TCP implementation, I haven't found an easy way to
    // tell when this has happened.  So for the time being, ports stay
    // allocated forever.  m_deallocate();
  }

  bool
  ConnectionRequest (Ptr<Socket> socket, const Address& address)
  {
    NS_LOG_FUNCTION_NOARGS();
    NS_LOG_FUNCTION (this << socket << address);
    // Always accept the connection.
    return true;
  }

  void
  NewConnectionCreated (Ptr<Socket> socket, const Address& address)
  {
    NS_LOG_FUNCTION_NOARGS();
    NS_LOG_FUNCTION (this << socket << address);
    socket->SetRecvCallback (MakeCallback (&Worker::Receive, this));
    socket->SetCloseCallbacks (MakeCallback (&Acceptor::Closed, this),
                               MakeNullCallback<void, Ptr<Socket> > ());
    // Begin sending, maybe.
    if (!m_adusToSend.empty ())
      {
        // The first ADU to send might be on recv_wait, so this won't
        // necessarily schedule anything.
        ScheduleNextSend (socket);
      }
    else
      {
        // Opened the connection, but have nothing to send. That's
        // fine, we might still have things to receive.
        NS_LOG_LOGIC ("Acceptor has no ADUs to send. Waiting.");
      }
    // Close the socket listening for new connections
    m_socket->Close ();
  }

private:
  InetSocketAddress m_localAddress;
  Callback<void> m_deallocate;
};

/**
 * Establishes a connection using Worker::m_socket, then starts the Worker proper.
 */
class Initiator : public Worker
{
public:
  Initiator (Ptr<Socket> socket, unsigned packetSize,
             const ConnectionVector& cvec, const InetSocketAddress& peerAddress,
             Callback<void, ConnectionVector> notifyCvecComplete)
    : Worker (socket, packetSize, cvec, ADU::INITIATOR, notifyCvecComplete),
      m_peerAddress (peerAddress)
  {
    NS_LOG_FUNCTION_NOARGS();
    NS_LOG_FUNCTION (this << socket);
    m_socket->SetConnectCallback (MakeCallback (&Initiator::ConnectionComplete,
                                                this), MakeCallback (&Initiator::ConnectionFailed, this));
    // Don't bother using SetAcceptCallback -- we don't listen for connections on this side.
  }

  virtual void
  Start ()
  {
    NS_LOG_FUNCTION_NOARGS();
   NS_LOG_DEBUG ("In that start");
    NS_LOG_FUNCTION (this);
    m_socket->Bind ();
    m_socket->Connect (m_peerAddress);
  }

  void
  ConnectionComplete (Ptr<Socket> socket)
  {
    NS_LOG_FUNCTION_NOARGS();
    NS_LOG_FUNCTION (this << socket);
    // Get ready to receive.
    socket->SetRecvCallback (MakeCallback (&Worker::Receive, this));
    socket->SetCloseCallbacks (MakeCallback (&Worker::Closed, this),
                               MakeNullCallback<void, Ptr<Socket> > ());
    // Begin sending, maybe.
    if (!m_adusToSend.empty ())
      {
        // The first ADU to send might be on recv_wait, so this won't
        // necessarily schedule anything.
        ScheduleNextSend (socket);
      }
    else
      {
        // Opened the connection, but have nothing to send. That's
        // fine, we might still have things to receive.
        NS_LOG_LOGIC ("Initiator has no ADUs to send. Waiting.");
      }
  }

  void
  ConnectionFailed (Ptr<Socket> socket)
  {
    NS_LOG_FUNCTION_NOARGS();
    NS_LOG_FUNCTION (this << socket);
    // Retry the connection I guess? This isn't really supposed to happen.
    NS_LOG_WARN ("Initiator failed to open connection. Retrying.");
    m_socket->Connect (m_peerAddress);
  }

private:
  InetSocketAddress m_peerAddress;
};

void
Application::StartConnectionVector (const ConnectionVector& cvec, uint16_t port,
                                    Callback<void, ConnectionVector> notifyCvecComplete,
                                    Callback<void> deallocatePort)
{
  NS_LOG_FUNCTION_NOARGS();
  Ptr<Socket> socket = Socket::CreateSocket (GetNode (),
                                             TcpSocketFactory::GetTypeId ());
  // FIXME: Increasing the buffer sizes like this is ugly. Fix the
  // Worker implementation so it is no longer necessary.  Note that
  // increasing this value beyond half of 2^32 will cause serious
  // problems (sequence numbers) in the current TCP implementation, so
  // don't do it.
  socket->SetAttribute ("SndBufSize", UintegerValue (2000000000));
  socket->SetAttribute ("RcvBufSize", UintegerValue (2000000000));
  socket->SetAttribute ("SegmentSize", UintegerValue (m_packetSize));
  // Disable symmetric mode on DelayBox because we need to specify a
  // different loss rate in each direction.  Since we only use
  // ConstantVariable random variables, and never set a bottleneck
  // link speed, this won't affect anything else.
  m_delayBox->SetSymmetric (false);
  Ptr<Worker> worker;
  if (m_side == ADU::INITIATOR)
    {
      socket->SetAttribute ("InitialCwnd", UintegerValue (
                              cvec.windowSizeInitiator));
      socket->SetAttribute ("SegmentSize", UintegerValue (
                              cvec.mssInitiator));
      m_delayBox->AddRule (m_localAddress, 0, m_peerAddress, port, DelayBoxRule (                 //HJB
                             (cvec.minRTT.GetSeconds () / 2.0), (
                               cvec.lossRateItoA), (0)));
      worker = CreateObject<Initiator> (socket, m_packetSize, cvec,
                                        InetSocketAddress (m_peerAddress, port), notifyCvecComplete);
    }
  else // m_side == ADU::ACCEPTOR
    {
      socket->SetAttribute ("InitialCwnd",
                            UintegerValue (cvec.windowSizeAcceptor));
      socket->SetAttribute ("SegmentSize", UintegerValue (
                              cvec.mssAcceptor));
      m_delayBox->AddRule (m_localAddress, port, m_peerAddress, 0, DelayBoxRule (                 //HJB
                             (cvec.minRTT.GetSeconds () / 2.0), (
                               cvec.lossRateAtoI), (0)));
      worker = CreateObject<Acceptor> (socket, m_packetSize, cvec,
                                       InetSocketAddress (m_localAddress, port), notifyCvecComplete,
                                       deallocatePort);
    }
  Simulator::ScheduleNow (MakeEvent (&Worker::Start, worker));
}

// TODO: this parser doesn't yet tolerate comments within the cvec
// header block or at the end of a line.
bool
ParseConnectionVector (std::istream &in, ConnectionVector& cvec)
{ 
  NS_LOG_FUNCTION_NOARGS();
  if (!in)
    {  
      return false;
    }
  in.setf (std::ios::skipws);
  char x;
  in >> x; 
  float startTimeMicroseconds;
  if (!in)
    { 
      return false;
    }
  if (x == 'S')
    {
      cvec.type = ConnectionVector::SEQUENTIAL;
      unsigned numExchanges;
      in >> startTimeMicroseconds >> numExchanges >> cvec.id1 >> cvec.id2;
    }
  else if (x == 'C')
    {
      cvec.type = ConnectionVector::CONCURRENT;
      unsigned numInitiatorAdus, numAcceptAdus;
      in >> startTimeMicroseconds >> numInitiatorAdus >> numAcceptAdus
      >> cvec.id1 >> cvec.id2;
    }
  else if (x == '#')
    {
      // Skip comment lines.
      in.ignore (std::numeric_limits<int>::max (), '\n');
      return ParseConnectionVector (in, cvec);
    }
  else
    {
      return false;
    }
   cvec.startTime = MicroSeconds (startTimeMicroseconds);
  in >> x; 
  if (x == 'm')
    {
      in >> cvec.mssInitiator >> cvec.mssAcceptor;
    }
  else
    {
      return false;
    }
  in >> x;
  if (x == 'w')
    {
      in >> cvec.windowSizeInitiator >> cvec.windowSizeAcceptor;
    }
  in >> x;
  if (x == 'r')
    {
      uint64_t rttMicroseconds;
      in >> rttMicroseconds;
      cvec.minRTT = MicroSeconds (rttMicroseconds);
    }
  else
    {
      return false;
    }
  in >> x;
  if (x == 'l')
    {
      in >> cvec.lossRateItoA >> cvec.lossRateAtoI;
    }
  else
    {
      return false;
    }
  cvec.adus.clear ();
  while (in)
    {
      ADU adu;
      in >> x;
      if (x == 'I')
        {
          adu.side = ADU::INITIATOR;
        }
      else if (x == 'A')
        {
          adu.side = ADU::ACCEPTOR;
        }
      else if (x == 'S' || x == 'C')
        {
          in.putback (x);
          return true;
        }
      else if (x == '#')
        {
          // Skip comment lines.
          in.ignore (std::numeric_limits<int>::max (), '\n');
          continue;
        }
      else if (x == '\n')
        {
          // Skip blank lines
        }
      else if (in.eof ())
        {
          return true;
        }
      else
        {
          return false;
        }
      uint64_t sendWaitTimeMicroseconds;
      uint64_t recvWaitTimeMicroseconds;
      in >> sendWaitTimeMicroseconds >> recvWaitTimeMicroseconds >> adu.size;
      adu.sendWaitTime = MicroSeconds (sendWaitTimeMicroseconds);
      adu.recvWaitTime = MicroSeconds (recvWaitTimeMicroseconds);
      if (in)
        {
          cvec.adus.push_back (adu);
        }
    }
  return true;
}

void
ConnectionVector::DebugPrint (std::ostream& out) const
{
  NS_LOG_FUNCTION_NOARGS();
  if (type == SEQUENTIAL)
    {
      out << 'S' << ' ' << startTime.GetMicroSeconds () << ' '
          << "[num_exchanges]" << ' ' << id1 << ' ' << id2 << std::endl;
    }
  else
    {
      out << 'C' << ' ' << startTime.GetMicroSeconds () << ' ' << "[num_init]"
          << ' ' << "[num_accept]" << ' ' << id1 << ' ' << id2 << std::endl;
    }
  out << 'w' << ' ' << windowSizeInitiator << ' ' << windowSizeAcceptor
      << std::endl;
  out << 'r' << ' ' << minRTT.GetMicroSeconds () << std::endl;
  out << 'l' << ' ' << lossRateItoA << ' ' << lossRateAtoI << std::endl;
  for (std::vector<ADU>::const_iterator i = adus.begin (); i != adus.end (); ++i)
    {
      i->DebugPrint (out);
      out << std::endl;
    }
}

std::ostream&
operator<< (std::ostream& lhs, const ConnectionVector& rhs)
{
  NS_LOG_FUNCTION_NOARGS();
  rhs.DebugPrint (lhs);
  return lhs;
}

void
ADU::DebugPrint (std::ostream& out) const
{
  NS_LOG_FUNCTION_NOARGS();
  out << (side == INITIATOR ? 'I' : 'A') << ' '
  << sendWaitTime.GetMicroSeconds () << ' '
  << recvWaitTime.GetMicroSeconds () << ' ' << size;
}

std::ostream&
operator<< (std::ostream& lhs, const ADU& rhs)
{
  NS_LOG_FUNCTION_NOARGS();
  rhs.DebugPrint (lhs);
  return lhs;
}

}
}
