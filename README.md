# DesktopPet 桌面宠物

一只住在 Windows 桌面上的小橘猫：基于 **WinUI 3 (C++/WinRT)** 的桌面宠物，支持语音交互。

## 功能

- 无边框透明窗口，悬浮于屏幕底部，置顶且不抢占任务栏
- 空闲时随机散步、眨眼、呼吸浮动；可用鼠标拖拽移动
- 右键菜单：聊天 / 喂食 / 散步 / 退出
- 聊天面板：文字输入对话（关键词规则 + 随机回复，语料在 `DesktopPet/data/quotes.json`）
- 语音交互：调用 Windows 语音识别（`Windows.Media.SpeechRecognition`），说出指令小橘会回应

## 环境要求

- Windows 10 17763+ / Windows 11
- Visual Studio 2022（17.8+），工作负载：
  - 使用 C++ 的桌面开发（MSVC v143、Windows 11 SDK 10.0.22621.0）
  - Windows 应用 SDK C++ 模板（或单独安装 Windows App SDK 1.6）
- [vcpkg](https://github.com/microsoft/vcpkg)，并执行过 `vcpkg integrate install`
  （仓库根目录的 `vcpkg.json` 会以清单模式自动安装 nlohmann-json、ms-gsl）

## 构建运行

1. `git clone` 本仓库
2. 用 Visual Studio 打开 `DesktopPet.sln`，等待 NuGet 还原（Microsoft.WindowsAppSDK / CppWinRT / SDK BuildTools）与 vcpkg 清单安装完成
3. 首次生成时，预生成事件会自动运行 `DesktopPet/make-images.ps1` 生成应用图标到 `DesktopPet/Images/`
4. 选择 `Debug|x64`，F5 运行（首次部署需开启 Windows 开发者模式）

> 工程默认关闭了 MSIX 签名（`AppxPackageSigningEnabled=false`），便于本地直接 F5 调试；
> 如需打包分发，请在 Visual Studio 的「打包」向导中创建临时证书或正式证书。

## 目录结构

```
DesktopPet/
├─ DesktopPet.sln
├─ vcpkg.json                  # C++ 依赖清单（nlohmann-json, ms-gsl）
└─ DesktopPet/
   ├─ DesktopPet.vcxproj       # WinUI3 C++/WinRT 工程
   ├─ App.xaml / App.xaml.cpp  # 应用入口
   ├─ MainWindow.xaml(.cpp)    # 宠物界面与全部行为逻辑
   ├─ Package.appxmanifest     # MSIX 清单（含麦克风能力声明）
   ├─ make-images.ps1          # 预生成事件：生成应用图标
   └─ data/quotes.json         # 聊天语料与关键词规则
```
