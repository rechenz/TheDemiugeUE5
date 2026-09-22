# AGENTS.md — TheDemiugeUE5 会话操作协议

> 这个文件是给**任何 AI 编码会话**(DSH / Claude Code / Codex / Cursor 等)看的。
> 每次新会话请先读完本文件的「开工前」一节再动手。

---

## 开工前必做

**按顺序读这三份,再开始任何工作:**

| 顺序 | 文件 | 看什么 |
|---|---|---|
| 1 | `Docs/PROGRESS.md` | **做到哪了、下一步做什么、有什么坑** ← 最重要 |
| 2 | `Docs/STATUS.md` | 机器采集的事实(git、文件、架构进度) |
| 3 | `Docs/ARCHITECTURE.md` | 架构设计与完整系统清单 |

**并先跑一次进度采集,看自上会话以来有什么变化:**

```powershell
pwsh -File Tools\ProjectStatus.ps1
```

---

## 收工前必做

1. **更新 `Docs/PROGRESS.md`** 的:刚做完 / 正在做 / 下一步 / 待修待决
2. **跑一次 `Tools/ProjectStatus.ps1`** 刷新事实快照
3. 产生了**新的设计决策** → 追加到 `Docs/ARCHITECTURE.md` 的设计决策章节
4. 有**未验证的假设或踩过的坑** → 写进 `PROGRESS.md` 的「待决」或「待修」

> 没做这几步的会话 = 下个会话要从零猜。**交接的成本远低于重来。**

---

## 项目结构

```
TheDemiugeUE5/                      UE5.8 项目根(本仓库)
├── AGENTS.md                       ← 本文件
├── Docs/
│   ├── ARCHITECTURE.md             架构设计 + 系统清单(人的判断)
│   ├── PROGRESS.md                 进度台账(人的判断)★ 手写
│   └── STATUS.md                   事实快照(脚本生成)★ 勿手改
├── Source/TheDemiugeUE5/
│   ├── Public/Subsystem/<模块>/    公共头文件
│   └── Private/Subsystem/<模块>/   实现
├── Content/                        关卡、蓝图、资源
├── Tools/
│   └── ProjectStatus.ps1           进度采集脚本
└── Config/
```

**姊妹项目**:`TheDemiuge-Bridge`(Go 后端,AI 通信桥)在 WSL `~/projects/TheDemiugeAgent/TheDemiuge-Bridge`。
它的架构文档在 `F:\project\TheDemiugeAgent\ARCHITECTURE.md` —— **和本项目是两回事,别搞混。**

---

## 环境事实

| 项 | 值 |
|---|---|
| 引擎 | `E:\Epicgames\games\UE_5.8`(源码构建版) |
| 引擎源码 | ✅ **完整**(`Engine\Source\Runtime\...`,可直接查,别再凭记忆猜) |
| 版本控制 | git,分支 `main` |

⚠️ `.uproject` 的 `EngineAssociation` 是 **GUID**,真实路径查注册表 `HKCU\SOFTWARE\Epic Games\Unreal Engine\Builds`。
⚠️ `E:\Epicgames\UE_5.7` 是个**残缺目录**(仅 1.1GB,没有 `Binaries\Win64`、没有 `Engine\Build`),**不是真引擎,别拿它编译**。
⚠️ **Live Coding**:编辑器开着时外部 `Build.bat` 会报 `Unable to build while Live Coding is active`。要么关编辑器,要么在编辑器里按 `Ctrl+Alt+F11`。

---

## 硬性架构约定

**改代码前必须遵守,违反会导致架构塌方:**

1. **依赖严格单向向下(L0 → L10),零例外**
   运行时的反向调用(回调、注册)不算违规,**前提是编译期只依赖下层接口**。

2. **一切状态变更走 Effect**
   事件总线是**通知**机制,订阅者只能读状态,不得直接改。

3. **时间是「可推进的累计量」,不是「读系统时钟」**
   游戏逻辑**禁止**使用 `FDateTime::Now()`。

4. **时效性数据存「绝对时间戳」,不存「剩余时长」**
   否则"睡觉跳过 8 小时"不会让冷却结束。

5. **定义与实例分离**
   任务、物品等:模板(只读数据)与运行时实例分开。

6. **Agent 只能提议,不能改状态**
   四条约束:只读只提议 / 知识视图 / 版本戳 / 拒绝反馈回路。

完整论证见 `Docs/ARCHITECTURE.md`。

---

## 代码风格

- 文件头:`// Copyright 2026 rechenz`
- 目录:`Public/Subsystem/<模块>/`、`Private/Subsystem/<模块>/`
- 蓝图可调用的加 `UFUNCTION(BlueprintCallable)` / `UPROPERTY(BlueprintReadWrite)`
- 枚举带 `E` 前缀(`ETimeCategory`,不是 `TimeCategory`)
- UCLASS 名与文件名一致

---

## 沟通偏好(重要)

- **要简短、直接、给清单**。不要长篇展开,不要客套
- **不要 AI 腔**("作为 AI 我……"这类一律不要)
- 会用吐槽和游戏术语交流,随意一点
- 改代码前先说清要改什么,别闷头改一堆

---

_本协议由项目主人在 2026-09-22 建立。改动请直接编辑本文件。_
