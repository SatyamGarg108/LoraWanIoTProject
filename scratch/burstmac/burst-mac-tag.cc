#include "burst-mac-tag.h"
#include "ns3/log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("BurstMacTag");
NS_OBJECT_ENSURE_REGISTERED (BurstMacTag);

TypeId
BurstMacTag::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::BurstMacTag")
    .SetParent<Tag> ()
    .AddConstructor<BurstMacTag> ();
  return tid;
}

TypeId
BurstMacTag::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
BurstMacTag::GetSerializedSize (void) const
{
  // 1 byte flags (Request, HasSlot), 4 bytes Slot, 4 bytes NodeId
  return 1 + 4 + 4;
}

void
BurstMacTag::Serialize (TagBuffer i) const
{
  uint8_t flags = 0;
  if (m_burstRequest) flags |= 0x01;
  if (m_hasSlot) flags |= 0x02;
  i.WriteU8 (flags);
  i.WriteU32 (m_slot);
  i.WriteU32 (m_sourceNodeId);
}

void
BurstMacTag::Deserialize (TagBuffer i)
{
  uint8_t flags = i.ReadU8 ();
  m_burstRequest = (flags & 0x01);
  m_hasSlot = (flags & 0x02);
  m_slot = i.ReadU32 ();
  m_sourceNodeId = i.ReadU32 ();
}

void
BurstMacTag::Print (std::ostream &os) const
{
  os << "BurstReq=" << m_burstRequest 
     << " HasSlot=" << m_hasSlot 
     << " Slot=" << m_slot
     << " Src=" << m_sourceNodeId;
}

void BurstMacTag::SetBurstRequest (bool req) { m_burstRequest = req; }
bool BurstMacTag::GetBurstRequest () const { return m_burstRequest; }

void BurstMacTag::SetSlotAssignment (uint32_t slot) { m_hasSlot = true; m_slot = slot; }
uint32_t BurstMacTag::GetSlotAssignment () const { return m_slot; }
bool BurstMacTag::HasSlotAssignment () const { return m_hasSlot; }

void BurstMacTag::SetSourceNodeId (uint32_t id) { m_sourceNodeId = id; }
uint32_t BurstMacTag::GetSourceNodeId () const { return m_sourceNodeId; }

} // namespace ns3
