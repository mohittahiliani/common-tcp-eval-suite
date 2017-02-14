/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef CREATE_TCP_EVAL_DUMBBELL_H
#define CREATE_TCP_EVAL_DUMBBELL_H

#include <string>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/traffic-control-module.h"

namespace ns3 {

class CreateDumbbell
{

public:

  CreateDumbbell(uint32_t num_of_nodes, std::string bot_delay, std::string bot_bw, std::string edge_bw,std::string edge_delay[], double bufferLimit);
 
  ~CreateDumbbell();

  void InstallStack(InternetStackHelper stack);

  void AssignIpv4Address(Ipv4AddressHelper leftIp, Ipv4AddressHelper rightIp, Ipv4AddressHelper routerIp);
  
  Ptr<Node> GetLeftRouter () const;

  Ptr<Node> GetLeft (uint32_t i) const;


  Ptr<Node> GetRight () const;

  Ptr<Node> GetRight (uint32_t i) const;

  Ipv4Address GetLeftIpv4Address (uint32_t i) const;

  Ipv4Address GetRightIpv4Address (uint32_t i) const;

private:
    
    uint32_t num_nodes;
    NodeContainer leftNodes;
    NodeContainer rightNodes;
    NodeContainer routerNodes;
    NetDeviceContainer leftDevices;
    NetDeviceContainer rightDevices;
    NetDeviceContainer routerDevices;
    NetDeviceContainer leftRouterDevices;     
    NetDeviceContainer rightRouterDevices;    
    Ipv4InterfaceContainer leftLeafInterfaces;
    Ipv4InterfaceContainer rightLeafInterfaces;
    Ipv4InterfaceContainer routerInterfaces;
    Ipv4InterfaceContainer leftRouterInterfaces;
    Ipv4InterfaceContainer rightRouterInterfaces;
    TrafficControlHelper tchBot;
    QueueDiscContainer Qdiscs;
};

}
#endif /* CREATE_TCP_EVAL_DUMBBELL_H */

