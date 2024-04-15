/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
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
 */

// Network topology
//
//                     Lan1
//                 ==================
//                 |    |    |      |
//       n0-------  n1   n2   n3   n4
//          p to p
//
// - Multicast source is at node n0;
// - Multicast forwarded by node n2 onto LAN1;
// - Nodes n0, n1, n2, n3, and n4 receive the multicast frame.
// - Node n4 listens for the data 

#include <iostream>
#include <fstream>
#include "ns3/point-to-point-module.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/csma-module.h"
#include "ns3/applications-module.h"
#include "ns3/internet-module.h"
#include "ns3/netanim-module.h"
#include "ns3/mobility-helper.h"
#include "ns3/constant-position-mobility-model.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("CsmaMulticastExample");

int 
main (int argc, char *argv[])
{
  //
  // Users may find it convenient to turn on explicit debugging
  // for selected modules; the below lines suggest how to do this
  //
  // LogComponentEnable ("CsmaMulticastExample", LOG_LEVEL_INFO);

  //
  // Set up default values for the simulation.
  //
  // Select DIX/Ethernet II-style encapsulation (no LLC/Snap header)
  Config::SetDefault ("ns3::CsmaNetDevice::EncapsulationMode", StringValue ("Dix"));

  // Allow the user to override any of the defaults at
  // run-time, via command-line arguments
  CommandLine cmd (__FILE__);
  cmd.Parse (argc, argv);

  NS_LOG_INFO ("Create nodes.");
  NodeContainer c; //create all the nodes first
  c.Create (5);
  // We will later want two subcontainers of these nodes, for the two LANs
  NodeContainer c0 = NodeContainer (c.Get (0), c.Get (1)); // p to p
  NodeContainer c1 = NodeContainer (c.Get(1),c.Get (2), c.Get (3), c.Get (4)); //lan 1 //see that 1 is repeated

//setting up pointTOpoint
  PointToPointHelper pointToPoint1;
  pointToPoint1.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  pointToPoint1.SetChannelAttribute ("Delay", StringValue ("2ms"));

  NetDeviceContainer nd0;
  nd0 = pointToPoint1.Install (c0);


  NS_LOG_INFO ("Build Topology.");
  CsmaHelper csma;
  csma.SetChannelAttribute ("DataRate", DataRateValue (DataRate (5000000)));
  csma.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (2))); //both the lans will have the same feature
 
  // We will use these NetDevice containers later, for IP addressing
  NetDeviceContainer nd1 = csma.Install (c1);  // First LAN
  

  NS_LOG_INFO ("Add IP Stack.");
  InternetStackHelper internet; // only one for all 
  internet.Install (c); //will it work even if it is hybrid

  NS_LOG_INFO ("Assign IP Addresses.");
  Ipv4AddressHelper ipv4Addr; // you can use the same address helper but different addresses set base
  ipv4Addr.SetBase ("10.1.1.0", "255.255.255.0"); // pTop  address
  ipv4Addr.Assign (nd0);
  ipv4Addr.SetBase ("10.1.2.0", "255.255.255.0"); // LAN address
  ipv4Addr.Assign (nd1);

  NS_LOG_INFO ("Configure multicasting.");
  //
  // Now we can configure multicasting.  As described above, the multicast 
  // source is at node zero, which we assigned the IP address of 10.1.1.1 
  // earlier.  We need to define a multicast group to send packets to.  This
  // can be any multicast address from 224.0.0.0 through 239.255.255.255
  // (avoiding the reserved routing protocol addresses).
  //

//   Ipv4Address multicastSource ("10.1.1.1"); //node 0
//   Ipv4Address multicastGroup ("225.1.2.4"); //no need multicast source

  // Now, we will set up multicast routing.  We need to do three things:
  // 1) Configure a (static) multicast route on node n2
  // 2) Set up a default multicast route on the sender n0 
  // 3) Have node n4 join the multicast group
  // We have a helper that can help us with static multicast
//   Ipv4StaticRoutingHelper multicast;

