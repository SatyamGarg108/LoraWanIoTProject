#include "ns3/point-to-point-helper.h"
#include "ns3/lora-helper.h"
#include "ns3/mobility-helper.h"
#include "ns3/node-container.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/command-line.h"
#include "ns3/random-variable-stream.h"
#include "ns3/network-server-helper.h"
#include "ns3/forwarder-helper.h"
#include "ns3/periodic-sender-helper.h"

#include <iostream>
#include <vector>

using namespace ns3;
using namespace lorawan;

NS_LOG_COMPONENT_DEFINE("LoraPdrSimulation");

uint64_t packetsSent = 0;
uint64_t packetsReceived = 0;

void OnTransmissionCallback(Ptr<const Packet> packet, uint32_t senderNodeId)
{
    NS_LOG_INFO("Packet sent by node " << senderNodeId);
    packetsSent++;
}

void OnPacketReceptionCallback(Ptr<const Packet> packet, uint32_t receiverNodeId)
{
    NS_LOG_INFO("Packet received by gateway " << receiverNodeId);
    packetsReceived++;
}

int main(int argc, char* argv[])
{
    int nNodes = 20;
    int nGateways = 2;
    double radiusMeters = 2000;
    double simulationTimeSeconds = 70.0;
    Time appStopTime = Seconds(simulationTimeSeconds);

    CommandLine cmd(__FILE__);
    cmd.AddValue("nNodes", "Number of end devices", nNodes);
    cmd.AddValue("x", "Number of gateways", nGateways);
    cmd.AddValue("radius", "Radius of the deployment area in meters", radiusMeters);
    cmd.Parse(argc, argv);

    LogComponentEnable("LoraPdrSimulation", LOG_LEVEL_INFO);

    LoraPhyHelper phyHelper = LoraPhyHelper();
    LorawanMacHelper macHelper = LorawanMacHelper();
    LoraHelper helper = LoraHelper();
    MobilityHelper mobility;

    Ptr<LogDistancePropagationLossModel> loss = CreateObject<LogDistancePropagationLossModel>();
    loss->SetPathLossExponent(3.76);
    loss->SetReference(1, 7.7);

    Ptr<PropagationDelayModel> delay = CreateObject<ConstantSpeedPropagationDelayModel>();
    Ptr<LoraChannel> channel = CreateObject<LoraChannel>(loss, delay);
    
    phyHelper.SetChannel(channel);
    helper.EnablePacketTracking();

    NodeContainer endDevices;
    endDevices.Create(nNodes);

    mobility.SetPositionAllocator("ns3::UniformDiscPositionAllocator",
                                  "rho", DoubleValue(radiusMeters),
                                  "X", DoubleValue(0.0),
                                  "Y", DoubleValue(0.0));
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(endDevices);

    phyHelper.SetDeviceType(LoraPhyHelper::ED);
    macHelper.SetDeviceType(LorawanMacHelper::ED_A);
    helper.Install(phyHelper, macHelper, endDevices);

    NodeContainer gateways;
    gateways.Create(nGateways);

    Ptr<ListPositionAllocator> gwAllocator = CreateObject<ListPositionAllocator>();
    gwAllocator->Add(Vector(0.0, 1000.0, 15.0));
    gwAllocator->Add(Vector(0.0, -1000.0, 15.0));
    mobility.SetPositionAllocator(gwAllocator);
    mobility.Install(gateways);

    phyHelper.SetDeviceType(LoraPhyHelper::GW);
    macHelper.SetDeviceType(LorawanMacHelper::GW);
    helper.Install(phyHelper, macHelper, gateways);

    LorawanMacHelper::SetSpreadingFactorsUp(endDevices, gateways, channel);

    NetworkServerHelper nsHelper = NetworkServerHelper();
    ForwarderHelper forHelper = ForwarderHelper();
    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    p2p.SetChannelAttribute("Delay", StringValue("2ms"));
    
    Ptr<Node> networkServer = CreateObject<Node>();
    P2PGwRegistration_t gwRegistration;
    for (uint32_t i = 0; i < gateways.GetN(); ++i)
    {
        auto container = p2p.Install(networkServer, gateways.Get(i));
        auto serverP2PNetDev = DynamicCast<PointToPointNetDevice>(container.Get(0));
        gwRegistration.emplace_back(serverP2PNetDev, gateways.Get(i));
    }

    nsHelper.SetGatewaysP2P(gwRegistration);
    nsHelper.SetEndDevices(endDevices);
    nsHelper.Install(networkServer);
    forHelper.Install(gateways);

    PeriodicSenderHelper appHelper = PeriodicSenderHelper();
    appHelper.SetPeriod(Seconds(6));
    appHelper.SetPacketSize(24);

    ApplicationContainer appContainer = appHelper.Install(endDevices);
    appContainer.Start(Time(0));
    appContainer.Stop(appStopTime);

    /*for (auto node = endDevices.Begin(); node != endDevices.End(); node++)
    {
        DynamicCast<LoraNetDevice>((*node)->GetDevice(0))
            ->GetPhy()
            ->TraceConnectWithoutContext("StartSending", MakeCallback(OnTransmissionCallback));
    }

    for (auto node = gateways.Begin(); node != gateways.End(); node++)
    {
        DynamicCast<LoraNetDevice>((*node)->GetDevice(0))
            ->GetPhy()
            ->TraceConnectWithoutContext("ReceivedPacket", MakeCallback(OnPacketReceptionCallback));
    }*/

    Simulator::Stop(appStopTime + Hours(1));
    NS_LOG_INFO("Running simulation...");
    Simulator::Run();
    Simulator::Destroy();

    /*double pdr = 0.0;
    if (packetsSent > 0)
    {
        pdr = (double)packetsReceived / packetsSent;
    }

    std::cout << "\n--- Simulation Results ---" << std::endl;
    std::cout << "Total packets sent: " << packetsSent << std::endl;
    std::cout << "Total packets received: " << packetsReceived << std::endl;
    std::cout << "Packet Delivery Ratio (PDR): " << pdr * 100.0 << "%" << std::endl;
    std::cout << "--------------------------" << std::endl;*/

    LoraPacketTracker& tracker = helper.GetPacketTracker(); 
    std::cout << tracker.CountMacPacketsGlobally(Seconds(0), appStopTime + Hours(1)) << std::endl;

    //improvement:
    //1. Gateway will not drop packet
    //2. Assigning SF based on signal quality
    //3. Error correction

    //LogComponentEnable("LoraChannel", LOG_LEVEL_INFO);
    LogComponentEnable("LoraPhy", LOG_LEVEL_INFO);
    //LogComponentEnable("EndDeviceLoraPhy", LOG_LEVEL_ALL);
    //LogComponentEnable("GatewayLoraPhy", LOG_LEVEL_ALL);
    //LogComponentEnable("LoraInterferenceHelper", LOG_LEVEL_ALL);
    //LogComponentEnable("LorawanMac", LOG_LEVEL_ALL);
    //LogComponentEnable("EndDeviceLorawanMac", LOG_LEVEL_ALL);
    //LogComponentEnable("ClassAEndDeviceLorawanMac", LOG_LEVEL_ALL);
    //LogComponentEnable("GatewayLorawanMac", LOG_LEVEL_ALL);
    //LogComponentEnable("LogicalLoraChannelHelper", LOG_LEVEL_ALL);
    //LogComponentEnable("LogicalLoraChannel", LOG_LEVEL_ALL);
    //LogComponentEnable("LoraHelper", LOG_LEVEL_ALL);
    //LogComponentEnable("LoraPhyHelper", LOG_LEVEL_ALL);
    //LogComponentEnable("LorawanMacHelper", LOG_LEVEL_ALL);
    //LogComponentEnable("OneShotSenderHelper", LOG_LEVEL_ALL);
    //LogComponentEnable("OneShotSender", LOG_LEVEL_ALL);
    //LogComponentEnable("LorawanMacHeader", LOG_LEVEL_ALL);
    //LogComponentEnable("LoraFrameHeader", LOG_LEVEL_ALL);

    //LogComponentEnableAll(LOG_PREFIX_FUNC);
    //LogComponentEnableAll(LOG_PREFIX_NODE);
    //LogComponentEnableAll(LOG_PREFIX_TIME);

    return 0;
}