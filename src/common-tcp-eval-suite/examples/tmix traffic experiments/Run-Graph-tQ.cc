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
    std::string transport_prot[] = { "ns3::TcpHtcp","ns3::TcpScalable","ns3::TcpHybla","ns3::TcpIllinois","ns3::TcpNewReno", "ns3::TcpHighSpeed","ns3::TcpVegas", "ns3::TcpBic","ns3::TcpWestwood"} ;
    std::string trans_prot[] = { "HTCP","Scalable","Hybla","Illinois","NewReno", "HighSpeed","Vegas", "BIC","Westwood"} ;
    std::string run = "./waf --run scratch/"+scenarioName+"Tmix";
    system (run.c_str());

    std::string gnuPlotTF = "";
    std::string gnuPlotTR = "";
    std::string gnuPlotQF = "";
    std::string gnuPlotQR = "";
    expt_num =1;

    	for(i=0, num_Tcp = 9; i<num_Tcp;i++)
        {  		

            gnuPlotTF = "gnuplot -e 'set terminal png size 640,640; set output \"tcp-eval-output/"+ scenarioName +"/EXPT-"+std::to_string(expt_num)+"/"+transport_prot[i]+"_Result-TPF.png\"; set xlabel \"Time (s)\"; set ylabel \"Throughput (Bps)\"; set xrange[] ; plot ";
            gnuPlotTR = "gnuplot -e 'set terminal png size 640,640; set output \"tcp-eval-output/"+ scenarioName +"/EXPT-"+std::to_string(expt_num)+"/"+transport_prot[i]+"_Result-TPR.png\"; set xlabel \"Time (s)\"; set ylabel \"Throughput (Bps)\"; set xrange[] ; plot ";

            gnuPlotQF = "gnuplot -e 'set terminal png size 640,640; set output \"tcp-eval-output/"+ scenarioName +"/EXPT-"+std::to_string(expt_num)+"/"+transport_prot[i]+"_Result-QDF.png\"; set xlabel \"Time (s)\"; set ylabel \"Queue Delay (ms)\";set xrange[] ; plot ";
            gnuPlotQR = "gnuplot -e 'set terminal png size 640,640; set output \"tcp-eval-output/"+ scenarioName +"/EXPT-"+std::to_string(expt_num)+"/"+transport_prot[i]+"_Result-QDR.png\"; set xlabel \"Time (sec)\"; set ylabel \"Queue Delay (ms)\";set xrange[] ; plot ";

             std::string awkTF = "awk 'BEGIN{sum=0;count=0;i=0;}{if(i==0){i=$1;sum=$2;count=1;}if($1-i<0.8){sum+=$2;count++;}else{i=$1;print $1 \" \" sum/count; sum=$2;count=1;}}' tcp-eval-output/"+ scenarioName +"/EXPT-"+std::to_string(expt_num)+"/"+transport_prot[i]+"_throughputF.dat"+" > tcp-eval-output/"+ scenarioName +"/EXPT-"+std::to_string(expt_num)+"/"+transport_prot[i]+"_AgTF.dat" ;
             std::string awkTR = "awk 'BEGIN{sum=0;count=0;i=0;}{if(i==0){i=$1;sum=$2;count=1;}if($1-i<0.8){sum+=$2;count++;}else{i=$1;print $1 \" \" sum/count; sum=$2;count=1;}}' tcp-eval-output/"+ scenarioName +"/EXPT-"+std::to_string(expt_num)+"/"+transport_prot[i]+"_throughputR.dat"+" > tcp-eval-output/"+ scenarioName +"/EXPT-"+std::to_string(expt_num)+"/"+transport_prot[i]+"_AgTR.dat" ;
             std::string awkQF = "awk 'BEGIN{sum=0;count=0;i=0;}{if(i==0){i=$1;sum=$2;count=1;}if($1-i<0.8){sum+=$2;count++;}else{i=$1;print $1 \" \" sum/count; sum=$2;count=1;}}' tcp-eval-output/"+ scenarioName +"/EXPT-"+std::to_string(expt_num)+"/"+transport_prot[i]+"_qdelF.dat"+" > tcp-eval-output/"+ scenarioName +"/EXPT-"+std::to_string(expt_num)+"/"+transport_prot[i]+"_AgQF.dat" ;
             std::string awkQR = "awk 'BEGIN{sum=0;count=0;i=0;}{if(i==0){i=$1;sum=$2;count=1;}if($1-i<0.8){sum+=$2;count++;}else{i=$1;print $1 \" \" sum/count; sum=$2;count=1;}}' tcp-eval-output/"+ scenarioName +"/EXPT-"+std::to_string(expt_num)+"/"+transport_prot[i]+"_qdelR.dat"+" > tcp-eval-output/"+ scenarioName +"/EXPT-"+std::to_string(expt_num)+"/"+transport_prot[i]+"_AgQR.dat" ;

            system(awkTF.c_str());
            system(awkTR.c_str());
            system(awkQF.c_str());
            system(awkQR.c_str());

            std::string graphNameTF =std::string("\"tcp-eval-output/"+ scenarioName +"/EXPT-"+std::to_string(expt_num)+"/"+transport_prot[i]+"_AgTF.dat\" title \"")+trans_prot[i]+std::string("\" with lines "); 
            std::string graphNameTR =std::string("\"tcp-eval-output/"+ scenarioName +"/EXPT-"+std::to_string(expt_num)+"/"+transport_prot[i]+"_AgTR.dat\" title \"")+trans_prot[i]+std::string("\" with lines ");
            std::string graphNameQF =std::string("\"tcp-eval-output/"+ scenarioName +"/EXPT-"+std::to_string(expt_num)+"/"+transport_prot[i]+"_AgQF.dat\" title \"")+trans_prot[i]+std::string("\" with lines"); 
            std::string graphNameQR =std::string("\"tcp-eval-output/"+ scenarioName +"/EXPT-"+std::to_string(expt_num)+"/"+transport_prot[i]+"_AgQR.dat\" title \"")+trans_prot[i]+std::string("\" with lines "); 
 

                gnuPlotTF = gnuPlotTF + graphNameTF + std::string("'");
                gnuPlotTR = gnuPlotTR + graphNameTR + std::string("'");
                gnuPlotQF = gnuPlotQF + graphNameQF + std::string("'");
                gnuPlotQR = gnuPlotQR + graphNameQR + std::string("'");
      

            system(gnuPlotTF.c_str());
        system(gnuPlotTR.c_str());
        system(gnuPlotQF.c_str());
        system(gnuPlotQR.c_str());
        
    	}
         
	return(0);
}
