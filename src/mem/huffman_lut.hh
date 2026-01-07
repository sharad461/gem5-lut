#ifndef __MEM_HUFFMAN_LUT_HH__
#define __MEM_HUFFMAN_LUT_HH__

#include <unordered_map>
#include "mem/port.hh"
#include "params/HuffmanLUT.hh"
#include "sim/sim_object.hh"

namespace gem5
{

class HuffmanLUT : public SimObject
{
  private:
    class LUTPort : public ResponsePort
    {
      private:
        HuffmanLUT* owner;
        bool rangeValid;

      public:
        LUTPort(const std::string& name, HuffmanLUT* owner)
            : ResponsePort(name), owner(owner), rangeValid(false) {}

        AddrRangeList getAddrRanges() const override;

        Tick recvAtomic(PacketPtr pkt) override;
        void recvFunctional(PacketPtr pkt) override;
        bool recvTimingReq(PacketPtr pkt) override;
        void recvRespRetry() override;

        void setRangeValid() { rangeValid = true; }
    };

    LUTPort port;
    const Cycles latency;
    const AddrRange addrRange;

    // The actual lookup table: encoded -> decoded
    std::unordered_map<uint32_t, uint32_t> lookupTable;

    // Pending responses for timing mode
    std::deque<PacketPtr> responseQueue;
    EventFunctionWrapper sendResponseEvent;

    void sendResponse();
    void handleRequest(PacketPtr pkt);
    void initializeLUT();

  public:
    HuffmanLUT(const HuffmanLUTParams &params);
    ~HuffmanLUT();

    Port& getPort(const std::string& if_name, PortID idx = InvalidPortID) override;
    void init() override;
};

} // namespace gem5

#endif // __MEM_HUFFMAN_LUT_HH__
