#include "../../p2scout-aie-generator/aie-generator.h"
#include "src/mm2s.h"

void dump_pt_eta_stream(hls::stream<qdma_axis<32,0,0,0>> &s, 
                const std::string &fout, 
                const uint32_t steps) {
    std::ofstream out_file(fout, std::ios::app);

    if (!out_file) throw std::runtime_error("Error in opening the out file");

    if (!s.empty()) {
        throw std::runtime_error("s is an empty stream");
    }

    if (steps % 2 != 0) {
        throw std::runtime_error("steps must be an even number, as the number of candidates"
                                    "per event is set to the next available even number");
    }

    // read pt
    for (unsigned int i = 0; i < steps; ++i) {
        qdma_axis<32,0,0,0> out = s.read();

        int32_t out_data = out.data.to_int();
        uint16_t out_data_l = out_data & 0xFFF; 
        uint16_t out_data_h = (out_data >> 16) & 0xFFF;

        out_file << std::right << std::setw(10) << out_data_l << std::setw(10) << out_data_h << "\n";
    }

    // read eta
    for (unsigned int i = 0; i < steps; ++i) {
        qdma_axis<32,0,0,0> out = s.read();

        int32_t out_data = out.data.to_int();
        int16_t out_data_l = out_data & 0xFFF; 
        int16_t out_data_h = (out_data >> 16) & 0xFFF;

        out_file << std::right << std::setw(10) << out_data_l << std::setw(10) << out_data_h << "\n";
    }
}

void dump_phi_pid_stream(hls::stream<qdma_axis<32,0,0,0>> &s, 
                const std::string &fout, 
                const uint32_t steps) {
    std::ofstream out_file(fout, std::ios::app);

    if (!out_file) throw std::runtime_error("Error in opening the out file");

    if (!s.empty()) {
        throw std::runtime_error("s is an empty stream");
    }

    if (steps % 2 != 0) {
        throw std::runtime_error("steps must be an even number, as the number of candidates"
                                    "per event is set to the next available even number");
    }

    // read phi
    for (unsigned int i = 0; i < steps; ++i) {
        qdma_axis<32,0,0,0> out = s.read();

        int32_t out_data = out.data.to_int();
        int16_t out_data_l = out_data & 0xFFF; 
        int16_t out_data_h = (out_data >> 16) & 0xFFF;

        out_file << std::right << std::setw(10) << out_data_l << std::setw(10) << out_data_h << "\n";
    }

    // read pid
    for (unsigned int i = 0; i < steps; ++i) {
        qdma_axis<32,0,0,0> out = s.read();

        int32_t out_data = out.data.to_int();
        uint8_t out_data_l = out_data & 0x0FF; 
        uint8_t out_data_h = (out_data >> 16) & 0x0FF;

        out_file << std::right << std::setw(10) << out_data_l << std::setw(10) << out_data_h << "\n";
    }
}

int main() {
    // create view of data
    aie::AIEGenerator gen("/home/gizago/pl-design/p2scout-aie-generator/raws/puppi_WTo3Pion_PU200_orbit1.raw");
    const auto& view = gen.get_orbit_view();
    const auto& map = gen.get_association_map();

    // create streams
    hls::stream<qdma_axis<32,0,0,0>> s0, s1, s2, s3, s4, s5;

    // run kernel
    mm2s(view.data(), s0, s1, s2, s3, s4, s5);

    // check results
    unsigned int stream_port_index = 0u;

    for (auto bx_idx : map.bx) {
        auto bx_begin = map.offset[bx_idx];
        auto bx_end = map.offset[bx_idx] + 1;
        auto bx_size = bx_end - bx_begin;

        if (bx_size % 2 != 0) {
            ++bx_size; // adapt bx size to the internal padding made by mm2s kernel
        }

        hls::stream<qdma_axis<32,0,0,0>>& stream_port0 = s0;
        hls::stream<qdma_axis<32,0,0,0>>& stream_port1 = s1;

        switch (stream_port_index) {
            case 0u: 
                dump_pt_eta_stream(s0, "/home/gizago/pl-design/hls/mm2s/aiesim/data0.txt", bx_size);
                dump_phi_pid_stream(s1, "/home/gizago/pl-design/hls/mm2s/aiesim/data1.txt", bx_size);
                break;

            case 1u: 
                dump_pt_eta_stream(s2, "/home/gizago/pl-design/hls/mm2s/aiesim/data0.txt", bx_size);
                dump_phi_pid_stream(s3, "/home/gizago/pl-design/hls/mm2s/aiesim/data1.txt", bx_size);
                break;

            default: 
                dump_pt_eta_stream(s4, "/home/gizago/pl-design/hls/mm2s/aiesim/data0.txt", bx_size);
                dump_phi_pid_stream(s5, "/home/gizago/pl-design/hls/mm2s/aiesim/data1.txt", bx_size);
                break;
        }

        stream_port_index = (stream_port_index + 1) % 3;
    }
}