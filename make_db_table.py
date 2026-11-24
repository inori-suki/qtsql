# -*- coding: utf-8 -*-
"""Render the actual patient rows from capture.db as a small SQL result view."""
import sqlite3
from PIL import Image, ImageDraw, ImageFont

DB = r"C:\Users\Administrator\Documents\deepseek-harness\default-workspace\qt_hospital\build\capture.db"
OUT = r"C:\Users\Administrator\Documents\deepseek-harness\default-workspace\qt_hospital\screenshots\12_db_table.png"

con = sqlite3.connect(DB)
rows = con.execute(
    "SELECT id, name, id_card, sex, age, mobile FROM patient ORDER BY id LIMIT 10"
).fetchall()
con.close()

W, H = 1240, 600
BG = "#F4F7FB"
WHITE = "#FFFFFF"
INK = "#17324D"
MUTED = "#6C7E8F"
BORDER = "#DCE6F0"
TEAL = "#1F6F8B"
TEAL_LIGHT = "#E7F2F6"

FONT_DIR = r"C:\Windows\Fonts"
REG = FONT_DIR + r"\msyh.ttc"
BOLD = FONT_DIR + r"\msyhbd.ttc"
MONO = FONT_DIR + r"\consola.ttf"


def f(path, size):
    return ImageFont.truetype(path, size)


img = Image.new("RGB", (W, H), BG)
d = ImageDraw.Draw(img)

# window / header
d.rounded_rectangle((20, 18, W - 20, H - 18), radius=12, fill=WHITE, outline=BORDER, width=1)
d.rectangle((20, 18, W - 20, 82), fill="#FFFFFF")
d.line((20, 82, W - 20, 82), fill=BORDER, width=1)
d.text((44, 32), "hospital.db", font=f(BOLD, 26), fill=INK)
d.text((44, 62), "SQLite 数据浏览 · patient 表", font=f(REG, 12), fill=MUTED)
d.text((W - 44, 36), "10 rows", font=f(BOLD, 15), fill=TEAL, anchor="ra")

# query bar
d.rounded_rectangle((40, 96, W - 40, 138), radius=7, fill="#F8FAFC", outline=BORDER, width=1)
d.text((54, 108), "SELECT id, name, id_card, sex, age, mobile FROM patient ORDER BY id;",
       font=f(MONO, 15), fill="#35516A")

# table header
cols = [("id", 70), ("name", 190), ("id_card", 380), ("sex", 90), ("age", 80), ("mobile", 280)]
x0 = 40
header_y = 154
d.rectangle((x0, header_y, W - 40, header_y + 38), fill=TEAL_LIGHT)
x = x0
for title, width in cols:
    d.text((x + 12, header_y + 10), title, font=f(BOLD, 14), fill=TEAL)
    x += width
# right edge fill
d.line((x0, header_y, W - 40, header_y), fill=BORDER, width=1)

row_h = 32
for i, row in enumerate(rows):
    y = header_y + 38 + i * row_h
    if i % 2 == 0:
        d.rectangle((x0, y, W - 40, y + row_h), fill="#FAFCFE")
    x = x0
    for (title, width), value in zip(cols, row):
        text = str(value)
        d.text((x + 12, y + 6), text, font=f(REG, 14), fill=INK)
        x += width
    d.line((x0, y + row_h, W - 40, y + row_h), fill="#EEF2F6", width=1)

# vertical column separators
x = x0
for _, width in cols[:-1]:
    x += width
    d.line((x, header_y, x, header_y + 38 + len(rows) * row_h), fill="#E7EDF4", width=1)

d.text((40, H - 54), "来源：build/capture.db，与界面表格使用同一份 SQLite 数据。",
       font=f(REG, 12), fill=MUTED)

img.save(OUT, "PNG")
print("saved", OUT, img.size, "rows", len(rows))
