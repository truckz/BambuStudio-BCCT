#include <algorithm>
#include <array>
#include <cmath>
#include <set>
#include <vector>

#include "../ClipperUtils.hpp"
#include "../Surface.hpp"

#include "FillBCCT.hpp"

namespace Slic3r {

namespace {

struct Segment3D
{
    Vec3d a;
    Vec3d b;
};

// Clip a segment against an axis-aligned box with the slab method.
bool clip_segment_to_box(
    const Vec3d &a,
    const Vec3d &b,
    const Vec3d &box_min,
    const Vec3d &box_max,
    Segment3D &clipped)
{
    constexpr double epsilon = 1e-10;
    const Vec3d delta = b - a;
    double t_min = 0.;
    double t_max = 1.;

    for (int axis = 0; axis < 3; ++ axis) {
        if (std::abs(delta(axis)) < epsilon) {
            if (a(axis) < box_min(axis) - epsilon || a(axis) > box_max(axis) + epsilon)
                return false;
            continue;
        }

        double t0 = (box_min(axis) - a(axis)) / delta(axis);
        double t1 = (box_max(axis) - a(axis)) / delta(axis);
        if (t0 > t1)
            std::swap(t0, t1);
        t_min = std::max(t_min, t0);
        t_max = std::min(t_max, t1);
        if (t_min > t_max + epsilon)
            return false;
    }

    clipped.a = a + t_min * delta;
    clipped.b = a + t_max * delta;
    return (clipped.b - clipped.a).squaredNorm() > epsilon * epsilon;
}

bool clip_segment_to_z(
    const Vec3d &a,
    const Vec3d &b,
    double z_min,
    double z_max,
    Segment3D &clipped)
{
    constexpr double epsilon = 1e-10;
    const double dz = b.z() - a.z();
    double t_min = 0.;
    double t_max = 1.;

    if (std::abs(dz) < epsilon) {
        if (a.z() < z_min - epsilon || a.z() > z_max + epsilon)
            return false;
    } else {
        double t0 = (z_min - a.z()) / dz;
        double t1 = (z_max - a.z()) / dz;
        if (t0 > t1)
            std::swap(t0, t1);
        t_min = std::max(t_min, t0);
        t_max = std::min(t_max, t1);
        if (t_min > t_max + epsilon)
            return false;
    }

    const Vec3d delta = b - a;
    clipped.a = a + t_min * delta;
    clipped.b = a + t_max * delta;
    return (clipped.b - clipped.a).squaredNorm() > epsilon * epsilon;
}

// Build the BCC graph in the paper's transformed coordinates, then crop the
// selected MG1. The graph itself is the standard BCC inference: every body
// center is joined to its eight surrounding corners.
std::vector<Segment3D> make_mg1(double d)
{
    const std::array<Vec3d, 3> basis = FillBCCT::lattice_basis(d);
    const Vec3d box_min(0., d, 0.);
    const Vec3d box_max(d, 2. * d, d);
    const Vec3d local_origin(0., d, 0.);

    std::vector<Segment3D> source_edges;
    source_edges.reserve(32);

    // r1, r2, r3 are the transformed x, z, y translations respectively.
    // These bounds cover every BCC cell that can intersect the MG1 crop.
    for (int ix = -3; ix <= 3; ++ ix)
        for (int iy = -3; iy <= 3; ++ iy)
            for (int iz = -3; iz <= 3; ++ iz) {
                const Vec3d cell_origin =
                    static_cast<double>(ix) * basis[0] +
                    static_cast<double>(iz) * basis[1] +
                    static_cast<double>(iy) * basis[2];
                const Vec3d center = cell_origin + 0.5 * (basis[0] + basis[1] + basis[2]);

                for (int sx = 0; sx <= 1; ++ sx)
                    for (int sy = 0; sy <= 1; ++ sy)
                        for (int sz = 0; sz <= 1; ++ sz) {
                            const Vec3d corner =
                                cell_origin +
                                static_cast<double>(sx) * basis[0] +
                                static_cast<double>(sz) * basis[1] +
                                static_cast<double>(sy) * basis[2];
                            Segment3D clipped;
                            if (clip_segment_to_box(center, corner, box_min, box_max, clipped)) {
                                clipped.a -= local_origin;
                                clipped.b -= local_origin;
                                source_edges.push_back(std::move(clipped));
                            }
                        }
            }

    // The paper specifies the three reflection planes but not their global
    // phase. We use each MG1's upper face (local coordinate d) as the FDM
    // reflection plane; copies then form a 2d-periodic supercell.
    // Canonicalize the reflected edges on the 2d-periodic domain. Some
    // members lie in a twin plane and would otherwise be emitted twice by
    // the adjacent reflected meta-grains.
    const double half_cell = 0.5 * d;
    constexpr int period_steps = 4;
    std::set<std::array<int, 6>> periodic_edges;
    for (int mask = 0; mask < 8; ++ mask)
        for (const Segment3D &source : source_edges) {
            Segment3D reflected = source;
            for (int axis = 0; axis < 3; ++ axis)
                if ((mask & (1 << axis)) != 0) {
                    reflected.a(axis) = 2. * d - reflected.a(axis);
                    reflected.b(axis) = 2. * d - reflected.b(axis);
                }

            std::array<int, 3> a;
            std::array<int, 3> b;
            for (int axis = 0; axis < 3; ++ axis) {
                a[axis] = int(std::llround(reflected.a(axis) / half_cell));
                b[axis] = int(std::llround(reflected.b(axis) / half_cell));
            }

            std::array<int, 6> forward;
            std::array<int, 6> reverse;
            for (int axis = 0; axis < 3; ++ axis) {
                const int delta = b[axis] - a[axis];
                forward[axis]     = (a[axis] % period_steps + period_steps) % period_steps;
                forward[axis + 3] = delta;
                reverse[axis]     = (b[axis] % period_steps + period_steps) % period_steps;
                reverse[axis + 3] = -delta;
            }
            periodic_edges.insert(std::min(forward, reverse));
        }

    std::vector<Segment3D> meta_grain;
    meta_grain.reserve(periodic_edges.size());
    for (const std::array<int, 6> &edge : periodic_edges) {
        Segment3D segment;
        for (int axis = 0; axis < 3; ++ axis) {
            segment.a(axis) = half_cell * edge[axis];
            segment.b(axis) = segment.a(axis) + half_cell * edge[axis + 3];
        }
        meta_grain.push_back(std::move(segment));
    }
    return meta_grain;
}

} // namespace

Vec3d FillBCCT::shear(const Vec3d &point)
{
    return Vec3d(point.x(), 0.5 * point.x() + point.y() + 0.5 * point.z(), point.z());
}

std::array<Vec3d, 3> FillBCCT::lattice_basis(double cell_size)
{
    // M*[d,0,0], M*[0,0,d], M*[0,d,0] = r1, r2, r3 in the paper.
    return {
        shear(Vec3d(cell_size, 0., 0.)),
        shear(Vec3d(0., 0., cell_size)),
        shear(Vec3d(0., cell_size, 0.))
    };
}

double FillBCCT::projected_edge_length_per_cell()
{
    const std::array<Vec3d, 3> basis = lattice_basis(1.);
    double total_length = 0.;
    for (int sx = -1; sx <= 1; sx += 2)
        for (int sy = -1; sy <= 1; sy += 2)
            for (int sz = -1; sz <= 1; sz += 2) {
                const Vec3d edge =
                    static_cast<double>(sx) * 0.5 * basis[0] +
                    static_cast<double>(sz) * 0.5 * basis[1] +
                    static_cast<double>(sy) * 0.5 * basis[2];
                total_length += edge.head<2>().norm();
            }
    return total_length;
}

double FillBCCT::cell_size_for_density(double density, double cross_section, double width, double layer_span)
{
    if (density <= 0. || cross_section <= 0. || width <= 0. || layer_span <= 0.)
        return 0.;
    // Each extrusion interval projects a slab of height layer_span + width.
    // Budget the resulting XY toolpaths, including overlap between slabs,
    // rather than the lengths of the underlying 3D struts.
    const double overlap_factor = (layer_span + width) / layer_span;
    return std::sqrt(cross_section * projected_edge_length_per_cell() * overlap_factor / density);
}

void FillBCCT::_fill_surface_single(
    const FillParams                &params,
    unsigned int                     thickness_layers,
    const std::pair<float, Point>   & /* direction */,
    ExPolygon                        expolygon,
    Polylines                       &polylines_out)
{
    if (params.density <= 0.f || params.flow.width() <= 0.f)
        return;

    const double flow_area = params.flow.mm3_per_mm();
    if (flow_area <= 0.)
        return;

    const double layer_height = params.layer_height > 0.f ? params.layer_height : params.flow.height();
    if (layer_height <= 0.)
        return;

    const double layer_span = layer_height * std::max(1u, thickness_layers);
    double d = cell_size_for_density(params.density, flow_area, params.flow.width(), layer_span);
    if (d <= 0.)
        return;

    // The BCC center-to-corner edges change Z by d/2. Round d upward so that
    // this half-cell height is an integer number of layers. This preserves the
    // layer-to-layer phase while making the requested density a first-order
    // target rather than an exact volume guarantee.
    d = 2. * layer_height * std::ceil(d / (2. * layer_height));
    d = std::max(d, 2. * layer_height);
    const double period = 2. * d;
    const double half_width = 0.5 * params.flow.width();
    const double z_min = this->z - layer_span - half_width;
    const double z_max = this->z + half_width;

    float infill_angle = std::isfinite(this->angle) ? this->angle : 0.f;
    const size_t output_start = polylines_out.size();
    if (std::abs(infill_angle) >= EPSILON)
        expolygon.rotate(-infill_angle);

    const BoundingBox bb = expolygon.contour.bounding_box();
    const double min_x = unscale<double>(bb.min.x());
    const double min_y = unscale<double>(bb.min.y());
    const double max_x = unscale<double>(bb.max.x());
    const double max_y = unscale<double>(bb.max.y());

    const int tile_x_min = static_cast<int>(std::floor(min_x / period)) - 1;
    const int tile_y_min = static_cast<int>(std::floor(min_y / period)) - 1;
    const int tile_z_min = static_cast<int>(std::floor(z_min / period)) - 1;
    const int tile_x_max = static_cast<int>(std::ceil(max_x / period)) + 1;
    const int tile_y_max = static_cast<int>(std::ceil(max_y / period)) + 1;
    const int tile_z_max = static_cast<int>(std::ceil(z_max / period)) + 1;

    const std::vector<Segment3D> meta_grain = make_mg1(d);
    Polylines layer_lines;
    layer_lines.reserve(meta_grain.size() * size_t(tile_x_max - tile_x_min + 1));

    for (int tx = tile_x_min; tx <= tile_x_max; ++ tx)
        for (int ty = tile_y_min; ty <= tile_y_max; ++ ty)
            for (int tz = tile_z_min; tz <= tile_z_max; ++ tz) {
                const Vec3d tile_offset(period * tx, period * ty, period * tz);
                for (const Segment3D &meta_segment : meta_grain) {
                    const Vec3d a = meta_segment.a + tile_offset;
                    const Vec3d b = meta_segment.b + tile_offset;
                    Segment3D layer_segment;
                    if (!clip_segment_to_z(a, b, z_min, z_max, layer_segment))
                        continue;

                    Polyline line;
                    line.points.emplace_back(Point::new_scale(layer_segment.a.x(), layer_segment.a.y()));
                    line.points.emplace_back(Point::new_scale(layer_segment.b.x(), layer_segment.b.y()));
                    if (line.points.front() != line.points.back())
                        layer_lines.push_back(std::move(line));
                }
            }

    // Clip each layer's projected strut portions with the normal infill
    // boundary. No connection operation is applied: the 3D graph, rather
    // than an XY linking heuristic, supplies the topology.
    layer_lines = intersection_pl(std::move(layer_lines), expolygon);
    for (Polyline &line : layer_lines)
        if (line.points.size() >= 2 && line.length() > SCALED_EPSILON)
            polylines_out.push_back(std::move(line));

    if (std::abs(infill_angle) >= EPSILON)
        for (auto it = polylines_out.begin() + output_start; it != polylines_out.end(); ++ it)
            it->rotate(infill_angle);
}

} // namespace Slic3r
