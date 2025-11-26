#/*******************************************************************************
#  burstmac-simulation.cc
#
#  Example simulation that demonstrates the Burst-MAC prototype. The file
#  builds a simple LoRaWAN network (end devices + gateways + network server)
#  and hooks a small centralized `BurstScheduler` plus `BurstNodeApp` instances
#  on each end device to emulate bursty traffic using slot assignments.
#
#  To run (from ns-3 workspace):
#    ./ns3 run "burstmac-simulation --nNodes=200 --burstPercent=50"
#
#*******************************************************************************/

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/lorawan-module.h"
#include "ns3/mobility-module.h"
#include "ns3/forwarder-helper.h"
#include "ns3/network-server-helper.h"
#include "ns3/basic-energy-source-helper.h"
#include "ns3/lora-radio-energy-model-helper.h"

#include "../src/burst-mac-helper.h"
#include "../src/burst-scheduler.h"
#include "../src/burst-node-app.h"

using namespace ns3;
using namespace lorawan;

NS_LOG_COMPONENT_DEFINE ("BurstMacSimulation");

// Global pointer to scheduler for the callback (kept global for easy access
// from the network-server trace callback in this small example).
Ptr<BurstScheduler> g_scheduler;

// Callback invoked by the NetworkServer when it receives an uplink packet.
// The scheduler inspects burst-related tags and may assign slots.
void
OnServerReceivedPacket (Ptr<const Packet> packet)
{
  if (g_scheduler)
    {
      g_scheduler->OnUplink (packet);
    }
}

