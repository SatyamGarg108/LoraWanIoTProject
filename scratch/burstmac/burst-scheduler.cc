#include "burst-scheduler.h"
#include "burst-mac-tag.h"
#include "ns3/log.h"
#include "ns3/lora-tag.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("BurstScheduler");
NS_OBJECT_ENSURE_REGISTERED (BurstScheduler);

TypeId
BurstScheduler::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::BurstScheduler")
    .SetParent<Object> ()
    .AddConstructor<BurstScheduler> ();
  return tid;
}

BurstScheduler::BurstScheduler ()
{
}

BurstScheduler::~BurstScheduler ()
{
}

void
BurstScheduler::OnReceivedPacket (Ptr<const Packet> packet)
{
  BurstMacTag btag;
  if (!packet->PeekPacketTag (btag))
    {
      return;
    }

  if (!btag.GetBurstRequest ())
    {
      return;
    }

  uint32_t nodeId = btag.GetSourceNodeId ();
  
  // Extract PHY info for VC grouping
  lorawan::LoraTag ltag;
  VirtualChannelKey key = {0, 0.0};
  if (packet->PeekPacketTag (ltag))
    {
      key.sf = ltag.GetDataRate ();
      key.freq = ltag.GetFrequency ();
    }
  
  // Find or create VC state
  if (m_vcStates.find (key) == m_vcStates.end ())
    {
      NS_LOG_INFO ("New VC detected: SF" << (int)key.sf << " Freq" << key.freq);
      m_vcStates[key] = VCState ();
    }
  
  VCState &state = m_vcStates[key];
  
  // Check if node already assigned
  if (state.assignments.find (nodeId) == state.assignments.end ())
    {
      // Hash-based initial slot (simple modulo)
      // In a real system, this might be derived from DevAddr
      // Here we use sequential assignment for collision resolution
      uint32_t slot = state.nextFreeSlot++;
      state.assignments[nodeId] = slot;
      state.nodeIds.push_back (nodeId);
      
      // Store in global map for "piggyback" simulation
      m_globalAssignments[nodeId] = slot;
      
      NS_LOG_INFO ("Assigned Slot " << slot << " to Node " << nodeId << " on VC(SF" << (int)key.sf << ")");
    }
}

uint32_t
BurstScheduler::GetAssignedSlot (uint32_t nodeId)
{
  auto it = m_globalAssignments.find (nodeId);
  if (it != m_globalAssignments.end ())
    {
      return it->second;
    }
  return UINT32_MAX;
}

} // namespace ns3
