"""Draws the BayuPandu brand files from one geometry.

The wing-and-wind mark here is the same shape firmware/src/UI/BrandMark.cpp draws
on the device; change the numbers in both places together. The wordmark is
Overpass Black (SIL OFL 1.1), converted to outlines so the SVGs need no font.

Run from the repository root:
    uv run --with fonttools --with cairosvg python assets/brand/gen_brand.py
"""

import math
import pathlib
import struct
import urllib.request
import zlib

from fontTools.pens.svgPathPen import SVGPathPen
from fontTools.pens.transformPen import TransformPen
from fontTools.ttLib import TTFont
from fontTools.varLib import instancer

OUT = pathlib.Path(__file__).resolve().parent
FONT_URL = "https://github.com/google/fonts/raw/main/ofl/overpass/Overpass%5Bwght%5D.ttf"

GREEN = "#00613F"
WHITE = "#FFFFFF"
INK = "#111111"
SURROUND = "#0E1A14"

# --- Mark geometry, for a mark 1 unit wide, origin at the wing's top centre ---------
HALF_SPAN_DEG = 42.0
CELLS = 9
GAP_DEG = 1.8
THICK_CENTRE = 0.18
THICK_TIP = 0.11
RADIUS = 0.5 / math.sin(math.radians(HALF_SPAN_DEG))
WIND = [(0.345, -0.18, 0.22), (0.410, -0.17, 0.21), (0.475, -0.07, 0.11)]  # y, x0, x1
WIND_THICK = 0.034
MARK_HEIGHT = WIND[2][0] + WIND_THICK / 2  # 0.492

# A favicon at 16 px cannot show nine cells and three lines; it keeps the idea with
# fewer, wider parts.
SMALL = dict(cells=5, gap_deg=6.0, wind=[(0.36, -0.20, 0.24), (0.47, -0.10, 0.14)], wind_thick=0.075)


def thickness(deg):
    f = deg / HALF_SPAN_DEG
    return THICK_CENTRE - (THICK_CENTRE - THICK_TIP) * f * f


def wing_point(rho, deg, cx, top, w):
    a = math.radians(deg)
    return cx + w * rho * math.sin(a), top + w * (RADIUS - rho * math.cos(a))


def fmt(v):
    return f"{v:.2f}".rstrip("0").rstrip(".")


def mark_svg(cx, top, w, color, cells=CELLS, gap_deg=GAP_DEG, wind=WIND, wind_thick=WIND_THICK):
    """SVG elements for the mark with its top centre at (cx, top), w wide."""
    parts = []
    cell_deg = (2 * HALF_SPAN_DEG - (cells - 1) * gap_deg) / cells
    for i in range(cells):
        a0 = -HALF_SPAN_DEG + i * (cell_deg + gap_deg)
        a1 = a0 + cell_deg
        steps = max(2, int(cell_deg))
        outer = [wing_point(RADIUS, a0 + (a1 - a0) * k / steps, cx, top, w) for k in range(steps + 1)]
        inner = [wing_point(RADIUS - thickness(a), a, cx, top, w)
                 for a in (a1 - (a1 - a0) * k / steps for k in range(steps + 1))]
        pts = " ".join(f"{fmt(x)},{fmt(y)}" for x, y in outer + inner)
        parts.append(f'<polygon points="{pts}" fill="{color}"/>')
    for side in (-1, 1):
        x, y = wing_point(RADIUS - THICK_TIP / 2, side * HALF_SPAN_DEG, cx, top, w)
        parts.append(f'<circle cx="{fmt(x)}" cy="{fmt(y)}" r="{fmt(w * THICK_TIP / 2)}" fill="{color}"/>')
    for y, x0, x1 in wind:
        parts.append(
            f'<line x1="{fmt(cx + w * x0)}" y1="{fmt(top + w * y)}" x2="{fmt(cx + w * x1)}" '
            f'y2="{fmt(top + w * y)}" stroke="{color}" stroke-width="{fmt(w * wind_thick)}" '
            f'stroke-linecap="round"/>')
    return "\n  ".join(parts)


def load_font(weight):
    cache = OUT / ".cache"
    cache.mkdir(exist_ok=True)
    var = cache / "Overpass.ttf"
    if not var.exists():
        urllib.request.urlretrieve(FONT_URL, var)
    return instancer.instantiateVariableFont(TTFont(var), {"wght": weight})


