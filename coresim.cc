#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/dce-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/internet-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/flow-monitor-helper.h"
#include <string>
#include <sstream>
#include <vector>
#include <iostream>
#include <fstream>
#include <iostream>
#include <map>
#include "parseXml.h"

using namespace ns3;
using namespace std;
NS_LOG_COMPONENT_DEFINE ("coresim");

//function to turn interger to string for iperf input
string DoubleToString (double a)
{
    ostringstream temp;
    temp<<a;
    return temp.str();
}

//fungtion to get the last value in tcp_mem for tcp_mem_max
string SplitLastValue (const std::string& str)
{
	//std::cout << "Splitting: " << str << '\n';
	unsigned found = str.find_last_of(" ");
	ostringstream temp;
	temp << str.substr(found+1);
	return temp.str();
}

//to remove comma if user make input with comma
string RemoveComma (std::string& str) 
{
	//std::cout << "test : " << str << '\n';
	int i = 0;
	std::string str2=str;
	for (i=0; i<3; i++)
	{
		
		std::size_t found=str.find(',');
		if (found!=std::string::npos) {
		str2 = str.replace(str.find(','),1," ");
		} else {
		//std::cout<<"no comma found.."<<std::endl;
		}
	}
	return str2;
}

static void RunIp (Ptr<Node> node, Time at, std::string str)
{
	DceApplicationHelper process;
	ApplicationContainer apps;
	process.SetBinary ("ip");
	process.SetStackSize (1 << 16);
	process.ResetArguments ();
	process.ParseArguments (str.c_str ());
	apps = process.Install (node);
	apps.Start (at);
}

void
PrintTcpFlags (std::string key, std::string value)
{
  NS_LOG_INFO (key << "=" << value);
}

