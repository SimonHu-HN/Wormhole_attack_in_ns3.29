#include "ns3/propagation-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/netanim-module.h"

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/config-store-module.h"
#include "ns3/wifi-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/olsr-helper.h"
#include "ns3/point-to-point-module.h"
#include "ns3/inet-socket-address.h"
#include "ns3/csma-module.h"

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
	//uint32_t  packet_size = 2500 ;

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
			StringValue("2500"));
	// turn off RTS/CTS for frames below 2200 bytes
	Config::SetDefault("ns3::WifiRemoteStationManager::RtsCtsThreshold",
			StringValue("2200"));
	// Fix non-unicast data rate to be the same as that of unicast
	Config::SetDefault("ns3::WifiRemoteStationManager::NonUnicastMode",
			StringValue(phyMode));

// Here, we will create n nodes.
	NS_LOG_INFO("Create nodes.");

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

	// NodeContainer sn;
	// NodeContainer cn;
	// sn.Create(1);
	// cn.Create(4);
	// NodeContainer all = NodeContainer(sn,cn);

	// NodeContainer sn1;
	// NodeContainer cn1;
	// sn1.Create(1);
	// cn1.Create(4);
	// NodeContainer all1 = NodeContainer(sn1,cn1);

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
//  NqosWifiMacHelper wifiMac = NqosWifiMacHelper::Default ();
	WifiMacHelper wifiMac;
	wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager", "DataMode",
			StringValue(phyMode), "ControlMode", StringValue(phyMode));
	// Set it to adhoc mode
	wifiMac.SetType("ns3::AdhocWifiMac");
	NetDeviceContainer devices = wifi.Install(wifiPhy, wifiMac, allNodes);
	// NetDeviceContainer devices1 = wifi.Install (wifiPhy, wifiMac, all);
	// NetDeviceContainer devices2 = wifi.Install (wifiPhy, wifiMac, all1);

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

	//positionAlloc->Add (Vector (200.0, 240.0, 0.0));

	// positionAlloc->Add (Vector (60.0, -80.0, 0.0));

	// positionAlloc->Add (Vector (140.0, 170.0, 0.0));
	// positionAlloc->Add (Vector (-100.0, 300.0, 300.0));
	// positionAlloc->Add (Vector (-50.0, 350.0, 300.0));
	// positionAlloc->Add (Vector (-0.0, 290.0, 300.0));
	// positionAlloc->Add (Vector (-50.0, 250.0, 300.0));

	mobility.SetPositionAllocator(positionAlloc);
	mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
	mobility.Install(allNodes);
	// mobility.Install (all);
	// mobility.Install (all1);

	InternetStackHelper internet;
	internet.Install(allNodes);
	// internet.Install (all);
	// internet.Install (all1);

	Ipv4AddressHelper ipv4;
	NS_LOG_INFO("Assign IP Addresses.");
	ipv4.SetBase("10.1.1.0", "255.255.255.0");
	Ipv4InterfaceContainer i = ipv4.Assign(devices);
	// Ipv4AddressHelper add;
	// add.SetBase ("10.1.2.0", "255.255.255.0");
	// Ipv4InterfaceContainer j = add.Assign (devices1);
	// Ipv4AddressHelper add1;
	// add1.SetBase ("10.1.3.0", "255.255.255.0");
	// Ipv4InterfaceContainer k = add1.Assign (devices2);

	// Create a packet sink on the star "hub" to receive these packets
	uint16_t port = 5000;
	Address sinkLocalAddress(InetSocketAddress(Ipv4Address::GetAny(), port));
	PacketSinkHelper sinkHelper("ns3::UdpSocketFactory", sinkLocalAddress);

	ApplicationContainer sinkApp = sinkHelper.Install(serverNode);
	// ApplicationContainer sinkApp_2 = sinkHelper.Install (sn);
	// ApplicationContainer sinkApp_3 = sinkHelper.Install (sn1);
	sinkApp.Start(Seconds(1.0));
	//sinkApp.Stop (Seconds (100.0));

	// sinkApp_2.Start(Seconds(1.0));
	// sinkApp_2.Stop (Seconds (100.0));

	// sinkApp_3.Start(Seconds(1.0));
	// sinkApp_3.Stop (Seconds (100.0));

	// Create the OnOff applications to send UDP to the server
	OnOffHelper clientHelper("ns3::UdpSocketFactory", Address());
	//Ptr<Socket> ns3UdpSocket = Socket::CreateSocket (cn_extra.Get (10), UdpSocketFactory::GetTypeId ());

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
	extra_1.Start(Seconds(1.0));
	//extra_1.Stop (Seconds (100.0));

	ApplicationContainer extra_2;
	AddressValue remoteAddress(InetSocketAddress(i.GetAddress(9), port));
	clientHelper.SetAttribute("Remote", remoteAddress);
	extra_2.Add(clientHelper.Install(cn_extra_2.Get(0)));
	extra_2.Start(Seconds(4.0));
	//extra_2.Stop (Seconds (100.0));

	ApplicationContainer extra_4;
	AddressValue remoteAddress1(InetSocketAddress(i.GetAddress(1), port));
	clientHelper.SetAttribute("Remote", remoteAddress1);
	extra_4.Add(clientHelper.Install(cn_extra_2.Get(1)));
	extra_4.Start(Seconds(1.0));
	//extra_4.Stop (Seconds (100.0));

	ApplicationContainer extra_5;
	AddressValue remoteAddress2(InetSocketAddress(i.GetAddress(4), port));
	clientHelper.SetAttribute("Remote", remoteAddress2);
	extra_5.Add(clientHelper.Install(cn_extra_2.Get(2)));
	extra_5.Start(Seconds(1.0));
	//extra_5.Stop (Seconds (100.0));

	ApplicationContainer extra_6;
	AddressValue remoteAddress3(InetSocketAddress(i.GetAddress(3), port));
	clientHelper.SetAttribute("Remote", remoteAddress3);
	extra_6.Add(clientHelper.Install(cn_extra_2.Get(3)));
	extra_6.Start(Seconds(1.0));
	//extra_6.Stop (Seconds (100.0));

	ApplicationContainer extra_7;
	AddressValue remoteAddress4(InetSocketAddress(i.GetAddress(4), port));
	clientHelper.SetAttribute("Remote", remoteAddress4);
	extra_7.Add(clientHelper.Install(cn_extra_2.Get(3)));
	extra_7.Start(Seconds(4.0));
	//extra_7.Stop (Seconds (100.0));

	ApplicationContainer extra_8;
	AddressValue remoteAddress5(InetSocketAddress(i.GetAddress(4), port));
	clientHelper.SetAttribute("Remote", remoteAddress5);
	extra_8.Add(clientHelper.Install(cn_extra_2.Get(1)));
	extra_8.Start(Seconds(1.0));
	//extra_8.Stop (Seconds (100.0));

	ApplicationContainer extra_9;
	AddressValue remoteAddress6(InetSocketAddress(i.GetAddress(11), port));
	clientHelper.SetAttribute("Remote", remoteAddress6);
	extra_9.Add(clientHelper.Install(cn_extra_2.Get(2)));
	extra_9.Start(Seconds(2.0));
	//extra_9.Stop (Seconds (100.0));

	ApplicationContainer extra_10;
	AddressValue remoteAddress7(InetSocketAddress(i.GetAddress(1), port));
	clientHelper.SetAttribute("Remote", remoteAddress7);
	extra_10.Add(clientHelper.Install(cn_extra.Get(0)));
	extra_10.Start(Seconds(3.0));
	//extra_10.Stop (Seconds (100.0));

	//configure tracing
	AsciiTraceHelper ascii;
	wifiPhy.EnableAsciiAll(ascii.CreateFileStream("udp-single-hop.tr"));
	wifiPhy.EnablePcapAll("udp-single-hop");

	// Install FlowMonitor on all nodes
	FlowMonitorHelper flowmon;
	Ptr<FlowMonitor> monitor = flowmon.InstallAll();

	AnimationInterface anim("withoutattack"); // Mandatory
