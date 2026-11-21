# Asset provenance

## Archive illustration

- Workspace asset: `bin/data/art/archive.png`
- Created specifically for this project with the built-in OpenAI image generation tool on 2026-11-22.
- Mode: `builtin`; original generation, no third-party reference images.
- Use: menu and result-screen illustration. Buttons, icons, typography and all puzzle observations are drawn by the program.
- No externally downloaded stock image, logo or copyrighted character is included.

Prompt:

> Use case: illustration-story. Asset type: original background illustration for a polished desktop computer-vision puzzle game called Mirror Archive; no lettering of any kind. Wide landscape 3:2 composition, 1536 by 1024 if possible. Create an exquisite painterly editorial illustration of a quiet, slightly magical natural-history archive at dusk. On the right half, a large antique oval brass observation mirror with a luminous teal glass face, delicate astronomical engraved metal, on a deep green wooden desk. Around it: a few blue-and-gold enamel celestial medallions, botanical specimen sheets, an old field notebook, a small glass vial, moth wings and a folded star chart. On the far right a tall arch-shaped window reveals a deep forest-green night. Left half has atmospheric empty parchment-colored wall and muted desk space with very sparse detail, intended for overlaid game title. Color palette warm ivory parchment #F3EFE5, forest green #173D38, oxidized turquoise #5C9D91, muted gold #B89352, small coral accent. Sophisticated hand-painted gouache and colored pencil with delicate ink outlines, visible subtle paper grain, elegant European illustrated mystery-game art direction. Warm low sunlight with soft teal reflections. Rich tactile surfaces, layered depth, refined gentle lighting, no photorealistic UI, no text, no letters, no numbers, no watermark, no buttons, no logos, no human characters. The scene should be beautiful and mysterious, inviting rather than spooky.

## Procedural puzzle images

`LevelFactory.h` generates paper grain, enamel seals, engraved plates and moth frames with fixed seeds. These are original code-generated graphics. Images in the evidence notebook are produced at runtime from the actual game pipeline.

## Fonts and dependencies

- Avenir Next and Baskerville are loaded from macOS system locations, not copied into the repository.
- openFrameworks and OpenCV are external build dependencies. Their source archives, binaries and licenses remain in the locally installed openFrameworks distribution; this repository does not repackage them.
- No analytics, external API, camera or microphone access is used at runtime.
