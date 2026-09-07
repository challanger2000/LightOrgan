from pathlib import Path
from PIL import Image, ImageEnhance, ImageFilter
import math

RESOURCE = Path(__file__).resolve().parents[1] / "resource"
MAIN = "LightOrgan_MainGUI"
LAMPS = [
    "lamp_sub_6f",
    "lamp_bass_6f",
    "lamp_low_mid_6f",
    "lamp_mid_6f",
    "lamp_high_mid_6f",
    "lamp_high_6f",
    "lamp_strobe_6f",
]

# Main panel: lossless PNG conversion. The current JPG remains only the temporary
# source until the original full-resolution GUI is installed.
with Image.open(RESOURCE / f"{MAIN}.jpg") as image:
    image.convert("RGB").save(RESOURCE / f"{MAIN}.png", format="PNG", optimize=True)
print(f"Converted {MAIN}.jpg -> {MAIN}.png")


def radial_mask(size, inner=0.48, outer=0.88):
    """Soft circular mask that affects the glass, not the surrounding panel/bezel."""
    w, h = size
    cx, cy = (w - 1) * 0.5, (h - 1) * 0.5
    radius = min(w, h) * 0.5
    mask = Image.new("L", size, 0)
    px = mask.load()
    for y in range(h):
        for x in range(w):
            d = math.hypot(x - cx, y - cy) / radius
            if d <= inner:
                a = 255
            elif d >= outer:
                a = 0
            else:
                t = (d - inner) / (outer - inner)
                a = int(255.0 * (1.0 - t * t * (3.0 - 2.0 * t)))
            px[x, y] = a
    return mask.filter(ImageFilter.GaussianBlur(1.2))


def make_off(frame, mask):
    # OFF should still look like coloured Fresnel glass, not like a black hole.
    # Keep most of the original lens colour/texture and only remove the impression
    # that the bulb is already lit behind it.
    dim = ImageEnhance.Brightness(frame).enhance(0.68)
    dim = ImageEnhance.Contrast(dim).enhance(1.04)
    dim = ImageEnhance.Color(dim).enhance(0.95)
    return Image.composite(dim, frame, mask)


def make_on(frame, mask, strobe=False):
    # Strong internal illumination. Keep the bezel untouched and push only the
    # glass towards a hot centre so 100% meter level visibly means FULL ON.
    lit = ImageEnhance.Brightness(frame).enhance(2.00 if not strobe else 2.35)
    lit = ImageEnhance.Contrast(lit).enhance(1.08)
    lit = ImageEnhance.Color(lit).enhance(1.30 if not strobe else 0.65)

    hot = Image.new("RGB", frame.size, (255, 250, 232) if not strobe else (255, 255, 255))
    core = radial_mask(frame.size, inner=0.12, outer=0.58)
    core = core.point(lambda p: int(p * (0.68 if not strobe else 0.90)))
    lit = Image.composite(hot, lit, core)
    return Image.composite(lit, frame, mask)


for name in LAMPS:
    src = RESOURCE / f"{name}.jpg"
    dst = RESOURCE / f"{name}.png"
    with Image.open(src) as image:
        sprite = image.convert("RGB")
        w, h = sprite.size
        if h % 6 != 0:
            raise RuntimeError(f"{src.name}: expected six vertical frames, got {w}x{h}")
        fh = h // 6
        if w != fh:
            raise RuntimeError(f"{src.name}: expected square frames, got {w}x{fh}")

        frames = [sprite.crop((0, i * fh, w, (i + 1) * fh)) for i in range(6)]
        mask = radial_mask((w, fh))
        off = make_off(frames[0], mask)
        on = make_on(frames[-1], mask, strobe=(name == "lamp_strobe_6f"))

        # LampView currently blends frame 0 and frame 5 continuously. Fill the
        # intermediate frames too, so the sprite remains useful if stepped
        # animation is reintroduced later.
        out = Image.new("RGB", (w, h))
        for i in range(6):
            t = i / 5.0
            frame = Image.blend(off, on, t)
            out.paste(frame, (0, i * fh))
        out.save(dst, format="PNG", optimize=True)
    print(f"Built refined lamp sprite {src.name} -> {dst.name}")
