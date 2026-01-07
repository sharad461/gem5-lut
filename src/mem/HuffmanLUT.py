from m5.params import *
from m5.SimObject import SimObject


class HuffmanLUT(SimObject):
    type = 'HuffmanLUT'
    cxx_header = "mem/huffman_lut.hh"
    cxx_class = 'gem5::HuffmanLUT'

    # Memory-side port for CPU to access this device
    port = ResponsePort("Response port for memory-mapped access")

    # Lookup table parameters
    latency = Param.Cycles(1, "Latency for a single lookup operation")
    size = Param.MemorySize('4kB', "Size of the lookup table")
    addr_range = Param.AddrRange("Address range for memory mapping")
