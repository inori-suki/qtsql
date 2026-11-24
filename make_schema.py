# -*- coding: utf-8 -*-
"""Render a clean SQLite schema/ER poster for the Qt hospital report."""
from PIL import Image, ImageDraw, ImageFont

W, H = 1500, 950
BG = "#F4F7FB"
WHITE = "#FFFFFF"
INK = "#17324D"
MUTED = "#6C7E8F"
BORDER = "#DCE6F0"
TEAL = "#1F6F8B"
TEAL_LIGHT = "#E7F2F6"
GREEN = "#2A9D8F"
AMBER = "#B7791F"

FONT_DIR = r"C:\Windows\Fonts"
REG = FONT_DIR + r"\msyh.ttc"
BOLD = FONT_DIR + r"\msyhbd.ttc"
MONO = FONT_DIR + r"\consola.ttf"


def f(path, size):
    return ImageFont.truetype(path, size)


def text(draw, xy, s, font, fill=INK, anchor=None):
    draw.text(xy, s, font=font, fill=fill, anchor=anchor)


def rounded(draw, box, r, fill, outline=None, width=1):
    draw.rounded_rectangle(box, radius=r, fill=fill, outline=outline, width=width)


def table_card(draw, x, y, w, name, rows, accent=TEAL):
    header_h = 54
    row_h = 30
    footer_pad = 18
    h = header_h + len(rows) * row_h + footer_pad
    # shadow
    rounded(draw, (x + 3, y + 4, x + w + 3, y + h + 4), 12, "#E7EDF4")
    rounded(draw, (x, y, x + w, y + h), 12, WHITE, BORDER, 1)
    # header
    draw.rounded_rectangle((x, y, x + w, y + header_h), radius=12, fill=accent)
    draw.rectangle((x, y + header_h - 12, x + w, y + header_h), fill=accent)
    text(draw, (x + 18, y + 13), name, f(BOLD, 22), WHITE)
    text(draw, (x + w - 18, y + 16), f"{len(rows)} fields", f(REG, 14), "#D9EEF4", anchor="ra")

    for i, (field, typ, key) in enumerate(rows):
        yy = y + header_h + i * row_h + 7
        if i % 2 == 0:
            draw.rectangle((x + 1, yy - 5, x + w - 1, yy + row_h - 5), fill="#FAFCFE")
        # key badge
        if key:
            badge_w = 32 if len(key) <= 2 else 40
            color = "#FDF1E7" if key == "FK" else ("#E8F5F2" if key == "UK" else "#E7F2F6")
            fg = AMBER if key == "FK" else (GREEN if key == "UK" else TEAL)
            rounded(draw, (x + 14, yy - 2, x + 14 + badge_w, yy + 18), 6, color)
            text(draw, (x + 14 + badge_w / 2, yy + 8), key, f(BOLD, 10), fg, anchor="mm")
            tx = x + 14 + badge_w + 10
        else:
            tx = x + 24
        text(draw, (tx, yy), field, f(MONO, 17), INK)
        text(draw, (x + w - 18, yy + 1), typ, f(REG, 14), MUTED, anchor="ra")
    return h


img = Image.new("RGB", (W, H), BG)
d = ImageDraw.Draw(img)

# top bar
d.rectangle((0, 0, W, 82), fill=WHITE)
d.line((0, 82, W, 82), fill=BORDER, width=1)
text(d, (30, 18), "hospital.db", f(BOLD, 30), INK)
text(d, (32, 54), "SQLite 数据库表结构设计 · 5 张表 · 外键约束已启用", f(REG, 14), MUTED)
text(d, (W - 30, 28), "Qt 6.12  ·  QSQLITE", f(REG, 14), TEAL, anchor="ra")
text(d, (W - 30, 52), "patient / doctor / department / user / history", f(REG, 12), MUTED, anchor="ra")

