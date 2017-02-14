#include <stdlib.h>
#include <string>
#include <iostream>
#include "ns3/core-module.h"

using namespace ns3;
	
int main(int argc, char *argv[])
{

	std::string scenarioName = "";

	CommandLine cmd;
	cmd.AddValue ("scenarioName", "Scenario Name ", scenarioName);
    cmd.Parse (argc,argv);

    uint32_t i =0, expt_num, num_Tcp;
    std::string transport_prot[] = { "ns3::TcpNewReno", "ns3::TcpHighSpeed","ns3::TcpVegas","ns3::TcpIllinois",  "ns3::TcpBic","ns3::TcpWestwood","ns3::TcpScalable", " ns3::TcpHybla","ns3::TcpHtcp"} ;
    std::string run = "./waf --run scratch/"+scenarioName+"Tmix";
    system (run.c_str());

    std::string gnuPlotF = "";
    std::string gnuPlotR = "";
    for(expt_num =1;expt_num<=3;expt_num++)
        {

	std::ofstream outfile;
      outfile.open((std::string("tcp-eval-output/")+scenarioName+"/EXPT-"+std::to_string(expt_num)+"/"+std::string("plot-shellF")).c_str(), std::ios::out | std::ios::trunc);
      outfile << "set terminal png size 1260, 800\n";
      outfile << "set output \"tcp-eval-output/"<<scenarioName.c_str()<<"/EXPT-"+std::to_string(expt_num)+"/qdel-throughputF.png\"\n set xlabel \"Queue Delay Range\"\nset ylabel \"Throughput \"\n";
      outfile << "set xrange[] reverse\nset grid\nshow grid\nunset key\n";
      outfile.close ();

      outfile.open((std::string("tcp-eval-output/")+scenarioName+"/EXPT-"+std::to_string(expt_num)+"/"+std::string("plot-shellR")).c_str(), std::ios::out | std::ios::trunc);
      outfile << "set terminal png size 1260, 800\n";
      outfile << "set output \"tcp-eval-output/"<<scenarioName.c_str()<<"/EXPT-"+std::to_string(expt_num)+"/qdel-throughputR.png\"\n set xlabel \"Range of Queue Delay \"\nset ylabel \"Throughput \"\n";
      outfile << "set xrange[] reverse\nset grid\nshow grid\nunset key\n";
      outfile.close ();

    	gnuPlotF = "plot ";
	gnuPlotR = "plot ";
    	for(i=0, num_Tcp = 9; i<num_Tcp;i++)
        {  		
    		std::string proQdelThr = std::string ("python src/common-tcp-eval-suite/generate-ellipseinput.py ") + scenarioName + " " + transport_prot[i] + " "+ std::to_string(expt_num);
    		std::string proEllipse = std::string ("python src/common-tcp-eval-suite/ellipsemaker ") + scenarioName + " " + transport_prot[i] + " "+ std::to_string(expt_num);
            std::string ResultF = proQdelThr + " F";
            std::string EllipseF = proEllipse + " F";
            std::string ResultR = proQdelThr + " R";
            std::string EllipseR = proEllipse + " R";
    		system(ResultF.c_str());
    		system(EllipseF.c_str());
            system(ResultR.c_str());
            system(EllipseR.c_str());
    		std::string graphNameF =std::string("\"tcp-eval-output/"+ scenarioName +"/EXPT-"+std::to_string(expt_num)+"/"+transport_prot[i]+"_ellipseF.dat\" title \"")+transport_prot[i]+std::string("\" with lines lt 1"); 
            std::string graphNameR =std::string("\"tcp-eval-output/"+ scenarioName +"/EXPT-"+std::to_string(expt_num)+"/"+transport_prot[i]+"_ellipseR.dat\" title \"")+transport_prot[i]+std::string("\" with lines lt 1"); 
            if (i!=num_Tcp-1)
            {
                gnuPlotF = gnuPlotF + graphNameF + std::string(",");
                gnuPlotR = gnuPlotR + graphNameR + std::string(",");
            }
            else
            {
                gnuPlotF += graphNameF; 
                gnuPlotR += graphNameR; 
            }
    	}

      outfile.open((std::string("tcp-eval-output/")+scenarioName+"/EXPT-"+std::to_string(expt_num)+"/"+std::string("plot-shellF")).c_str(), std::ios::out | std::ios::app);
      outfile<<gnuPlotF.c_str();
      outfile.close();
	
      outfile.open((std::string("tcp-eval-output/")+scenarioName+"/EXPT-"+std::to_string(expt_num)+"/"+std::string("plot-shellR")).c_str(), std::ios::out | std::ios::app);
      outfile<<gnuPlotR.c_str();
      outfile.close();

       system ((std::string("gnuplot tcp-eval-output/")+scenarioName+"/EXPT-"+std::to_string(expt_num)+"/"+std::string("plot-shellF")).c_str());
       system ((std::string("gnuplot tcp-eval-output/")+scenarioName+"/EXPT-"+std::to_string(expt_num)+"/"+std::string("plot-shellR")).c_str());

        }    
	return(0);
}
