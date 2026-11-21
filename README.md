# Mirror Archive · 魔镜档案

**Every picture keeps a secret.** A small, fully offline, mouse-driven image-processing mystery built with **openFrameworks 0.12.1 + OpenCV** on macOS.

玩家扮演档案修复员，用有限能量选择局部观察工具，收集证据，再判断隐藏的真品。三个案件分别利用颜色与形状、局部特征、时间信息；无需摄像头、实体道具或网络。

![Mirror Archive](docs/menu.png)

## Three cases

| Case | 玩家任务 | 实际计算 |
| --- | --- | --- |
| 01 · Lost seal | 在受噪声干扰的照片中找到青绿色七角徽章 | 中值滤波、强度窗口、HSV 分割、形态学开闭运算、Canny |
| 02 · The original | 在旋转、缩放的版画中鉴定真品 | SIFT 特征与描述子、L2 近邻、ratio test、RANSAC 单应性 |
| 03 · Night moths | 在相同外观的飞蛾中发现逆向飞行者 | 帧差、阈值化、Farneback 稠密光流、方向筛选 |

所有工具从**观测图像**计算。答案区域仅用于点击判定；没有用答案坐标伪造分割、匹配或光流。第三关是程序生成的连续图像序列，不是摄像头视频；运动方向依然由图像帧计算。

## Build and run

目前支持 macOS；在 Apple Silicon、Xcode 27.2 和现有 OF 0.12.1 环境中开发验证。界面采用原创 5×7 位图字体、640×384 像素画布、整数倍最近邻显示，并开启 Retina 支持。无需系统字体。游戏内为英文，项目说明为中文。

1. 安装 Xcode，并准备 [openFrameworks 0.12.1 macOS](https://openframeworks.cc/download/) 完整发行包（含自带的 `ofxOpenCv`）。不需要额外下载 addon。
2. 在仓库根目录运行：

```bash
./scripts/configure.sh /absolute/path/to/of_v0.12.1_osx_release
./tests/run_tests.sh
./scripts/build.sh
open build/Build/Products/Release/MirrorArchive.app
```

也可在 configure 后打开 `MirrorArchive.xcodeproj`，选择 **MirrorArchive Release → My Mac**，按 ⌘R。`configure.sh` 只在 `.deps/` 建立本地依赖链接，不把整个 OF 发行包加入仓库。

当前课程工作区布局可直接运行 `./scripts/build.sh`，脚本会找到 `../../of_v0.12.1_osx_release`。

## Controls

| 操作 | 效果 |
| --- | --- |
| 鼠标左键拖动 | 用当前工具局部擦镜；第二关 Match 为点击扫描 |
| `1` / `2` / `3` | 切换该关的三个工具 |
| 滚轮 / `[` / `]` | 调整画笔半径 |
| `E` | 保存当前工具的证据快照到笔记本 |
| `N` | 打开证据笔记本 |
| 笔记本中 `S` | 导出证据 PNG 与 JSON 记录 |
| `M` | 进入答案标记模式，再点击目标 |
| `Tab` | 切换原始观测图与处理图 |
| `X` | 第一关 Edges 切换原始 Canny / 中值去噪后的 Canny |
| `Space` | 第三关冻结 / 继续时间 |
| `R` | 重开当前关卡 |
| `H` | 操作说明 |
| `Esc` | 关闭弹窗或返回档案首页 |

第一关需用 `E` 留下 Color 和 Edges 两种证据；第二关需得到可靠的几何匹配；第三关需留下实际测到运动的 Flow / Leftward 证据。然后才能标记目标。

每个工具只对**新揭示面积**扣能量，同一层重复观察免费。第二关特征验证每次 9 能量，错误判断扣 12，提示扣 12。剩余至少 50 能量且没有错误/提示可获得三枚印章。无倒计时，可随时重试。

最高评级自动保存在 `~/Library/Application Support/MirrorArchive/progress.json`。导出的证据位于同目录的 `evidence/` 中。不采集个人数据，不进行网络通信。

## Structure

```text
src/main.cpp          窗口及 QA 模式入口
src/ofApp.*           界面、交互、关卡生命周期、证据笔记本
src/VisionEngine.h    不读取答案的图像处理函数
src/LevelFactory.h    可复现的关卡观测图、连续帧和判定区域
src/GameModel.h       面积计费、证据条件和评分
src/PixelUI.h         原创位图字体、像素图标与木框控件
src/PixelGrid.h       整数缩放与鼠标坐标的统一映射
tests/VisionTests.cpp 实际 OpenCV 算法与游戏规则回归检查
bin/data/art/         原创生成插图
scripts/              本地依赖配置和构建
```

[技术说明](docs/TECHNICAL.md) · [演示路线](docs/DEMO.md) · [素材来源与生成记录](docs/ASSETS.md)

## Validation

```bash
./tests/run_tests.sh
# Native window smoke tour; takes screenshots and exits without changing progress.
build/Build/Products/Release/MirrorArchive.app/Contents/MacOS/MirrorArchive --qa
```

回归检查覆盖真品/仿品鉴定、静止帧与运动帧、真实运动方向、去噪的边缘改善、掩膜端点、重复探索计费、余额不足、重置与评分。38 项检查也覆盖 Retina、普通显示器和缩放窗口下的坐标对应；原生 QA 路线依次完成三关并生成界面截图。
截图保存在 `build/Build/Products/qa/`，不会写入应用包或修改玩家存档。

本仓库提供程序、素材、说明和演示路线。课程要求的最终 PDF 报告和在线演示视频仍需整理、录制并提交；不要把源代码仓库链接当成演示视频链接。
