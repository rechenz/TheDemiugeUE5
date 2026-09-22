# 进度台账

> **这是给「下一个会话」看的交接文档。**
> 机器采集的事实看 `Docs/STATUS.md`(由脚本生成),这里只写**判断和计划**。
> 最后更新:2026-09-22 19:22

---

## 一、项目一句话

**TheDemiuge** — Agent 驱动的 RPG。UE5.8 + C++ 游戏本体,配一个 Go 写的 AI 通信桥。

两个独立项目:

| 项目 | 技术栈 | 位置 | 状态 |
|---|---|---|---|
| **TheDemiugeUE5** | UE5.8 / C++ | `F:\project\TheDemiugeAgent\TheDemiugeUE5` | **← 当前主战场** |
| **TheDemiuge-Bridge** | Go / MCP + ReAct | WSL `~/projects/TheDemiugeAgent/TheDemiuge-Bridge` | P0 已完成 |

UE 引擎装在 `E:\Epicgames\games\UE_5.8`(源码构建版,**有完整 Runtime 源码**)。

> `.uproject` 的 `EngineAssociation` 是 GUID,真实路径在注册表 `HKCU\SOFTWARE\Epic Games\Unreal Engine\Builds`。
> `E:\Epicgames\UE_5.7` 是**残缺目录**(仅 1.1GB,无 `Binaries\Win64`、无 `Engine\Build`),不是真引擎,别拿它编译。

---

## 二、当前阶段

**架构阶段 0:L0 基础层**

依据 `Docs/ARCHITECTURE.md` 的八阶段实施顺序:

```
[0] L0 三件套(时钟/句柄/事件总线)+ 输入采集 + 配置层   ← 在这里
[1] L1 实体 + L2 属性/修饰器
[2] L5 Effect + 存档策略定型
[3] L9 UI 绑定
[4] L4 物品 + L3 条件引擎
[5] L8 最小 Agent 竖切
[6] L7 任务 + L6 世界标记
[7] L10 存档落地
[8] 横向铺开
```

---

## 三、刚做完

- ✅ 架构文档 rev.2(`Docs/ARCHITECTURE.md`)—— 依据外部审查修订过一轮
- ✅ `UGameClockSubsystem` 骨架(`Source/TheDemiugeUE5/Public|Private/Subsystem/clock/`)
- ✅ 建立进度跟进机制(本文件 + `Tools/ProjectStatus.ps1` + `AGENTS.md`)

## 四、正在做

- 🔨 **GameClockSubsystem 实现** — 骨架已写,存在若干问题(见下「待修」)

## 五、下一步

1. **修时钟的问题**(见第六节,`Tick` 里 TimeScale 没生效是致命的)
2. 补 `AdvanceGameTime` / `SetGameTime` / 存档序列化
3. 补 GameDelta / RealDelta 暴露给表现层
4. 实现 L0 剩下的:**事件总线** → **配置数据层** → **句柄/生命周期**
5. 起一个测试 Level,验证时钟 Tick 真的在跑(`Content/Levels/testLevel` 已存在)

---

## 六、待修 / 待决(重要)

### 6.1 GameClockSubsystem 的问题

| 严重度 | 问题 | 说明 |
|---|---|---|
**✅ 2026-09-22 21:30 重构完成**(修掉 4 个 🔴 + 实现时间刻系统):

| 改动 | 说明 |
|---|---|
| 修 `FFormatTime` 定义位置 | 移到 UCLASS **之前** → 编译错误消除 |
| 修 `AdvanceGameTime` | 默认参数移回 `.h` 声明;`Secs` 真正用于推进时间 |
| 修 `FFormatTime` 计算 | 先 `secs % SecondsPerDay` 再算时分秒,Days/Hours 不再错位 |
| 新增 `SetGameTime` | 读档用。**不回放中途时间刻**,只对齐计数器 |
| 新增 `GetSaveState` / `ApplySaveState` | 存档接口 |
| **时间刻系统** | 见下 |
| `DayTime` 默认值 | 从 `5184000`(60 天)改为 **`86400`(24 小时)** |

**时间刻系统(热尘的设计)**:一天分成 `ClockTimes` 刻,每刻一个事件槽。

```cpp
// C++ 注册
FClockTickHandle H = Clock->RegisterTickEvent(6, [](int32 Slot){ /* 早上 6 点刷怪 */ });
Clock->UnregisterTickEvent(H);

// 蓝图:订阅 OnClockTick,按 Slot 分支
```

