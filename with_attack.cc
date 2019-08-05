#include "ns3/propagation-module.h"
#include "ns3/flow-monitor-module.h"

#include "ns3/aodv-module.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/config-store-module.h"
#include "ns3/wifi-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "myapp.h"
#include "ns3/netanim-module.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cassert>

NS_LOG_COMPONENT_DEFINE("WifiSimpleAdhoc");

using namespace ns3;

int main(int argc, char *argv[]) {
	std::string phyMode("DsssRate1Mbps");
	double rss = -80;  // -dBm
	bool verbose = false;

	// Set up some default values for the simulation.
	Config::SetDefault("ns3::OnOffApplication::PacketSize",
			StringValue("1024"));
	Config::SetDefault("ns3::OnOffApplication::DataRate",
			StringValue("25kb/s"));

	CommandLine cmd;

	cmd.AddValue("phyMode", "Wifi Phy mode", phyMode);
	cmd.AddValue("rss", "received signal strength", rss);
	cmd.AddValue("verbose", "turn on all WifiNetDevice log components",
			verbose);

	cmd.Parse(argc, argv);

// Convert to time object
	//Time interPacketInterval = Seconds (interval);

	// disable fragmentation for frames below 2200 bytes
	Config::SetDefault("ns3::WifiRemoteStationManager::FragmentationThreshold",
			StringValue("2200"));
	// turn off RTS/CTS for frames below 2200 bytes
	Config::SetDefault("ns3::WifiRemoteStationManager::RtsCtsThreshold",
			StringValue("2200"));
	// Fix non-unicast data rate to be the same as that of unicast
	Config::SetDefault("ns3::WifiRemoteStationManager::NonUnicastMode",
			StringValue(phyMode));

// Here, we will create nodes.
	NS_LOG_INFO("Create nodes.");
	NodeContainer malicious;
	NodeContainer not_malicious;
	NodeContainer serverNode;
	NodeContainer clientNodes;
	serverNode.Create(1);
	clientNodes.Create(4);

	NodeContainer cn;
	cn.Create(4);

	NodeContainer cn_extra;
	cn_extra.Create(1);

	NodeContainer cn_extra_2;
	cn_extra_2.Create(4);

	NodeContainer allNodes = NodeContainer(serverNode, clientNodes, cn,
			cn_extra, cn_extra_2);

	not_malicious.Add(serverNode.Get(0));
	not_malicious.Add(clientNodes.Get(0));
	not_malicious.Add(clientNodes.Get(1));
	not_malicious.Add(clientNodes.Get(2));
	not_malicious.Add(clientNodes.Get(3));

	not_malicious.Add(cn.Get(0));
	not_malicious.Add(cn.Get(1));
	malicious.Add(cn.Get(2));
	not_malicious.Add(cn.Get(3));

	not_malicious.Add(cn_extra.Get(0));

	not_malicious.Add(cn_extra_2.Get(0));
	not_malicious.Add(cn_extra_2.Get(1));
	malicious.Add(cn_extra_2.Get(2));
	not_malicious.Add(cn_extra_2.Get(3));

	// The below set of helpers will help us to put together the wifi NICs we want
	WifiHelper wifi;
	if (verbose) {
		wifi.EnableLogComponents();  // Turn on all Wifi logging
	}
	wifi.SetStandard(WIFI_PHY_STANDARD_80211b);

	YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default();
	// This is one parameter that matters when using FixedRssLossModel
	// set it to zero; otherwise, gain will be added
	wifiPhy.Set("RxGain", DoubleValue(0));
	// ns-3 supports RadioTap and Prism tracing extensions for 802.11b
	wifiPhy.SetPcapDataLinkType(YansWifiPhyHelper::DLT_IEEE802_11_RADIO);

	YansWifiChannelHelper wifiChannel;
	wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
	// The below FixedRssLossModel will cause the rss to be fixed regardless
	// of the distance between the two stations, and the transmit power

	wifiChannel.AddPropagationLoss("ns3::FixedRssLossModel", "Rss",
			DoubleValue(rss));
	wifiPhy.SetChannel(wifiChannel.Create());

	// Add a non-QoS upper mac, and disable rate control
//  WifiMacHelper wifiMac = WifiMacHelper::Default ();
	WifiMacHelper wifiMac;
	wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager", "DataMode",
			StringValue(phyMode), "ControlMode", StringValue(phyMode));
	// Set it to adhoc mode
	wifiMac.SetType("ns3::AdhocWifiMac");
	NetDeviceContainer devices = wifi.Install(wifiPhy, wifiMac, allNodes);
	NetDeviceContainer mal_devices = wifi.Install(wifiPhy, wifiMac, malicious);

	//  Enable AODV
	AodvHelper aodv;
	AodvHelper malicious_aodv;

	// Note that with FixedRssLossModel, the positions below are not
	// used for received signal strength.
	MobilityHelper mobility;
	Ptr<ListPositionAllocator> positionAlloc = CreateObject<
			ListPositionAllocator>();
	positionAlloc->Add(Vector(150.0, 150.0, 150.0));
	positionAlloc->Add(Vector(100.0, 200.0, 0.0));
	positionAlloc->Add(Vector(150.0, 210.0, 0.0));
	positionAlloc->Add(Vector(110.0, 110.0, 0.0));
	positionAlloc->Add(Vector(80.0, 150.0, 0.0));

	// positionAlloc->Add (Vector (160.0, 160.0, 160.0));
	positionAlloc->Add(Vector(210.0, 140.0, 300.0));
	positionAlloc->Add(Vector(150.0, 90.0, 300.0));
	positionAlloc->Add(Vector(205.0, 190.0, 300.0));
	positionAlloc->Add(Vector(190.0, 110.0, 300.0));

	positionAlloc->Add(Vector(100.0, 250.0, 0.0));
	positionAlloc->Add(Vector(120.0, 280.0, 0.0));
	positionAlloc->Add(Vector(50.0, 210.0, 0.0));
	positionAlloc->Add(Vector(0.0, 150.0, 0.0));
	positionAlloc->Add(Vector(30.0, 100.0, 0.0));

	mobility.SetPositionAllocator(positionAlloc);
	mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
	mobility.Install(allNodes);

	InternetStackHelper internet;
	internet.SetRoutingHelper(aodv);
	internet.Install(not_malicious);
	malicious_aodv.Set("EnableWrmAttack", BooleanValue(true)); // putting *false* instead of *true* would disable the malicious behavior of the node

	malicious_aodv.Set("FirstEndWifiWormTunnel", Ipv4AddressValue("10.0.1.1"));
	malicious_aodv.Set("FirstEndWifiWormTunnel", Ipv4AddressValue("10.0.1.2"));

	internet.SetRoutingHelper(malicious_aodv);
	internet.Install(malicious);

	Ipv4AddressHelper ipv4;
	NS_LOG_INFO("Assign IP Addresses.");
	ipv4.SetBase("10.1.1.0", "255.255.255.0");
	Ipv4InterfaceContainer i = ipv4.Assign(devices);

	ipv4.SetBase("10.1.2.0", "255.255.255.0");
	Ipv4InterfaceContainer mal_ifcont = ipv4.Assign(mal_devices);

	// Create a packet sink on the star "hub" to receive these packets
	uint16_t port = 50000;
	Address sinkLocalAddress(InetSocketAddress(Ipv4Address::GetAny(), port));
	PacketSinkHelper sinkHelper("ns3::UdpSocketFactory", sinkLocalAddress);

	ApplicationContainer sinkApp = sinkHelper.Install(serverNode);
	sinkApp.Start(Seconds(1.0));
	//sinkApp.Stop (Seconds (100.0));

	// Create the OnOff applications to send UDP to the server
	OnOffHelper clientHelper("ns3::UdpSocketFactory", Address());

	clientHelper.SetAttribute("OnTime",
			StringValue("ns3::ConstantRandomVariable[Constant=1]"));
	clientHelper.SetAttribute("OffTime",
			StringValue("ns3::ConstantRandomVariable[Constant=0]"));
