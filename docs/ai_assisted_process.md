# AI 辅助开发过程记录

## 问题

首次实现 `DatabaseManager::seed()` 时，建表成功，但事务提交失败，程序无法完成初始化：

```
初始化数据提交失败：cannot commit transaction - SQL statements in progress
```

该问题会导致 `patient` 表为空，登录后表格无数据，属于会直接阻断实验的功能性错误。

## 提示词

> Qt 6 + QSQLITE：在一个事务里执行多条 INSERT 后调用 `db.commit()`，返回
> “cannot commit transaction - SQL statements in progress”。建表和 INSERT 都没有报错，
> 可能是什么原因？QSqlQuery 需要怎样释放？PRAGMA 语句会有影响吗？

## AI 方案

AI 给出的排查方向包括：

1. `QSqlQuery` 对象在 `commit()` 时仍处于活动状态，尤其是 SELECT/PRAGMA 返回了结果集但没有读完或 `finish()`。
2. 检查是否在循环中重复 `addBindValue()` 导致参数累积。
3. 确保提交前所有查询结束，必要时使用作用域或 `finish()`/`clear()`。
4. 连接对象的生命周期要与 `removeDatabase()` 匹配，避免 “connection is still in use”。

## 运行与数据验证

按建议先给 `countQuery` 和 `query` 增加 `finish()`，仍然失败；继续检查发现 `open()` 中的
`PRAGMA journal_mode = WAL` 也返回结果集，且 `pragma` 对象一直存活到 `seed()` 提交事务时。
在 `pragma` 后补上 `pragma.finish()`，并调整 `open()` 失败路径中 `QSqlDatabase` 句柄的释放顺序后，
重新运行：

```powershell
.\build\HospitalQt.exe --selftest --db build\selftest.db
```

结果：8 项测试全部 PASS，`patient` 表记录数 = 10，新增 10 → 11、删除 11 → 10。

## 发现的问题与自己的修改

AI 最初只提示“查询未 finish”，但没有指出 `PRAGMA journal_mode` 同样会返回结果集；这部分是通过
Qt 运行日志和逐步注释定位到的。最终修改包括：

- 在 `countQuery`、`query` 和 `pragma` 使用完毕后显式调用 `finish()`；
- 在 `seed()` 的 `db.commit()` 前确保没有活动语句；
- 在 `open()` 失败时先释放局部 `QSqlDatabase` 句柄，再调用 `removeDatabase()`；
- 将自测写入 `selftest.txt`，每次改动后重新执行，避免“看起来能运行”但数据库未落盘。

该过程说明：AI 可以快速给出排查方向，但必须用实际数据库结果和自测输出验证，不能直接相信生成的代码。
