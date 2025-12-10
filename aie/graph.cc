#include "graph.h"
 
TestGraph mygraph;

#if defined(__AIESIM__) || defined(__X86SIM__)

int main(void) 
{
    int num_iter = 2;

    mygraph.init();
    mygraph.run(num_iter);
    mygraph.end();

    return 0;
}

#endif