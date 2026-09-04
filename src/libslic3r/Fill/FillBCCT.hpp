#ifndef slic3r_FillBCCT_hpp_
#define slic3r_FillBCCT_hpp_

#include <array>

#include "../Point.hpp"

#include "FillBase.hpp"

namespace Slic3r {

class FillBCCT : public Fill
{
public:
    Fill* clone() const override { return new FillBCCT(*this); }

    bool use_bridge_flow() const override { return false; }
    bool is_self_crossing() override { return true; }

    // BCCT's exact shear from the paper: x2 = x1,
    // y2 = 0.5*x1 + y1 + 0.5*z1, z2 = z1.
    static Vec3d shear(const Vec3d &point);

    // The sheared BCC translations, in the paper's r1, r2, r3 order.
    static std::array<Vec3d, 3> lattice_basis(double cell_size);

    // Sum of the eight sheared center-to-corner edge lengths for one
    // conventional BCC cell of unit side length.
    static double sheared_edge_length_per_cell();

    // First-order FDM density mapping. The cross section is the extrusion
    // flow area in mm^2 and density is the requested sparse-fill fraction.
    static double cell_size_for_density(double density, double cross_section);

protected:
    void _fill_surface_single(
        const FillParams                &params,
        unsigned int                     thickness_layers,
        const std::pair<float, Point>   &direction,
        ExPolygon                        expolygon,
        Polylines                       &polylines_out) override;
};

} // namespace Slic3r

#endif // slic3r_FillBCCT_hpp_
