#include <stdlib.h>
#include <string>
#include <iostream>
#include "ns3/core-module.h"

using namespace ns3;
	
int main(int argc, char *argv[])
{

	std::string scenarioName = "Accesslink";

	CommandLine cmd;
	cmd.AddValue ("scenarioName", "Scenario Name ", scenarioName);
    cmd.Parse (argc,argv);

    uint32_t i =0, expt_num;
    std::string transport_prot[] = { "ns3::TcpNewReno", "ns3::TcpHybla","ns3::TcpHighSpeed","ns3::TcpVegas", "ns3::TcpScalable","ns3::TcpHtcp", "ns3::TcpVeno", "ns3::TcpBic", "ns3::TcpYeah", "ns3::TcpIllinois","ns3::TcpWestwood", "ns3::TcpWestwoodPlus"} ;
    std::string run = "./waf --run scratch/"+scenarioName+"Tmix --vis";
    std::cout << run;
    system (run.c_str());

    for(i=0; i<1;i++)
    {
    	for(expt_num =1;expt_num<=1;expt_num++)
    	{
    		std::string gnuPlot = "gnuplot -e 'set terminal png size 640,640; set output \"TCPEvalOutput/"+ scenarioName +"/EXPT-"+std::to_string(expt_num)+"/"+transport_prot[i]+"_Result.png\"; set xrange[] reverse; plot ";
    		std::string proQdelThr = std::string ("python src/common-tcp-eval-suit/generate-ellipseinput.py ") + scenarioName + " " + transport_prot[i] + " "+ std::to_string(expt_num);
    		std::string proEllipse = std::string ("python src/common-tcp-eval-suit/ellipsemaker ") + scenarioName + " " + transport_prot[i] + " "+ std::to_string(expt_num);
    		system(proQdelThr.c_str());
    		system(proEllipse.c_str());
    		std::string graphName =std::string("\"TCPEvalOutput/"+ scenarioName +"/EXPT-"+std::to_string(expt_num)+"/"+transport_prot[i]+"_result.dat\" title \"")+transport_prot[i]+std::string("\" with lines'"); 
    		gnuPlot+=graphName;
    		system(gnuPlot.c_str());
    	}
    }
	return(0);
}
