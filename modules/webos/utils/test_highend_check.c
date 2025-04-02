#include "highend_check.c"
#include <assert.h>

int main() {
    assert(Is_Machine_HighEnd("o22") == true);
    assert(Is_Machine_HighEnd("o22n") == true);
    assert(Is_Machine_HighEnd("o22n2") == true);
    assert(Is_Machine_HighEnd("o22n3") == true);
    assert(Is_Machine_HighEnd("o22n28k") == true);
    assert(Is_Machine_HighEnd("k8hp") == true);
    assert(Is_Machine_HighEnd("k8hpt") == true);
    assert(Is_Machine_HighEnd("k9hp") == true);
    assert(Is_Machine_HighEnd("k7lp") == false);
    assert(Is_Machine_HighEnd("k6hp") == false);
    return 0;
}