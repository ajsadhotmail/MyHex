# MyHex

轻量级跨平台命令行十六进制文件查看器，支持 TUI 与 ANSI 彩色高亮。

## 功能

- **TUI 交互模式**（默认）：键盘翻页浏览，自适应终端尺寸，支持十六进制偏移跳转
- **Dump 模式**：非交互输出，适合管道/重定向
- ANSI 彩色高亮（地址、控制字节、高字节、可打印字符各用不同颜色）
- 右侧 ASCII 字符面板
- 可配置每行字节数、起始偏移、最大显示长度

## 平台支持

| 平台 | 终端输入 | 终端尺寸 | ANSI 颜色 |
|---|---|---|---|
| Windows 10+ | `_getch()` + Console API | `GetConsoleScreenBufferInfo` | 虚拟终端 |
| Linux / macOS | `termios` raw mode + `select()` | `ioctl(TIOCGWINSZ)` | 原生支持 |

若需使用 ncurses，编译时加 `-DUSE_NCURSES` 并链接 `-lncurses`（需另行适配）。

## 编译

需要 C++17 及以上标准，**零外部依赖**：

```sh
# Linux / macOS
g++ -std=c++17 -O2 -Wall -o myhex MyHex.cpp

# Windows (MinGW)
g++ -std=c++17 -O2 -Wall -o myhex MyHex.cpp

# Windows (MSVC)
cl /std:c++17 /O2 /EHsc MyHex.cpp
```

## 用法

```
myhex [options] <filename>
```

### 选项

| 选项 | 说明 |
|---|---|
| `-n`, `--cols <N>` | 每行字节数（默认 16，最大 64） |
| `-s`, `--skip <N>` | 跳过文件开头 N 个字节 |
| `-l`, `--length <N>` | 最多显示 N 个字节 |
| `-d`, `--dump` | 非交互输出模式（适合管道） |
| `-C`, `--color` | 彩色输出（仅 dump 模式；TUI 模式始终彩色） |
| `--no-ascii` | 隐藏右侧 ASCII 字符面板 |
| `--lower` | 使用小写十六进制字母 |
| `-h`, `--help` | 显示帮助 |

### TUI 按键

| 按键 | 功能 |
|---|---|
| `↑` / `↓` | 上下滚动一行 |
| `PgUp` / `PgDn` | 上下翻整页 |
| `Home` / `End` | 跳到开头 / 结尾 |
| `g` | 输入十六进制偏移量跳转 |
| `q` / `Esc` | 退出 |

## 示例

```sh
# 交互 TUI 浏览（默认）
myhex file.bin

# TUI：每行 8 字节
myhex -n 8 file.bin

# 从偏移 0x100 开始，只显示 256 字节（彩色，非交互）
myhex -d -C --skip 256 --length 256 file.bin

# 管道：查找包含 "MZ" 的偏移
myhex -d file.exe | grep "MZ"

# 查看 PE 文件头（前 64 字节）
myhex -d -l 64 program.exe
```

## 彩色说明

TUI 模式始终彩色；`--dump` 模式需加 `-C` / `--color` 开启（Windows 10+ 自动启用 ANSI）：

| 颜色 | 含义 |
|---|---|
| 暗灰 | `0x00` 零字节、分隔符 |
| 红色 | 控制字符（`< 0x20` 或 `0x7F`） |
| 白色 | 可打印 ASCII（`0x20`–`0x7E`） |
| 青色 | 高字节（`>= 0x80`） |
| 绿色 | ASCII 面板中的可打印字符 |
