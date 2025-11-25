#ifndef BURST_MAC_TAG_H
#define BURST_MAC_TAG_H

#include "ns3/tag.h"
#include "ns3/type-id.h"

namespace ns3 {

/**
 * @brief Tag used to carry Burst-MAC control information (Burst Request, Slot Assignment).
 */
class BurstMacTag : public Tag
{
public:
  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (TagBuffer i) const;
  virtual void Deserialize (TagBuffer i);
  virtual void Print (std::ostream &os) const;

  // Accessors
  void SetBurstRequest (bool req);
  bool GetBurstRequest () const;

  void SetSlotAssignment (uint32_t slot);
  uint32_t GetSlotAssignment () const;
  bool HasSlotAssignment () const;

  void SetSourceNodeId (uint32_t id);
  uint32_t GetSourceNodeId () const;

private:
  bool m_burstRequest;
  bool m_hasSlot;
  uint32_t m_slot;
  uint32_t m_sourceNodeId;
};

} // namespace ns3

#endif /* BURST_MAC_TAG_H */