//normally wouldn't need a loop here but the server IP address is different
	//on each p2p subnet
	ApplicationContainer clientApps;
	for (uint32_t j = 0; j < clientNodes.GetN(); ++j) {
		AddressValue remoteAddress(InetSocketAddress(i.GetAddress(0), port));
		clientHelper.SetAttribute("Remote", remoteAddress);
		clientApps.Add(clientHelper.Install(clientNodes.Get(j)));
	}
	clientApps.Start(Seconds(3.0));
	//clientApps.Stop (Seconds (100.0));

	ApplicationContainer clientApps_2;
	for (uint32_t k = 0; k < cn.GetN(); ++k) {
		AddressValue remoteAddress(InetSocketAddress(i.GetAddress(0), port));
		clientHelper.SetAttribute("Remote", remoteAddress);
		clientApps_2.Add(clientHelper.Install(cn.Get(k)));
	}
	clientApps_2.Start(Seconds(5.0));
	//clientApps_2.Stop (Seconds (100.0));

	ApplicationContainer extra_1;
	for (uint32_t k = 0; k < cn_extra.GetN(); ++k) {
		AddressValue remoteAddress(InetSocketAddress(i.GetAddress(2), port));
		clientHelper.SetAttribute("Remote", remoteAddress);
		extra_1.Add(clientHelper.Install(cn_extra.Get(k)));
	}
	extra_1.Start(Seconds(7.0));
	//extra_1.Stop (Seconds (100.0));

	ApplicationContainer extra_2;
	AddressValue remoteAddress(InetSocketAddress(i.GetAddress(9), port));
	clientHelper.SetAttribute("Remote", remoteAddress);
	extra_2.Add(clientHelper.Install(cn_extra_2.Get(0)));
	extra_2.Start(Seconds(4.0));
	//extra_2.Stop (Seconds (100.0));

	// ApplicationContainer extra_3;
	// AddressValue remoteAddress0 (InetSocketAddress (i.GetAddress (2), port));
	// clientHelper.SetAttribute ("Remote", remoteAddress0);
	// extra_3.Add (clientHelper.Install (cn_extra_2.Get (0)));

	ApplicationContainer extra_4;
	AddressValue remoteAddress1(InetSocketAddress(i.GetAddress(1), port));
	clientHelper.SetAttribute("Remote", remoteAddress1);
	extra_4.Add(clientHelper.Install(cn_extra_2.Get(1)));

	ApplicationContainer extra_5;
	AddressValue remoteAddress2(InetSocketAddress(i.GetAddress(4), port));
	clientHelper.SetAttribute("Remote", remoteAddress2);
	extra_5.Add(clientHelper.Install(cn_extra_2.Get(2)));

	ApplicationContainer extra_6;
	AddressValue remoteAddress3(InetSocketAddress(i.GetAddress(3), port));
	clientHelper.SetAttribute("Remote", remoteAddress3);
	extra_6.Add(clientHelper.Install(cn_extra_2.Get(3)));

	//configure tracing
	AsciiTraceHelper ascii;
	wifiPhy.EnableAsciiAll(ascii.CreateFileStream("udp-single-hop.tr"));
	wifiPhy.EnablePcapAll("aodv/udp-single-hop");

	// Install FlowMonitor on all nodes
	FlowMonitorHelper flowmon;
	Ptr<FlowMonitor> monitor = flowmon.InstallAll();

	// Run simulation for 10 seconds
	Simulator::Stop(Seconds(20));

	AnimationInterface anim("with_attack.xml"); // Mandatory
