#include "../../p2scout-aie-generator/aie-generator.h"
#include "src/mm2s.h"
#include "tb_utils.h"

int main() {
    aie::AIEGenerator gen("/home/gizago/pl-design/p2scout-aie-generator/raws/puppi_WTo3Pion_PU200_orbit1.raw");
    const auto& view = gen.get_orbit_view();
    const auto& map = gen.get_association_map();

    hls::stream<qdma_axis<32,0,0,0>> s0, s1, s2, s3, s4, s5;

    // use static memory because dynamic memory is not allowed in cosim
    constexpr size_t mem_size = 200000;
    static uint64_t mem[mem_size] = { 0u };
    std::memcpy(mem, view.data(), view.size() * sizeof(uint64_t));

    mm2s(mem, s0, s1, s2, s3, s4, s5);

    // check results
    unsigned int stream_port_index = 0u;

    for (auto bx_idx : map.bx) {
        auto bx_begin = map.offset[bx_idx];
        auto bx_end = map.offset[bx_idx + 1];

        std::cout << " ---- Reading output for bx " << bx_idx << " ----" << std::endl;
        std::cout << "bx begin: " << bx_begin << std::endl;
        std::cout << "bx_end: " << bx_end << std::endl;

        auto bx_size = bx_end - bx_begin;
        std::cout << "Original bx size: " << bx_size << std::endl;

        if (bx_size % 2 != 0) {
            bx_size += 1; // adapt bx size to the internal padding made by mm2s kernel
        }

        std::cout << "Adjusted bx size: " << bx_size << std::endl;

        auto steps = bx_size / 2;

        std::cout << "Steps: " << steps << std::endl;

        bool split = false;
        switch (stream_port_index) {
            case 0u: 
                dump_bx_cands_header(s0, "/home/gizago/pl-design/hls/mm2s/aiesim/data0.txt", split=split);
                dump_orbit_header(s1, "/home/gizago/pl-design/hls/mm2s/aiesim/data1.txt", split=split);
                dump_pt_eta_stream(s0, "/home/gizago/pl-design/hls/mm2s/aiesim/data0.txt", steps, split=split);
                dump_pid_phi_stream(s1, "/home/gizago/pl-design/hls/mm2s/aiesim/data1.txt", steps, split=split);
                break;

            case 1u: 
                dump_bx_cands_header(s2, "/home/gizago/pl-design/hls/mm2s/aiesim/data0.txt", split=split);
                dump_orbit_header(s3, "/home/gizago/pl-design/hls/mm2s/aiesim/data1.txt", split=split);
                dump_pt_eta_stream(s2, "/home/gizago/pl-design/hls/mm2s/aiesim/data0.txt", steps, split=split);
                dump_pid_phi_stream(s3, "/home/gizago/pl-design/hls/mm2s/aiesim/data1.txt", steps, split=split);
                break;

            default: 
                dump_bx_cands_header(s4, "/home/gizago/pl-design/hls/mm2s/aiesim/data0.txt", split=split);
                dump_orbit_header(s5, "/home/gizago/pl-design/hls/mm2s/aiesim/data1.txt", split=split);
                dump_pt_eta_stream(s4, "/home/gizago/pl-design/hls/mm2s/aiesim/data0.txt", steps, split=split);
                dump_pid_phi_stream(s5, "/home/gizago/pl-design/hls/mm2s/aiesim/data1.txt", steps, split=split);
                break;
        }

        stream_port_index = (stream_port_index + 1) % 3;
    }
    
    return 0;
}