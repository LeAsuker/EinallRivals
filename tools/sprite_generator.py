#!/usr/bin/env python3
"""
Sprite Generator — Procedural pixel-art scaffold for Grand-Grid-Dynasty.

Usage:
    python3 tools/sprite_generator.py --all
    python3 tools/sprite_generator.py --terrain plains --size 32
    python3 tools/sprite_generator.py --structure warg_lair --size 32
    python3 tools/sprite_generator.py --unit darkus_militia --size 32

Adding a new sprite:
    1. Write a generator function (see examples below).
    2. Register it in the TERRAIN_GENERATORS, STRUCTURE_GENERATORS,
       or UNIT_GENERATORS dict.
    3. Run: python3 tools/sprite_generator.py --<type> <name>

The script uses only Pillow (PIL) — no external deps.
"""

import argparse
import math
import os
import random
from typing import Callable, Dict, List, Tuple

from PIL import Image, ImageDraw, ImageFilter

# ─────────────────────────────────────────────────────────────────────────────
# Constants
# ─────────────────────────────────────────────────────────────────────────────

PROJECT_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
RESOURCES_DIR = os.path.join(PROJECT_ROOT, "resources")

DEFAULT_SIZE = 32

# Pixel-art palettes (R, G, B)
PALETTES = {
    "plains": [
        (34, 139, 34),   # forest green
        (50, 205, 50),   # lime green
        (60, 179, 113),  # medium sea green
        (107, 142, 35),  # olive drab
        (154, 205, 50),  # yellow green
    ],
    "sea": [
        (0, 105, 148),   # sea blue
        (0, 149, 182),   # pacific blue
        (70, 130, 180),  # steel blue
        (25, 25, 112),   # midnight blue
        (0, 191, 255),   # deep sky blue
    ],
    "mountain": [
        (105, 105, 105), # dim gray
        (128, 128, 128), # gray
        (169, 169, 169), # dark gray
        (192, 192, 192), # silver
        (220, 220, 220), # gainsboro
        (47, 79, 79),    # dark slate gray
    ],
    "forest": [
        (0, 100, 0),     # dark green
        (34, 139, 34),   # forest green
        (0, 128, 0),     # green
        (85, 107, 47),   # dark olive green
        (107, 142, 35),  # olive drab
    ],
    "deep_forest": [
        (0, 50, 0),
        (10, 60, 10),
        (0, 80, 20),
        (20, 40, 20),
        (5, 30, 5),
    ],
    "arctic": [
        (240, 248, 255), # alice blue
        (220, 220, 220), # gainsboro
        (176, 196, 222), # light steel blue
        (119, 136, 153), # light slate gray
        (255, 250, 250), # snow
    ],
    "deep_sea": [
        (0, 0, 139),     # dark blue
        (0, 0, 128),     # navy
        (25, 25, 112),   # midnight blue
        (0, 0, 205),     # medium blue
        (0, 0, 80),
    ],
    "base": [
        (210, 105, 30),  # chocolate
        (205, 133, 63),  # peru
        (222, 184, 135), # burlywood
        (139, 69, 19),   # saddle brown
        (244, 164, 96),  # sandy brown
    ],
    "darkus": [
        (75, 0, 130),    # indigo
        (48, 25, 52),    # dark purple
        (128, 0, 128),   # purple
        (25, 0, 51),
    ],
    "ventus": [
        (0, 128, 128),   # teal
        (64, 224, 208),  # turquoise
        (0, 206, 209),   # dark turquoise
        (0, 80, 80),
    ],
    "gaia": [
        (34, 139, 34),   # forest green
        (85, 107, 47),   # dark olive green
        (107, 142, 35),  # olive drab
        (0, 100, 0),
    ],
    "warg": [
        (80, 60, 40),    # brown fur
        (120, 90, 60),   # lighter fur
        (40, 30, 20),    # dark fur
        (200, 50, 50),   # red eyes
    ],
    "structure_wood": [
        (139, 90, 43),   # sienna
        (160, 120, 90),  # light wood
        (101, 67, 33),   # dark wood
        (80, 50, 30),
    ],
    "structure_stone": [
        (100, 100, 110),
        (130, 130, 140),
        (80, 80, 90),
        (60, 60, 70),
    ],
}


