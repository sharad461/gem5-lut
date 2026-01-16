#include <stdio.h>
#include <stdint.h>

// Base address where the Huffman LUT is mapped
//#define HUFFMAN_LUT_BASE 0x10000000UL
#define HUFFMAN_LUT_BASE 0x200000000UL

// Function to perform a lookup
uint32_t huffman_lookup(uint32_t encoded) {
    // Read from the LUT by accessing memory at base + encoded value
    volatile uint32_t *lut_addr = (volatile uint32_t *)(HUFFMAN_LUT_BASE + encoded);
    return *lut_addr;
}

int main() {
    printf("Testing Huffman LUT\n");

    // Test encoded values
    uint32_t test_values[] = {0b110, 0b111, 0b100, 0b101, 0b10, 0b11, 0b0, 0b1};
    int num_tests = sizeof(test_values) / sizeof(test_values[0]);

    for (int i = 0; i < num_tests; i++) {
        uint32_t encoded = test_values[i];
        uint32_t decoded = huffman_lookup(encoded);
        printf("Lookup: 0x%x -> 0x%x (binary: ", encoded, decoded);

        // Print binary representation
        for (int b = 7; b >= 0; b--) {
            printf("%d", (decoded >> b) & 1);
        }
        printf(")\n");
    }

    // Test batch processing simulation
    printf("\nBatch processing 100 lookups:\n");
    uint32_t results[100];
    for (int i = 0; i < 100; i++) {
        results[i] = huffman_lookup(test_values[i % num_tests]);
    }
    printf("Completed 100 lookups\n");

    return 0;
}