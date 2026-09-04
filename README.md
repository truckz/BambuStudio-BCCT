# Bambu Studio BCCT Experimental Infill

This repository backs up an experimental sparse-infill implementation for Bambu Studio based on the triply-twinned body-centred-cubic lattice described in:

- *Triply-Twinned Metamaterials: Unraveling the Mechanics and Failure Pathways Through High-Resolution XCT*
- Advanced Materials (2026)
- DOI: https://doi.org/10.1002/adma.202516173

The current source build is on the `codex/bcct-infill` branch, based on Bambu Studio commit `66e405477`. It uses Bambu Studio's normal layerwise infill system rather than adding a general-purpose 3D lattice engine.

## Repository contents

- `codex/bcct-infill`: the active source branch, including BCCT infill, its UI icon, and the macOS Bambu Connect print handoff.
- `patches/`: an earlier two-patch snapshot of the BCCT implementation and macOS print-error UI fix, retained for reference.
- `changed-files/`: browsable copies of every source and test file changed by those patches.
- `output/`: generated 3MF and pre-sliced G-code 3MF test files.
- `LICENSE`: Bambu Studio's upstream AGPL-3.0 license.

### 42 mm visual test

The `BCCT_42mm_Visual_Cube_With_Base` files are a native 42 × 42 × 42 mm test model, not a scaled 3-inch object. Its print settings are:

- BCCT sparse infill at 5%.
- 0.60 mm sparse-infill line width with a 0.4 mm nozzle.
- Three 0.16 mm bottom layers for a 0.48 mm foundation.
- Zero side walls and zero top layers.

Open the ordinary `.3mf` with the BCCT-enabled source build to inspect or reslice it. The `_OFFICIAL_SEND.gcode.3mf` files are retained as legacy test artifacts; they are no longer required by the current macOS workflow.

## Printing from the macOS development build

The current `codex/bcct-infill` build changes the macOS print action to **Send with Bambu Connect**. After slicing, the development build exports the plate as a `.gcode.3mf` package and hands it directly to the officially signed Bambu Connect application. An officially signed copy of Bambu Studio is no longer needed as an intermediate sender.

This workflow requires Bambu Connect to be installed, signed in to the user's Bambu account, and able to see the target printer. It was tested successfully on Trevor's Mac with Bambu Connect and a Bambu Lab P1S. Other operating systems and printer models have not yet been verified.

The handoff does not bypass certificate or signature checks and does not modify Bambu's proprietary network module. Bambu Connect remains responsible for authenticated printer communication.

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

For the current implementation, check out the repository's `codex/bcct-infill` branch and build Bambu Studio normally. The older patch snapshot can still be reproduced by checking out Bambu Studio at commit `bdfd004de8e9` and applying the files in `patches/` in numeric order with `git am`.

The source-built application can slice BCCT directly. On macOS, use **Send with Bambu Connect** to pass the sliced job to Bambu Connect for printer selection and upload.