# ─────────────────────────────────────────────────────────────────────────────
# Low-level helpers
# ─────────────────────────────────────────────────────────────────────────────

def new_image(size: int, color: Tuple[int, int, int] = (0, 0, 0, 0)) -> Image.Image:
    """Create a new RGBA image of `size x size`."""
    return Image.new("RGBA", (size, size), color)


def put_pixel(img: Image.Image, x: int, y: int, color: Tuple[int, ...]) -> None:
    """Set a single pixel if in bounds."""
    if 0 <= x < img.width and 0 <= y < img.height:
        img.putpixel((x, y), color)


def random_color(palette: List[Tuple[int, int, int]], alpha: int = 255) -> Tuple[int, ...]:
    """Pick a random color from a palette and add alpha."""
    c = random.choice(palette)
    return (*c, alpha)


def lerp(a: float, b: float, t: float) -> float:
    """Linear interpolation."""
    return a + (b - a) * t


def lerp_color(
    c1: Tuple[int, int, int], c2: Tuple[int, int, int], t: float
) -> Tuple[int, int, int]:
    """Blend two RGB colors."""
    return (
        int(lerp(c1[0], c2[0], t)),
        int(lerp(c1[1], c2[1], t)),
        int(lerp(c1[2], c2[2], t)),
    )


def value_noise(x: float, y: float, seed: int = 0) -> float:
    """Simple value noise in [0, 1]."""
    random.seed(int(x * 374761393 + y * 668265263 + seed))
    val = random.random()
    random.seed()  # reset
    return val


def smooth_noise(x: float, y: float, seed: int = 0) -> float:
    """Bilinear-interpolated value noise."""
    ix, iy = int(x), int(y)
    fx, fy = x - ix, y - iy

    a = value_noise(ix, iy, seed)
    b = value_noise(ix + 1, iy, seed)
    c = value_noise(ix, iy + 1, seed)
    d = value_noise(ix + 1, iy + 1, seed)

    # smoothstep
    fx = fx * fx * (3 - 2 * fx)
    fy = fy * fy * (3 - 2 * fy)

    return lerp(lerp(a, b, fx), lerp(c, d, fx), fy)


def fbm(x: float, y: float, octaves: int = 4, seed: int = 0) -> float:
    """Fractal Brownian Motion noise in [0, 1]."""
    total = 0.0
    amplitude = 1.0
    frequency = 1.0
    max_value = 0.0
    for _ in range(octaves):
        total += smooth_noise(x * frequency, y * frequency, seed) * amplitude
        max_value += amplitude
        amplitude *= 0.5
        frequency *= 2.0
    return total / max_value


def ordered_dither(img: Image.Image, palette: List[Tuple[int, int, int]]) -> Image.Image:
    """
    Reduce an image to a limited palette using ordered (Bayer) dithering.
    Returns a new RGBA image.
    """
    bayer = [
        [0, 8, 2, 10],
        [12, 4, 14, 6],
        [3, 11, 1, 9],
        [15, 7, 13, 5],
    ]
    out = new_image(img.width)
    for y in range(img.height):
        for x in range(img.width):
            r, g, b, a = img.getpixel((x, y))
            if a < 128:
                continue
            threshold = bayer[y % 4][x % 4] / 16.0
            # Find nearest palette color with dither bias
            best_idx = 0
            best_dist = float("inf")
            for i, (pr, pg, pb) in enumerate(palette):
                dr = r + (threshold * 30 - 15) - pr
                dg = g + (threshold * 30 - 15) - pg
                db = b + (threshold * 30 - 15) - pb
                dist = dr * dr + dg * dg + db * db
                if dist < best_dist:
                    best_dist = dist
                    best_idx = i
            out.putpixel((x, y), (*palette[best_idx], a))
    return out


def draw_rect(
    img: Image.Image,
    x: int,
    y: int,
    w: int,
    h: int,
    color: Tuple[int, ...],
    outline: Tuple[int, ...] = None,
    outline_width: int = 1,
) -> None:
    """Draw a filled rectangle with optional outline."""
    draw = ImageDraw.Draw(img)
    draw.rectangle([x, y, x + w - 1, y + h - 1], fill=color, outline=outline, width=outline_width)