int
main (int argc, char *argv[])
{
  // Default Parameters
  int nNodes = 200;
  int nGateways = 1;
  double radius = 2000.0;
  double simTime = 120.0;
  int burstPercent = 50;
  double burstStart = 20.0;
  double burstStop = 100.0;
  int nChannels = 8; // Not directly used in this simplified PHY setup but good for config

  CommandLine cmd;
  cmd.AddValue ("nNodes", "Number of end devices", nNodes);
  cmd.AddValue ("nGateways", "Number of gateways", nGateways);
  cmd.AddValue ("burstPercent", "Percentage of nodes to burst", burstPercent);
  cmd.AddValue ("nChannels", "Number of virtual channels (informational)", nChannels);
  cmd.Parse (argc, argv);

  // Logging
  LogComponentEnable ("BurstMacSimulation", LOG_LEVEL_INFO);
  LogComponentEnable ("BurstScheduler", LOG_LEVEL_INFO);
  LogComponentEnable ("BurstNodeApp", LOG_LEVEL_INFO);

  // 1. Mobility & Channel
  // Set up node positions and a simple propagation model used by the Lora PHY.
  MobilityHelper mobility;
  mobility.SetPositionAllocator ("ns3::UniformDiscPositionAllocator",
                                 "rho", DoubleValue (radius),
                                 "X", DoubleValue (0.0),
                                 "Y", DoubleValue (0.0));
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");

  Ptr<LogDistancePropagationLossModel> loss = CreateObject<LogDistancePropagationLossModel> ();
  loss->SetPathLossExponent (3.76);
  loss->SetReference (1, 7.7);
  Ptr<PropagationDelayModel> delay = CreateObject<ConstantSpeedPropagationDelayModel> ();
  Ptr<LoraChannel> channel = CreateObject<LoraChannel> (loss, delay);

  // 2. Helpers
  LoraPhyHelper phyHelper;
  phyHelper.SetChannel (channel);
  LorawanMacHelper macHelper;
  LoraHelper helper;
  helper.EnablePacketTracking ();

  // 3. Create Nodes
  NodeContainer endDevices;
  endDevices.Create (nNodes);
  mobility.Install (endDevices);

  phyHelper.SetDeviceType (LoraPhyHelper::ED);
  macHelper.SetDeviceType (LorawanMacHelper::ED_A);
  NetDeviceContainer endDevicesNet = helper.Install (phyHelper, macHelper, endDevices);

  // 4. Create Gateways
  NodeContainer gateways;
  gateways.Create (nGateways);
  Ptr<ListPositionAllocator> gwAlloc = CreateObject<ListPositionAllocator> ();
  gwAlloc->Add (Vector (0.0, 0.0, 15.0)); // Central GW
  mobility.SetPositionAllocator (gwAlloc);
  mobility.Install (gateways);

  phyHelper.SetDeviceType (LoraPhyHelper::GW);
  macHelper.SetDeviceType (LorawanMacHelper::GW);
  NetDeviceContainer gwDevs = helper.Install (phyHelper, macHelper, gateways);

  // INSTALL ENERGY MODEL
  BasicEnergySourceHelper basicSourceHelper;
  basicSourceHelper.Set ("BasicEnergySourceInitialEnergyJ", DoubleValue (10000.0));
  basicSourceHelper.Set ("BasicEnergySupplyVoltageV", DoubleValue (3.3));

  LoraRadioEnergyModelHelper radioEnergyHelper;
  radioEnergyHelper.Set ("StandbyCurrentA", DoubleValue (0.0014));
  radioEnergyHelper.Set ("TxCurrentA", DoubleValue (0.028));
  radioEnergyHelper.Set ("SleepCurrentA", DoubleValue (0.0000015));
  radioEnergyHelper.Set ("RxCurrentA", DoubleValue (0.0112));

  // Install energy sources and device models the same way as other lorawan examples
  EnergySourceContainer sources = basicSourceHelper.Install (endDevices);
  DeviceEnergyModelContainer deviceModels = radioEnergyHelper.Install (endDevicesNet, sources);

  LorawanMacHelper::SetSpreadingFactorsUp (endDevices, gateways, channel);

  // 5. Network Server
  // Network Server setup (use P2P registration as in lorawan examples)
  Ptr<Node> nsNode = CreateObject<Node> ();
  NetworkServerHelper nsHelper;
  PointToPointHelper p2p;
  p2p.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("2ms"));

  P2PGwRegistration_t gwRegistration;
  for (uint32_t i = 0; i < gateways.GetN (); ++i)
    {
      NetDeviceContainer devs = p2p.Install (nsNode, gateways.Get (i));
      Ptr<PointToPointNetDevice> serverDev = DynamicCast<PointToPointNetDevice> (devs.Get (0));
      gwRegistration.emplace_back (serverDev, gateways.Get (i));
    }

  nsHelper.SetGatewaysP2P (gwRegistration);
  nsHelper.SetEndDevices (endDevices);
  ApplicationContainer nsApps = nsHelper.Install (nsNode);

  ForwarderHelper forHelper;
  forHelper.Install (gateways);

  // 6. Burst-MAC Components
  g_scheduler = CreateObject<BurstScheduler> ();
  
  // Hook Scheduler to Server Trace
  Ptr<NetworkServer> ns = nsApps.Get (0)->GetObject<NetworkServer> ();
  ns->TraceConnectWithoutContext ("ReceivedPacket", MakeCallback (&OnServerReceivedPacket));

  // Install Node Apps
  BurstMacHelper burstHelper;
  burstHelper.SetScheduler (g_scheduler);
  
  // Normal: 60s, Burst Slot: 50ms (High throughput)
  ApplicationContainer nodeApps = burstHelper.Install (endDevices, Seconds (60), Seconds (0.05));
  
  nodeApps.Start (Seconds (1.0));
  nodeApps.Stop (Seconds (simTime));

  // 7. Schedule Burst Events
  int burstCount = nNodes * burstPercent / 100;
  NS_LOG_INFO ("Scheduling Burst for " << burstCount << " nodes from " << burstStart << "s to " << burstStop << "s");

  for (int i = 0; i < burstCount; ++i)
    {
      Ptr<BurstNodeApp> app = nodeApps.Get (i)->GetObject<BurstNodeApp> ();
      Simulator::Schedule (Seconds (burstStart), &BurstNodeApp::TriggerBurst, app);
      Simulator::Schedule (Seconds (burstStop), &BurstNodeApp::StopBurst, app);
    }

  // 8. Run
  Simulator::Stop (Seconds (simTime + 10));
  Simulator::Run ();
  Simulator::Destroy ();

  // 9. Metrics
  LoraPacketTracker &tracker = helper.GetPacketTracker ();
  std::cout << "\n--- Burst-MAC Results ---\n";
  std::cout << tracker.CountMacPacketsGlobally (Seconds (0), Seconds (simTime + 10)) << std::endl;

  return 0;
}
