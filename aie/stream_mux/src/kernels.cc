#include "kernels.h"

void stream_mux(input_stream<int32> * __restrict in0, 
                input_stream<int32> * __restrict in1, 
                output_stream<int16> * __restrict out) {
    // unpack bx and num_cands
    const int32 word = readincr(in0);
    const int16 bx = word & 0x0FFF;
    const int16 num_cands = (word >> 16) & 0x0FFF;

    // read orbit number
    const uint32_t orbit = readincr(in1);

    #if defined(__X86SIM__)
    printf("----------------------------\n");
    printf("bx = %d, num_cands = %d, orbit = %d\n", bx, num_cands, orbit);
    #endif

    // vectors
    aie::vector<int16, VEC_SIZE> pt[MAX_VECTS] = { aie::broadcast<int16, VEC_SIZE>(0) };
    aie::vector<int16, VEC_SIZE> eta[MAX_VECTS]= { aie::broadcast<int16, VEC_SIZE>(0) };
    aie::vector<int16, VEC_SIZE> phi[MAX_VECTS]= { aie::broadcast<int16, VEC_SIZE>(0) };
    aie::vector<int16, VEC_SIZE> pid[MAX_VECTS]= { aie::broadcast<int16, VEC_SIZE>(0) };
    
    // buffers
    alignas(aie::vector_decl_align) int16 pt_buff[MAX_CANDS] = {};
    alignas(aie::vector_decl_align) int16 eta_buff[MAX_CANDS] = {};
    alignas(aie::vector_decl_align) int16 phi_buff[MAX_CANDS] = {};
    alignas(aie::vector_decl_align) int16 pid_buff[MAX_CANDS] = {};
    
    int16 foo = (num_cands + (VEC_SIZE - 1)) / VEC_SIZE;  // ceil division
    const int16 num_vects = (foo == 0) ? 1 : (foo > MAX_VECTS) ? MAX_VECTS : foo; // cap num_vects between 1 and MAX_VECTS
    const int16 num_ele = num_vects * VEC_SIZE;
    const int16 num_cands_even = ((num_cands % 2) == 0) ? num_cands : num_cands + 1;
    
    #if defined(__X86SIM__)
    printf("num_vects = %d, num_ele = %d, num_cands_even = %d\n", num_vects, num, num_cands_even);
    #endif

    int32 pt_word, pid_word;
    for (int32_t ii = 0; ii < num_cands_even; ii+=2) {
        pt_word = readincr(in0);
        pid_word = readincr(in1);

        pt_buff[ii] = pt_word & 0xFFF;
        pt_buff[ii + 1] = (pt_word >> 16) & 0xFFF;
        pid_buff[ii] = pid_word & 0xFF;
        pid_buff[ii + 1] = (pid_word >> 16) & 0xFF;
    }

    int32 eta_word, phi_word;
    for (int32_t ii = 0; ii < num_cands_even; ii+=2) {
        eta_word = readincr(in0);
        phi_word = readincr(in1);

        eta_buff[ii] = eta_word & 0xFFFF;
        eta_buff[ii + 1] = (eta_word >> 16) & 0xFFFF;
        phi_buff[ii] = phi_word & 0xFFFF;
        phi_buff[ii + 1] = (phi_word >> 16) & 0xFFFF;
    }

    #if defined(__X86SIM__)
    printf("pt_buff: ");
    for (int32_t ii = 0; ii < num_ele; ++ii) {
        printf("%d  ", pt_buff[ii]);
    }
    printf("\n");

    printf("pid_buff: ");
    for (int32_t ii = 0; ii < num_ele; ++ii) {
        printf("%d  ", pid_buff[ii]);
    }
    printf("\n");

    printf("eta_buff: ");
    for (int32_t ii = 0; ii < num_ele; ++ii) {
        printf("%d  ", eta_buff[ii]);
    }
    printf("\n");

    printf("phi_buff: ");
    for (int32_t ii = 0; ii < num_ele; ++ii) {
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
    }

    #if defined(__X86SIM__)
    for (unsigned int vec_idx = 0; vec_idx < num_vects; ++vec_idx) {
        aie::print(pt[vec_idx], true, "pt[vec_idx]: ");
    }
    for (unsigned int vec_idx = 0; vec_idx < num_vects; ++vec_idx) {
        aie::print(pid[vec_idx], true, "pid[vec_idx]: ");
    }
    for (unsigned int vec_idx = 0; vec_idx < num_vects; ++vec_idx) {
        aie::print(eta[vec_idx], true, "eta[vec_idx]: ");
    }
    for (unsigned int vec_idx = 0; vec_idx < num_vects; ++vec_idx) {
        aie::print(phi[vec_idx], true, "phi[vec_idx]: ");
    }
    #endif

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