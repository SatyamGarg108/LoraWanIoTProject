// Implementation of the BurstNodeApp. The app alternates between a normal
// randomized transmit mode and a slot-based burst mode. When entering burst
// mode it registers with the (simulated) scheduler and polls for a slot
// assignment; once assigned it transmits in the assigned slot during each
// superframe.

#include "burst-node-app.h"
#include "burst-mac-tag.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/double.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("BurstNodeApp");
NS_OBJECT_ENSURE_REGISTERED (BurstNodeApp);

TypeId
BurstNodeApp::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::BurstNodeApp")
    .SetParent<Application> ()
    .AddConstructor<BurstNodeApp> ();
  return tid;
}

BurstNodeApp::BurstNodeApp ()
  : m_mac (0),
    m_scheduler (0),
    m_normalInterval (Seconds (0)),
    m_burstInterval (Seconds (0)),
    m_superframeDuration (Seconds (1.0)),
    m_isBurst (false),
    m_nodeId (0),
    m_assignedSlot (UINT32_MAX)
{
  // random scalar used to jitter normal-mode transmissions
  m_random = CreateObject<UniformRandomVariable> ();
}

BurstNodeApp::~BurstNodeApp ()
{
}

void
BurstNodeApp::Setup (Ptr<lorawan::EndDeviceLorawanMac> mac, Ptr<BurstScheduler> scheduler,
                     Time normalInterval, Time burstInterval)
{
  m_mac = mac;
  m_scheduler = scheduler;
  m_normalInterval = normalInterval;
  m_burstInterval = burstInterval; // Slot Duration
  m_nodeId = mac->GetDevice ()->GetNode ()->GetId ();
}

void
BurstNodeApp::StartApplication (void)
{
  // Begin in normal mode with periodic transmissions
  ScheduleNextTx ();
}

void
BurstNodeApp::StopApplication (void)
{
  Simulator::Cancel (m_sendEvent);
  Simulator::Cancel (m_slotEvent);
}

void
BurstNodeApp::TriggerBurst (void)
{
  if (m_isBurst) return;
  m_isBurst = true;
  NS_LOG_INFO ("Node " << m_nodeId << " Entering BURST Mode");
  
  // Cancel any pending normal-mode transmission and register for burst
  Simulator::Cancel (m_sendEvent);
  
  // Send Registration Packet immediately so the scheduler can observe this node
  SendPacket ();
  
  // Start polling for slot assignment (Simulates waiting for Downlink)
  m_slotEvent = Simulator::Schedule (Seconds (0.5), &BurstNodeApp::SlotLoop, this);
}

void
BurstNodeApp::StopBurst (void)
{
  if (!m_isBurst) return;
  m_isBurst = false;
  m_assignedSlot = UINT32_MAX;
  NS_LOG_INFO ("Node " << m_nodeId << " Leaving BURST Mode");
  
  Simulator::Cancel (m_slotEvent);
  ScheduleNextTx ();
}

void
BurstNodeApp::ScheduleNextTx (void)
{
  if (m_isBurst) return; // Handled by SlotLoop when bursting

  // Normal Mode: Random Interval around the configured mean
  double scalar = m_random->GetValue (0.8, 1.2);
  Time delay = Seconds (m_normalInterval.GetSeconds () * scalar);
  m_sendEvent = Simulator::Schedule (delay, &BurstNodeApp::SendPacket, this);
}

void
BurstNodeApp::SendPacket (void)
{
  Ptr<Packet> packet = Create<Packet> (20); // Payload size (bytes)
  
  // Attach a BurstMacTag so the server/scheduler can detect burst-capable nodes
  BurstMacTag tag;
  tag.set_burst (m_isBurst);
  tag.set_src (m_nodeId);
  packet->AddPacketTag (tag);
  
  m_mac->Send (packet);
  
  if (!m_isBurst)
    {
      // Continue normal operation scheduling
      ScheduleNextTx ();
    }
}

void
BurstNodeApp::SlotLoop (void)
{
  if (!m_isBurst) return;

  // Check for assignment from the scheduler. If not assigned yet, schedule
  // another check shortly. When assigned, align to the next superframe tick.
  if (m_assignedSlot == UINT32_MAX)
    {
      uint32_t slot = m_scheduler->GetAssignedSlot (m_nodeId);
      if (slot != UINT32_MAX)
        {
          m_assignedSlot = slot;
          NS_LOG_INFO ("Node " << m_nodeId << " Received Slot Assignment: " << m_assignedSlot);
          // Align to next superframe
          Simulator::ScheduleNow (&BurstNodeApp::SuperframeTick, this);
          return;
        }
      // Retry later if still unassigned
      m_slotEvent = Simulator::Schedule (Seconds (0.1), &BurstNodeApp::SlotLoop, this);
    }
}

void
BurstNodeApp::SuperframeTick (void)
{
  if (!m_isBurst) return;

  // Compute the offset within the superframe for the assigned slot and
  // schedule a SendInSlot at that time. Then schedule the next superframe.
  Time offset = m_burstInterval * m_assignedSlot;
  
  // Schedule transmission in this slot
  Simulator::Schedule (offset, &BurstNodeApp::SendInSlot, this);
  
  // Schedule next Superframe tick
  m_slotEvent = Simulator::Schedule (m_superframeDuration, &BurstNodeApp::SuperframeTick, this);
}

void
BurstNodeApp::SendInSlot (void)
{
  if (!m_isBurst) return;
  SendPacket ();
}

} // namespace ns3
