#ifndef BURST_MAC_APP_H
#define BURST_MAC_APP_H

#include "ns3/application.h"
#include "ns3/nstime.h"
#include "ns3/random-variable-stream.h"
#include "ns3/end-device-lorawan-mac.h"
#include "burst-scheduler.h"

namespace ns3 {

/**
 * @brief Application that simulates Burst-MAC behavior on end devices.
 * Handles burst detection, beacon synchronization (simulated), and slot-based transmission.
 */
class BurstMacApp : public Application
{
public:
  static TypeId GetTypeId (void);
  BurstMacApp ();
  virtual ~BurstMacApp ();

  void Setup (Ptr<lorawan::EndDeviceLorawanMac> mac, Ptr<BurstScheduler> scheduler);
  void SetBurstParams (Time burstStart, Time burstStop, uint32_t queueSize);

protected:
  virtual void StartApplication (void);
  virtual void StopApplication (void);

private:
  void SendNormal ();
  void StartBurst ();
  void StopBurst ();
  void SlotLoop ();
  void SuperframeTick ();
  void SendInSlot ();

  Ptr<lorawan::EndDeviceLorawanMac> m_mac;
  Ptr<BurstScheduler> m_scheduler; // Pointer to server scheduler for simulated piggyback

  EventId m_sendEvent;
  EventId m_slotEvent;

  // Params
  Time m_normalInterval;
  Time m_burstStart;
  Time m_burstStop;
  uint32_t m_queueSize;
  uint32_t m_initialQueueSize;
  
  // State
  bool m_isBurst;
  uint32_t m_nodeId;
  uint32_t m_assignedSlot;
  
  // Class B / TDMA params
  Time m_superframeDuration;
  Time m_slotDuration;
  
  Ptr<UniformRandomVariable> m_rng;
};

} // namespace ns3

#endif /* BURST_MAC_APP_H */
