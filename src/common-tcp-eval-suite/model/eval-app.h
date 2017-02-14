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

#ifndef EVAL_APPLICATION_H
#define EVAL_APPLICATION_H

#include "ns3/address.h"
#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/traced-callback.h"

namespace ns3 {
class EvalApp : public Application
{
public:
  EvalApp ();
  virtual ~EvalApp ();

  void Setup (Ptr<Socket> socket, Address address, uint32_t packetSize, uint64_t maxByte);
 
  Time GetFlowCompletionTime();

private:
  virtual void StartApplication (void);
  virtual void StopApplication (void);

  /**
   * \brief Send data until the L4 transmission buffer is full.
   */
  void SendData ();

  /**
   * \brief Connection Succeeded (called by Socket through a callback)
   * \param socket the connected socket
   */
  void ConnectionSucceeded (Ptr<Socket> socket);
  /**
   * \brief Connection Failed (called by Socket through a callback)
   * \param socket the connected socket
   */
  void ConnectionFailed (Ptr<Socket> socket);
  /**
   * \brief Send more data as soon as some has been transmitted.
   */
  void DataSend (Ptr<Socket>, uint32_t); // for socket's SetSendCallback

  Ptr<Socket>     m_socket;
  Address         m_peer;
  uint32_t        m_sendSize;
  uint64_t        m_maxBytes;
  uint64_t        m_totBytes;
  bool            m_connected;
  Time            m_flowStart;
  Time            m_flowStop;
};
}
#endif
