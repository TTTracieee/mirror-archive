# Asset provenance

## Current story: A Light Left On (2026-11-22)

- `bin/data/art/family-home.png`: generated with the built-in OpenAI imagegen tool, new original image, no reference input. Original retained in the tool output directory; a project copy is bundled. The runtime samples the image and draws the 208 plaque before degradation and filtering.
- `src/ofApp.cpp`: original code-drawn road topology, trees, paper UI, envelope, and folding-letter animation. Generated house sprites and terrain are listed below. The red roof, lantern, teal door and flower pots connect the photo and map.
- `bin/data/digits/digits.png`: official OpenCV 4.10.0 sample dataset, https://raw.githubusercontent.com/opencv/opencv/4.10.0/samples/data/digits.png . Upstream Apache-2.0 license included as `bin/data/digits/LICENSE-OpenCV.txt`.
- Current camera input uses OpenCV capture and the official MediaPipe hand model below. No camera image is saved or uploaded. Optical flow remains only as a historical regression implementation.
- Historical town-photo and older scenes below remain regression fixtures, not current story artwork.

Final image-generation prompt (builtin mode):

> Use case: illustration-story. Asset type: original old family photograph scene for a cozy pixel-art restoration game. Landscape 3:2. Authentic detailed 16-bit pixel art, crisp square pixels, no blur, no UI or border. A little East Asian girl with short dark hair and a yellow dress holds her mother's hand outside their small childhood home. Mother wears a teal dress and has dark hair in a bun. Warm, tender quiet memory, both facing the camera. The house is a small cream-plaster cottage with a RED terracotta triangular roof, a central dark teal front door, one glowing brass porch lantern beside the door, two blue windows, and terracotta pots of white daisies. Cobblestone path, low wooden fence, flowering shrubs, a leafy tree to the left, a small stream and wooden bridge to the right. Full house visible, large and central. A small blank ivory rectangular address plaque ABOVE the door (no text; game draws its own numbers). Rich hand-crafted pixel clusters, intentional dithering, autumn ochre grass, sage foliage, warm amber lighting. Charming and intimate, no additional people, no pigs, no lettering, no logos, no watermark. At 480px wide the house and mother and daughter should read clearly.


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
- No system font files are redistributed. The current body font is the bundled OFL Departure Mono listed below.

## Dependencies

openFrameworks and OpenCV are external build dependencies. Their source archives, binaries and licenses remain in the locally installed openFrameworks distribution. No analytics, runtime network requests or microphone access is used. The current story optionally uses a local webcam in chapter three.

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


## Historical town photograph and tonton

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

## Interface refresh: Lia and the paper map (2026-11-22)

All three assets: mode `builtin`, generated by OpenAI imagegen with no reference-image input. Originals remain at the generation output destination; project copies are bundled. Runtime nearest-neighbor sampling and sprite cropping preserve crisp logical pixels. These are original assets, not extracted game artwork.

### lia-portraits.png

Project asset: `bin/data/art/lia-portraits.png`. Transparent background: true.

Full final prompt:

> Use case: illustration-story. Asset type: original pixel-art character portrait expression sheet for a cozy story game, transparent background. A single horizontal row of THREE equally sized square portrait cells, no borders. SAME young adult woman Lia in every cell: chestnut brown shoulder-length bob, warm brown eyes, cream blouse, burgundy knitted cardigan, tiny daisy hairclip, gentle approachable face, three-quarter bust facing slightly left. Cell 1 calm attentive, cell 2 sad reflective eyes, cell 3 relieved warm small smile. Real detailed 16-bit RPG pixel portrait art: deliberate square pixel clusters, beautiful shading, strong warm palette, charming expressive eyes, crisp edges, not vector, not a block mannequin. Each cell centered and fully contained with 10 percent margin, perfectly aligned equal scale, equal 1/3 canvas width, head and shoulders shown. Transparent background throughout. No text, no letters, no symbols, no logos, no other objects. Wide 3:1 sheet. Suitable for sampling each portrait at 96x96 pixels. Original character, no copied game characters.

### garden-map.png

Project asset: `bin/data/art/garden-map.png`. Transparent background: false.

Full final prompt:

> Use case: stylized-concept. Asset type: countryside terrain background for a playable old-paper maze, landscape 3:2. Beautiful intricate authentic 16-bit pixel art viewed straight from above like a charming illustrated village map. An aged folded parchment sheet fills the entire image, with an illustrated soft sage meadow over its middle 85 percent. Delicate hand drawn grassy tufts, small white daisies, tiny ochre leaves, gentle dotted texture; keep central terrain LOW CONTRAST and fairly open because the game will draw all roads and houses over it. Rich detailed scenery concentrated around the outer edges: mossy rounded rocks, miniature leafy trees with variegated foliage, short fence segments, little flower beds, garden pots, a narrow blue creek along far right margin, corner ponds with lily pads. Cream paper visible at outer border, subtle vertical and horizontal paper fold marks. Warm vintage sage, olive, terracotta, faded teal, amber colors. Crisp intentional square pixel clusters, no soft blur. No paths, roads, buildings, maze walls, text, numbers, UI, frame, characters, logos. This is supporting terrain, not a completed maze. Original art, no borrowed game assets.

### village-buildings.png

Project asset: `bin/data/art/village-buildings.png`. Transparent background: true.

Full final prompt:

> Use case: stylized-concept. Asset type: three original village building sprites for cozy 16-bit pixel art map, transparent background. Horizontal 3:1 sheet with exactly THREE equally spaced square cells. All front-facing slightly top-down RPG buildings, same small scale, each entirely within its third, centered, 12 percent transparent margin. Left cell: cream cottage with red terracotta tiled roof, central dark teal door, two blue windows, brass glowing porch lamp to right of door and two terracotta pots of white daisies. Middle: charming bakery with muted blue slate roof, striped cream and burgundy awning, basket of bread and tiny blue window. Right: small village post office with dusty violet roof, teal door, brass mail slot, a red mailbox, a flower planter. Intricate crisp pixel clusters, rich warm texture, shaded tiles, bricks, tiny weathered wood details. Each house reads clearly at 64x64 pixels. No labels, numbers, words or letters anywhere. No ground tile, shadows outside sprites, UI, scene background, people or logos. Transparent cutout sprites, not a screenshot.

### Departure Mono

`bin/data/fonts/DepartureMono-Regular.otf` and `OFL.txt`: Departure Mono 1.500 by Helena Zhang, SIL Open Font License 1.1. Official source: https://github.com/rektdeckard/departure-mono/releases/tag/v1.500 . Downloaded from the author's `DepartureMono-1.500.zip` release. The design is based on an 11px grid. The game rasterizes only at exact 11px multiples into a display-resolution FBO (e.g. 33px dialogue in 2x output), without resampling text through the low-resolution art grid. Antialiasing is disabled. The prior Pixelify Sans experiment was removed. Title bitmap font remains original code. Historical no-additional-font statements above do not apply to this refresh.
