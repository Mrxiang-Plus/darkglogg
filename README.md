# darkglogg - 日志分析工具定制版

基于 [glogg](https://github.com/nickbnf/glogg) 开源日志浏览器的深度定制版本，专为程序员和系统管理员设计，提供快速、交互式的日志浏览和正则表达式搜索功能。

## 版本

当前版本：**Ubuntu_V1.3**

## 主要特性

- 跨平台 Qt(C++) 开发
- 快速打开 1GB 以上日志文件
- 支持正则表达式搜索及过滤
- 支持拖拽压缩包解压并直接打开 (.zip .tar.gz .gz .tar .rar .7z)
- 一键开启 Logcat (支持进程过滤, F1 开始 / F2 停止)
- 支持 vim/wasd 风格浏览日志
- 多主题切换 (Dark+, Dark, Dracula, Monokai, White)
- 中文界面支持，即时语言切换
- 底部常用过滤词条栏，一键过滤匹配 log
- 带高亮色复制到 Jira (Ctrl+Shift+C)
- A&B 对比功能 (Shift+A / Shift+B)
- 支持选中 proguard 日志自动 retrace
- 多级过滤层层筛选 (Ctrl+Shift+T)
- 关键 log 一键上传 git 仓自动 merge

## 主题系统

使用 JSON 配置主题，内置 5 套主题：

| 主题 | 风格 |
|------|------|
| VSCode Dark+ | 经典深色，参考 VSCode |
| Dark | 现代暗色，Tokyo Night 风格 |
| Dracula | 紫色调暗色 |
| Monokai | 绿色调暗色 |
| White | 亮色默认 |

自定义主题：将 `.json` 文件放入 `~/.glogg/themes/` 目录即可。

## 快捷键

| 快捷键 | 功能 |
|--------|------|
| y / t | 添加到过滤器 |
| i / x | 标记 (mark) |
| b | mark 并添加到过滤器 |
| m | 标记当前行 |
| Ctrl+Z | 最大化过滤器窗口 |
| Space | 切换窗口 |
| - / = | 调整窗口大小 |
| . | 切换到过滤器输入框 |
| Ctrl+O | 打开文件 (支持解压 bugreport) |
| Ctrl+S | 另存为并在新窗口打开 |
| Ctrl+C | 复制 |
| Ctrl+Shift+C | 带高亮色复制到 Jira |
| c | 底部按钮1 并清空其他 (常用log) |
| e | 底部按钮2 并清空其他 (error) |
| Alt+num | 底部按钮 1,2,3...9 |
| r | 重置底部按钮按压状态 |
| Shift+A / Shift+B | A&B 对比 |
| ? | 保存过滤器 comment > filter > mark |
| v | marks/matches 视图切换 |
| [ / ] | 跳转到上/下一个标记行 |
| Ctrl+J / Ctrl+K | 下拉框上/下一个选项 |
| Ctrl+P | 复制 filter 到当前过滤框 |
| Ctrl+. | 直接下拉列表 |
| Ctrl+num | 切换指定窗口 |
| j/k/h/l | 移动 (vim 风格) |
| w/a/s/d | 移动 (wasd 风格) |
| Ctrl+U/D, PgUp/PgDn | 快速翻页 |
| [num] g | 跳转到指定行 |
| p | 添加注释 |
| F1 / F2 | 开始/停止 Logcat |
| Ctrl+R | 反混淆 (Retrace) |
| Ctrl+Shift+R | 重新格式化 |
| Shift+F12 | 显示/隐藏菜单 |
| Shift+F | 最大化/还原窗口 |
| Escape | 取消焦点 |
| Ctrl+F 或 / | 查找 |
| f | 跟随模式 |

## 编译

依赖：
- GCC 4.8+
- Qt 5.2+
- Boost program-options

```bash
qmake
make -j$(nproc)
./release/glogg
```

注意：如果系统中有 Anaconda 的 Qt 库冲突，使用系统 Qt：
```bash
/usr/lib/qt5/bin/qmake && make -j$(nproc)
```

## 安装

```bash
sudo cp ./release/glogg /usr/bin/glogg
```

## 配置文件

- 配置文件：`~/.config/glogg/glogg.ini`
- 脚本目录：`~/.glogg/`
- 自定义主题：`~/.glogg/themes/*.json`

## 源码仓库

https://git.n.xiaomi.com/common-tool/darkglogg
