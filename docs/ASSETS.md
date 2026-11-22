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
- `src/LevelFactory.h`: runtime nearest-neighbor scene sampling, damaged observations, reference crop and alpha-composited film sequence.
- `docs/*.png`: actual running-app captures, not mockups.
- No system font files are required or redistributed.

## Dependencies

openFrameworks and OpenCV are external build dependencies. Their source archives, binaries and licenses remain in the locally installed openFrameworks distribution. No analytics, runtime network requests or microphone access is used. The current game optionally uses a local webcam in chapter two.

## Connected story scenes (2026-11-22)

Mode: `builtin` (OpenAI image generation tool), original generation with no reference-image inputs. Generated originals are preserved; project copies are loaded and sampled at runtime, not externally repainted. These are original assets inspired by cozy 16-bit pixel games, not copied Stardew Valley assets. No additional fonts or addons were downloaded.

### storeroom

Asset: `bin/data/art/storeroom.png`. Transparent background: false.

Full prompt:

> Use case: stylized-concept. Original playable hidden-object game scene for Mirror Archive, a cozy pixel-art game where the player is an archive restorer recovering the lighthouse keeper's lost record. Rich and carefully authored 16-bit pixel art, crisp stepped edges and small deliberate pixel clusters, warm Stardew-Valley-like atmosphere but original scenery and objects. Wide landscape 8:5 composition. Fill the whole image with the actual room, no UI, no text, no letters, no numbers, no border, no logo, no humans. Many charming detailed objects to search among, coherent natural arrangement, not a grid or a set of isolated cards. Readable at 480x300 game pixels. Chapter 1: a rain-damaged old coastal archive storeroom in warm afternoon lamplight. Deep wooden shelves packed with leather-bound journals, folded fabric, rolled nautical charts, brass instruments, colored ceramic pots, glass bottles, a globe, a miniature lighthouse, old clocks, bundles of letters, baskets and wooden crates. An arched window upper left shows coastal rain. In the LOWER RIGHT quadrant, a small single brass skeleton key with a distinctive TURQUOISE TEAL rounded bow rests diagonally on a pale folded cloth on a crate, partly surrounded by a brown journal and dried flowers. The key is a real scene object about 7 percent of image width, entirely visible but integrated among clutter. Include a few teal glass jars elsewhere as color distractors, and an ordinary copper key higher left as a shape distractor. Avoid any other teal key. The lower center has scattered papers and a rug. Layered dense detail, inviting discovery, warm chestnut, candle gold, faded sage and restrained turquoise. Do not make a giant key icon or highlight the target with an outline or pointer.

### maproom

Asset: `bin/data/art/maproom.png`. Transparent background: false.

Full prompt:

> Use case: stylized-concept. Original playable hidden-object game scene for Mirror Archive, a cozy pixel-art game where the player is an archive restorer recovering the lighthouse keeper's lost record. Rich and carefully authored 16-bit pixel art, crisp stepped edges and small deliberate pixel clusters, warm Stardew-Valley-like atmosphere but original scenery and objects. Wide landscape 8:5 composition. Fill the whole image with the actual room, no UI, no text, no letters, no numbers, no border, no logo, no humans. Many charming detailed objects to search among, coherent natural arrangement, not a grid or a set of isolated cards. Readable at 480x300 game pixels. Chapter 2: the lighthouse keeper's cartography room, now opened with the recovered key. A beautiful richly cluttered study with deep wood bookcases, nautical maps on the walls, brass telescope, globe, compass, shell specimens, blue ink bottles, open drawers, pinned sketches, lanterns and plants. A broad angled desk across the foreground and books on side shelves create a real lived-in environment. In the LOWER LEFT / lower-middle area of the desk, place one aged cream map showing a densely detailed small circular navy-and-gold lighthouse seal: lighthouse, wave bands, fine branching leaf ornaments, tiny dots and engraved lines, no lettering. The complete map sheet occupies about 20 percent of image width and is tilted slightly clockwise; its illustrated seal should be about 11 percent of image width, visibly detailed enough to compare visual texture. Several other papers naturally scattered across different depths show similarly ornate seals with a ship, an anchor or a sea bird, not lighthouses. The lighthouse map is not isolated on a card or aligned in a row. Weathered parchment, teal curtains, amber lamps, copper tools, abundant tiny interesting things. No target glow or pointer. No text or glyphs on any papers.

### greenhouse

Asset: `bin/data/art/greenhouse.png`. Transparent background: false.

Full prompt:

