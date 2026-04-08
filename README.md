# 🌱 Plants vs Zombies - 植物大战僵尸

使用 **C语言** 和 **EasyX 图形库** 写的一个小游戏，适合初学者学习参考。

### 🎬 游戏演示

[![点击观看演示]](https://github.com/user-attachments/assets/a3215ed9-3de2-4510-8c6e-b7b582dcff27)

### 🎮 下载安装
> 直接下载即可游玩，无需安装任何环境！

| 步骤 | 操作 |
|------|------|
| 1 | 点击右侧 **Releases** |
| 2 | 下载最新版本的 `Plants-vs.-Zombies.zip` |
| 3 | 解压到任意文件夹 |
| 4 | 双击 `Plants-vs.-Zombies.exe` 开始游戏 |

### 🎯 环境要求

- Windows 7 / 8 / 10 / 11
- [Visual Studio 2026](https://visualstudio.microsoft.com/zh-hans/)
- [EasyX 图形库](https://easyx.cn/)

> 本人在 Windows 11 下开发测试，其他版本应该也能正常运行。

### 🛠️ 编译步骤

1. 安装 Visual Studio 2026（社区版免费）
2. 安装 EasyX 图形库
3. 用 Visual Studio 新建一个空项目
4. 将以下源码文件添加到项目：
   - `main.cpp`
   - `tools.cpp`
   - `tools.h`
   - `vector2.cpp`
   - `vector2.h`
5. 将 `res` 文件夹放到项目目录下（与 `main.cpp` 文件同级）
6. **菜单栏 → 项目 → 属性 → 配置属性 → 高级 → 字符集 → 改为"使用多字节字符集"**
7. 编译运行（按 F5）

### 🐛 常见问题

| 问题 | 解决方法 |
|------|---------|
| 编译报错 | 字符集改为"使用多字节字符集" |
| 音效没声音 | 音频文件名改为英文 |
| 图片不显示 | 检查 `res` 文件夹是否存在 |

---
<p align="center">
  <a href="https://github.com/AlbertWaker/Plants-vs.-Zombies">
    <img src="https://img.shields.io/github/stars/AlbertWaker/Plants-vs.-Zombies?style=social" alt="Star">
  </a>
</p>

<p align="center">
  如果对你有帮助，欢迎给个 Star ⭐
</p>
