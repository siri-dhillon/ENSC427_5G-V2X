/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

// ENSC427 project: 5G v2V Network Analysis
//
// scenrio_1: 4 vehicles moving at reqpective constant speeds; communicating via a 5G V2V connection in UDP;
//	      experience an interruption
//  
// description: TODO: add description 
//              
//              
// 
// author/editor: SIRPREET KAUR DHILLON (skd24@sfu.ca, ID:301318486)
//
// extended from: Millicar Examples - ns3mmwave/src/millicar/vehicular-simple-one.cc - 
//	 	  Copyright (c) 2020 University of Padova, Dep. of Information Engineering,
//		  SIGNET lab.

// Date created: 


/*
******************** LICENSE INFORMATION BEGIN *********************************
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License version 2 as
*   published by the Free Software Foundation;
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program; if not, write to the Free Software
*   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
******************** LICENSE INFORMATION END ************************************
*/

#include "ns3/mmwave-sidelink-spectrum-phy.h"
#include "ns3/mmwave-vehicular-net-device.h"
#include "ns3/mmwave-vehicular-helper.h"
#include "ns3/constant-position-mobility-model.h"
#include "ns3/mobility-module.h"
#include "ns3/config.h"
#include "ns3/isotropic-antenna-model.h"
#include "ns3/spectrum-helper.h"
#include "ns3/mmwave-spectrum-value-helper.h"
#include "ns3/applications-module.h"
#include "ns3/internet-module.h"
#include "ns3/core-module.h"
#include "ns3/netanim-module.h" //NetAnim - for animating our simulation
#include "ns3/flow-monitor-helper.h"

NS_LOG_COMPONENT_DEFINE ("Scenario1");

using namespace ns3;
using namespace millicar;

uint32_t g_rxPackets; // total number of received packets
uint32_t g_txPackets; // total number of transmitted packets

Time g_firstReceived; // timestamp of the first time a packet is received
Time g_lastReceived; // timestamp of the last received packet

static void Rx (Ptr<OutputStreamWrapper> stream, Ptr<const Packet> p)
{
 g_rxPackets++;
 SeqTsHeader header;

 p->PeekHeader(header);

 //interrupting signal 
 if (Simulator::Now() < Seconds(5) && Simulator::Now() > Seconds(4.5)){
        *stream->GetStream () << Simulator::Now ().GetSeconds () << "\t" << 0 << "\t" << 0 << "\t" << 0 << std::endl;
 }
 else {
       *stream->GetStream () << Simulator::Now ().GetSeconds () << "\t" << p->GetSize() << "\t" << header.GetSeq() << "\t" << header.GetTs().GetSeconds() << std::endl; 
 }
 //*stream->GetStream () << Simulator::Now ().GetSeconds () << "\t" << p->GetSize() << "\t" << header.GetSeq() << "\t" << header.GetTs().GetSeconds() << std::endl;

 if (g_rxPackets > 1){

   g_lastReceived = Simulator::Now();
 }
 else{
   g_firstReceived = Simulator::Now();
 }
}

