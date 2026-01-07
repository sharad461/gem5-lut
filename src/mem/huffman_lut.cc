#include "mem/huffman_lut.hh"

#include "base/trace.hh"
#include "debug/HuffmanLUT.hh"
#include "mem/packet.hh"
#include "mem/packet_access.hh"

namespace gem5
{

HuffmanLUT::HuffmanLUT(const HuffmanLUTParams &params)
    : SimObject(params),
      port(params.name + ".port", this),
      latency(params.latency),
      addrRange(params.addr_range),
      sendResponseEvent([this]{ sendResponse(); }, name())
{
    initializeLUT();
}

HuffmanLUT::~HuffmanLUT()
{
}

void
HuffmanLUT::initializeLUT()
{
    // Initialize with some example Huffman mappings
    // Format: encoded (as int) -> decoded exponent (8 bits)
    // You'll replace this with your actual Huffman table

    // Example mappings (treating binary as decimal for simplicity)
    lookupTable[0b110] = 0b11101011;  // 110 -> 11101011
    lookupTable[0b111] = 0b10101010;  // 111 -> 10101010
    lookupTable[0b100] = 0b11110000;  // 100 -> 11110000
    lookupTable[0b101] = 0b00001111;  // 101 -> 00001111
    lookupTable[0b10]  = 0b10101111;  // 10  -> 10101111
    lookupTable[0b11]  = 0b01010101;  // 11  -> 01010101
    lookupTable[0b0]   = 0b00000000;  // 0   -> 00000000
    lookupTable[0b1]   = 0b11111111;  // 1   -> 11111111

    DPRINTF(HuffmanLUT, "Initialized LUT with %d entries\n", lookupTable.size());
}

Port&
HuffmanLUT::getPort(const std::string& if_name, PortID idx)
{
    if (if_name == "port") {
        return port;
    }
    return SimObject::getPort(if_name, idx);
}

void
HuffmanLUT::init()
{
    SimObject::init();

    // Tell the port it's ready
    port.setRangeValid();

    // Send range change to the connected port
    port.sendRangeChange();
}

AddrRangeList
HuffmanLUT::LUTPort::getAddrRanges() const
{
    AddrRangeList ranges;
    ranges.push_back(owner->addrRange);
    return ranges;
}

void
HuffmanLUT::handleRequest(PacketPtr pkt)
{
    // Extract the encoded value from the address offset
    // Simple protocol: write encoded value, read decoded value

    if (pkt->isWrite()) {
        // Not supporting writes in this simple version
        pkt->makeResponse();
        return;
    }

    if (pkt->isRead()) {
        // Use the lower bits of address as the encoded value to lookup
        Addr offset = pkt->getAddr() - addrRange.start();
        uint32_t encoded = offset & 0xFFFFFFFF;

        uint32_t decoded = 0;
        auto it = lookupTable.find(encoded);
        if (it != lookupTable.end()) {
            decoded = it->second;
            DPRINTF(HuffmanLUT, "Lookup: 0x%x -> 0x%x\n", encoded, decoded);
        } else {
            DPRINTF(HuffmanLUT, "Lookup miss for: 0x%x\n", encoded);
        }

        // Write the decoded value into the packet
        assert(pkt->getSize() <= sizeof(uint32_t));
        pkt->setData((uint8_t*)&decoded);
        pkt->makeResponse();
    }
}

Tick
HuffmanLUT::LUTPort::recvAtomic(PacketPtr pkt)
{
    owner->handleRequest(pkt);
    return owner->latency;
}

void
HuffmanLUT::LUTPort::recvFunctional(PacketPtr pkt)
{
    owner->handleRequest(pkt);
}

bool
HuffmanLUT::LUTPort::recvTimingReq(PacketPtr pkt)
{
    // Handle the request
    owner->handleRequest(pkt);

    // Schedule response after latency
    owner->responseQueue.push_back(pkt);
    if (!owner->sendResponseEvent.scheduled()) {
        owner->schedule(owner->sendResponseEvent,
                       curTick() + owner->latency);
    }

    return true;
}

void
HuffmanLUT::sendResponse()
{
    assert(!responseQueue.empty());

    PacketPtr pkt = responseQueue.front();

    if (port.sendTimingResp(pkt)) {
        responseQueue.pop_front();

        // Schedule next response if queue not empty
        if (!responseQueue.empty()) {
            schedule(sendResponseEvent, curTick() + latency);
        }
    } else {
        // Retry later
        DPRINTF(HuffmanLUT, "Response blocked, will retry\n");
    }
}

void
HuffmanLUT::LUTPort::recvRespRetry()
{
    owner->sendResponse();
}

} // namespace gem5
