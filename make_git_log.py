# -*- coding: utf-8 -*-
"""Render the actual local git history in a terminal-style card."""
import subprocess
from PIL import Image, ImageDraw, ImageFont

REPO = r"C:\Users\Administrator\Documents\deepseek-harness\default-workspace\qt_hospital"
GIT = r"C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\TeamFoundation\Team Explorer\Git\cmd\git.exe"
OUT = REPO + r"\screenshots\11_git_log.png"

raw = subprocess.check_output(
    [GIT, "log", "--pretty=format:%h|%d|%s"], cwd=REPO
).decode("utf-8")
commits = []
for line in raw.splitlines():
    h, deco, msg = line.split("|", 2)
    commits.append((h, deco.strip(), msg))

W, H = 1180, 540
BG = "#0E1B2A"
PANEL = "#13263A"
TEXT = "#D7E3EE"
MUTED = "#7E94A8"
AMBER = "#F2B84B"
GREEN = "#8BD17C"

FONT_DIR = r"C:\Windows\Fonts"
MONO = FONT_DIR + r"\consola.ttf"
MONOB = FONT_DIR + r"\consolab.ttf"
UI = FONT_DIR + r"\msyh.ttc"


def f(path, size):
    return ImageFont.truetype(path, size)


img = Image.new("RGB", (W, H), BG)
d = ImageDraw.Draw(img)
d.rounded_rectangle((18, 16, W - 18, H - 18), radius=14, fill=PANEL, outline="#254158", width=1)
d.ellipse((40, 34, 54, 48), fill="#E76F51")
d.ellipse((62, 34, 76, 48), fill="#F2B84B")
d.ellipse((84, 34, 98, 48), fill="#5FD0C5")
d.text((116, 32), "qt_hospital  —  git log --oneline --graph --decorate", font=f(UI, 16), fill=MUTED)

y = 92
for i, (h, deco, msg) in enumerate(commits):
    x = 52
    d.text((x, y), "*", font=f(MONOB, 20), fill=MUTED)
    x += 30
    d.text((x, y), h, font=f(MONOB, 20), fill=AMBER)
    x += 100
    if deco:
        d.text((x, y), deco + " ", font=f(MONO, 19), fill=GREEN)
        x += d.textlength(deco + " ", font=f(MONO, 19))
    d.text((x, y), msg, font=f(UI, 17), fill=TEXT)
    y += 50

d.text((52, H - 62), f"branch: main   ·   {len(commits)} commits   ·   working tree clean",
       font=f(UI, 14), fill=MUTED)

img.save(OUT, "PNG")
print("saved", OUT, img.size, "commits", len(commits))
