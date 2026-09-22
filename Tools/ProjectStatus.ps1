<#
.SYNOPSIS
    TheDemiugeUE5 项目进度采集器

.DESCRIPTION
    扫描 git 状态、文件变化、架构文档勾选进度,生成 Docs/STATUS.md(机器事实快照)。
    并打印一份"自上次跟进以来的变化"摘要。

    本脚本只采集事实,不做判断。
    人的判断写在 Docs/PROGRESS.md 里,两者配合使用。

.PARAMETER NoBaseline
    不更新 baseline 文件(只读模式,用于临时查看)

.EXAMPLE
    pwsh -File Tools\ProjectStatus.ps1
#>
[CmdletBinding()]
param(
    [switch]$NoBaseline
)

$ErrorActionPreference = 'Continue'

# ---------- 路径 ----------
$ToolsDir  = $PSScriptRoot
$Root      = Split-Path -Parent $ToolsDir
$DocsDir   = Join-Path $Root 'Docs'
$StatusMd  = Join-Path $DocsDir 'STATUS.md'
$Baseline  = Join-Path $DocsDir '.status-baseline.json'

if (-not (Test-Path $DocsDir)) { New-Item -ItemType Directory -Path $DocsDir -Force | Out-Null }

# 采集时排除的目录(编译产物 / 缓存 / IDE)
$ExcludePattern = '\\(\.git|Intermediate|DerivedDataCache|Binaries|Saved|\.vs|\.idea|node_modules|\.venv)\\'

function Get-TrackedFiles {
    Get-ChildItem -Path $Root -Recurse -File -ErrorAction SilentlyContinue |
        Where-Object { $_.FullName -notmatch $ExcludePattern }
}

# ---------- Git ----------
$gitOk = Test-Path (Join-Path $Root '.git')
$branch = ''; $head = ''; $commits = @(); $porcelain = @()

if ($gitOk) {
    $branch    = (git -C $Root rev-parse --abbrev-ref HEAD 2>$null) -join ''
    $head      = (git -C $Root rev-parse --short HEAD 2>$null) -join ''
    $commits   = @(git -C $Root log --oneline -10 2>$null)
    $porcelain = @(git -C $Root status --short 2>$null)
}

# ---------- 架构进度(勾选框统计) ----------
$archPath = Join-Path $DocsDir 'ARCHITECTURE.md'
$archStats = @()
$archTotal = 0; $archDone = 0

if (Test-Path $archPath) {
    $archLines = Get-Content $archPath -Encoding UTF8
    $currentSection = '(未分层)'
    $perSection = [ordered]@{}

    foreach ($line in $archLines) {
        if ($line -match '^###\s+(.+)$') {
            $currentSection = $Matches[1].Trim()
            if (-not $perSection.Contains($currentSection)) { $perSection[$currentSection] = @{Done=0; Total=0} }
        }
        # 只统计顶层条目(行首无缩进),子项不算独立系统
        if ($line -match '^-\s+\[( |x|X)\]\s+(.+)$') {
            $checked = $Matches[1] -ne ' '
            $label   = $Matches[2].Trim()
            if (-not $perSection.Contains($currentSection)) { $perSection[$currentSection] = @{Done=0; Total=0} }
            $perSection[$currentSection].Total++
            if ($checked) { $perSection[$currentSection].Done++ }
            $archTotal++
            if ($checked) { $archDone++ }
            $archStats += [pscustomobject]@{ Section=$currentSection; Done=$checked; Label=$label }
        }
    }
}

