# 医院诊疗信息管理系统（Qt SQL / Model-View 实验）

## 1. 项目概述

本项目是《Qt 应用程序开发》实验三的完整实现：使用 Qt 6 Widgets、Qt SQL 与 SQLite 完成一个医院诊疗系统的患者信息管理应用。界面采用登录页 + 工作台的双层 `QStackedWidget` 结构，工作台内使用 `QTableView` + `QSqlTableModel` + `QSortFilterProxyModel` 实现患者档案的显示、查询、新增、修改和删除。

## 2. 功能清单

- 登录校验：`user` 表中保存 SHA-256 密码摘要，错误凭据给出红色提示并记录日志。
- 患者管理：列表显示、按姓名/身份证号/手机号/全部字段查询、新增、编辑、删除。
- 数据校验：姓名、18 位身份证号、手机号、出生日期、身高体重范围校验；身份证号唯一性检查。
- 删除确认：自定义 `ConfirmDialog`，确认后才执行 `DELETE`。
- 数据刷新：增删改后重新 `select()`，`QSortFilterProxyModel` 自动刷新视图。
- 自定义代理：`SexDelegate` 使用 `QComboBox` 编辑性别列。
- 医生/科室：只读查询页面，展示 `doctor`、`department` 表及科室医生人数。
- 操作日志：登录成功/失败、增删改等写入 `history` 表，可在“操作日志”页查询。
- 异常处理：数据库打开失败弹出自定义错误对话框；查询无结果时状态栏提示。

## 3. 目录结构

```
qt_hospital/
├── CMakeLists.txt
├── src/
│   ├── main.cpp
│   ├── config.h              # 开发者学号/姓名（请替换）
│   ├── theme.h               # 全局 QSS
│   ├── patient.h
│   ├── database.h/.cpp       # SQLite 连接、建表、种子数据、登录、日志
│   ├── patientmodel.h/.cpp   # QSqlTableModel 子类 + 数据校验/CRUD
│   ├── dialogs.h/.cpp        # 新增/编辑、删除确认、数据库错误对话框
│   ├── loginpage.h/.cpp      # 登录页
│   ├── workspace.h/.cpp      # 工作台、患者页、医生/科室/日志页
│   ├── mainwindow.h/.cpp     # 根 QStackedWidget 与登录切换
│   ├── capture.h/.cpp        # 自动截图模式
│   └── selftest.h/.cpp       # 数据库/模型自测
├── screenshots/              # 报告用截图
├── docs/
├── make_schema.py            # 生成数据库结构图
└── build/                    # CMake 构建目录（不入库）
```

## 4. 构建与运行

环境：Windows 10/11 64 位，Qt 6.12.0 MinGW 64-bit，CMake 3.21+，Ninja，MinGW 13.1。

```powershell
$env:PATH="C:\Qt\6.12.0\mingw_64\bin;C:\Qt\Tools\mingw1310_64\bin;C:\Qt\Tools\Ninja;C:\Qt\Tools\CMake_64\bin;$env:PATH"
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release `
  -DCMAKE_PREFIX_PATH=C:/Qt/6.12.0/mingw_64 `
  -DCMAKE_CXX_COMPILER=C:/Qt/Tools/mingw1310_64/bin/g++.exe
cmake --build build
.\build\HospitalQt.exe
```

默认数据库为 `build/hospital.db`，首次运行自动建表并写入 10 条患者、5 名医生、5 个科室和 2 个演示账号。

演示账号：

- `admin / 123456`（系统管理员）
- `doctor / 123456`（医生）

## 5. 测试与截图

```powershell
# 数据库/模型自测：8 项，输出 build/selftest.txt
.\build\HospitalQt.exe --selftest --db build\selftest.db

# 自动生成报告截图到 screenshots/
.\build\HospitalQt.exe --capture screenshots --db build\capture.db
```

## 6. 数据库表

`patient`、`doctor`、`department`、`user`、`history` 五张表，外键：

- `doctor.department_id → department.id`
- `history.user_id → user.id`

字段与约束详见 `docs/schema.sql`。

## 7. 说明

- `src/config.h` 中的 `kStudentId`、`kStudentName` 为占位值，请替换为本人学号与姓名后重新编译，登录页和工作台左下角会同步显示。
- 本实验使用 AI 工具辅助排查 Qt SQL 事务提交问题，AI 生成的建议均通过 `--selftest` 和实际界面操作复核，详见 `docs/ai_assisted_process.md`。
