// Helper implementation: installs BurstNodeApp on nodes and wires up the scheduler

#include "burst-mac-helper.h"
#include "burst-node-app.h"
#include "ns3/lora-net-device.h"

namespace ns3 {

BurstMacHelper::BurstMacHelper ()
{
}

void
BurstMacHelper::SetScheduler (Ptr<BurstScheduler> scheduler)
{
  // store the scheduler instance so that installed apps can query it
  m_scheduler = scheduler;
}

ApplicationContainer
BurstMacHelper::Install (NodeContainer c, Time normalInterval, Time burstInterval)
{
  ApplicationContainer apps;
  // Iterate through nodes and create a BurstNodeApp on each.
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      Ptr<Node> node = *i;

      // The example expects the first device to be the LoRa device.
      Ptr<lorawan::LoraNetDevice> loraDev = node->GetDevice (0)->GetObject<lorawan::LoraNetDevice> ();
      Ptr<lorawan::EndDeviceLorawanMac> mac = loraDev->GetMac ()->GetObject<lorawan::EndDeviceLorawanMac> ();

      // Create and configure the node-side application
      Ptr<BurstNodeApp> app = CreateObject<BurstNodeApp> ();
      app->Setup (mac, m_scheduler, normalInterval, burstInterval);
      node->AddApplication (app);
      apps.Add (app);
    }
  return apps;
}

} // namespace ns3
