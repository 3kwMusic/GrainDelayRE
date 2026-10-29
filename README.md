# Grain Delay RE

基于 JUCE 8.0.4 和 C++17 开发的立体声颗粒延迟插件，提供 VST3 和 Standalone。当前版本：**1.0.4**。

支持延迟、颗粒频率、变调、随机变调、Spray、交叉反馈和 Dry/Wet，包含节拍同步、Swing、XY 参数控制和宿主状态保存。

[下载 Windows 插件及完整对应源码](https://github.com/3kwMusic/GrainDelayRE/releases/latest)

## 构建

需要 Windows、Visual Studio C++ 工具链、CMake 3.22 或更新版本，以及 JUCE 8.0.4。

```powershell
git clone --branch 8.0.4 --depth 1 https://github.com/juce-framework/JUCE.git third_party/JUCE
cmake -S . -B build
cmake --build build --config Release --target GrainDelayRE_VST3 GrainDelayRE_Standalone
```

产物位于 `build/GrainDelayRE_artefacts/Release/VST3/` 和 `build/GrainDelayRE_artefacts/Release/Standalone/`。
已有 JUCE 源码时，也可设置 `JUCE_DIR` 环境变量或传入 `-DJUCE_DIR=<源码目录>`。
完整对应源码发行包已含 `third_party/JUCE`，可跳过下载步骤。

## 开发与验证

架构及测试命令见 [开发指南](docs/DEVELOPMENT.md)。`tests/` 保存 DSP、处理器、XY 交互和参数回归测试。发布前运行 Release 构建及 pluginval 严格级别 7 验证。

JUCE 依赖不直接提交到此仓库；完整对应源码发行包包含构建所需的 JUCE 源码。

## 许可与使用

Copyright (C) 2026 TriM。本项目代码采用 **GNU Affero General Public License v3.0 only**
（SPDX：`AGPL-3.0-only`），全文见 [LICENSE](LICENSE)。第三方组件保留各自许可和版权，
详见 [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md) 和 [licenses/](licenses/)。

欢迎将插件用于个人或商业音乐制作、演出、发行和音频服务；普通音频作品不因使用本插件
而受其软件许可证约束，无需向作者付费或署名。软件本身也允许按 AGPL 条件收费分发。
分发插件或修改版时，须保留许可及版权声明，并按许可证提供完整对应源码。

本软件按“现状”提供，在适用法律允许的范围内不提供担保并限制责任，具体以
LICENSE 第 15、16 条为准。许可与免责说明不能代替对第三方权利的核查。

## 发布与源码

变更见 [CHANGELOG.md](CHANGELOG.md)。发布流程见 [docs/RELEASING.md](docs/RELEASING.md)。
发布二进制时同时提供包含完整 JUCE 依赖的对应源码包；GitHub 自动生成的仓库源码包
本身不包含 JUCE。插件内的 **About / License** 可离线查看许可、第三方声明和源码地址。