int main (int argc, char *argv[])
{
	double errRate = 0.001;
	double errRate2 = 0.001;
	std::string tcp_cc = "reno";
	std::string tcp_mem_user = "4096 8192 8388608";
	std::string tcp_mem_user_wmem = "4096 8192 8388608";
	std::string tcp_mem_user_rmem = "4096 8192 8388608";

	std::string tcp_mem_server = "4096 8192 8388608";
	std::string tcp_mem_server_wmem = "4096 8192 8388608";
	std::string tcp_mem_server_rmem = "4096 8192 8388608";

	std::string udp_bw="1";

	std::string user_bw_down = "150Mbps";
	std::string user_bw_up = "150Mbps";

	double k_up = 0;
	double pdv_up = 0;
	double avg_delay_up=1;

	double k_dw = 0;
	double pdv_dw = 0;
	double avg_delay_dw=1;

	int monitor = 1;
	int mode = 0;

	int ErrorModel = 1;
    int ErrorModel2 = 1;
    double SimuTime = 50;
    int htmlSize = 2; // in mega bytes
    char TypeOfConnection = 'p'; // iperf tcp connection
    bool downloadMode = true;
    bool inputFromXml = false;
     
    
    CommandLine cmd;
      
    cmd.AddValue ("inputFromXml", "flag for reading input from xml file",inputFromXml);
    cmd.AddValue ("TypeOfConnection", "Link type: p for iperf-tcp, u for iperf-udp and w for wget-thttpd, default to iperf-tcp", TypeOfConnection);
    cmd.AddValue ("ModeOperation", "If true it's download mode for UE, else will do upload. http will always do download", downloadMode);
    cmd.AddValue ("tcp_cc", "TCP congestion control algorithm. Default is reno. Other options: bic, cubic, highspeed, htcp, hybla, illinois, lp, probe, scalable, vegas, veno, westwood, yeah", tcp_cc);
    cmd.AddValue ("tcp_mem_user", "put 3 values (min, default, max) separaed by comma for tcp_mem in user, range 4096-16000000", 		     tcp_mem_user);
      
    cmd.AddValue ("tcp_mem_user_wmem", "put 3 values (min, default, max) separaed by comma for tcp_mem in user, range 4096-16000000", 		     tcp_mem_user_wmem);
      
    cmd.AddValue ("tcp_mem_user_rmem", "put 3 values (min, default, max) separaed by comma for tcp_mem in user, range 4096-16000000", 		     tcp_mem_user_rmem);
      
    cmd.AddValue ("tcp_mem_server", "put 3 values (min, default, max) separaed by comma for tcp_mem in server, range 4096-54000000", tcp_mem_server);
      
    cmd.AddValue ("tcp_mem_server_wmem", "put 3 values (min, default, max) separaed by comma for tcp_mem in server, range 4096-54000000", tcp_mem_server_wmem);
            
    cmd.AddValue ("tcp_mem_server_rmem", "put 3 values (min, default, max) separaed by comma for tcp_mem in server, range 4096-54000000", tcp_mem_server_rmem);
                  
    cmd.AddValue ("user_bw_down", "bandwidth between user and BS, in Mbps. Default is 150", user_bw_down);
    cmd.AddValue ("user_bw_up", "bandwidth between server and BS, in Gbps. Default is 10", user_bw_up);

     
    cmd.AddValue ("errRate", "download Error rate.", errRate);
    cmd.AddValue ("errRate2", "upload Error rate.", errRate2);
    cmd.AddValue ("ErrorModel", "Choose error model you want to use. options: 1 -rate error model-default, 2 - burst error model", ErrorModel);
    cmd.AddValue ("ErrorModel2", "Choose error model you want to use. options: 1 -rate error model-default, 2 - burst error model", ErrorModel);
    cmd.AddValue ("udp_bw","banwidth set for UDP, default is 1M", udp_bw);
    cmd.AddValue ("htmlSize","banwidth set for UDP, default is 1M", htmlSize);
    cmd.AddValue ("SimuTime", "time to do the simulaton, in second", SimuTime);

    cmd.AddValue ("avg_delay_up", "average delay upstream.", avg_delay_up);
    cmd.AddValue ("delay_pdv_up", "theta for normal random distribution in server-BS conection upstream", pdv_up);
    cmd.AddValue ("chan_k_up", " Normal random distribution k in server-BS conection upstream", k_up);
    
    cmd.AddValue ("avg_delay_dw", "average delay downstream.", avg_delay_dw);
    cmd.AddValue ("delay_pdv_dw", "theta for normal random distribution in server-BS conection downstream", pdv_dw);
    cmd.AddValue ("chan_k_dw", " Normal random distribution k in server-BS conection downstream", k_dw);
	
    cmd.Parse (argc, argv);     
    	
	TypeOfConnection = tolower (TypeOfConnection);
	switch (TypeOfConnection){
	    case 'u': //iperf udp connection
	    	std::cout << "IPERF-UDP connection is selected" << std::endl;
	    	break;
	    case 'w': //thttpd - wget connection, always in download mode
	        std::cout << "HTTP connection is selected" << std::endl;
			break;
	    case 'p': //thttpd - wget connection, always in download mode
	        std::cout << "IPERF-TCP connection is selected" << std::endl;
			break;
	    default:
			std::cout << "Unknown link type : " << TypeOfConnection << " ?" << std::endl;
			//return 1;
    }   

	// topologies
    std::cout << "Building topologies.." << std::endl;
    NS_LOG_INFO ("Create nodes.");
    NodeContainer mobile,BS,core,router;
    mobile.Create(1);
    router.Create(2);
    BS.Create(2);
    core.Create (1);
    NodeContainer mobileRouter = NodeContainer (mobile.Get (0), router.Get (0));
    NodeContainer routerAll = NodeContainer (router.Get (0), router.Get (1));
    NodeContainer routerCore = NodeContainer (router.Get (1), core.Get (0));
    
    DceManagerHelper dceManager;
        
#ifdef KERNEL_STACK
    dceManager.SetNetworkStack ("ns3::LinuxSocketFdFactory", "Library", StringValue ("liblinux.so"));
    LinuxStackHelper stack;
    LinuxStackHelper routerStack;
    stack.Install (mobile);
    routerStack.Install (router);
    stack.Install (BS);
    stack.Install (core);
    dceManager.Install (mobile);
    dceManager.Install (router);
    dceManager.Install (BS);
    dceManager.Install (core);
    
    std::cout << "Setting memory size.." << std::endl;
    //setting memory size for user and server
    
    //let's remove the comma
	tcp_mem_user = RemoveComma(tcp_mem_user);
	tcp_mem_user_wmem = RemoveComma(tcp_mem_user_wmem);
	tcp_mem_user_rmem = RemoveComma(tcp_mem_user_rmem);
	
	tcp_mem_server = RemoveComma(tcp_mem_server);
	tcp_mem_server_wmem = RemoveComma(tcp_mem_server_wmem);
	tcp_mem_server_rmem = RemoveComma(tcp_mem_server_rmem);
	
	//assume coma has been removed
	std::string tcp_mem_user_max_wmem = SplitLastValue(tcp_mem_server_wmem);
	std::string tcp_mem_user_max_rmem = SplitLastValue(tcp_mem_server_rmem);
	
	std::string tcp_mem_server_max_wmem = SplitLastValue(tcp_mem_server_wmem);
	std::string tcp_mem_server_max_rmem = SplitLastValue(tcp_mem_server_rmem);
		
    std::string IperfTime = DoubleToString(SimuTime);	
	
    stack.SysctlSet (mobile.Get(0), ".net.ipv4.tcp_mem", tcp_mem_user);
    stack.SysctlSet (mobile.Get(0), ".net.ipv4.tcp_wmem", tcp_mem_user_wmem);
    stack.SysctlSet (mobile.Get(0), ".net.ipv4.tcp_rmem", tcp_mem_user_rmem);
    stack.SysctlSet (mobile.Get(0), ".net.core.wmem_max", tcp_mem_user_max_wmem);
    stack.SysctlSet (mobile.Get(0), ".net.core.rmem_max", tcp_mem_user_max_rmem);
    stack.SysctlSet (mobile.Get(0), ".net.core.netdev_max_backlog", "250000");
        
    stack.SysctlSet (core.Get(0), ".net.ipv4.tcp_mem", tcp_mem_server);
    stack.SysctlSet (core.Get(0), ".net.ipv4.tcp_wmem", tcp_mem_server_wmem);
    stack.SysctlSet (core.Get(0), ".net.ipv4.tcp_rmem", tcp_mem_server_rmem);
    stack.SysctlSet (core.Get(0), ".net.core.wmem_max", tcp_mem_server_max_wmem);
    stack.SysctlSet (core.Get(0), ".net.core.rmem_max", tcp_mem_server_max_rmem);
    stack.SysctlSet (core.Get(0), ".net.core.netdev_max_backlog", "250000");
    
    stack.SysctlSet (mobile, ".net.ipv4.tcp_congestion_control", tcp_cc);
    stack.SysctlSet (BS, ".net.ipv4.tcp_congestion_control", tcp_cc);
    stack.SysctlSet (core, ".net.ipv4.tcp_congestion_control", tcp_cc);
    stack.SysctlSet (router, ".net.ipv4.tcp_congestion_control", tcp_cc);

#else
    NS_LOG_ERROR ("Linux kernel stack for DCE is not available. build with dce-linux module.");
    //silently exit
    return 0;
#endif
	
    std::cout << "Setting link.." << std::endl;


	if (downloadMode) {
		std::cout << "Download mode is used "<< std::endl;
		mode = 0;
	}
	if (!downloadMode) {
		std::cout << "Upload mode is used "<< std::endl;
		mode = 1;
	}

	// channel for mobile router to BS
	NS_LOG_INFO ("Create channels.");
	PointToPointHelper p2p;
	p2p.SetDeviceAttribute ("DataRate", StringValue (user_bw_down));
	p2p.SetChannelAttribute ("Delay", StringValue ("1ms"));
	NetDeviceContainer chanRouterAll = p2p.Install (routerAll);

	// channel for mobile router to mobile and mobile router to core
	p2p.SetDeviceAttribute ("DataRate", StringValue ("200Gbps"));
	p2p.SetChannelAttribute ("Delay", StringValue ("1ms"));
	p2p.SetChannelAttribute ("transparent", UintegerValue (1));
	p2p.SetChannelAttribute ("coreRouter", UintegerValue (0));
	p2p.SetChannelAttribute ("monitor", UintegerValue (monitor));
	p2p.SetChannelAttribute ("mode",UintegerValue (mode) );
	NetDeviceContainer chanMobileRouter = p2p.Install (mobileRouter);
	
	p2p.SetDeviceAttribute ("DataRate", StringValue ("200Gbps"));
	p2p.SetChannelAttribute ("Delay", StringValue ("1ms"));
	p2p.SetChannelAttribute ("transparent", UintegerValue (1));
	p2p.SetChannelAttribute ("coreRouter", UintegerValue (1));
	p2p.SetChannelAttribute ("monitor", UintegerValue (monitor));
	p2p.SetChannelAttribute ("mode",UintegerValue (mode) );
	NetDeviceContainer chanRouterCore = p2p.Install (routerCore);

	// IP Address
    NS_LOG_INFO ("Assign IP Addresses.");
    std::cout << "Setting IP addresses" << std::endl;
    Ipv4AddressHelper ipv4;
    //for router All
	ipv4.SetBase ("10.8.1.0", "255.255.255.0");
	Ipv4InterfaceContainer IPRouterBSDown = ipv4.Assign (chanRouterAll);

	// for router to mobile and core
	ipv4.SetBase ("10.9.1.0", "255.255.255.0");
	Ipv4InterfaceContainer IPMobileRouter = ipv4.Assign (chanMobileRouter);
	
	ipv4.SetBase ("10.9.2.0", "255.255.255.0");
	Ipv4InterfaceContainer IPRouterCore = ipv4.Assign (chanRouterCore);
        
	// Create router nodes, initialize routing database and set up the routing tables in the nodes.
    std::cout << "Creating routing table" << std::endl;
    std::ostringstream cmd_oss;
    	
    //Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
    // setup ip routes
    // mobile
	cmd_oss.str ("");
	cmd_oss << "route add "<< "10.9.2.2"<<"/255.255.255.255" <<" via " <<"10.9.1.2";
	LinuxStackHelper::RunIp (mobile.Get (0), Seconds (0.1), cmd_oss.str ().c_str ());
    // core
	cmd_oss.str ("");
	cmd_oss << "route add "<< "10.9.1.1"<<"/255.255.255.255" <<" via " <<"10.9.2.1";
	LinuxStackHelper::RunIp (core.Get (0), Seconds (0.1), cmd_oss.str ().c_str ());
       
	// mobile router
	cmd_oss.str ("");
	cmd_oss << "route add "<< "10.9.2.2"<<"/255.255.255.255" <<" via " <<"10.8.1.2";
	LinuxStackHelper::RunIp (router.Get (0), Seconds (0.1), cmd_oss.str ().c_str ());
	    
	// core router
	cmd_oss.str ("");
	cmd_oss << "route add "<< "10.9.1.1"<<"/255.255.255.255" <<" via " <<"10.8.1.1";
	LinuxStackHelper::RunIp (router.Get (1), Seconds (0.1), cmd_oss.str ().c_str ());

#ifdef KERNEL_STACK
    LinuxStackHelper::PopulateRoutingTables ();
#endif

	// Application
    NS_LOG_INFO ("Create Applications.");
    std::cout << "Creating Applications.." << std::endl;
    DceApplicationHelper dce;

	dce.SetStackSize (1 << 20);
	int EndTime = 2*SimuTime;;
	
	if (EndTime<=100) {
	   EndTime=100;	
	}
	
	switch (TypeOfConnection){
		case 'p':
		{
        if (downloadMode){
            // Launch iperf server on node 0 (mobile device)
            
            dce.SetBinary ("iperf");
            dce.ResetArguments ();
            dce.ResetEnvironment ();
            dce.AddArgument ("-s");
            dce.AddArgument ("-P");
            dce.AddArgument ("1");
            ApplicationContainer SerApps0 = dce.Install (mobile.Get (0));
            SerApps0.Start (Seconds (1));
            //SerApps0.Stop (Seconds (SimuTime+(SimuTime*25/100)));

            // Launch iperf client on node 2
            dce.SetBinary ("iperf");
            dce.ResetArguments ();
            dce.ResetEnvironment ();
            dce.AddArgument ("-c");
            dce.AddArgument ("10.9.1.1");
            dce.AddArgument ("-i");
            dce.AddArgument ("1");
            dce.AddArgument ("--time");
            dce.AddArgument (IperfTime);
            ApplicationContainer ClientApps0 = dce.Install (core.Get (0));
            ClientApps0.Start (Seconds (1));
            //ClientApps0.Stop (Seconds (SimuTime+(SimuTime*25/100)));
        }
            else{
			// Launch iperf server on node 2
			// server will receive tcp message
			dce.SetBinary ("iperf");
			dce.ResetArguments ();
			dce.ResetEnvironment ();
			dce.AddArgument ("-s");
			dce.AddArgument ("-P");
			dce.AddArgument ("1");
			ApplicationContainer SerApps0 = dce.Install (core.Get (0));
			SerApps0.Start (Seconds (1));
			//SerApps0.Stop (Seconds (SimuTime+(SimuTime*25/100)));

			// Launch iperf client on node 0
			dce.SetBinary ("iperf");
			dce.ResetArguments ();
			dce.ResetEnvironment ();
			dce.AddArgument ("-c");
			dce.AddArgument ("10.9.2.2");
			dce.AddArgument ("-i");                
			dce.AddArgument ("1");                        
			dce.AddArgument ("--time");                
			dce.AddArgument (IperfTime);
			ApplicationContainer ClientApps0 = dce.Install (mobile.Get (0));
			ClientApps0.Start (Seconds (1));
			//ClientApps0.Stop (Seconds (SimuTime+(SimuTime*25/100)));
            }
        }
		break;    
    }

    NS_LOG_INFO ("Run Simulation.");
    
    //std::cout << "Simulation will take about "<< (SimuTime) <<"seconds." << std::endl;

    Simulator::Stop (Seconds (EndTime));
    
    std::cout << "Running simulation" <<std::endl;
    Simulator::Run ();
    std::cout << "Simulation is completed" << std::endl;
    Simulator::Destroy ();
    NS_LOG_INFO ("Done.");

  return 0;
}