int main (int argc, char *argv[])
{
  
  std::string animFile = "animation.xml" ;  // Name of file for animation output

  // system parameters
  Time endTime = Seconds (10.0);
  double bandwidth = 1e8; // bandwidth in Hz
  double frequency = 28e9; // the carrier frequency
  uint32_t numerology = 3; // the numerology

  // applications
  uint32_t packetSize = 1024; // UDP packet size in bytes
  //uint32_t startTime = 50; // application start time in milliseconds
  //uint32_t endTime = 2000; // application end time in milliseconds
  uint32_t interPacketInterval = 30; // interpacket interval in microseconds
  uint32_t maxPacketCount = 0xFFFFFFFF; //max packet count 

  // mobility
  double speed = 20; // speed of the vehicles m/s
  double intraGroupDistance = 10; // distance between two vehicles belonging to the same group
  double antennaHeight = 2.0; // the height of the antenna

  CommandLine cmd;
  //
  cmd.AddValue ("bandwidth", "used bandwidth", bandwidth);
  cmd.AddValue ("iip", "inter packet interval, in microseconds", interPacketInterval);
  cmd.AddValue ("intraGroupDistance", "distance between two vehicles belonging to the same group, y-coord", intraGroupDistance);
  cmd.AddValue ("numerology", "set the numerology to use at the physical layer", numerology);
  cmd.AddValue ("frequency", "set the carrier frequency", frequency);
  cmd.Parse (argc, argv);

  Config::SetDefault ("ns3::MmWaveSidelinkMac::UseAmc", BooleanValue (true));
  Config::SetDefault ("ns3::MmWavePhyMacCommon::CenterFreq", DoubleValue (frequency));
  Config::SetDefault ("ns3::MmWaveVehicularHelper::Bandwidth", DoubleValue (bandwidth));
  Config::SetDefault ("ns3::MmWaveVehicularHelper::Numerology", UintegerValue (numerology));
  Config::SetDefault ("ns3::MmWaveVehicularPropagationLossModel::ChannelCondition", StringValue ("a"));
  Config::SetDefault ("ns3::MmWaveVehicularPropagationLossModel::Shadowing", BooleanValue (true));
  Config::SetDefault ("ns3::MmWaveVehicularSpectrumPropagationLossModel::UpdatePeriod", TimeValue (MilliSeconds (1)));
  Config::SetDefault ("ns3::MmWaveVehicularAntennaArrayModel::AntennaElements", UintegerValue (16));
  Config::SetDefault ("ns3::MmWaveVehicularAntennaArrayModel::AntennaElementPattern", StringValue ("3GPP-V2V"));
  Config::SetDefault ("ns3::MmWaveVehicularAntennaArrayModel::IsotropicAntennaElements", BooleanValue (true));
  Config::SetDefault ("ns3::MmWaveVehicularAntennaArrayModel::NumSectors", UintegerValue (2));

  Config::SetDefault ("ns3::MmWaveVehicularNetDevice::RlcType", StringValue("LteRlcUm"));
  Config::SetDefault ("ns3::MmWaveVehicularHelper::SchedulingPatternOption", EnumValue(2)); // use 2 for SchedulingPatternOption=OPTIMIZED, 1 or SchedulingPatternOption=DEFAULT
  Config::SetDefault ("ns3::LteRlcUm::MaxTxBufferSize", UintegerValue (500*1024));

  // create the nodes
  NodeContainer n;
  n.Create (4);
  // create the mobility models
  MobilityHelper mobility;
  mobility.SetMobilityModel ("ns3::ConstantVelocityMobilityModel");
  mobility.Install (n);

  n.Get (0)->GetObject<MobilityModel> ()->SetPosition (Vector (0,0,antennaHeight));
  n.Get (0)->GetObject<ConstantVelocityMobilityModel> ()->SetVelocity (Vector (0, speed, 0));

  n.Get (1)->GetObject<MobilityModel> ()->SetPosition (Vector (0, intraGroupDistance,  antennaHeight));
  n.Get (1)->GetObject<ConstantVelocityMobilityModel> ()->SetVelocity (Vector (0, speed, 0));
  
  n.Get (2)->GetObject<MobilityModel> ()->SetPosition (Vector (intraGroupDistance,0,antennaHeight));
  n.Get (2)->GetObject<ConstantVelocityMobilityModel> ()->SetVelocity (Vector (0, -speed*2, 0));
  
  n.Get (3)->GetObject<MobilityModel> ()->SetPosition (Vector (2*intraGroupDistance,0,antennaHeight));
  n.Get (3)->GetObject<ConstantVelocityMobilityModel> ()->SetVelocity (Vector (speed, 0, 0));

  // create and configure the helper
  Ptr<MmWaveVehicularHelper> helper = CreateObject<MmWaveVehicularHelper> ();
  helper->SetNumerology (3);
  helper->SetPropagationLossModelType ("ns3::MmWaveVehicularPropagationLossModel");
  helper->SetSpectrumPropagationLossModelType ("ns3::MmWaveVehicularSpectrumPropagationLossModel");
  NetDeviceContainer devs = helper->InstallMmWaveVehicularNetDevices (n);

  // Install the TCP/IP stack in the two nodes

  InternetStackHelper internet;
  internet.Install (n);

  Ipv4AddressHelper ipv4;
  NS_LOG_INFO ("Assign IP Addresses.");
  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer i = ipv4.Assign (devs);

  // Need to pair the devices in order to create a correspondence between transmitter and receiver
  // and to populate the < IP addr, RNTI > map.
  helper->PairDevices(devs);

  // Set the routing table
  Ipv4StaticRoutingHelper ipv4RoutingHelper;  
  
  Ptr<Ipv4StaticRouting> staticRouting = ipv4RoutingHelper.GetStaticRouting (n.Get (0)->GetObject<Ipv4> ());
  staticRouting->SetDefaultRoute (n.Get (1)->GetObject<Ipv4> ()->GetAddress (1, 0).GetLocal () , 2 );
  
  staticRouting = ipv4RoutingHelper.GetStaticRouting (n.Get (2)->GetObject<Ipv4> ());
  staticRouting->SetDefaultRoute (n.Get (1)->GetObject<Ipv4> ()->GetAddress (1, 0).GetLocal () , 2 );
  
  staticRouting = ipv4RoutingHelper.GetStaticRouting (n.Get (3)->GetObject<Ipv4> ());
  staticRouting->SetDefaultRoute (n.Get (1)->GetObject<Ipv4> ()->GetAddress (1, 0).GetLocal () , 2 );

  NS_LOG_DEBUG("IPv4 Address node 0: " << n.Get (0)->GetObject<Ipv4> ()->GetAddress (1, 0).GetLocal ());
  NS_LOG_DEBUG("IPv4 Address node 1: " << n.Get (1)->GetObject<Ipv4> ()->GetAddress (1, 0).GetLocal ());
  NS_LOG_DEBUG("IPv4 Address node 2: " << n.Get (2)->GetObject<Ipv4> ()->GetAddress (1, 0).GetLocal ());
  NS_LOG_DEBUG("IPv4 Address node 3: " << n.Get (3)->GetObject<Ipv4> ()->GetAddress (1, 0).GetLocal ());
  
  Ptr<mmwave::MmWaveAmc> m_amc = CreateObject <mmwave::MmWaveAmc> (helper->GetConfigurationParameters());
  //double availableRate = m_amc->GetTbSizeFromMcsSymbols(28, 14) / 0.001; // bps

  // setup the applications
  Config::SetDefault ("ns3::UdpClient::MaxPackets", UintegerValue (maxPacketCount));
  Config::SetDefault ("ns3::UdpClient::Interval", TimeValue (MicroSeconds (interPacketInterval)));
  Config::SetDefault ("ns3::UdpClient::PacketSize", UintegerValue (packetSize));

  // create the applications
  uint32_t port = 4000; // echo-apps port
  uint16_t port_2 = 4001;

  UdpEchoServerHelper server (port);
  ApplicationContainer echoApps = server.Install (n.Get (1));
  echoApps.Start (Seconds (0.0));

  AsciiTraceHelper asciiTraceHelper;
  Ptr<OutputStreamWrapper> stream = asciiTraceHelper.CreateFileStream ("simple-two-stats_user_1.txt");
  echoApps.Get(0)->TraceConnectWithoutContext ("Rx", MakeBoundCallback (&Rx, stream));
  
  UdpEchoServerHelper server2 (port_2);
  
  ApplicationContainer apps2 = server2.Install (n.Get (2));
  apps2.Start (Seconds (0.0));

  Ptr<OutputStreamWrapper> stream_2 = asciiTraceHelper.CreateFileStream ("simple-two-stats_user_2.txt");
  apps2.Get(0)->TraceConnectWithoutContext ("Rx", MakeBoundCallback (&Rx, stream_2));

  
  UdpClientHelper client (n.Get (2)->GetObject<Ipv4> ()->GetAddress (1, 0).GetLocal (), port);
  client.SetAttribute ("MaxPackets", UintegerValue (maxPacketCount));
  client.SetAttribute ("Interval", TimeValue (MicroSeconds (interPacketInterval)));
  client.SetAttribute ("PacketSize", UintegerValue (packetSize));
  ApplicationContainer clientApps = client.Install (n.Get (0));
  clientApps.Start (Seconds(1.0));
  clientApps.Stop (endTime);

  UdpClientHelper client2 (n.Get (2)->GetObject<Ipv4> ()->GetAddress (1, 0).GetLocal (), port_2);
  client2.SetAttribute ("MaxPackets", UintegerValue (maxPacketCount));
  client2.SetAttribute ("Interval", TimeValue (MicroSeconds (interPacketInterval)));
  client2.SetAttribute ("PacketSize", UintegerValue (packetSize));
  ApplicationContainer clientApps2 = client2.Install (n.Get (1));
  clientApps2.Start (Seconds(1.0));
  clientApps2.Stop (endTime);
  
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

   // Flow monitor
   Ptr<FlowMonitor> flowMonitor;
   FlowMonitorHelper flowHelper;
   flowMonitor = flowHelper.InstallAll();

  Simulator::Stop (Seconds(18.0));

  AnimationInterface anim (animFile);
  anim.EnablePacketMetadata (true);
  anim.SetMobilityPollInterval (Seconds (1));
  anim.EnableIpv4L3ProtocolCounters (Seconds (0), Seconds (10)); // Optional
  
  Simulator::Run ();
  
  std::cout << "Animation Trace file created:" << animFile.c_str ()<< std::endl;
  
  flowMonitor->SerializeToXmlFile("flowmonitor.xml", true, true); // Flow monitor
  
  Simulator::Destroy ();

  std::cout << "----------- Statistics: Scenario 1-----------" << std::endl;
  std::cout << "Antenna Height:\t\t" << antennaHeight << " m" << std::endl;
  std::cout << "Bandwidth:\t\t" << bandwidth << " Hz" << std::endl;
  std::cout << "Packets size:\t\t" << packetSize << " Bytes" << std::endl;
  std::cout << "Packets received:\t" << g_rxPackets << std::endl;
  std::cout << "Average Throughput:\t" << (double(g_rxPackets)*(double(packetSize)*8)/double( g_lastReceived.GetSeconds() - g_firstReceived.GetSeconds()))/1e6 << " Mbps" << std::endl;

  return 0;
}//
