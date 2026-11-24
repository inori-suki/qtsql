-- 医院诊疗信息管理系统 SQLite 数据库结构
-- 与 src/database.cpp 中的 initSchema() 保持一致

PRAGMA foreign_keys = ON;

CREATE TABLE IF NOT EXISTS department (
    id   INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL UNIQUE
);

CREATE TABLE IF NOT EXISTS doctor (
    id            INTEGER PRIMARY KEY AUTOINCREMENT,
    employee_no   TEXT NOT NULL UNIQUE,
    name          TEXT NOT NULL,
    department_id INTEGER NOT NULL,
    title         TEXT,
    FOREIGN KEY (department_id) REFERENCES department(id)
);

CREATE TABLE IF NOT EXISTS patient (
    id         INTEGER PRIMARY KEY AUTOINCREMENT,
    name       TEXT NOT NULL,
    id_card    TEXT NOT NULL UNIQUE,
    sex        TEXT NOT NULL CHECK (sex IN ('男', '女')),
    age        INTEGER,
    dob        TEXT NOT NULL,
    height     REAL,
    weight     REAL,
    mobile     TEXT NOT NULL,
    created_at TEXT NOT NULL DEFAULT (datetime('now', 'localtime'))
);

CREATE TABLE IF NOT EXISTS user (
    id            INTEGER PRIMARY KEY AUTOINCREMENT,
    fullname      TEXT NOT NULL,
    username      TEXT NOT NULL UNIQUE,
    password_hash TEXT NOT NULL,
    role          TEXT NOT NULL DEFAULT 'doctor'
);

CREATE TABLE IF NOT EXISTS history (
    id         INTEGER PRIMARY KEY AUTOINCREMENT,
    user_id    INTEGER,
    username   TEXT NOT NULL,
    event      TEXT NOT NULL,
    created_at TEXT NOT NULL DEFAULT (datetime('now', 'localtime')),
    FOREIGN KEY (user_id) REFERENCES user(id)
);

CREATE INDEX IF NOT EXISTS idx_patient_name   ON patient(name);
CREATE INDEX IF NOT EXISTS idx_patient_mobile ON patient(mobile);
CREATE INDEX IF NOT EXISTS idx_history_time   ON history(created_at);
