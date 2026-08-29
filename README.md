# Bambu Studio BCCT Experimental Infill

This repository backs up an experimental sparse-infill implementation for Bambu Studio based on the triply-twinned body-centred-cubic lattice described in:

- *Triply-Twinned Metamaterials: Unraveling the Mechanics and Failure Pathways Through High-Resolution XCT*
- Advanced Materials (2026)
- DOI: https://doi.org/10.1002/adma.202516173

The implementation is based on Bambu Studio commit `bdfd004de8e9` (version `02.08.02.61`). It uses Bambu Studio's normal layerwise infill system rather than adding a general-purpose 3D lattice engine.

## Repository contents

- `patches/`: two Git patches that apply the BCCT implementation and macOS print-error UI fix to the base commit.
- `changed-files/`: browsable copies of every source and test file changed by those patches.
- `output/`: generated 3MF and pre-sliced G-code 3MF test files.
- `LICENSE`: Bambu Studio's upstream AGPL-3.0 license.

## Geometry status

Directly reproduced from the paper:

- The BCCT shear matrix and approximately 27-degree transformation.
- The published lattice basis vectors.
- The MG1 crop and reflections across XY, YZ, and XZ.

Mathematically inferred because the paper does not publish a complete edge list or global reflection phase:

- Standard BCC centre-to-eight-corners connectivity.
- A periodic reflection origin that preserves coincident twin-boundary nodes.

Added specifically for FDM:

- Layer-slab intersection of 3D struts.
- Extrusion-width overlap between layers.
- Cell-size quantization to whole layer intervals.
- Density mapping based on extrusion cross-section and sheared member length.

No claim is made that an FDM print reproduces the paper's reported mechanical improvements. Physical testing is required.

## Applying the patches

Check out Bambu Studio at commit `bdfd004de8e9`, then apply the files in `patches/` in numeric order with `git am`.

The source-built application can slice BCCT directly. Bambu's proprietary networking module requires an officially signed Bambu application for printer upload, so pre-sliced `.gcode.3mf` files should be opened and sent using an official signed Bambu Studio release.
