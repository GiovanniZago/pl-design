#include "kernels.h"

void stream_mux(input_stream<int32> * __restrict in0, 
                input_stream<int32> * __restrict in1, 
                output_stream<int16> * __restrict out) {
    // unpack bx and num_cands
    const int32 word = readincr(in0);
    const uint16_t bx = word & 0x0FFF;
    const uint16_t num_cands = (word >> 16) & 0x0FFF;

    // read orbit number
    const uint32_t orbit = readincr(in1);

    #if defined(__X86SIM__)
    printf("----------------------------\n");
    printf("bx = %d, num_cands = %d, orbit = %d\n", bx, num_cands, orbit);
    #endif

    // vectors
    div_t res = div(num_cands, VEC_SIZE);
    const int16 num_vects = (res.quot == 0) ? 1 : (res.rem == 0) ? res.quot : res.quot + 1;
    aie::vector<int16, VEC_SIZE> pt[num_vects], eta[num_vects], phi[num_vects], pid[num_vects];

    #if defined(__X86SIM__)
    printf("num_vects = %d\n", num_vects);
    #endif
    
    // buffers
    const int16 num_ele = num_vects * VEC_SIZE;
    alignas(aie::vector_decl_align) int16 pt_buff[num_ele];
    std::memset(pt_buff, 0, sizeof(pt_buff));
    alignas(aie::vector_decl_align) int16 eta_buff[num_ele];
    std::memset(eta_buff, 0, sizeof(eta_buff));
    alignas(aie::vector_decl_align) int16 phi_buff[num_ele];
    std::memset(phi_buff, 0, sizeof(phi_buff));
    alignas(aie::vector_decl_align) int16 pid_buff[num_ele];
    std::memset(pid_buff, 0, sizeof(pid_buff));
    
    const int16 num_read = ((num_cands % 2) == 0) ? num_cands / 2 : (num_cands + 1) / 2;

    #if defined(__X86SIM__)
    printf("num_ele = %d, num_read = %d\n", num_ele, num_read);
    #endif
    
    int32 pt_word, pid_word;
    for (int32_t ii = 0; ii < num_read; ii += 2) {
        pt_word = readincr(in0);
        pid_word = readincr(in1);

        pt_buff[ii] = pt_word & 0xFFF;
        pt_buff[ii + 1] = (pt_word >> 16) & 0xFFF;
        pid_buff[ii] = pid_word & 0xFF;
        pid_buff[ii + 1] = (pid_word >> 16) & 0xFF;
    }

    int32 eta_word, phi_word;
    for (int32_t ii = 0; ii < num_read; ii += 2) {
        eta_word = readincr(in0);
        phi_word = readincr(in1);

        eta_buff[ii] = eta_word & 0xFFFF;
        eta_buff[ii + 1] = (eta_word >> 16) & 0xFFFF;
        phi_buff[ii] = phi_word & 0xFFFF;
        phi_buff[ii + 1] = (phi_word >> 16) & 0xFFFF;
    }

    #if defined(__X86SIM__)
    for (int32_t ii = 0; ii < num_read; ++ii) {
        printf("%d  ", pt_buff[ii]);
    }
    printf("\n");

    for (int32_t ii = 0; ii < num_read; ++ii) {
        printf("%d  ", pid_buff[ii]);
    }
    printf("\n");

    for (int32_t ii = 0; ii < num_read; ++ii) {
        printf("%d  ", eta_buff[ii]);
    }
    printf("\n");

    for (int32_t ii = 0; ii < num_read; ++ii) {
        printf("%d  ", phi_buff[ii]);
    }
    printf("\n");
    #endif

    // vector pointers
    aie::vector<int16, VEC_SIZE> *pt_ptr = (aie::vector<int16, VEC_SIZE> *) pt_buff;
    aie::vector<int16, VEC_SIZE> *eta_ptr = (aie::vector<int16, VEC_SIZE> *) eta_buff;
    aie::vector<int16, VEC_SIZE> *phi_ptr = (aie::vector<int16, VEC_SIZE> *) phi_buff;
    aie::vector<int16, VEC_SIZE> *pid_ptr = (aie::vector<int16, VEC_SIZE> *) pid_buff;
    
    // fill vectors
    for (unsigned int vec_idx = 0; vec_idx < num_vects; ++vec_idx) {
        pt[vec_idx] = pt_ptr[vec_idx];
        eta[vec_idx] = eta_ptr[vec_idx];
        phi[vec_idx] = phi_ptr[vec_idx];
        pid[vec_idx] = pid_ptr[vec_idx];
        ++pt_ptr;
        ++eta_ptr;
        ++phi_ptr;
        ++pid_ptr;
    }

    // data loopback
    for (unsigned int vec_idx = 0; vec_idx < num_vects; ++vec_idx) {
        writeincr(out, pt[vec_idx]);
    }

    for (unsigned int vec_idx = 0; vec_idx < num_vects; ++vec_idx) {
        writeincr(out, eta[vec_idx]);
    }

    for (unsigned int vec_idx = 0; vec_idx < num_vects; ++vec_idx) {
        writeincr(out, phi[vec_idx]);
    }

    for (unsigned int vec_idx = 0; vec_idx < num_vects; ++vec_idx) {
        writeincr(out, pid[vec_idx]);
    }
}