# ---------- 源码清单 ----------
$srcDir = Join-Path $Root 'Source'
$srcFiles = @()
if (Test-Path $srcDir) {
    $srcFiles = Get-ChildItem $srcDir -Recurse -File -Include *.h,*.cpp,*.cs |
        Sort-Object FullName |
        ForEach-Object {
            $rel   = $_.FullName.Replace("$Root\", '')
            $lines = (Get-Content $_.FullName -ErrorAction SilentlyContinue | Measure-Object).Count
            [pscustomobject]@{ Rel=$rel; Lines=$lines; Modified=$_.LastWriteTime }
        }
}

# ---------- 与 baseline 对比 ----------
$allFiles = Get-TrackedFiles
$snapshot = @{}
foreach ($f in $allFiles) {
    $snapshot[$f.FullName.Replace("$Root\",'')] = $f.LastWriteTime.ToString('o')
}

$prev = $null
if (Test-Path $Baseline) {
    try { $prev = Get-Content $Baseline -Raw -Encoding UTF8 | ConvertFrom-Json } catch { $prev = $null }
}

$added = @(); $changed = @(); $removed = @()
if ($prev -and $prev.Files) {
    $prevHash = @{}
    $prev.Files.PSObject.Properties | ForEach-Object { $prevHash[$_.Name] = $_.Value }

    foreach ($k in $snapshot.Keys) {
        if (-not $prevHash.ContainsKey($k)) { $added += $k }
        elseif ($prevHash[$k] -ne $snapshot[$k]) { $changed += $k }
    }
    foreach ($k in $prevHash.Keys) {
        if (-not $snapshot.ContainsKey($k)) { $removed += $k }
    }
}
$hasBaseline = ($null -ne $prev)

# ---------- 生成 STATUS.md ----------
$now = Get-Date -Format 'yyyy-MM-dd HH:mm:ss'
$sb = [System.Text.StringBuilder]::new()

[void]$sb.AppendLine('# 项目状态快照')
[void]$sb.AppendLine()
[void]$sb.AppendLine('> 生成时间:' + $now)
[void]$sb.AppendLine('> **本文件由 `Tools/ProjectStatus.ps1` 自动生成,不要手改。**')
[void]$sb.AppendLine('> 人的判断与计划写在 `Docs/PROGRESS.md`。')
[void]$sb.AppendLine()
[void]$sb.AppendLine('---')
[void]$sb.AppendLine()

# Git
[void]$sb.AppendLine('## 一、Git 状态')
[void]$sb.AppendLine()
if ($gitOk) {
    [void]$sb.AppendLine("| 项 | 值 |")
    [void]$sb.AppendLine("|---|---|")
    [void]$sb.AppendLine("| 分支 | $branch |")
    [void]$sb.AppendLine("| HEAD | $head |")
    [void]$sb.AppendLine("| 未提交改动 | $($porcelain.Count) 项 |")
    [void]$sb.AppendLine()
    [void]$sb.AppendLine('### 最近提交')
    [void]$sb.AppendLine()
    [void]$sb.AppendLine('```')
    foreach ($c in $commits) { [void]$sb.AppendLine($c) }
    [void]$sb.AppendLine('```')
    [void]$sb.AppendLine()
    if ($porcelain.Count -gt 0) {
        [void]$sb.AppendLine('### 工作区改动')
        [void]$sb.AppendLine()
        [void]$sb.AppendLine('```')
        foreach ($p in $porcelain) { [void]$sb.AppendLine($p) }
        [void]$sb.AppendLine('```')
        [void]$sb.AppendLine()
    }
} else {
    [void]$sb.AppendLine('_(不是 git 仓库)_')
    [void]$sb.AppendLine()
}

# 变化
[void]$sb.AppendLine('## 二、自上次跟进以来的变化')
[void]$sb.AppendLine()
if (-not $hasBaseline) {
    [void]$sb.AppendLine('_首次运行,无 baseline 可比对。_')
    [void]$sb.AppendLine()
} else {
    [void]$sb.AppendLine("| 类型 | 数量 |")
    [void]$sb.AppendLine("|---|---|")
    [void]$sb.AppendLine("| 新增 | $($added.Count) |")
    [void]$sb.AppendLine("| 修改 | $($changed.Count) |")
    [void]$sb.AppendLine("| 删除 | $($removed.Count) |")
    [void]$sb.AppendLine()
    [void]$sb.AppendLine("_baseline 时间:$($prev.Time)_")
    [void]$sb.AppendLine()
    if ($added.Count -gt 0) {
        [void]$sb.AppendLine('### 新增文件')
        [void]$sb.AppendLine()
        foreach ($a in ($added | Sort-Object)) { [void]$sb.AppendLine("- ``$a``") }
        [void]$sb.AppendLine()
    }
    if ($changed.Count -gt 0) {
        [void]$sb.AppendLine('### 修改文件')
        [void]$sb.AppendLine()
        foreach ($c in ($changed | Sort-Object)) { [void]$sb.AppendLine("- ``$c``") }
        [void]$sb.AppendLine()
    }
    if ($removed.Count -gt 0) {
        [void]$sb.AppendLine('### 删除文件')
        [void]$sb.AppendLine()
        foreach ($r in ($removed | Sort-Object)) { [void]$sb.AppendLine("- ``$r``") }
        [void]$sb.AppendLine()
    }
}

# 架构进度
[void]$sb.AppendLine('## 三、架构清单进度')
[void]$sb.AppendLine()
if ($archTotal -gt 0) {
    $pct = [math]::Round(100.0 * $archDone / $archTotal, 1)
    [void]$sb.AppendLine("**总计:$archDone / $archTotal($pct%)**")
    [void]$sb.AppendLine()
    [void]$sb.AppendLine('| 层 | 完成 | 总数 |')
    [void]$sb.AppendLine('|---|---|---|')
    foreach ($k in $perSection.Keys) {
        $s = $perSection[$k]
        [void]$sb.AppendLine("| $k | $($s.Done) | $($s.Total) |")
    }
    [void]$sb.AppendLine()
    $done = $archStats | Where-Object { $_.Done }
    if ($done) {
        [void]$sb.AppendLine('### 已完成条目')
        [void]$sb.AppendLine()
        foreach ($d in $done) { [void]$sb.AppendLine("- $($d.Label)") }
        [void]$sb.AppendLine()
    }
} else {
    [void]$sb.AppendLine('_(未找到 Docs/ARCHITECTURE.md 或其中没有勾选项)_')
    [void]$sb.AppendLine()
}

# 源码清单
[void]$sb.AppendLine('## 四、源码清单')
[void]$sb.AppendLine()
if ($srcFiles.Count -gt 0) {
    [void]$sb.AppendLine("| 文件 | 行数 | 最后修改 |")
    [void]$sb.AppendLine("|---|---|---|")
    foreach ($f in $srcFiles) {
        [void]$sb.AppendLine("| ``$($f.Rel)`` | $($f.Lines) | $($f.Modified.ToString('MM-dd HH:mm')) |")
    }
    [void]$sb.AppendLine()
    $totalLines = ($srcFiles | Measure-Object -Property Lines -Sum).Sum
    [void]$sb.AppendLine("合计 **$($srcFiles.Count)** 个源文件,**$totalLines** 行。")
    [void]$sb.AppendLine()
} else {
    [void]$sb.AppendLine('_(Source 下暂无源文件)_')
    [void]$sb.AppendLine()
}

[void]$sb.AppendLine('---')
[void]$sb.AppendLine()
[void]$sb.AppendLine('_由 `Tools/ProjectStatus.ps1` 生成 · 事实快照,不含判断_')

Set-Content -Path $StatusMd -Value $sb.ToString() -Encoding UTF8

# ---------- 更新 baseline ----------
if (-not $NoBaseline) {
    $payload = [pscustomobject]@{
        Time  = $now
        Head  = $head
        Files = $snapshot
    }
    $payload | ConvertTo-Json -Depth 5 | Set-Content -Path $Baseline -Encoding UTF8
}

# ---------- 控制台摘要 ----------
Write-Host ''
Write-Host '════════ TheDemiugeUE5 状态 ════════' -ForegroundColor Cyan
if ($gitOk) { Write-Host "Git      : $branch @ $head  ($($porcelain.Count) 项未提交)" }
if ($hasBaseline) {
    Write-Host "变化     : +$($added.Count)  ~$($changed.Count)  -$($removed.Count)  (自 $($prev.Time))" -ForegroundColor Yellow
} else {
    Write-Host "变化     : 首次运行,已建立 baseline" -ForegroundColor Yellow
}
if ($archTotal -gt 0) {
    $pct = [math]::Round(100.0 * $archDone / $archTotal, 1)
    Write-Host "架构进度 : $archDone / $archTotal  ($pct%)"
}
Write-Host "源码     : $($srcFiles.Count) 个文件"
Write-Host "快照     : Docs/STATUS.md" -ForegroundColor Green
Write-Host '════════════════════════════════════' -ForegroundColor Cyan
Write-Host ''
