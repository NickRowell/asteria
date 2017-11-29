#include "source.h"

Source::Source() {

}

Source::Source(const Source& copyme) : pixels(copyme.pixels), adu(copyme.adu), sigma_adu(copyme.sigma_adu), i(copyme.i), j(copyme.j),
c_ii(copyme.c_ii), c_ij(copyme.c_ij), c_jj(copyme.c_jj), l1(copyme.l1), l2(copyme.l2), orientation(copyme.orientation) {

}


Source& Source::operator=(const Source& copyme) {
    pixels = copyme.pixels;
    adu = copyme.adu;
    sigma_adu = copyme.sigma_adu;
    i = copyme.i;
    j = copyme.j;
    c_ii = copyme.c_ii;
    c_ij = copyme.c_ij;
    c_jj = copyme.c_jj;
    l1 = copyme.l1;
    l2 = copyme.l2;
    orientation = copyme.orientation;
    return *this;
}
