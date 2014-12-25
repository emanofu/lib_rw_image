#include "igs_resource_irw.h"
#include "igs_tga_pixel.h"

bool igs::image::pixel::equal(
const igs::image::pixel::rgba32& p1
,const igs::image::pixel::rgba32& p2
) { return (p1.r==p2.r) && (p1.g==p2.g) && (p1.b==p2.b) && (p1.a==p2.a);
}
bool igs::image::pixel::equal(
const igs::image::pixel::rgb24& p1
,const igs::image::pixel::rgb24& p2
) { return (p1.r==p2.r) && (p1.g==p2.g) && (p1.b==p2.b);
}
bool igs::image::pixel::equal(
const igs::image::pixel::gray8& p1
,const igs::image::pixel::gray8& p2
) { return p1.value8==p2.value8;
}