def draw_circle(
    img: Image.Image,
    cx: int,
    cy: int,
    r: int,
    color: Tuple[int, ...],
    outline: Tuple[int, ...] = None,
    outline_width: int = 1,
) -> None:
    """Draw a filled circle with optional outline."""
    draw = ImageDraw.Draw(img)
    bbox = [cx - r, cy - r, cx + r, cy + r]
    draw.ellipse(bbox, fill=color, outline=outline, width=outline_width)


def draw_triangle(
    img: Image.Image,
    p1: Tuple[int, int],
    p2: Tuple[int, int],
    p3: Tuple[int, int],
    color: Tuple[int, ...],
    outline: Tuple[int, ...] = None,
) -> None:
    """Draw a filled triangle."""
    draw = ImageDraw.Draw(img)
    draw.polygon([p1, p2, p3], fill=color, outline=outline)


def shade_from_top(img: Image.Image, color_top: Tuple[int, ...], color_bot: Tuple[int, ...]) -> None:
    """Apply a vertical gradient."""
    for y in range(img.height):
        t = y / (img.height - 1)
        c = lerp_color(color_top[:3], color_bot[:3], t)
        for x in range(img.width):
            r, g, b, a = img.getpixel((x, y))
            if a > 0:
                img.putpixel((x, y), (*c, a))


def add_noise_overlay(
    img: Image.Image,
    palette: List[Tuple[int, int, int]],
    density: float = 0.3,
    seed: int = 0,
) -> None:
    """Sprinkle random palette pixels on top of existing content."""
    random.seed(seed)
    for y in range(img.height):
        for x in range(img.width):
            if random.random() < density:
                r, g, b, a = img.getpixel((x, y))
                if a > 0:
                    c = random.choice(palette)
                    img.putpixel((x, y), (*c, a))
    random.seed()


# ─────────────────────────────────────────────────────────────────────────────
# Terrain generators
# ─────────────────────────────────────────────────────────────────────────────

def gen_plains(size: int) -> Image.Image:
    """Grass-like terrain with gentle noise variation."""
    img = new_image(size)
    pal = PALETTES["plains"]
    seed = random.randint(0, 100000)
    for y in range(size):
        for x in range(size):
            n = fbm(x / 8.0, y / 8.0, octaves=3, seed=seed)
            idx = int(n * (len(pal) - 1))
            idx = max(0, min(len(pal) - 1, idx))
            # Add some darker patches
            if n > 0.75 and random.random() > 0.5:
                idx = min(len(pal) - 1, idx + 1)
            img.putpixel((x, y), (*pal[idx], 255))
    return ordered_dither(img, pal)


def gen_sea(size: int) -> Image.Image:
    """Water with wave-like horizontal bands."""
    img = new_image(size)
    pal = PALETTES["sea"]
    seed = random.randint(0, 100000)
    for y in range(size):
        for x in range(size):
            wave = math.sin((x + y * 0.3) * 0.4 + seed)
            n = fbm(x / 6.0, y / 6.0, octaves=2, seed=seed)
            val = (wave * 0.3 + n * 0.7 + 1.0) / 2.0
            idx = int(val * (len(pal) - 1))
            idx = max(0, min(len(pal) - 1, idx))
            img.putpixel((x, y), (*pal[idx], 255))
    return ordered_dither(img, pal)


def gen_mountains(size: int) -> Image.Image:
    """Rocky gray terrain with jagged noise."""
    img = new_image(size)
    pal = PALETTES["mountain"]
    seed = random.randint(0, 100000)
    for y in range(size):
        for x in range(size):
            n = fbm(x / 5.0, y / 5.0, octaves=4, seed=seed)
            # Bias toward lighter at top (snow caps)
            bias = 1.0 - (y / size)
            val = n * 0.7 + bias * 0.3
            idx = int(val * (len(pal) - 1))
            idx = max(0, min(len(pal) - 1, idx))
            img.putpixel((x, y), (*pal[idx], 255))
    return ordered_dither(img, pal)


