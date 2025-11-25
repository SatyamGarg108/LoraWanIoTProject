#include "burst-mac-app.h"
#include "burst-mac-tag.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/packet.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("BurstMacApp");
NS_OBJECT_ENSURE_REGISTERED (BurstMacApp);

TypeId
BurstMacApp::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::BurstMacApp")
    .SetParent<Application> ()
    .AddConstructor<BurstMacApp> ();
  return tid;
}

BurstMacApp::BurstMacApp ()
  : m_normalInterval (Seconds (60)),
    m_queueSize (0),
    m_isBurst (false),
    m_nodeId (0),
    m_assignedSlot (UINT32_MAX),
    m_superframeDuration (Seconds (1.0)),
    m_slotDuration (Seconds (0.050)) // 50ms slots
{
  m_rng = CreateObject<UniformRandomVariable> ();
}

BurstMacApp::~BurstMacApp ()
{
}

void
BurstMacApp::Setup (Ptr<lorawan::EndDeviceLorawanMac> mac, Ptr<BurstScheduler> scheduler)
{
  m_mac = mac;
  m_scheduler = scheduler;
  m_nodeId = mac->GetDevice ()->GetNode ()->GetId ();
}

void
BurstMacApp::SetBurstParams (Time burstStart, Time burstStop, uint32_t queueSize)
{
  m_burstStart = burstStart;
  m_burstStop = burstStop;
  m_initialQueueSize = queueSize;
  m_queueSize = queueSize;
}

void
BurstMacApp::StartApplication (void)
{
  // Schedule normal traffic
  m_sendEvent = Simulator::Schedule (Seconds (m_rng->GetValue (0, 10)), &BurstMacApp::SendNormal, this);

  // Schedule burst events
  Simulator::Schedule (m_burstStart, &BurstMacApp::StartBurst, this);
  Simulator::Schedule (m_burstStop, &BurstMacApp::StopBurst, this);
}

void
BurstMacApp::StopApplication (void)
{
  Simulator::Cancel (m_sendEvent);
  Simulator::Cancel (m_slotEvent);
}

void
BurstMacApp::SendNormal (void)
{
  if (m_isBurst) return;

  // Send a normal packet (Class A)
  Ptr<Packet> p = Create<Packet> (20);
  BurstMacTag tag;
  tag.SetSourceNodeId (m_nodeId);
  tag.SetBurstRequest (false);
  p->AddPacketTag (tag);
  
  m_mac->Send (p);

  // Schedule next normal
  m_sendEvent = Simulator::Schedule (m_normalInterval, &BurstMacApp::SendNormal, this);
}

void
BurstMacApp::StartBurst (void)
{
  NS_LOG_INFO ("Node " << m_nodeId << " Entering Burst Mode");
  m_isBurst = true;
  Simulator::Cancel (m_sendEvent); // Stop normal traffic

  // Send Burst Request (Registration)
  Ptr<Packet> p = Create<Packet> (10);
  BurstMacTag tag;
  tag.SetSourceNodeId (m_nodeId);
  tag.SetBurstRequest (true); // Request slot
  p->AddPacketTag (tag);
  m_mac->Send (p);

  // Start polling for slot assignment (simulating beacon/downlink wait)
  m_slotEvent = Simulator::Schedule (Seconds (1.0), &BurstMacApp::SlotLoop, this);
}

void
BurstMacApp::StopBurst (void)
{
  NS_LOG_INFO ("Node " << m_nodeId << " Leaving Burst Mode");
  m_isBurst = false;
  Simulator::Cancel (m_slotEvent);
  
  // Resume normal
  m_sendEvent = Simulator::Schedule (Seconds (1.0), &BurstMacApp::SendNormal, this);
}

void
BurstMacApp::SlotLoop (void)
{
  if (!m_isBurst) return;

  // Check if we have a slot
  if (m_assignedSlot == UINT32_MAX)
    {
      // Poll "piggyback" info from scheduler
      uint32_t slot = m_scheduler->GetAssignedSlot (m_nodeId);
      if (slot != UINT32_MAX)
        {
          m_assignedSlot = slot;
          NS_LOG_INFO ("Node " << m_nodeId << " Acquired Slot " << m_assignedSlot);
          // Align to superframe
          m_slotEvent = Simulator::ScheduleNow (&BurstMacApp::SuperframeTick, this);
          return;
        }
      
      // Retry later
      m_slotEvent = Simulator::Schedule (Seconds (0.5), &BurstMacApp::SlotLoop, this);
    }
}

void
BurstMacApp::SuperframeTick (void)
{
  if (!m_isBurst) return;

  // Calculate time to my slot
  // In a real Class B, this would be relative to the Beacon
  // Here we assume global synchronization for simplicity
  Time now = Simulator::Now ();
  Time superframeStart = Seconds (std::floor (now.GetSeconds () / m_superframeDuration.GetSeconds ()) * m_superframeDuration.GetSeconds ());
  Time myTxTime = superframeStart + (m_slotDuration * m_assignedSlot);
  
  if (myTxTime < now)
    {
      myTxTime += m_superframeDuration;
    }

  Simulator::Schedule (myTxTime - now, &BurstMacApp::SendInSlot, this);
  
  // Schedule next superframe check
  m_slotEvent = Simulator::Schedule (m_superframeDuration, &BurstMacApp::SuperframeTick, this);
}

void
BurstMacApp::SendInSlot (void)
{
  if (m_queueSize > 0)
    {
      Ptr<Packet> p = Create<Packet> (50); // Payload
      BurstMacTag tag;
      tag.SetSourceNodeId (m_nodeId);
      tag.SetBurstRequest (true); // Keep flag up
      p->AddPacketTag (tag);
      
      m_mac->Send (p);
      m_queueSize--;
    }
}

} // namespace ns3
