# Mirror Archive · 魔镜档案

**抢救镇上的第一张大合照。** openFrameworks 0.12.1 + OpenCV 像素解谜游戏。

玩家是一名档案修复员，与小猪助手 **tonton** 一起修复记录、打开照片柜、拼回镇民合照。首页只有 START，三关连续推进，不需要实体道具。tonton 的简短提示会自动消失，点击它或 ? 可以重看。

![Workshop](docs/menu.png)

| 章节 | 操作 | 实际计算 |
| --- | --- | --- |
| 雨损记录 | 拖 CLEAN / CLEAR，读出记录上的密码，KEEP 带走记录 | 中值滤波、对比度调整、反锐化掩模 |
| 照片柜 | 横向拖动三个联动拨轮；CAMERA 开启捏合操作 | OpenCV 摄像头、MediaPipe 21 点手部追踪、尺度归一化、防抖与捏合迟滞 |
| 第一张镇民合照 | 拖动重叠扫描片，TURN 或 R 旋转，接近正确位置自动吸附 | SIFT、L2 匹配、比值筛选、RANSAC 相似变换、实际扫描片配准及羽化融合 |

![Photo restoration](docs/case3.png)

第二关 RECORD 可重看修复后的记录，REPAIR 可回去继续调节。左拨轮带动中间，中间带动右边；没有计时器。鼠标始终可用。摄像头仅在主动点击 CAMERA 后开启，离开关卡或关闭游戏时释放；图像不保存、不上传。

## 运行

macOS / Apple Silicon，现有课程 openFrameworks 环境：

```bash
./scripts/configure.sh /absolute/path/to/of_v0.12.1_osx_release
./scripts/build.sh
open build/Build/Products/Release/MirrorArchive.app
```

摄像头功能需要 Python 3.11+，首次安装运行：

```bash
./scripts/setup-camera.sh
```

此脚本创建项目内 .venv，安装锁定版本并预热手部模型。首次字体缓存可能需要数分钟。模型已随源资源提供，游戏运行时不联网。当前摄像头依赖版本需要 macOS 15+。应用从自身父目录寻找 .venv，因此请保留 app 在项目 build 目录中；也可用 MIRROR_PYTHON 指定其它已安装依赖的 Python。首次点击 CAMERA 时游戏会请求系统摄像头权限，获准后再启动 Python 追踪进程。

## 代码

- src/ofApp.*：界面、三关流程、输入路由、原生自动试玩。
- src/ArchiveGame.h：实时滤波、联动密码锁、扫描片配准及拼接。
- src/HandInput.h + bin/data/hand/tracker.py：离线手部追踪进程、捏合防抖、超时与资源释放。
- src/CameraAccess.h / .mm：macOS 原生摄像头授权。
- src/Tonton.h：原创像素小猪、眨眼、盖章、跳跃、抱相册庆祝。
- src/PixelUI.h / PixelGrid.h：5×7 位图字、640×384 画布、整数倍最近邻显示。
- tests/：真实素材算法测试、手势状态测试、旧版本算法回归检查。
- LevelFactory.h / VisionEngine.h：保留的早期关卡算法和回归样本，当前游戏不调用。

## 验证

```bash
./tests/run_tests.sh
python3 tests/test_hand.py
.venv/bin/python bin/data/hand/tracker.py --model bin/data/hand/hand_landmarker.task --probe
build/Build/Products/Release/MirrorArchive.app/Contents/MacOS/MirrorArchive --qa
```

85 项 C++ 检查、4 项手势状态测试。原生 QA 通过玩家同一套鼠标事件依次完成三关并截图到 build/Build/Products/qa。摄像头探针测试真实模型加载和空白帧推理；自动 QA 不等于真人手势体验测试。

[技术与课程对应](docs/TECHNICAL.md) · [演示路线（含剧透）](docs/DEMO.md) · [素材、模型来源与生成提示词](docs/ASSETS.md)

课程 PDF 报告与实际在线演示视频需另行整理提交。
