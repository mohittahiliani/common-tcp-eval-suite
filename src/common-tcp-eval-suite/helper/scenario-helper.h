/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef SCENARIO_HELPER_H
#define SCENARIO_HELPER_H

#include "ns3/create-dumbbell.h"
//#include <ns3/evaluation.h>

namespace ns3 {

class scenario
{
public:

	scenario();

	~scenario();

	void settrafficparam(uint32_t payload, std::string Rate, double simTime, uint32_t source_node_num, uint32_t sink_node_num);
	void settopologyparam(uint32_t num_of_nodes, std::string bot_delay, std::string bot_bw, std::string edge_bw);
	void runscenario(std::string scenarioName, std::string edge_delay[],double bufferLimit);
private:
	std::string fileName = "";
	std::ofstream fp;
	uint32_t payloadSize;
	std::string dataRate;
	double simulationTime;
	uint32_t source_node;
	uint32_t sink_node;
	uint32_t numnodes;
	std::string bottleneck_delay;
	std::string bottleneck_bw;
	std::string edgelink_bw;
	//std::string edgelink_delay[];

};

}

#endif /* SCENARIO_HELPER_H */

