# Asset provenance

## Pixel cottage

- Asset: `bin/data/art/pixel-cottage.png`
- Generated for this project on 2026-11-22 with the built-in OpenAI image generation tool.
- Mode: `builtin`; new original scene, no reference-image inputs.
- The prior painted archive background is retired. The current menu and result screen use this pixel cottage.
- The runtime draws original bitmap letters, sprites, frames and buttons; no game sprites, fonts or logos are copied from Stardew Valley.

Final prompt:

> Use case: stylized-concept. Asset type: original background art for a cozy pixel-art desktop mystery game, Mirror Archive. Create a wide 5:3 landscape scene in authentic hand-placed 16-bit pixel art, evoking the warmth of Stardew Valley but entirely original scenery. A tiny magical cottage workshop at blue hour: honey-colored timber beams, chestnut plank floor, shelves of dusty colorful books, potted plants and hanging leafy vines on both sides, a softly glowing oval turquoise magic mirror on a central wooden workbench. Through the right window a violet-blue sky with a crescent moon and fir trees. A few tiny firefly lights. Cozy amber lamplight, rich russet wood, wheat gold, leafy green, deep indigo and luminous teal. Compose it as a game menu background: upper center relatively quiet and dark for an overlaid wooden title sign; lower quarter a quiet plank floor for three overlaid level buttons. Mirror prominent in the middle band. Absolutely crisp square pixel clusters, stepped silhouettes, limited palette, deliberate subtle dithering, charming readable environment. The look of a 320x192 pixel scene enlarged with nearest-neighbor, not a smooth painting with a mosaic filter. No text, letters, numbers, labels, logos, UI, buttons, borders, photorealism, smooth gradients or anti-aliased curves. No humans or borrowed game sprites. Landscape image.

## Code-drawn assets

- `src/PixelUI.h`: original 5×7 bitmap alphabet, sprites and wooden UI frames.
- `src/LevelFactory.h`: deterministic pixel-grid seals, engravings, moths and observation sequences.
- `docs/*.png`: actual running-app captures, not mockups.
- No system font files are required or redistributed.

## Dependencies

openFrameworks and OpenCV are external build dependencies. Their source archives, binaries and licenses remain in the locally installed openFrameworks distribution. No analytics, network requests, camera or microphone access is used at runtime.
