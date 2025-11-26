// File: burst-node-app.h
// Purpose: Application that runs on end devices to support Burst-MAC behavior.
// It manages normal vs burst transmission modes, sends a registration packet
// when entering burst mode, polls the scheduler for slot assignments and
// transmits in assigned slots (superframe semantics).

#ifndef BURST_NODE_APP_H
#define BURST_NODE_APP_H

#include "ns3/application.h"
#include "ns3/end-device-lorawan-mac.h"
#include "ns3/random-variable-stream.h"
#include "burst-scheduler.h"

namespace ns3 {

/**
 * @brief Application for Burst-MAC End Device.
 *
 * Implements:
 * - Mode Switching (Normal <-> Burst)
 * - Burst Detection (Trigger)
 * - Class-B-like Synchronization (Superframe)
 * - Slot-based Transmission
 */
class BurstNodeApp : public Application
{
public:
  static TypeId GetTypeId (void);
  BurstNodeApp ();
  virtual ~BurstNodeApp ();

  /**
   * @brief Setup the application.
   * @param mac Pointer to the LoRaWAN MAC.
   * @param scheduler Pointer to the global scheduler (for simulated piggyback).
   * @param normalInterval Mean interval for normal traffic.
   * @param burstInterval Interval for burst traffic (slot duration).
   */
  void Setup (Ptr<lorawan::EndDeviceLorawanMac> mac, Ptr<BurstScheduler> scheduler,
              Time normalInterval, Time burstInterval);

  /**
   * @brief Trigger the burst mode (Simulates event detection).
   */
  void TriggerBurst (void);

  /**
   * @brief Stop the burst mode.
   */
  void StopBurst (void);

protected:
  virtual void StartApplication (void);
  virtual void StopApplication (void);

private:
  // Core behavior methods
  void SendPacket (void);
  void ScheduleNextTx (void);
  void SlotLoop (void);
  void SuperframeTick (void);
  void SendInSlot (void);

  // Pointers to MAC and scheduler
  Ptr<lorawan::EndDeviceLorawanMac> m_mac;
  Ptr<BurstScheduler> m_scheduler;
  
  // Timing parameters
  Time m_normalInterval;
  Time m_burstInterval; // Used as Slot Duration in Burst Mode
  Time m_superframeDuration;
  
  // State
  bool m_isBurst;
  uint32_t m_nodeId;
  uint32_t m_assignedSlot;
  
  // Events used to schedule transmissions and polling
  EventId m_sendEvent;
  EventId m_slotEvent;
  
  Ptr<UniformRandomVariable> m_random;
};

} // namespace ns3

#endif /* BURST_NODE_APP_H */