//   // 1) Configure a (static) multicast route on node n2 (multicastRouter)
//   Ptr<Node> multicastRouter = c.Get (1);  // The node in question //the joining point
//   Ptr<NetDevice> inputIf = nd0.Get (1);  // The input NetDevice
//   NetDeviceContainer outputDevices;  // A container of output NetDevices
//   outputDevices.Add (nd1.Get (0));  // (we only need one NetDevice here) //input and output are the same node

//   multicast.AddMulticastRoute (multicastRouter, multicastSource, 
//                                multicastGroup, inputIf, outputDevices);

//   // 2) Set up a default multicast route on the sender n0 
//   Ptr<Node> sender = c.Get (0);
//   Ptr<NetDevice> senderIf = nd0.Get (0);
//   multicast.SetDefaultMulticastRoute (sender, senderIf);

  //
  // Create an OnOff application to send UDP datagrams from node zero to the
  // multicast group (node four will be listening).
  //
  NS_LOG_INFO ("Create Applications.");

  uint16_t port = 9;   // Discard port (RFC 863)

  // Configure a multicast packet generator that generates a packet
  // every few seconds
  OnOffHelper onoff ("ns3::UdpSocketFactory", 
                     Address (InetSocketAddress (Ipv4Address ("255.255.255.255"), port))); // this part is different from multicast
  onoff.SetConstantRate (DataRate ("255b/s"));
  onoff.SetAttribute ("PacketSize", UintegerValue (128));

  ApplicationContainer srcC = onoff.Install (c1.Get (0)); //installing it for node 0

  //
  // Tell the application when to start and stop.
  //
  srcC.Start (Seconds (1.));
  srcC.Stop (Seconds (10.));

  // Create an optional packet sink to receive these packets
  PacketSinkHelper sink ("ns3::UdpSocketFactory",
                         InetSocketAddress (Ipv4Address::GetAny (), port));
                         //multicast occurs between two subnets but broadcast cant so one is the source

  ApplicationContainer sinkC = sink.Install (c0.Get (0));  // broadcasting back, adding sink to n2// n1 is the broadcast source
  sinkC.Add (sink.Install (c1.Get (1)));// Node n4  is receiving all the packets
  // Start the sink
  sinkC.Start (Seconds (1.0));
  sinkC.Stop (Seconds (10.0));

  NS_LOG_INFO ("Configure Tracing.");
  //
  // Configure tracing of all enqueue, dequeue, and NetDevice receive events.
  // Ascii trace output will be sent to the file "csma-multicast.tr"
  //
  AsciiTraceHelper ascii;
  csma.EnableAsciiAll (ascii.CreateFileStream ("csma-multicast.tr"));

  // Also configure some tcpdump traces; each interface will be traced.
  // The output files will be named:
  //     csma-multicast-<nodeId>-<interfaceId>.pcap
  // and can be read by the "tcpdump -r" command (use "-tt" option to
  // display timestamps correctly)
  csma.EnablePcapAll ("csma-multicast", false);

  //
  // Now, do the actual simulation.
  //
  NS_LOG_INFO ("Run Simulation.");
  AnimationInterface anim("MTl4q1multicast.xml");
  MobilityHelper mobility;
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (c);
 
  Ptr<ConstantPositionMobilityModel> s1 = c.Get (0)->GetObject<ConstantPositionMobilityModel> ();
  Ptr<ConstantPositionMobilityModel> s2 = c.Get (1)->GetObject<ConstantPositionMobilityModel> ();
  Ptr<ConstantPositionMobilityModel> s3 = c.Get (2)->GetObject<ConstantPositionMobilityModel> ();
  Ptr<ConstantPositionMobilityModel> s4 = c.Get (3)->GetObject<ConstantPositionMobilityModel> ();
  Ptr<ConstantPositionMobilityModel> s5 = c.Get (4)->GetObject<ConstantPositionMobilityModel> ();
 
  s1->SetPosition (Vector ( 0, 0, 0  ));
  s2->SetPosition (Vector ( 10, 10, 0  ));
  s3->SetPosition (Vector ( 20, 10, 0 ));
  s4->SetPosition (Vector ( 30, 10, 0 ));
  s5->SetPosition (Vector ( 40, 10, 0 ));
  Simulator::Run ();
  Simulator::Destroy ();
  NS_LOG_INFO ("Done.");
}