def text_path(font, text, x, baseline, size, color, tracking_em=0.0):
    """Outlines of `text` set at `size` px with its baseline at y, as one SVG path."""
    gs = font.getGlyphSet()
    cmap = font.getBestCmap()
    upm = font["head"].unitsPerEm
    scale = size / upm
    pen = SVGPathPen(gs)
    cursor = x
    for ch in text:
        name = cmap[ord(ch)]
        gs[name].draw(TransformPen(pen, (scale, 0, 0, -scale, cursor, baseline)))
        cursor += gs[name].width * scale + tracking_em * size
    width = cursor - x - tracking_em * size
    return f'<path d="{pen.getCommands()}" fill="{color}"/>', width


def text_width(font, text, size, tracking_em=0.0):
    _, w = text_path(font, text, 0, 0, size, "#000", tracking_em)
    return w


def svg(width, height, body, title):
    return (f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 {fmt(width)} {fmt(height)}" '
            f'width="{fmt(width)}" height="{fmt(height)}" role="img" aria-label="{title}">\n'
            f'  <title>{title}</title>\n  {body}\n</svg>\n')


def sign_panel(width, height, radius, border):
    inset = border / 2
    return (f'<rect x="{fmt(inset)}" y="{fmt(inset)}" width="{fmt(width - border)}" '
            f'height="{fmt(height - border)}" rx="{fmt(radius)}" fill="{GREEN}" '
            f'stroke="{WHITE}" stroke-width="{fmt(border)}"/>')


def lockup(black):
    """Mark beside the wordmark. On a sign panel, or bare in one colour."""
    size = 96
    tracking = -1 / 32  # the device sets it 1 px tighter at 32 px
    cap = 0.70 * size
    mark_w = 190
    mark_h = mark_w * MARK_HEIGHT
    pad_x, gap = 44, 40
    word_w = text_width(BLACK_FONT, "BAYUPANDU", size, tracking)
    # Bare versions still need a margin: the wing's rounded tips reach past its chord.
    margin = mark_w * THICK_TIP
    height = 220 if black is None else mark_h + 2 * margin
    width = pad_x * 2 + mark_w + gap + word_w if black is None else mark_w + gap + word_w + 2 * margin
    ox = pad_x if black is None else margin
    color = WHITE if black is None else black
    mark_top = (height - mark_h) / 2
    baseline = height / 2 + cap / 2
    word, _ = text_path(BLACK_FONT, "BAYUPANDU", ox + mark_w + gap, baseline, size, color, tracking)
    body = []
    if black is None:
        body.append(sign_panel(width, height, 22, 8))
    body.append(mark_svg(ox + mark_w / 2, mark_top, mark_w, color))
    body.append(word)
    return svg(width, height, "\n  ".join(body), "BayuPandu")


def tile(size, small=False, border=True):
    """Square sign tile with the mark, for avatars, app icons and favicons."""
    radius = size * 0.16
    b = size * 0.035 if border else 0
    mark_w = size * (0.82 if small else 0.68)
    params = SMALL if small else {}
    mark_h = mark_w * ((params["wind"][-1][0] + params["wind_thick"] / 2) if small else MARK_HEIGHT)
    if border:
        body = [sign_panel(size, size, radius, b)]
    else:
        body = [f'<rect width="{fmt(size)}" height="{fmt(size)}" rx="{fmt(radius)}" fill="{GREEN}"/>']
    body.append(mark_svg(size / 2, (size - mark_h) / 2, mark_w, WHITE, **params))
    return svg(size, size, "\n  ".join(body), "BayuPandu")


def mark_alone(color):
    w = 400
    h = w * MARK_HEIGHT
    pad = w * 0.06  # room for the rounded tips
    return svg(w + 2 * pad, h + 2 * pad, mark_svg(w / 2 + pad, pad, w, color), "BayuPandu")