def gen_forest(size: int) -> Image.Image:
    """Green terrain with tree-top blobs."""
    img = new_image(size)
    pal = PALETTES["forest"]
    seed = random.randint(0, 100000)
    for y in range(size):
        for x in range(size):
            n = fbm(x / 7.0, y / 7.0, octaves=3, seed=seed)
            idx = int(n * (len(pal) - 1))
            idx = max(0, min(len(pal) - 1, idx))
            img.putpixel((x, y), (*pal[idx], 255))
    # Add darker tree blobs
    random.seed(seed + 1)
    for _ in range(size // 2):
        tx = random.randint(2, size - 3)
        ty = random.randint(2, size - 3)
        for dy in range(-1, 2):
            for dx in range(-1, 2):
                put_pixel(img, tx + dx, ty + dy, (*pal[0], 255))
    random.seed()
    return ordered_dither(img, pal)


def gen_deep_forest(size: int) -> Image.Image:
    """Very dark green, nearly black, dense forest."""
    img = new_image(size)
    pal = PALETTES["deep_forest"]
    seed = random.randint(0, 100000)
    for y in range(size):
        for x in range(size):
            n = fbm(x / 4.0, y / 4.0, octaves=4, seed=seed)
            idx = int(n * (len(pal) - 1))
            idx = max(0, min(len(pal) - 1, idx))
            img.putpixel((x, y), (*pal[idx], 255))
    # Add dense black blobs
    random.seed(seed + 1)
    for _ in range(size):
        tx = random.randint(1, size - 2)
        ty = random.randint(1, size - 2)
        put_pixel(img, tx, ty, (5, 10, 5, 255))
        put_pixel(img, tx + 1, ty, (0, 5, 0, 255))
        put_pixel(img, tx, ty + 1, (0, 5, 0, 255))
    random.seed()
    return ordered_dither(img, pal)


def gen_arctic(size: int) -> Image.Image:
    """Snowy / hilly terrain with white and ice-blue patches."""
    img = new_image(size)
    pal = PALETTES["arctic"]
    seed = random.randint(0, 100000)
    for y in range(size):
        for x in range(size):
            n = fbm(x / 6.0, y / 6.0, octaves=3, seed=seed)
            # Snow on top, exposed rock below
            bias = 1.0 - (y / size) * 0.5
            val = n * 0.6 + bias * 0.4
            idx = int(val * (len(pal) - 1))
            idx = max(0, min(len(pal) - 1, idx))
            img.putpixel((x, y), (*pal[idx], 255))
    return ordered_dither(img, pal)


def gen_deep_sea(size: int) -> Image.Image:
    """Dark, ominous deep water."""
    img = new_image(size)
    pal = PALETTES["deep_sea"]
    seed = random.randint(0, 100000)
    for y in range(size):
        for x in range(size):
            n = fbm(x / 5.0, y / 5.0, octaves=3, seed=seed)
            idx = int(n * (len(pal) - 1))
            idx = max(0, min(len(pal) - 1, idx))
            img.putpixel((x, y), (*pal[idx], 255))
    return ordered_dither(img, pal)


def gen_base(size: int) -> Image.Image:
    """Fortified ground / player base tile."""
    img = new_image(size)
    pal = PALETTES["base"]
    seed = random.randint(0, 100000)
    for y in range(size):
        for x in range(size):
            n = fbm(x / 6.0, y / 6.0, octaves=2, seed=seed)
            idx = int(n * (len(pal) - 1))
            idx = max(0, min(len(pal) - 1, idx))
            img.putpixel((x, y), (*pal[idx], 255))
    # Draw a small fort emblem in center
    cx, cy = size // 2, size // 2
    draw_rect(img, cx - 4, cy - 3, 8, 6, (180, 60, 30, 255), outline=(100, 30, 15, 255))
    draw_rect(img, cx - 2, cy - 5, 4, 2, (180, 60, 30, 255))
    return ordered_dither(img, pal)


TERRAIN_GENERATORS: Dict[str, Callable[[int], Image.Image]] = {
    "plains": gen_plains,
    "mountain": gen_mountains,
    "sea": gen_sea,
    "forest": gen_forest,
    "deep_forest": gen_deep_forest,
    "arctic": gen_arctic,
    "deep_sea": gen_deep_sea,
    "base": gen_base,
}


# ─────────────────────────────────────────────────────────────────────────────
# Structure generators
# ─────────────────────────────────────────────────────────────────────────────

def gen_warg_lair(size: int) -> Image.Image:
    """A dark burrow / nest structure."""
    img = new_image(size)
    pal = PALETTES["structure_stone"]
    stone = random_color(pal[:3])
    dark = (40, 35, 30, 255)

    # Mound base
    draw_rect(img, 4, 12, 24, 18, stone, outline=dark)
    # Cave mouth
    draw_rect(img, 10, 14, 12, 8, dark)
    # Stalactite-like teeth
    put_pixel(img, 12, 14, (200, 200, 200, 255))
    put_pixel(img, 19, 14, (200, 200, 200, 255))
    return img


def gen_abandoned_hut(size: int) -> Image.Image:
    """Small wooden shack."""
    img = new_image(size)
    wood = random_color(PALETTES["structure_wood"])
    dark = (60, 40, 25, 255)
    roof = (120, 50, 30, 255)

    # Walls
    draw_rect(img, 6, 12, 20, 16, wood, outline=dark)
    # Roof (triangle)
    draw_triangle(img, (4, 12), (16, 2), (28, 12), roof, outline=dark)
    # Door
    draw_rect(img, 13, 18, 6, 10, dark)
    return img


def gen_fort(size: int) -> Image.Image:
    """Stone fort with battlements."""
    img = new_image(size)
    stone = (140, 140, 150, 255)
    dark = (80, 80, 90, 255)
    roof = (120, 60, 30, 255)

    # Main keep body
    draw_rect(img, 4, 6, 24, 20, stone, outline=dark)
    # Battlements on top
    for x in range(4, 28, 4):
        draw_rect(img, x, 2, 3, 5, stone, outline=dark)
    # Door
    draw_rect(img, 12, 20, 8, 8, dark)
    # Flag on top
    draw_rect(img, 15, 0, 2, 4, (160, 40, 40, 255))
    draw_rect(img, 16, 1, 6, 3, (200, 50, 50, 255))
    return img


STRUCTURE_GENERATORS: Dict[str, Callable[[int], Image.Image]] = {
    "warg_lair": gen_warg_lair,
    "abandoned_hut": gen_abandoned_hut,
    "fort": gen_fort,
}


# ─────────────────────────────────────────────────────────────────────────────
# Unit generators
# ─────────────────────────────────────────────────────────────────────────────

def _draw_humanoid(
    img: Image.Image,
    body_color: Tuple[int, ...],
    accent_color: Tuple[int, ...],
    outline_color: Tuple[int, ...],
    weapon: str = "spear",
) -> None:
    """Draw a simple humanoid warrior. Weapon can be 'spear', 'sword', or None."""
    w, h = img.width, img.height
    cx, cy = w // 2, h // 2

    # Head
    draw_circle(img, cx, cy - 6, 5, body_color, outline=outline_color)
    # Body
    draw_rect(img, cx - 4, cy - 1, 8, 10, body_color, outline=outline_color)
    # Legs
    draw_rect(img, cx - 4, cy + 9, 3, 6, outline_color)
    draw_rect(img, cx + 1, cy + 9, 3, 6, outline_color)
    # Arms
    draw_rect(img, cx - 7, cy, 3, 6, body_color, outline=outline_color)
    draw_rect(img, cx + 4, cy, 3, 6, body_color, outline=outline_color)

    # Weapon
    if weapon == "spear":
        draw_rect(img, cx + 5, cy - 8, 2, 14, accent_color, outline=outline_color)
        draw_rect(img, cx + 4, cy - 8, 4, 2, accent_color)
    elif weapon == "sword":
        # Blade
        draw_rect(img, cx + 6, cy - 6, 2, 10, accent_color, outline=outline_color)
        # Crossguard
        draw_rect(img, cx + 5, cy + 3, 4, 1, accent_color)
        # Hilt
        draw_rect(img, cx + 6, cy + 4, 2, 3, outline_color)


def gen_darkus_militia(size: int) -> Image.Image:
    """Darkus faction militia — purple-toned."""
    img = new_image(size)
    body = (90, 50, 130, 255)
    accent = (180, 160, 200, 255)
    outline = (30, 10, 50, 255)
    _draw_humanoid(img, body, accent, outline, weapon="spear")
    return img


def gen_ventus_militia(size: int) -> Image.Image:
    """Ventus faction militia — teal-toned."""
    img = new_image(size)
    body = (50, 120, 120, 255)
    accent = (150, 220, 220, 255)
    outline = (10, 50, 50, 255)
    _draw_humanoid(img, body, accent, outline, weapon="spear")
    return img


def gen_swordsman(size: int) -> Image.Image:
    """Generic swordsman — steel and leather."""
    img = new_image(size)
    body = (100, 80, 60, 255)   # leather brown
    accent = (180, 190, 200, 255)  # steel silver
    outline = (40, 30, 20, 255)
    _draw_humanoid(img, body, accent, outline, weapon="sword")
    # Helmet visor
    cx = size // 2
    draw_rect(img, cx - 3, size // 2 - 10, 6, 2, (60, 50, 40, 255))
    return img


def gen_spearman(size: int) -> Image.Image:
    """Generic spearman — bronze and cloth."""
    img = new_image(size)
    body = (140, 120, 80, 255)   # tan cloth
    accent = (180, 140, 60, 255)  # bronze
    outline = (60, 50, 30, 255)
    _draw_humanoid(img, body, accent, outline, weapon="spear")
    # Small shield on left arm
    cx = size // 2
    draw_circle(img, cx - 8, size // 2 + 2, 3, (120, 100, 70, 255), outline=outline)
    return img


def gen_warg(size: int) -> Image.Image:
    """Warg / wolf beast."""
    img = new_image(size)
    pal = PALETTES["warg"]
    fur = pal[0]
    light = pal[1]
    dark = pal[2]
    eye = pal[3]
    outline = (20, 15, 10, 255)

    # Body (horizontal oval)
    draw_rect(img, 6, 14, 20, 10, (*fur, 255), outline=outline)
    # Head
    draw_rect(img, 22, 10, 8, 8, (*fur, 255), outline=outline)
    # Snout
    draw_rect(img, 28, 14, 3, 3, (*light, 255), outline=outline)
    # Ears
    draw_triangle(img, (24, 10), (26, 4), (28, 10), (*dark, 255), outline=outline)
    draw_triangle(img, (28, 10), (30, 4), (30, 10), (*dark, 255), outline=outline)
    # Legs
    draw_rect(img, 8, 24, 3, 5, (*dark, 255), outline=outline)
    draw_rect(img, 14, 24, 3, 5, (*dark, 255), outline=outline)
    draw_rect(img, 20, 24, 3, 5, (*dark, 255), outline=outline)
    # Tail
    draw_rect(img, 4, 16, 4, 2, (*dark, 255))
    # Eyes
    put_pixel(img, 26, 12, eye)
    put_pixel(img, 28, 12, eye)
    return img


UNIT_GENERATORS: Dict[str, Callable[[int], Image.Image]] = {
    "darkus_militia": gen_darkus_militia,
    "ventus_militia": gen_ventus_militia,
    "warg": gen_warg,
    "swordsman": gen_swordsman,
    "spearman": gen_spearman,
}


# ─────────────────────────────────────────────────────────────────────────────
# High-level API
# ─────────────────────────────────────────────────────────────────────────────

def generate(
    name: str,
    size: int = DEFAULT_SIZE,
    category: str = None,
    seed: int = None,
) -> Image.Image:
    """
    Generate a sprite by name.

    Args:
        name: Sprite identifier (e.g. 'plains', 'warg_lair', 'darkus_militia').
        size: Output width/height in pixels (default 32).
        category: Optional override ('terrain', 'structure', 'unit').
                  If omitted, searches all three dicts.
        seed: Optional RNG seed for reproducible output.

    Returns:
        PIL Image in RGBA mode.
    """
    if seed is not None:
        random.seed(seed)

    gen = None
    if category == "terrain" or category is None:
        gen = TERRAIN_GENERATORS.get(name)
    if gen is None and (category == "structure" or category is None):
        gen = STRUCTURE_GENERATORS.get(name)
    if gen is None and (category == "unit" or category is None):
        gen = UNIT_GENERATORS.get(name)

    if gen is None:
        known = list(TERRAIN_GENERATORS.keys()) + list(STRUCTURE_GENERATORS.keys()) + list(UNIT_GENERATORS.keys())
        raise ValueError(f"Unknown sprite name '{name}'. Known: {known}")

    img = gen(size)

    if seed is not None:
        random.seed()

    return img


def save_sprite(
    name: str,
    size: int = DEFAULT_SIZE,
    category: str = None,
    seed: int = None,
    out_dir: str = None,
) -> str:
    """
    Generate and save a sprite to the resources directory.

    Returns the written file path.
    """
    img = generate(name, size, category, seed)

    # Guess category for path if not explicitly given
    if out_dir is None:
        if category == "terrain" or (category is None and name in TERRAIN_GENERATORS):
            folder = "terrain"
        elif category == "structure" or (category is None and name in STRUCTURE_GENERATORS):
            folder = "structures"
        elif category == "unit" or (category is None and name in UNIT_GENERATORS):
            folder = "units"
        else:
            folder = ""
        out_dir = os.path.join(RESOURCES_DIR, folder)

    os.makedirs(out_dir, exist_ok=True)
    # Use original naming conventions from the game
    filename = f"{name}.png"
    if name == "plains":
        filename = "plains_ter.png"
    elif name == "mountain":
        filename = "mountain_ter2.png"
    elif name == "sea":
        filename = "sea_ter.png"
    elif name == "forest":
        filename = "forest_ter2.png"
    elif name == "deep_forest":
        filename = "deep_forest_ter.png"
    elif name == "arctic":
        filename = "arctic_ter.png"
    elif name == "deep_sea":
        filename = "deep_sea_ter.png"
    elif name == "base":
        filename = "base_ter.png"

    path = os.path.join(out_dir, filename)
    img.save(path)
    return path


def generate_all(size: int = DEFAULT_SIZE, seed: int = None) -> List[str]:
    """Generate every registered sprite and return the list of saved paths."""
    paths = []
    for name in TERRAIN_GENERATORS:
        paths.append(save_sprite(name, size, "terrain", seed))
    for name in STRUCTURE_GENERATORS:
        paths.append(save_sprite(name, size, "structure", seed))
    for name in UNIT_GENERATORS:
        paths.append(save_sprite(name, size, "unit", seed))
    return paths


# ─────────────────────────────────────────────────────────────────────────────
# CLI
# ─────────────────────────────────────────────────────────────────────────────

def main():
    parser = argparse.ArgumentParser(description="Procedural sprite generator")
    parser.add_argument("--all", action="store_true", help="Regenerate every registered sprite")
    parser.add_argument("--terrain", metavar="NAME", help="Generate a terrain sprite")
    parser.add_argument("--structure", metavar="NAME", help="Generate a structure sprite")
    parser.add_argument("--unit", metavar="NAME", help="Generate a unit sprite")
    parser.add_argument("--size", type=int, default=DEFAULT_SIZE, help="Sprite size in pixels")
    parser.add_argument("--seed", type=int, default=None, help="RNG seed for reproducible output")
    parser.add_argument("--out", default=None, help="Custom output directory")
    args = parser.parse_args()

    if args.all:
        paths = generate_all(size=args.size, seed=args.seed)
        for p in paths:
            print(f"Saved: {p}")
    elif args.terrain:
        p = save_sprite(args.terrain, args.size, "terrain", args.seed, args.out)
        print(f"Saved: {p}")
    elif args.structure:
        p = save_sprite(args.structure, args.size, "structure", args.seed, args.out)
        print(f"Saved: {p}")
    elif args.unit:
        p = save_sprite(args.unit, args.size, "unit", args.seed, args.out)
        print(f"Saved: {p}")
    else:
        parser.print_help()


if __name__ == "__main__":
    main()
