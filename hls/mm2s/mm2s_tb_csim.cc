#include "../../p2scout-aie-generator/aie-generator.h"
#include "src/mm2s.h"

void dump_pt_eta_stream(hls::stream<qdma_axis<32,0,0,0>> &s, 
                const std::string &fout, 
                const uint32_t steps) {
    std::ofstream out_file(fout, std::ios::app);

    if (!out_file) throw std::runtime_error("Error in opening the out file");

    if (s.empty()) {
        throw std::runtime_error("s is an empty stream");
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
        int16_t out_data_l = out_data & 0xFFFF; // must keep all 16 bits otherwise, if we use 0xFFF, the top nibble is set to zero -> positive value
        int16_t out_data_h = (out_data >> 16) & 0xFFFF;

        out_file << std::right << std::setw(10) << out_data_l << std::setw(10) << out_data_h << "\n";
    }
}

void dump_pid_phi_stream(hls::stream<qdma_axis<32,0,0,0>> &s, 
                const std::string &fout, 
                const uint32_t steps) {
    std::ofstream out_file(fout, std::ios::app);

    if (!out_file) throw std::runtime_error("Error in opening the out file");

    if (s.empty()) {
        throw std::runtime_error("s is an empty stream");
    }

    // read pid
    for (unsigned int i = 0; i < steps; ++i) {
        qdma_axis<32,0,0,0> out = s.read();

        int32_t out_data = out.data.to_int();
        uint8_t out_data_l = out_data & 0xFF; 
        uint8_t out_data_h = (out_data >> 16) & 0xFF;

        out_file << std::right << std::setw(10) << unsigned(out_data_l) << std::setw(10) << unsigned(out_data_h) << "\n";
    }

    // read phi
    for (unsigned int i = 0; i < steps; ++i) {
        qdma_axis<32,0,0,0> out = s.read();

        int32_t out_data = out.data.to_int();
        int16_t out_data_l = out_data & 0xFFFF; 
        int16_t out_data_h = (out_data >> 16) & 0xFFFF;

        out_file << std::right << std::setw(10) << out_data_l << std::setw(10) << out_data_h << "\n";
    }
}

void dump_bx_cands_header(hls::stream<qdma_axis<32,0,0,0>> &s, const std::string &fout) {
    std::ofstream out_file(fout, std::ios::app);

    if (!out_file) throw std::runtime_error("Error in opening the out file");

    if (s.empty()) {
        throw std::runtime_error("s is an empty stream");
    }
    
    qdma_axis<32,0,0,0> out = s.read();

    int32_t out_data = out.data.to_int();
    uint16_t bx = out_data & 0x0FFF;
    uint16_t num_cands = (out_data >> 16) & 0x0FFF;

    out_file << std::right << std::setw(10) << bx << std::setw(10) << num_cands << "\n";
}

void dump_orbit_header(hls::stream<qdma_axis<32,0,0,0>> &s, const std::string &fout) {
    std::ofstream out_file(fout, std::ios::app);

    if (!out_file) throw std::runtime_error("Error in opening the out file");

    if (s.empty()) {
        throw std::runtime_error("s is an empty stream");
    }

    qdma_axis<32,0,0,0> out = s.read();

    int32_t orbit_data = out.data.to_int();
    uint16_t obit_number_l = orbit_data & 0xFFFF;
    uint16_t obit_number_h = (orbit_data >> 16) & 0xFFFF;

    out_file << std::right << std::setw(10) << "/" << std::setw(10) << orbit << "\n";
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
        std::cout << " ---- Reading output for bx " << bx_idx << " ----" << std::endl;
        auto bx_begin = map.offset[bx_idx];
        auto bx_end = map.offset[bx_idx + 1];
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

        switch (stream_port_index) {
            case 0u: 
                dump_bx_cands_header(s0, "/home/gizago/pl-design/hls/mm2s/aiesim/data0.txt");
                dump_orbit_header(s1, "/home/gizago/pl-design/hls/mm2s/aiesim/data1.txt");
                dump_pt_eta_stream(s0, "/home/gizago/pl-design/hls/mm2s/aiesim/data0.txt", steps);
                dump_pid_phi_stream(s1, "/home/gizago/pl-design/hls/mm2s/aiesim/data1.txt", steps);
                break;

            case 1u: 
                dump_bx_cands_header(s2, "/home/gizago/pl-design/hls/mm2s/aiesim/data0.txt");
                dump_orbit_header(s3, "/home/gizago/pl-design/hls/mm2s/aiesim/data1.txt");
                dump_pt_eta_stream(s2, "/home/gizago/pl-design/hls/mm2s/aiesim/data0.txt", steps);
                dump_pid_phi_stream(s3, "/home/gizago/pl-design/hls/mm2s/aiesim/data1.txt", steps);
                break;

            default: 
                dump_bx_cands_header(s4, "/home/gizago/pl-design/hls/mm2s/aiesim/data0.txt");
                dump_orbit_header(s5, "/home/gizago/pl-design/hls/mm2s/aiesim/data1.txt");
                dump_pt_eta_stream(s4, "/home/gizago/pl-design/hls/mm2s/aiesim/data0.txt", steps);
                dump_pid_phi_stream(s5, "/home/gizago/pl-design/hls/mm2s/aiesim/data1.txt", steps);
                break;
        }

        stream_port_index = (stream_port_index + 1) % 3;
    }

    return(0);
}