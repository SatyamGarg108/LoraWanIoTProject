#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/lorawan-module.h"
#include "ns3/mobility-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"

#include "burst-mac-app.h"
#include "burst-scheduler.h"
#include "burst-mac-tag.h"

#include <iostream>
#include <fstream>

using namespace ns3;
using namespace lorawan;

NS_LOG_COMPONENT_DEFINE ("BurstMacSimulation");

// Global scheduler instance
Ptr<BurstScheduler> g_scheduler;

// Callback to hook NetworkServer trace to our Scheduler
void OnServerReceivedPacket (Ptr<const Packet> packet)
{
  if (g_scheduler)
    {
      g_scheduler->OnReceivedPacket (packet);
    }
}

int main (int argc, char *argv[])
{
  // Default parameters
  int nNodes = 20;
  int nGateways = 1;
  double radius = 1000.0;
  double simTime = 60.0;
  double burstStart = 10.0;
  double burstStop = 50.0;
  int burstPercent = 100;
  int queueSize = 50;
  // int nVirtualChannels = 8; // Not directly used in sim config but affects collision probability logic if we had multiple channels

  CommandLine cmd;
  cmd.AddValue ("nNodes", "Number of end devices", nNodes);
  cmd.AddValue ("nGateways", "Number of gateways", nGateways);
  cmd.AddValue ("burstPercent", "Percentage of nodes bursting", burstPercent);
  cmd.AddValue ("queueSize", "Packets per burst", queueSize);
  cmd.Parse (argc, argv);

  // Logging
  LogComponentEnable ("BurstMacSimulation", LOG_LEVEL_INFO);
  LogComponentEnable ("BurstMacApp", LOG_LEVEL_INFO);
  LogComponentEnable ("BurstScheduler", LOG_LEVEL_INFO);

  // Create Scheduler
  g_scheduler = CreateObject<BurstScheduler> ();

  // --- LoRaWAN Setup ---
  LoraPhyHelper phyHelper = LoraPhyHelper ();
  LorawanMacHelper macHelper = LorawanMacHelper ();
  LoraHelper helper = LoraHelper ();
  MobilityHelper mobility;

  // Channel
  Ptr<LogDistancePropagationLossModel> loss = CreateObject<LogDistancePropagationLossModel> ();
  loss->SetPathLossExponent (3.76);
  loss->SetReference (1, 7.7);
  Ptr<PropagationDelayModel> delay = CreateObject<ConstantSpeedPropagationDelayModel> ();
  Ptr<LoraChannel> channel = CreateObject<LoraChannel> (loss, delay);
  phyHelper.SetChannel (channel);
  helper.EnablePacketTracking ();

  // Gateways
  NodeContainer gateways;
  gateways.Create (nGateways);
  Ptr<ListPositionAllocator> gwAlloc = CreateObject<ListPositionAllocator> ();
  for (int i=0; i<nGateways; ++i) gwAlloc->Add (Vector (0.0, 0.0, 15.0));
  mobility.SetPositionAllocator (gwAlloc);
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (gateways);
  phyHelper.SetDeviceType (LoraPhyHelper::GW);
  macHelper.SetDeviceType (LorawanMacHelper::GW);
  helper.Install (phyHelper, macHelper, gateways);

  // End Devices
  NodeContainer endDevices;
  endDevices.Create (nNodes);
  mobility.SetPositionAllocator ("ns3::UniformDiscPositionAllocator", "rho", DoubleValue (radius), "X", DoubleValue (0.0), "Y", DoubleValue (0.0));
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (endDevices);
  phyHelper.SetDeviceType (LoraPhyHelper::ED);
  macHelper.SetDeviceType (LorawanMacHelper::ED_A);
  helper.Install (phyHelper, macHelper, endDevices);

  // Network Server
  NetworkServerHelper nsHelper;
  ForwarderHelper forHelper;
  PointToPointHelper p2p;
  p2p.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("2ms"));
  Ptr<Node> networkServer = CreateObject<Node> ();
  P2PGwRegistration_t gwRegistration;
  for (uint32_t i = 0; i < gateways.GetN (); ++i)
    {
      auto c = p2p.Install (networkServer, gateways.Get (i));
      gwRegistration.emplace_back (DynamicCast<PointToPointNetDevice> (c.Get (0)), gateways.Get (i));
    }
  nsHelper.SetGatewaysP2P (gwRegistration);
  nsHelper.SetEndDevices (endDevices);
  ApplicationContainer nsApps = nsHelper.Install (networkServer);
  forHelper.Install (gateways);

  // Hook Scheduler to Server Trace
  Ptr<NetworkServer> ns = DynamicCast<NetworkServer> (nsApps.Get (0));
  ns->TraceConnectWithoutContext ("ReceivedPacket", MakeCallback (&OnServerReceivedPacket));

  // Install BurstMacApp
  int burstCount = nNodes * burstPercent / 100;
  for (int i = 0; i < nNodes; ++i)
    {
      Ptr<Node> node = endDevices.Get (i);
      Ptr<LoraNetDevice> loraNetDevice = DynamicCast<LoraNetDevice> (node->GetDevice (0));
      Ptr<EndDeviceLorawanMac> mac = DynamicCast<EndDeviceLorawanMac> (loraNetDevice->GetMac ());

      Ptr<BurstMacApp> app = CreateObject<BurstMacApp> ();
      app->Setup (mac, g_scheduler);
      
      if (i < burstCount)
        {
          app->SetBurstParams (Seconds (burstStart), Seconds (burstStop), queueSize);
        }
      else
        {
          app->SetBurstParams (Seconds (simTime + 1), Seconds (simTime + 2), 0); // Never burst
        }
      
      node->AddApplication (app);
      app->SetStartTime (Seconds (0.5));
      app->SetStopTime (Seconds (simTime));
    }

  Simulator::Stop (Seconds (simTime + 1));
  Simulator::Run ();
  Simulator::Destroy ();

  // Metrics
  LoraPacketTracker &tracker = helper.GetPacketTracker ();
  std::cout << "--- Results ---" << std::endl;
  std::cout << tracker.CountMacPacketsGlobally (Seconds (0), Seconds (simTime + 1)) << std::endl;

  return 0;
}
