from pathlib import Path
from PIL import Image

RESOURCE = Path(__file__).resolve().parents[1] / "resource"
NAMES = [
    "LightOrgan_MainGUI",
    "lamp_sub_6f",
    "lamp_bass_6f",
    "lamp_low_mid_6f",
    "lamp_mid_6f",
    "lamp_high_mid_6f",
    "lamp_high_6f",
    "lamp_strobe_6f",
]

for name in NAMES:
    src = RESOURCE / f"{name}.jpg"
    dst = RESOURCE / f"{name}.png"
    with Image.open(src) as image:
        image.convert("RGB").save(dst, format="PNG", optimize=True)
    print(f"Converted {src.name} -> {dst.name}")
