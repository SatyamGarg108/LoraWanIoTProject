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
  // very simple and not-very-clean implementation
  BurstMacTag tag;
  if (! packet->PeekPacketTag (tag))
    return; // nothing to do

  if (! tag.is_burst ())
    return; // not burst

  uint32_t node = tag.src ();

  lorawan::LoraTag ltag;
  VirtualChannel vc = {0, 0.0};
  if (packet->PeekPacketTag (ltag)) {
    vc.sf = ltag.GetDataRate ();
    vc.freq = ltag.GetFrequency ();
  }

  if (vc_map.find (vc) == vc_map.end ()) {
    VCState st;
    vc_map[vc] = st;
  }

  VCState &st = vc_map[vc];
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
