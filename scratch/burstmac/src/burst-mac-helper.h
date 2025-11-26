// File: burst-mac-helper.h
// Purpose: Small ns-3 helper to install the BurstNodeApp on a set of nodes
// and provide the global BurstScheduler to the applications.

#ifndef BURST_MAC_HELPER_H
#define BURST_MAC_HELPER_H

#include "ns3/node-container.h"
#include "ns3/application-container.h"
#include "ns3/end-device-lorawan-mac.h"
#include "burst-scheduler.h"

namespace ns3 {

/**
 * Convenience helper that installs `BurstNodeApp` on end devices and wires
 * the global `BurstScheduler` to each app instance. The helper mirrors the
 * typical ns-3 pattern for application installers.
 */
class BurstMacHelper
{
public:
  BurstMacHelper ();
  
  // Provide the scheduler that nodes will consult (simulates server-driven assignments)
  void SetScheduler (Ptr<BurstScheduler> scheduler);
  
  // Install the BurstNodeApp on a container of nodes. `normalInterval` is
  // the average inter-packet time in normal mode; `burstInterval` is the
  // slot duration used when the node is in burst mode.
  ApplicationContainer Install (NodeContainer c, Time normalInterval, Time burstInterval);

private:
  Ptr<BurstScheduler> m_scheduler;
};

} // namespace ns3

#endif /* BURST_MAC_HELPER_H */
