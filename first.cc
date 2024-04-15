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

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/netanim-module.h"
#include "ns3/mobility-helper.h"
#include "ns3/constant-position-mobility-model.h"

// Default Network Topology
//
//       10.1.1.0       10.1.2.0
// n0 -------------- n1----------n2
//    point-to-point
//
 
using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("FirstScriptExample");

int
main (int argc, char *argv[])
{
  CommandLine cmd (__FILE__);
  cmd.Parse (argc, argv);
  
  Time::SetResolution (Time::NS);
  LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);

  NodeContainer nodes;
  nodes.Create (3); //changed the nodes to 3


  //there are two pointtopoints
  PointToPointHelper pointToPoint1;
  pointToPoint1.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  pointToPoint1.SetChannelAttribute ("Delay", StringValue ("2ms"));

  PointToPointHelper pointToPoint2;
  pointToPoint2.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  pointToPoint2.SetChannelAttribute ("Delay", StringValue ("2ms"));

  NetDeviceContainer devices1, devices2;
  devices1 = pointToPoint1.Install (nodes.Get(0), nodes.Get(1));
  devices2 = pointToPoint1.Install (nodes.Get(1), nodes.Get(2));


  InternetStackHelper stack;
  stack.Install (nodes); //internet stack helper only one is enough

  Ipv4AddressHelper address1;
  address1.SetBase ("10.1.1.0", "255.255.255.0");

  Ipv4InterfaceContainer interfaces1 = address1.Assign (devices1);

  Ipv4AddressHelper address2;
  address1.SetBase ("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces2 = address1.Assign (devices2); //assign address2 to container 2..if not, it goes only through one link

  UdpEchoServerHelper echoServer (9);

  ApplicationContainer serverApps = echoServer.Install (nodes.Get (0)); //server is node 0
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (10.0));

  UdpEchoClientHelper echoClient (interfaces1.GetAddress (0), 9); //client is node 2 needs node 0 (server's address)
  echoClient.SetAttribute ("MaxPackets", UintegerValue (6));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

  ApplicationContainer clientApps = echoClient.Install (nodes.Get (2)); // client is node 2
  clientApps.Start (Seconds (2.0));
  clientApps.Stop (Seconds (10.0));
  //both server and client have application container, we just gave names

  AnimationInterface anim("MTl1q1.xml");
  MobilityHelper mobility;
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (nodes);
 
  Ptr<ConstantPositionMobilityModel> s1 = nodes.Get (0)->GetObject<ConstantPositionMobilityModel> ();
  Ptr<ConstantPositionMobilityModel> s2 = nodes.Get (1)->GetObject<ConstantPositionMobilityModel> ();
  Ptr<ConstantPositionMobilityModel> s3 = nodes.Get (2)->GetObject<ConstantPositionMobilityModel> ();
 
  s1->SetPosition (Vector ( 0, 0, 0  ));
  s2->SetPosition (Vector ( 30, 90, 0  ));
  s3->SetPosition (Vector ( 90, 90, 0 ));


  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}