> Use case: stylized-concept. Original playable hidden-object game scene for Mirror Archive, a cozy pixel-art game where the player is an archive restorer recovering the lighthouse keeper's lost record. Rich and carefully authored 16-bit pixel art, crisp stepped edges and small deliberate pixel clusters, warm Stardew-Valley-like atmosphere but original scenery and objects. Wide landscape 8:5 composition. Fill the whole image with the actual room, no UI, no text, no letters, no numbers, no border, no logo, no humans. Many charming detailed objects to search among, coherent natural arrangement, not a grid or a set of isolated cards. Readable at 480x300 game pixels. Chapter 3: a richly detailed old coastal greenhouse at night, seen as a frame of the keeper's final film reel. Full-height arched glass panes and dark iron ribs, a violet starry sky, a distant lighthouse visible through the upper-right windows, hanging vines, dense ferns, potted flowers, seed trays, a stone fountain, a bench with an old film canister and gardening tools, wooden planters and a lantern near the bottom. Moonlit teal-blue foliage, purple shadows and small pools of warm golden light. Reserve the middle and upper-middle air above the plants for six small animated moths that will be added by the game: keep those air spaces naturally dark and readable. Absolutely NO moths, butterflies, birds, fireflies or other flying creatures already in the image. It must be a beautiful detailed environment, not empty black space or horizontal lanes. Same cozy handcrafted pixel-art world as the archive storeroom and cartography study.

### moth

Asset: `bin/data/art/moth.png`. Transparent background: true.

Full prompt:

> Use case: stylized-concept. Game sprite asset, a single beautifully detailed small golden-ivory archive moth on a genuinely transparent background. Front/top view with two wings spread symmetrically left and right, wings mostly horizontal, delicate scalloped edges, amber eye spots and dark cocoa vein patterns, a little teal thorax, two antennae. Original 16-bit pixel art matching a cozy magical greenhouse mystery game. Deliberate crisp square pixel clusters; no smooth antialiasing, no painterly blur. Rich, stable non-repeating wing texture that will remain readable at approximately 64 by 44 pixels in the game. Centered single moth filling most of the canvas with a little transparent padding. No glow, shadow outside the body, background, lettering, logo, frame, sprite sheet, or other objects.


## Town photograph and tonton (current three-chapter story)

- Asset: `bin/data/art/town-photo.png`; generated 2026-11-22 using the built-in imagegen tool (`builtin`), new image, no reference inputs. Original generation retained at the tool destination; project has its own copy.
- Runtime nearest-neighbor sampling and overlapping crops in `ArchiveGame.h`; the exhibit is computed from registered scans.
- `src/Tonton.h` is an original code-native pixel sprite with separate limbs and expression/prop states. No external character sprites or fonts are used.
- Earlier storeroom/maproom/greenhouse/moth assets remain as regression samples and are not in the current story.

Final imagegen prompt:

> Use case: illustration-story. Asset type: central photograph for a cozy pixel art archive restoration game, landscape 3:2. A charming old town's very first group photograph, 12 distinct villagers and a tiny round pink pig archive assistant together in front of a detailed warm timber town hall, flower boxes, brick paths, bunting, old camera on tripod at left, bicycle at right, leafy trees. Everyone poses happily. Rich crisp authentic 16-bit pixel art with visible square pixels, limited warm autumn palette of honey gold, terracotta, moss green, dusty teal, pink. Fine distinctive details distributed throughout entire image including corners to support feature matching: varied flowers cobblestones building shingles windows clothes. Flat front facing composition, no perspective frame, no border, no text or letters, no watermark, no blur, no gradients. Whole image is the photo content. Charming storybook farming-game atmosphere. High quality detailed hand crafted pixel scene.

## Hand tracking

- `bin/data/hand/hand_landmarker.task`: official Google MediaPipe float16 model, version 1.
- Download: https://storage.googleapis.com/mediapipe-models/hand_landmarker/hand_landmarker/float16/1/hand_landmarker.task
- Model documentation: https://ai.google.dev/edge/mediapipe/solutions/vision/hand_landmarker
- MediaPipe SDK: https://github.com/google-ai-edge/mediapipe (Apache-2.0); upstream license is retained in `bin/data/hand/LICENSE-MediaPipe.txt`, and package notices remain in the Python environment. The model is distributed with the official MediaPipe solution; see the linked model documentation and upstream license.
- Python dependency pins are in `requirements-camera.txt`; installation environment is excluded from Git. No camera images are included in this repository.
