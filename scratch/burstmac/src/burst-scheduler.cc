// Implementation of the BurstScheduler: groups bursting nodes by a lightweight
// 'VirtualChannel' key and assigns simple incremental slots. This code is
// intentionally straightforward and suitable for demonstration and testing.

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
BurstScheduler::OnUplink (Ptr<const Packet> packet)
{
  // Inspect the packet for the BurstMacTag, which indicates a node wanting
  // to join the burst group and be assigned a slot.
  BurstMacTag tag;
  if (! packet->PeekPacketTag (tag))
    return; // nothing to do

  if (! tag.is_burst ())
    return; // not a burst registration

  uint32_t node = tag.src ();

  // Use lorawan::LoraTag (if present) to derive a simple VirtualChannel
  // key (data rate + frequency) so that nodes on different channels are
  // scheduled independently.
  lorawan::LoraTag ltag;
  VirtualChannel vc = {0, 0.0};
  if (packet->PeekPacketTag (ltag)) {
    vc.sf = ltag.GetDataRate ();
    vc.freq = ltag.GetFrequency ();
  }

  // Ensure a VCState exists for this channel
  if (vc_map.find (vc) == vc_map.end ()) {
    VCState st;
    vc_map[vc] = st;
  }

  VCState &st = vc_map[vc];

  // If this node is new in the VC, assign it the next free slot
  bool found = false;
  for (size_t i = 0; i < st.nodeIds.size (); ++i) {
    if (st.nodeIds[i] == node) { found = true; break; }
  }

  if (! found) {
    st.nodeIds.push_back (node);
    uint32_t gsize = (uint32_t) st.nodeIds.size ();
    uint32_t initial = (gsize > 0) ? (node % gsize) : 0;
    uint32_t finalslot = st.nextFreeSlot;
    st.nextFreeSlot = st.nextFreeSlot + 1;
    st.slotAssignments[node] = finalslot;
    node_slot_map[node] = finalslot;
    NS_LOG_INFO ("(simple) node " << node << " hash=" << initial << " assigned=" << finalslot);
  }
}

uint32_t
BurstScheduler::GetAssignedSlot (uint32_t nodeId)
{
  auto it = node_slot_map.find (nodeId);
  if (it != node_slot_map.end ()) {
    return it->second;
  }
  return UINT32_MAX;
}

} // namespace ns3
