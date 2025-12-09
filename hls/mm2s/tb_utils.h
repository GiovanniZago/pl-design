#include "/tools/Xilinx/Vitis_HLS/2022.1/include/ap_axi_sdata.h"
#include "/tools/Xilinx/Vitis_HLS/2022.1/include/hls_stream.h"
#include <fstream>

void dump_pt_eta_stream(hls::stream<qdma_axis<32,0,0,0>> &s, const std::string &fout, const uint32_t steps);

void dump_pid_phi_stream(hls::stream<qdma_axis<32,0,0,0>> &s, const std::string &fout, const uint32_t steps);

void dump_bx_cands_header(hls::stream<qdma_axis<32,0,0,0>> &s, const std::string &fout);

void dump_orbit_header(hls::stream<qdma_axis<32,0,0,0>> &s, const std::string &fout);