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
  m_scheduler = scheduler;
}

ApplicationContainer
BurstMacHelper::Install (NodeContainer c, Time normalInterval, Time burstInterval)
{
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      Ptr<Node> node = *i;
      Ptr<lorawan::LoraNetDevice> loraDev = node->GetDevice (0)->GetObject<lorawan::LoraNetDevice> ();
      Ptr<lorawan::EndDeviceLorawanMac> mac = loraDev->GetMac ()->GetObject<lorawan::EndDeviceLorawanMac> ();

      Ptr<BurstNodeApp> app = CreateObject<BurstNodeApp> ();
      app->Setup (mac, m_scheduler, normalInterval, burstInterval);
      node->AddApplication (app);
      apps.Add (app);
    }
  return apps;
}

} // namespace ns3