//   AnimationInterface::SetConstantPosition (serverNode.Get (0), 500,500);
//   AnimationInterface::SetConstantPosition (clientNodes.Get (0), 750,500);
//   AnimationInterface::SetConstantPosition (clientNodes.Get (1), 750, 750);
//   AnimationInterface::SetConstantPosition (clientNodes.Get (2), 500, 750);
//   AnimationInterface::SetConstantPosition (clientNodes.Get (3), 250, 750);
//   AnimationInterface::SetConstantPosition (clientNodes.Get (4), 250, 500);
//   AnimationInterface::SetConstantPosition (clientNodes.Get (5), 250, 250);
//   AnimationInterface::SetConstantPosition (clientNodes.Get (6), 500, q250);
//   AnimationInterface::SetConstantPosition (clientNodes.Get (7), 750, 250);
	Simulator::Run();

	// Print per flow statistics
	monitor->CheckForLostPackets();

	Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier>(
			flowmon.GetClassifier());
	std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats();

	uint32_t txPacketsum = 0;
	uint32_t rxPacketsum = 0;
	uint32_t rxBytesum = 0;
	double DropPacketsum = 0;
	uint32_t LostPacketsum = 0;
	double packet_loss_threshold = 2.0;
	double delay_threshold = 1.5;
	double Delaysum = 0;
	//double etf = 40 ;
	int j = 0;
	for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i =
			stats.begin(); i != stats.end(); ++i) {
		Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow(i->first);
		std::cout << j + 1 << std::endl;
		j++;
		//for counting the total result
		txPacketsum += i->second.txPackets;
		rxPacketsum += i->second.rxPackets;
		rxBytesum += i->second.rxBytes;
		LostPacketsum += i->second.lostPackets;
		//DropPacketsum += i->second.packetsDropped.size();
		for (uint32_t j = 0; j < i->second.packetsDropped.size(); j++) {
			DropPacketsum += i->second.packetsDropped[j];
		}

		Delaysum += i->second.delaySum.GetSeconds();
		//end of counting the total result

		if (t.sourceAddress != "10.1.1.1") {
			std::cout << "Flow : " << i->first << " (" << t.sourceAddress
					<< " -> " << t.destinationAddress << ")  ";
			std::cout << "FP: " << i->second.txPackets << "("
					<< i->second.txBytes << ") ";
			std::cout << "RP: " << i->second.rxPackets << "("
					<< i->second.rxBytes << ") ";
			//std::cout << "  Lost Packets:   " << i->second.lostPackets << "\n";
			//std::cout << "  Drop Packets:   " << i->second.packetsDropped.size() << "\n";
			//std::cout << "  Packets Delivery Ratio: " << ((rxPacketsum * 100) /txPacketsum) << "%" << "\n";
			//std::cout << "  Packets Lost Ratio: " << ((LostPacketsum * 100) /txPacketsum) << "%" << "\n";
			//std::cout << "Throughput: " << i->second.rxBytes * 8.0 / 10.0 / 1024 / 1024  << " Mbps ";
			std::cout << "Delay : "
					<< (i->second.delaySum.GetSeconds() / i->second.txPackets);
			std::cout << "  ";
			if ((i->second.txPackets - i->second.rxPackets) != 0) {
				if ((i->second.txPackets - i->second.rxPackets)
						> packet_loss_threshold) {
					if ((i->second.delaySum.GetSeconds() / i->second.txPackets)
							< delay_threshold) {
						std::cout << " Wormhole Link Detected ! \n";
					} else {
						std::cout << "\n";
					}
				} else {
					std::cout << "\n";
				}
			} else {
				std::cout << "\n";
			}
		}
	}
	std::cout
			<< "\n\n\n ######################## Final Conclusion ########################"
			<< "\n\n";
	std::cout << "  All Sent Packets: " << txPacketsum << "                  \n"
			<< "  All Received Packets: " << rxPacketsum << "\n";
	std::cout << "  All Lost Packets: " << LostPacketsum << "                \n"
			<< "  All Drop Packets: " << DropPacketsum << "\n";
	std::cout << "  Packet Drop Ratio: "
			<< (double) ((DropPacketsum / txPacketsum) * 100) << "% \n";
	//std::cout << "  Packets Delivery Ratio: " << ((rxPacketsum * 100) /txPacketsum) << "%" << "          \n" << "  Packets Lost Ratio: " << ((LostPacketsum * 100) /txPacketsum) << "%" << "\n";
	std::cout << "  All Delay: " << Delaysum / txPacketsum << "\n";
	std::cout << "  Average END-TO-END delay: "
			<< ((Delaysum / txPacketsum) / rxPacketsum) * 1000000 << "\n";
	std::cout << "  Throughput: "
			<< (rxBytesum * 8.0 / 10.0 / 1024 / 1024) * 100 << " bits/s\n";

	Simulator::Destroy();

	return 0;
}
