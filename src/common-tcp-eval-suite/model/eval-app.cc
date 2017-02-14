/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2017 NITK Surathkal
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Authors: Ankit Deepak <adadeepak8@gmail.com>
 *          Shravya Ks <shravya.ks0@gmail.com>
 *          Mohit P. Tahiliani <tahiliani@nitk.edu.in>
 */

#include "ns3/log.h"
#include "ns3/address.h"
#include "ns3/node.h"
#include "ns3/nstime.h"
#include "ns3/socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/tcp-socket-factory.h"
#include "eval-app.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("EvalApp");

NS_OBJECT_ENSURE_REGISTERED (EvalApp);

EvalApp::EvalApp ()
  : m_socket (0),
    m_peer (),
    m_sendSize (0),
    m_maxBytes (0),
    m_totBytes (0),
    m_connected (false)
{
}

EvalApp::~EvalApp ()
{
  m_socket = 0;
}

void
EvalApp::Setup (Ptr<Socket> socket, Address address, uint32_t packetSize, uint64_t maxByte)
{
  m_socket = socket;
  m_peer = address;
  m_sendSize = packetSize;
  m_maxBytes = maxByte;
}

void
EvalApp::StartApplication (void)
{
  if (Inet6SocketAddress::IsMatchingType (m_peer))
    {
      m_socket->Bind6 ();
    }
  else if (InetSocketAddress::IsMatchingType (m_peer))
    {
      m_socket->Bind ();
    }

  m_socket->Connect (m_peer);
  m_socket->ShutdownRecv ();
  m_socket->SetConnectCallback (
    MakeCallback (&EvalApp::ConnectionSucceeded, this),
    MakeCallback (&EvalApp::ConnectionFailed, this));
  m_socket->SetSendCallback (
    MakeCallback (&EvalApp::DataSend, this));
  if (m_connected)
    {
      m_flowStart = Simulator::Now ();
      SendData ();
    }
}

void
EvalApp::StopApplication (void)
{
  if (m_socket != 0)
    {
      m_flowStop = Simulator::Now ();
      m_socket->Close ();
      m_connected = false;
    }
  else
    {
      NS_LOG_WARN ("Application found null socket to close in StopApplication");
    }
}

void EvalApp::SendData (void)
{
  NS_LOG_FUNCTION (this);

  while (m_maxBytes == 0 || m_totBytes < m_maxBytes)
    { // Time to send more

      // uint64_t to allow the comparison later.
      // the result is in a uint32_t range anyway, because
      // m_sendSize is uint32_t.
      uint64_t toSend = m_sendSize;
      // Make sure we don't send too many
      if (m_maxBytes > 0)
        {
          toSend = std::min (toSend, m_maxBytes - m_totBytes);
        }

      NS_LOG_LOGIC ("sending packet at " << Simulator::Now ());
      Ptr<Packet> packet = Create<Packet> (toSend);

      int actual = m_socket->Send (packet);
      if (actual > 0)
        {
          m_totBytes += actual;
        }
      // We exit this loop when actual < toSend as the send side
      // buffer is full. The "DataSent" callback will pop when
      // some buffer space has freed ip.
      if ((unsigned)actual != toSend)
        {
          break;
        }
    }
  // Check if time to close (all sent)
  if (m_totBytes == m_maxBytes && m_connected)
    {
      m_flowStop = Simulator::Now ();
      m_socket->Close ();
      m_connected = false;
    }
}

Time EvalApp::GetFlowCompletionTime()
{
  return m_flowStop - m_flowStart;
}

void EvalApp::ConnectionSucceeded (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  NS_LOG_LOGIC ("Application Connection succeeded");
  m_connected = true;
  SendData ();
}

void EvalApp::ConnectionFailed (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  NS_LOG_LOGIC ("Application, Connection Failed");
}

void EvalApp::DataSend (Ptr<Socket>, uint32_t)
{
  NS_LOG_FUNCTION (this);

  if (m_connected)
    { // Only send new data if the connection has completed
      SendData ();
    }
}
}
