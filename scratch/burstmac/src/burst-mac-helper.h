#ifndef BURST_MAC_HELPER_H
#define BURST_MAC_HELPER_H

#include "ns3/node-container.h"
#include "ns3/application-container.h"
#include "ns3/end-device-lorawan-mac.h"
#include "burst-scheduler.h"

namespace ns3 {

class BurstMacHelper
{
public:
  BurstMacHelper ();
  
  void SetScheduler (Ptr<BurstScheduler> scheduler);
  
  ApplicationContainer Install (NodeContainer c, Time normalInterval, Time burstInterval);

private:
  Ptr<BurstScheduler> m_scheduler;
};

} // namespace ns3

#endif /* BURST_MAC_HELPER_H */