# sidebar
d.rectangle((0, 82, 252, H), fill=WHITE)
d.line((252, 82, 252, H), fill=BORDER, width=1)
text(d, (24, 108), "数据库对象", f(BOLD, 13), MUTED)
items = [("patient", "10"), ("doctor", "5"), ("department", "3"), ("user", "2"), ("history", "6")]
for idx, (name, count) in enumerate(items):
    yy = 140 + idx * 46
    if idx == 0:
        rounded(d, (14, yy - 8, 238, yy + 30), 8, TEAL_LIGHT)
        text(d, (30, yy), name, f(BOLD, 16), TEAL)
        text(d, (226, yy), count, f(BOLD, 13), TEAL, anchor="ra")
    else:
        text(d, (30, yy), name, f(REG, 15), INK)
        text(d, (226, yy), count, f(REG, 13), MUTED, anchor="ra")
text(d, (24, 405), "关系", f(BOLD, 13), MUTED)
text(d, (24, 434), "history.user_id → user.id", f(REG, 12), MUTED)
text(d, (24, 456), "doctor.department_id →", f(REG, 12), MUTED)
text(d, (24, 476), "department.id", f(REG, 12), MUTED)
text(d, (24, H - 42), "PK 主键   UK 唯一键   FK 外键", f(REG, 11), MUTED)

# table cards
patient_rows = [
    ("id", "INTEGER", "PK"),
    ("name", "TEXT", ""),
    ("id_card", "TEXT", "UK"),
    ("sex", "TEXT", ""),
    ("age", "INTEGER", ""),
    ("dob", "TEXT", ""),
    ("height", "REAL", ""),
    ("weight", "REAL", ""),
    ("mobile", "TEXT", ""),
    ("created_at", "TEXT", ""),
]
h_patient = table_card(d, 292, 112, 530, "patient", patient_rows, TEAL)

user_rows = [
    ("id", "INTEGER", "PK"),
    ("fullname", "TEXT", ""),
    ("username", "TEXT", "UK"),
    ("password_hash", "TEXT", ""),
    ("role", "TEXT", ""),
]
h_user = table_card(d, 872, 112, 398, "user", user_rows, "#3C7A89")

history_rows = [
    ("id", "INTEGER", "PK"),
    ("user_id", "INTEGER", "FK"),
    ("username", "TEXT", ""),
    ("event", "TEXT", ""),
    ("created_at", "TEXT", ""),
]
h_history = table_card(d, 872, 382, 398, "history", history_rows, "#6B7FA3")

doctor_rows = [
    ("id", "INTEGER", "PK"),
    ("employee_no", "TEXT", "UK"),
    ("name", "TEXT", ""),
    ("department_id", "INTEGER", "FK"),
    ("title", "TEXT", ""),
]
h_doctor = table_card(d, 292, 618, 530, "doctor", doctor_rows, "#2A9D8F")

department_rows = [
    ("id", "INTEGER", "PK"),
    ("name", "TEXT", "UK"),
]
table_card(d, 872, 662, 398, "department", department_rows, "#7B8FA6")

# relationships
# history -> user (vertical)
x = 872 + 74
d.line((x, 382, x, 334), fill=TEAL, width=2)
d.ellipse((x - 4, 330, x + 4, 338), fill=WHITE, outline=TEAL, width=2)
d.line((x - 9, 382, x + 9, 382), fill=TEAL, width=2)
d.line((x - 9, 382, x - 9, 372), fill=TEAL, width=2)
d.line((x + 9, 382, x + 9, 372), fill=TEAL, width=2)
text(d, (x + 14, 347), "1 : N", f(REG, 11), TEAL)

# doctor -> department (horizontal)
y = 618 + 54 + 3 * 30 + 15
d.line((822, y, 848, y), fill=GREEN, width=2)
d.line((848, y, 872, y), fill=GREEN, width=2)
d.ellipse((842, y - 4, 850, y + 4), fill=WHITE, outline=GREEN, width=2)
d.line((872, y - 8, 872, y + 8), fill=GREEN, width=2)
text(d, (836, y - 24), "1 : N", f(REG, 11), GREEN)

# footer
text(d, (292, 888), "字段类型：TEXT / INTEGER / REAL；patient.id_card 唯一，user.username 唯一，"
                    "doctor.employee_no 唯一；删除患者仅删除 patient 记录。", f(REG, 13), MUTED)

img.save(r"C:\Users\Administrator\Documents\deepseek-harness\default-workspace\qt_hospital\screenshots\00_schema.png", "PNG")
print("saved", img.size)