//   AnimationInterface::SetConstantPosition (serverNode.Get (0), 500,500);
//   AnimationInterface::SetConstantPosition (clientNodes.Get (0), 750,500);
//   AnimationInterface::SetConstantPosition (clientNodes.Get (1), 750, 750);
//   AnimationInterface::SetConstantPosition (clientNodes.Get (2), 500, 750);
//   AnimationInterface::SetConstantPosition (clientNodes.Get (3), 250, 750);
//   AnimationInterface::SetConstantPosition (clientNodes.Get (4), 250, 500);
//   AnimationInterface::SetConstantPosition (clientNodes.Get (5), 250, 250);
//   AnimationInterface::SetConstantPosition (clientNodes.Get (6), 500, 250);
//   AnimationInterface::SetConstantPosition (clientNodes.Get (7), 750, 250);
//   Run simulation
	Simulator::Stop(Seconds(200));
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
	double Delaysum = 0;

	for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i =
			stats.begin(); i != stats.end(); ++i) {
		Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow(i->first);
		std::cout << "Flow : " << i->first << " (" << t.sourceAddress << " -> "
				<< t.destinationAddress << ")  ";

		txPacketsum += i->second.txPackets;
		rxPacketsum += i->second.rxPackets;
		rxBytesum += i->second.rxBytes;
		LostPacketsum += i->second.lostPackets;
		DropPacketsum += i->second.packetsDropped.size();
		Delaysum += i->second.delaySum.GetSeconds();

		std::cout << "FP: " << i->second.txPackets << "(" << i->second.txBytes
				<< ") ";
		std::cout << "RP: " << i->second.rxPackets << "(" << i->second.rxBytes
				<< ") ";
		//std::cout << "  Lost Packets:   " << i->second.lostPackets << "\n";
		//std::cout << "  Drop Packets:   " << i->second.packetsDropped.size() << "\n";
		//std::cout << "  Packets Delivery Ratio: " << ((rxPacketsum * 100) /txPacketsum) << "%" << "\n";
		//std::cout << "  Packets Lost Ratio: " << ((LostPacketsum * 100) /txPacketsum) << "%" << "\n";
		//std::cout << "Throughput: " << i->second.rxBytes * 8.0 / 10.0 / 1024 / 1024  << " Mbps ";
		std::cout << "Delay : "
				<< (i->second.delaySum.GetSeconds() / i->second.txPackets);
		std::cout << "\n";
	}
	std::cout
			<< "\n\n\n ######################## Final Conclusion ########################"
			<< "\n\n";
	std::cout << "  All Sent Packets: " << txPacketsum << "                  \n"
			<< "  All Received Packets: " << rxPacketsum << "\n";
	std::cout << "  All Lost Packets: " << (txPacketsum - rxPacketsum)
			<< "                \n" << "  All Drop Packets: " << DropPacketsum
			<< "\n";
	std::cout << "  Packet Drop Ratio: "
			<< (double) ((DropPacketsum / txPacketsum) * 100) << "% \n";
	//std::cout << "  Packets Delivery Ratio: " << ((rxPacketsum * 100) /txPacketsum) << "%" << "          \n" << "  Packets Lost Ratio: " << (double)((LostPacketsum * 100) /txPacketsum) << "%" << "\n";
	std::cout << "  All Delay: " << Delaysum / txPacketsum << "\n";
	std::cout << "  Average END-TO-END delay: "
			<< ((Delaysum / txPacketsum) / rxPacketsum) * 2990000 << "\n";
	std::cout << "  Throughput: "
			<< (rxBytesum * 8.0 / 10.0 / 1024 / 1024) * 100 << " bits/s\n";

	// //hop count
	// Ipv4Header header;
	// packet->PeekHeader (&header);
	// uint8_t ttl = header.GetTtl();
	// std::cout<<"Hop Count: "<<ttl<<"\n";
	// //end of hop count

	Simulator::Destroy();

	return 0;
}