def social_preview():
    """1280x640 card for the GitHub repository's social preview."""
    w, h = 1280, 640
    lock = lockup(None)
    lock_w = float(lock.split('width="')[1].split('"')[0])
    lock_h = 220
    scale = 1000 / lock_w
    inner = lock.split("</title>\n  ", 1)[1].rsplit("\n</svg>", 1)[0]
    tag, tag_w = text_path(BOLD_FONT, "OPEN-SOURCE PARAGLIDING FLIGHT COMPUTER", 0, 0, 34, WHITE, 0.06)
    tag, _ = text_path(BOLD_FONT, "OPEN-SOURCE PARAGLIDING FLIGHT COMPUTER", (w - tag_w) / 2, 470, 34,
                       WHITE, 0.06)
    body = (f'<rect width="{w}" height="{h}" fill="{SURROUND}"/>\n'
            f'  <g transform="translate({fmt((w - 1000) / 2)} {fmt(150)}) scale({fmt(scale)})">{inner}</g>\n'
            f'  {tag}')
    return svg(w, h, body, "BayuPandu: open-source paragliding flight computer")


def stamp_origin(png, origin):
    """Records where a PNG came from in a tEXt chunk, so the file explains itself."""
    data = png.read_bytes()
    body = b"impeccable:prompt\x00" + origin.encode("latin-1")
    chunk = struct.pack(">I", len(body)) + b"tEXt" + body
    chunk += struct.pack(">I", zlib.crc32(b"tEXt" + body) & 0xFFFFFFFF)
    ihdr_end = 8 + 12 + struct.unpack(">I", data[8:12])[0]
    png.write_bytes(data[:ihdr_end] + chunk + data[ihdr_end:])


# The 16 px favicon is drawn on the pixel grid by hand: the vector mark scaled that
# small blurs into grey. Five cells (the centre one highest, the tips lowest, a one-
# pixel notch between them) over two wind lines. '#' is white, 'o' is green, '.' is
# transparent (the rounded corners).
FAVICON_16 = [
    "..oooooooooooo..",
    ".oooooooooooooo.",
    "oooooooooooooooo",
    "oooooooooooooooo",
    "oooooo####oooooo",
    "ooo##o####o##ooo",
    "ooo##oooooo##ooo",
    "o##oooooooooo##o",
    "o##oooooooooo##o",
    "oooooooooooooooo",
    "oooo########oooo",
    "oooooooooooooooo",
    "oooooo####oooooo",
    "oooooooooooooooo",
    ".oooooooooooooo.",
    "..oooooooooooo..",
]


def favicon_16_svg():
    rects = []
    for y, row in enumerate(FAVICON_16):
        for x, c in enumerate(row):
            if c != ".":
                rects.append(f'<rect x="{x}" y="{y}" width="1" height="1" fill="{WHITE if c == "#" else GREEN}"/>')
    return svg(16, 16, "\n  ".join(rects), "BayuPandu").replace("<svg ", '<svg shape-rendering="crispEdges" ', 1)


def main():
    global BLACK_FONT, BOLD_FONT
    BLACK_FONT = load_font(900)
    BOLD_FONT = load_font(800)

    files = {
        "bayupandu-lockup.svg": lockup(None),
        "bayupandu-lockup-black.svg": lockup(INK),
        "bayupandu-lockup-white.svg": lockup(WHITE),
        "bayupandu-mark.svg": tile(512),
        "bayupandu-mark-black.svg": mark_alone(INK),
        "bayupandu-mark-white.svg": mark_alone(WHITE),
        "favicon.svg": tile(64, small=True, border=False),
        "favicon-16.svg": favicon_16_svg(),
        "social-preview.svg": social_preview(),
    }
    for name, content in files.items():
        (OUT / name).write_text(content)

    import cairosvg
    renders = [("favicon-16.svg", "favicon-16.png", 16, 16), ("favicon.svg", "favicon-32.png", 32, 32),
               ("bayupandu-mark.svg", "bayupandu-mark-180.png", 180, 180),
               ("bayupandu-mark.svg", "bayupandu-mark-512.png", 512, 512),
               ("social-preview.svg", "social-preview.png", 1280, 640)]
    for src, dst, w, h in renders:
        cairosvg.svg2png(url=str(OUT / src), write_to=str(OUT / dst), output_width=w, output_height=h)
        stamp_origin(OUT / dst, f"Rendered from {src} by assets/brand/gen_brand.py (vector geometry "
                                f"shared with firmware/src/UI/BrandMark.cpp; no image model).")
    print("brand files written to", OUT)


if __name__ == "__main__":
    main()
