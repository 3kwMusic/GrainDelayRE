# Grain Delay RE

基于 JUCE 8.0.4 和 C++17 开发的立体声颗粒延迟插件，提供 VST3 和 Standalone。当前版本：**1.0.1**。

支持延迟、颗粒频率、变调、随机变调、Spray、交叉反馈和 Dry/Wet，包含节拍同步、Swing、XY 参数控制和宿主状态保存。

## 构建

需要 Windows、Visual Studio C++ 工具链、CMake 3.22 或更新版本，以及 JUCE 8.0.4。

```powershell
cmake -S . -B build -DJUCE_DIR="third_party/JUCE"
cmake --build build --config Release --target GrainDelayRE_VST3 GrainDelayRE_Standalone
```

产物位于 `build/GrainDelayRE_artefacts/Release/VST3/` 和 `build/GrainDelayRE_artefacts/Release/Standalone/`。

## 开发与验证

架构及测试命令见 [开发指南](docs/DEVELOPMENT.md)。`tests/` 保存 DSP、处理器、XY 交互和参数回归测试。发布前运行 Release 构建及 pluginval 严格级别 7 验证。

JUCE、验证工具、构建产物和本机工作文件不随仓库分发。
