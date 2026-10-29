# 发布流程

本流程处理源码许可、依赖说明、验证和打包，不构成对代码来源或第三方权利的法律认证。

## 许可与版本

- 本项目由 TriM 有权授权的代码采用 `AGPL-3.0-only`；许可全文见根目录 `LICENSE`。
- 保留源码中的版权、SPDX 标识以及 `THIRD_PARTY_NOTICES.md` 和 `licenses/`。
- 标准 AGPL 允许商业使用和收费分发；不得附加禁售、禁止商用或强制给作者付费的限制。
- 音乐制作、商业发行和音频服务不受限制；普通音频输出不因使用本效果器而变成 AGPL 软件。
- `About / License` 提供版权、许可、无担保说明、源码地址及完整许可文本。
- 在 `CMakeLists.txt`、README 和 changelog 中同步版本。保留插件 ID 与参数 ID，以兼容已有工程。

## 发布前验证

1. 按 `docs/DEVELOPMENT.md` 完成 Release 构建、三组回归测试和 pluginval 严格级别 7。
2. 启动 Standalone，检查正常界面和 `About / License`；确认其可关闭、文字可滚动。
3. 核对所有文件的发布权利及第三方许可，完成代码来源审查；此项不能由自动构建替代。
4. 提交源码，再为该提交创建匹配版本的 `v<version>` 标签。构建和打包期间工作区必须保持干净。

## 对应源码与离线构建

GitHub 自动生成的源码 ZIP 不包含 JUCE，因此发布二进制时还应提供完整对应源码。
运行打包脚本需要 Python 3.9 或更新版本。
仓库中的打包脚本会保存当前提交的项目源码、实际使用的完整 JUCE 源码、构建说明、
版本信息和文件 SHA-256。脚本不自动上传、签名、安装或发布任何资产。

```powershell
python scripts/package_release.py --juce-dir "third_party/JUCE"
# 通过验证后，使用刚才测试的 Release 产物生成用户下载包：
python scripts/package_release.py --juce-dir "third_party/JUCE" --include-binaries
```

产物位于 `build/releases/<version>/`：

- `GrainDelayRE-<version>-source.zip`：可离线构建的完整对应源码。
- `GrainDelayRE-<version>-windows.zip`：VST3 bundle、Standalone、许可及用户说明，仅在指定二进制选项时生成。
- `SHA256SUMS.txt`：下载包校验值。

离线复建：解压源码包，进入 `GrainDelayRE-<version>-source`，执行：

```powershell
cmake -S . -B build -DJUCE_DIR=third_party/JUCE
cmake --build build --config Release --target GrainDelayRE_VST3 GrainDelayRE_Standalone
```

离线包仍需要用户自行安装 Windows C++ 工具链与 CMake；这些通用工具不属于随附源码。

## GitHub Release

在对应标签上创建 Release，同时上传经过验证的 Windows 包、完整源码包和校验文件。
下载说明紧邻二进制提供完整源码链接；保证该源码在分发二进制期间持续可得。
发布说明列出变更和验证范围，不能将跳过的检查写成通过。

安装时将完整插件 bundle 复制到宿主扫描目录，并重新扫描插件。
