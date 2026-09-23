# 项目状态快照

> 生成时间:2026-09-23 08:29:43
> **本文件由 `Tools/ProjectStatus.ps1` 自动生成,不要手改。**
> 人的判断与计划写在 `Docs/PROGRESS.md`。

---

## 一、Git 状态

| 项 | 值 |
|---|---|
| 分支 | main |
| HEAD | 2666c7c |
| 未提交改动 | 1 项 |

### 最近提交

```
2666c7c wip:实现L0时间系统
5aa1593 更新gitignore
844feda init: UE5 project skeleton (Config/Source/uproject)
f6299df Initial commit
```

### 工作区改动

```
 M Source/TheDemiugeUE5/Public/Subsystem/clock/GameClockSubsystem.h
```

## 二、自上次跟进以来的变化

| 类型 | 数量 |
|---|---|
| 新增 | 0 |
| 修改 | 6 |
| 删除 | 0 |

_baseline 时间:2026-09-22 21:23:19_

### 修改文件

- `AGENTS.md`
- `Docs\.status-baseline.json`
- `Docs\PROGRESS.md`
- `Docs\STATUS.md`
- `Source\TheDemiugeUE5\Private\Subsystem\clock\GameClockSubsystem.cpp`
- `Source\TheDemiugeUE5\Public\Subsystem\clock\GameClockSubsystem.h`

## 三、架构清单进度

**总计:0 / 63(0%)**

| 层 | 完成 | 总数 |
|---|---|---|
| 1.1 关于"零例外" | 0 | 0 |
| 2.1 数据引用 ≠ 代码依赖 | 0 | 0 |
| 2.2 事件总线是通知,不是变更通道 | 0 | 0 |
| 2.3 Effect 的边界定义 | 0 | 0 |
| L0 基础层 | 0 | 8 |
| L1 实体层 | 0 | 4 |
| L2 数值层 | 0 | 5 |
| L3 条件层 | 0 | 2 |
| L4 物品层 | 0 | 6 |
| L5 行为层 | 0 | 7 |
| L6 世界层 | 0 | 6 |
| L7 叙事层 | 0 | 5 |
| L8 Agent 层 | 0 | 9 |
| L9 表现层 | 0 | 4 |
| L10 横切层 | 0 | 7 |
| 选定方案:Effect 日志 + 周期快照 | 0 | 0 |
| 约束 1:只读 + 只提议 | 0 | 0 |
| 约束 2:知识视图(信息隔离) | 0 | 0 |
| 约束 3:乐观并发(版本戳) | 0 | 0 |
| 约束 4:仲裁失败的反馈回路 | 0 | 0 |
| 附带约束:记忆不是第二个真相源 | 0 | 0 |
| 1. 依赖是「拉取式条件」,不是「推送式硬连线」 | 0 | 0 |
| 2. 条件引擎依赖倒置 | 0 | 0 |
| 3. 定义与实例分离 | 0 | 0 |
| 4. Effect 是唯一的状态变更通道 | 0 | 0 |
| 5. 数值依赖用惰性求值 | 0 | 0 |
| 阶段 5 为什么必须提前 | 0 | 0 |

## 四、源码清单

| 文件 | 行数 | 最后修改 |
|---|---|---|
| `Source\TheDemiugeUE5.Target.cs` | 15 | 08-05 20:19 |
| `Source\TheDemiugeUE5\Private\Subsystem\clock\GameClockSubsystem.cpp` | 67 | 09-22 22:07 |
| `Source\TheDemiugeUE5\Public\Subsystem\clock\GameClockSubsystem.h` | 131 | 09-23 08:29 |
| `Source\TheDemiugeUE5\TheDemiugeUE5.Build.cs` | 23 | 08-05 20:19 |
| `Source\TheDemiugeUE5\TheDemiugeUE5.cpp` | 6 | 08-05 20:19 |
| `Source\TheDemiugeUE5\TheDemiugeUE5.h` | 6 | 08-05 20:19 |
| `Source\TheDemiugeUE5Editor.Target.cs` | 15 | 08-05 20:19 |

合计 **7** 个源文件,**263** 行。

---

_由 `Tools/ProjectStatus.ps1` 生成 · 事实快照,不含判断_

