import m5
from m5.objects import *
import argparse

# Parse arguments
parser = argparse.ArgumentParser()
parser.add_argument('--cmd', type=str, help='Binary to run')
args = parser.parse_args()

# Create the system
system = System()

# Set up clock and voltage domain
system.clk_domain = SrcClockDomain()
system.clk_domain.clock = '1GHz'
system.clk_domain.voltage_domain = VoltageDomain()

# Set up memory
system.mem_mode = 'timing'
# system.mem_ranges = [AddrRange('512MB')]
system.mem_ranges = [AddrRange('8192MB')]

# Create CPU
system.cpu = X86TimingSimpleCPU()

# Create the Huffman LUT
# Map it to a specific memory address range
system.huffman_lut = HuffmanLUT()
system.huffman_lut.addr_range = AddrRange(0x10000000, size='4kB')
system.huffman_lut.latency = 10  # 10 cycles per lookup

# Create memory bus
system.membus = SystemXBar()

# Connect CPU to membus
system.cpu.icache_port = system.membus.cpu_side_ports
system.cpu.dcache_port = system.membus.cpu_side_ports

system.iobus = IOXBar()
system.iobus.mem_side_ports = system.membus.cpu_side_ports

# Connect Huffman LUT to membus
# system.huffman_lut.port = system.membus.mem_side_ports
system.huffman_lut.port = system.iobus.mem_side_ports

# Create memory controller
system.mem_ctrl = MemCtrl()
system.mem_ctrl.dram = DDR3_1600_8x8()
system.mem_ctrl.dram.range = system.mem_ranges[0]
system.mem_ctrl.port = system.membus.mem_side_ports

# Set up interrupt controller (required for x86)
system.cpu.createInterruptController()
system.cpu.interrupts[0].pio = system.membus.mem_side_ports
system.cpu.interrupts[0].int_requestor = system.membus.cpu_side_ports
system.cpu.interrupts[0].int_responder = system.membus.mem_side_ports

# Set up the workload
system.workload = SEWorkload.init_compatible(args.cmd)

# Create the process
process = Process()
process.cmd = [args.cmd]
system.cpu.workload = process
system.cpu.createThreads()

# Create the root object
root = Root(full_system=False, system=system)

# Instantiate
m5.instantiate()

print("Starting simulation...")
print(f"Huffman LUT mapped at: {system.huffman_lut.addr_range}")

# Run simulation
exit_event = m5.simulate()
print(f"Simulation ended: {exit_event.getCause()}")
