# 开发指南

## 架构

项目采用 JUCE 8.0.4、C++17、CMake 和 Visual Studio 工具链，输出 VST3 与 Standalone。

| 文件 | 职责 |
| --- | --- |
| `Source/Parameters.h/.cpp` | 参数 ID、范围、选项和 XY 映射 |
| `Source/PluginProcessor.h/.cpp` | 宿主接口、APVTS、音频块处理、手势状态和持久化 |
| `Source/GrainEngine.h/.cpp` | 读头轮换、随机化、交叉反馈、干湿混合和参数平滑 |
| `Source/RingBuffer.h/.cpp` | 预分配延迟缓冲、回绕和线性插值 |
| `Source/PluginEditor.h/.cpp` | 控件布局、参数附件和交互同步 |
| `Source/XYPad.h/.cpp` | Pad 绘制、坐标和鼠标手势 |
| `Source/ModernLookAndFeel.h/.cpp` | 控件绘制和路由高亮 |

宿主和界面通过 APVTS 写入参数，处理器生成 `GrainEngine::Settings`，引擎逐块处理立体声音频。缓冲在 `prepareToPlay()` 时分配，音频循环不得分配内存或访问 UI 对象。

## 参数与 XY

读取 DSP 参数使用 APVTS 原始值；向宿主写值前调用参数的 `convertTo0to1()`。Pad 坐标通过 `NormalisableRange` 映射到真实参数范围。

选择目标只改变高亮。Pad 手势写回真实参数，释放后保留最后的值。手动旋钮操作优先，在 `Slider::onDragStart` 中停止对应路由的活动手势。X/Y 同目标时 X 优先。Sync 开启时，选择 Delay 目标保留同步；实际操作 Pad 的 Delay 路由才切换到自由延迟。

## 构建

```powershell
cmake -S . -B build -DJUCE_DIR="third_party/JUCE"
cmake --build build --config Release --target GrainDelayRE_VST3 GrainDelayRE_Standalone
```

VST3 路径：`build/GrainDelayRE_artefacts/Release/VST3/Grain Delay RE.vst3`。

Standalone 路径：`build/GrainDelayRE_artefacts/Release/Standalone/Grain Delay RE.exe`。

## 回归测试

三个测试项目各自编译生产源码。以下命令在项目根目录运行，其他 JUCE 位置可在配置时添加 `-DJUCE_DIR=<path>`。

```powershell
cmake -S tests/grain-probe -B build/tests/grain-probe
cmake --build build/tests/grain-probe --config Release
& build/tests/grain-probe/GrainEngineProbe_artefacts/Release/GrainEngineProbe.exe

cmake -S tests/processor-probe -B build/tests/processor-probe
cmake --build build/tests/processor-probe --config Release
& build/tests/processor-probe/GrainAudioProbe_artefacts/Release/GrainAudioProbe.exe

cmake -S tests/parameter-probe -B build/tests/parameter-probe
cmake --build build/tests/parameter-probe --config Release
& build/tests/parameter-probe/Release/GrainParameterAudit.exe
```

DSP 测试覆盖相位与读头轮换、变调、分数读取、冲激延迟、干湿端点和极端参数。处理器测试覆盖音频块、Sync、XY 写回、高亮及手动优先。参数测试覆盖 Sync 目标选择和百分比文本往返。

发布前使用本地 pluginval：

```powershell
& pluginval --validate "build/GrainDelayRE_artefacts/Release/VST3/Grain Delay RE.vst3" --strictness-level 7 --timeout-ms 120000 --output-dir build/validation --output-filename pluginval-level7.txt
```

pluginval 中独立的 Steinberg validator 需要额外配置；跳过的步骤不能记为已通过。修改布局后启动 Standalone 检查界面。

## 安装

将完整 VST3 bundle 复制到宿主扫描的目录。系统目录 `%CommonProgramFiles%\VST3` 需要管理员权限。构建后先核对宿主加载文件与构建文件的 SHA-256，再进行宿主测试。
