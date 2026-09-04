# Bambu Studio BCCT Experimental Infill

This is an active source fork of Bambu Studio that adds an experimental sparse-infill pattern based on the triply-twinned body-centred-cubic lattice described in:

- *Triply-Twinned Metamaterials: Unraveling the Mechanics and Failure Pathways Through High-Resolution XCT*
- *Advanced Materials* (2026)
- DOI: https://doi.org/10.1002/adma.202516173

The buildable source, BCCT implementation, tests, UI icon, and macOS Bambu Connect handoff are all maintained on the `main` branch. The BCCT work was last rebased onto upstream Bambu Studio commit `66e405477`.

## What this fork changes

- Adds **BCCT Experimental** to the sparse-infill pattern list.
- Generates the structure through Bambu Studio's normal layerwise infill system.
- Varies the clipped strut geometry with Z to preserve three-dimensional BCC connectivity and reflected twin domains.
- Maps requested density to cell size using extrusion cross-section and sheared member length.
- Includes the BCCT pattern icon at `resources/images/param_bcct.svg`.
- On macOS, changes the print action to **Send with Bambu Connect**.

This is research software. No claim is made that an FDM print reproduces the mechanical improvements reported in the paper. Physical testing is required.

## Build from source

Clone `main` normally:

```bash
git clone https://github.com/truckz/BambuStudio-BCCT.git
cd BambuStudio-BCCT
```

Then follow Bambu Lab's platform build guide:

- [macOS compile guide](https://github.com/bambulab/BambuStudio/wiki/Mac-Compile-Guide)
- [Windows compile guide](https://github.com/bambulab/BambuStudio/wiki/Windows-Compile-Guide)
- [Linux compile guide](https://github.com/bambulab/BambuStudio/wiki/Linux-Compile-Guide)

The BCCT source is compiled as part of the ordinary Bambu Studio build. No separate BCCT patch or module installation is required.

## Printing from the macOS development build

The macOS print action exports the sliced plate as a `.gcode.3mf` package and hands it directly to the officially signed Bambu Connect application. An officially signed copy of Bambu Studio is not needed as an intermediate sender.

This workflow requires Bambu Connect to be installed, signed in to the user's Bambu account, and able to see the target printer. It was tested successfully on Trevor's Mac with Bambu Connect and a Bambu Lab P1S. Other operating systems and printer models have not yet been verified.

The handoff does not bypass certificate or signature checks and does not modify Bambu's proprietary network module. Bambu Connect remains responsible for authenticated printer communication.

## BCCT geometry status

Directly reproduced from the paper:

- The BCCT shear matrix and approximately 27-degree transformation.
- The published lattice basis vectors.
- The MG1 crop and reflections across XY, YZ, and XZ.

Mathematically inferred because the paper does not publish a complete edge list or global reflection phase:

- Standard BCC centre-to-eight-corners connectivity.
- A periodic reflection origin that preserves coincident twin-boundary nodes.

Added specifically for FDM:

- Layer-slab intersection of three-dimensional struts.
- Extrusion-width overlap between layers.
- Cell-size quantization to whole layer intervals.
- Density mapping based on extrusion cross-section and sheared member length.

## Generated test files

The `output/` directory contains generated 3MF and pre-sliced G-code 3MF examples. The `BCCT_42mm_Visual_Cube_With_Base` model is a native 42 × 42 × 42 mm test object with these saved settings:

- BCCT sparse infill at 5%.
- 0.60 mm sparse-infill line width with a 0.4 mm nozzle.
- Three 0.16 mm bottom layers for a 0.48 mm foundation.
- Zero side walls and zero top layers.

Open the ordinary `.3mf` with this source build to inspect or reslice it. Files ending in `_OFFICIAL_SEND.gcode.3mf` are legacy test artifacts from before the Bambu Connect handoff and are no longer required by the current macOS workflow.

## Historical patch snapshot

The `patches/` and `changed-files/` directories contain an earlier snapshot based on Bambu Studio commit `bdfd004de8e9`. They are retained for reference but do not contain the later BCCT icon or Bambu Connect work. Build the current `main` branch instead.

## Upstream and license

This fork is based on [Bambu Studio](https://github.com/bambulab/BambuStudio), which is based on [PrusaSlicer](https://github.com/prusa3d/PrusaSlicer) and [Slic3r](https://github.com/Slic3r/Slic3r).

Bambu Studio and this fork are licensed under the GNU Affero General Public License, version 3. See `LICENSE` for the full terms. Bambu's optional networking component contains non-free libraries and is not part of the BCCT implementation.