实现要点:
- 用「绝对刻号 × 刻长」和 GameTime 比较,**不累加浮点**,长时间运行不漏刻
- `AdvanceGameTime` 逐刻推进,跳过的每一刻都按时间顺序触发
- `SetGameTime` 反过来:**不回放**,只对齐计数器
- 单帧/单次跳变有触发上限(64 / 4096),超了记警告并跳过,防止 TimeScale 极大时卡死
- `FireTick` 先拷贝回调列表再调用,允许回调内部注销自己

**仍待决:**

| 严重度 | 问题 | 说明 |
|---|---|---|
| 🟡 中 | 没采用固定步长 | `GameTime += DeltaTime * TimeScale` 浮点累加不确定 → 将来 Effect 日志回放会对不齐。**架构级决定,需热尘拍板** |
| 🟡 中 | 存档序列化未接 | `GetSaveState`/`ApplySaveState` 已备好,等阶段 2 定存档格式 |
| 🟢 低 | float/double 混用 | `GameDelta`/`RealDelta`/`TimeScale` 是 float,`GameTime` 是 double |
| 🟢 低 | `TimeCategory` 仍未使用 | 用或删,留着是噪音 |

### 6.2 待决的设计问题

- [ ] **时钟是否还需要 Timer 服务?** 讨论过:阈值跨越(冷却结束/Buff 到期)应该走集中调度的 Timer 服务,而不是每个系统自己比较时间戳。要决定 Timer 服务放 L0 还是别处
- [ ] **`FFormatTime` 的定位** — 它是纯展示用的结构,放在时钟子系统里合适吗?还是应该属于表现层?
- [ ] **引擎源码要不要装?** 二进制版导致很多细节只能靠猜(已踩过一次 `FTickableGameObject` 注册机制的坑)

---

## 七、已确立的设计决策(不要再反复讨论)

这些在 `Docs/ARCHITECTURE.md` 里有完整论证,这里只列结论:

1. **分层 L0~L10,依赖严格单向向下,零例外**
   - 运行时反向调用(回调/注册)不算依赖违规,只要编译期只依赖下层接口
2. **时间是"可推进的累计量",不是"读系统时钟"**
3. **时效性数据存「绝对时间戳(游戏时间)」,绝不存「剩余时长」**
   - 否则睡觉跳过 8 小时,冷却不会结束
4. **时间通知三分:连续量→拉取;阈值跨越→Timer 集中调度;控制状态→广播**
5. **TimeScale 变化要广播,但要幂等;提供语义化的 Paused/Resumed 事件**
6. **Effect 是唯一状态变更通道**,事件总线只通知不改状态
7. **条件引擎用 Provider 注册制**(依赖倒置),接口在 L3
8. **任务/物品等:定义(模板)与实例(运行时)分离**
9. **Agent 四条硬约束**:只读只提议 / 知识视图(信息隔离) / 版本戳(乐观并发) / 拒绝反馈回路
10. **存档策略:全量快照 + Effect 日志增量**,骨架期定型

---

## 八、环境备忘

| 项 | 值 |
|---|---|
| 引擎 | `E:\Epicgames\games\UE_5.8`(源码构建版,**有 Runtime 源码**) |
| 项目根 | `F:\project\TheDemiugeAgent\TheDemiugeUE5` |
| 架构文档 | `Docs/ARCHITECTURE.md` |
| 事实快照 | `Docs/STATUS.md`(脚本生成,勿手改) |
| 刷新命令 | `pwsh -File Tools\ProjectStatus.ps1` |
| git | 仓库已初始化,分支 `main` |

**代码风格约定**(他已有的习惯,保持一致):
- 源文件头:`// Copyright 2026 rechenz`
- 目录结构按功能分:`Public/Subsystem/<模块>/`、`Private/Subsystem/<模块>/`
- 蓝图可调用的接口加 `UFUNCTION(BlueprintCallable)`

---

## 九、给下一个会话的提醒

1. **先跑 `pwsh -File Tools\ProjectStatus.ps1`** 看有什么变化
2. 他**偏好简短直接**,要清单不要长篇展开;被纠正过三次"别啰嗦"
3. 遇到不确定的 UE 行为,**别猜** —— 二进制引擎看不到源码,要么装源码,要么明确说"这点需要验证"
4. 改代码前先跟他确认方向,他思路很快,经常一次给很多信息
