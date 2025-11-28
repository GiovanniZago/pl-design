#include "../../p2scout-aie-generator/aie-generator.h"
#include "../../p2scout-aie-generator/puppi-unpackers.h"
#include "src/mm2s.h"

int main() {
    // create view of data
    aie::AIEGenerator gen("/home/gizago/pl-design/p2scout-aie-generator/raws/puppi_WTo3Pion_PU200_orbit1.raw");
    const auto& view = gen.get_orbit_view();

    // create streams
    hls::stream<qdma_axis<32,0,0,0>> s0, s1, s2, s3, s4, s5;

    // run kernel
    mm2s(view.data(), s0, s1, s2, s3, s4, s5);

    // check results
    while (!s0.empty()) {
        // read data from one of the output streams
        qdma_axis<32,0,0,0> out = s0.read();
        int32_t out_data = out.data.to_int();
        int16_t out_data_h = (out_data >> 16) & 0xFFF;
        int16_t out_data_l = out_data & 0xFFF;

        std::cout << "out_data_h: " << out_data_h
                << " out_data_l: " << out_data_l
                << "  TKEEP: 0x" << std::hex << (int)out.keep
                << "  TLAST: " << std::dec << (int)out.last
                << std::endl;
    }
}