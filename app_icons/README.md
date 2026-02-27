# 应用图标资源说明

## 图标来源
本图标资源来自 [Simple Icons](https://github.com/simple-icons/simple-icons) 开源项目，采用 **CC0 1.0 通用公共领域贡献** 许可证，意味着：
- ✅ 可免费商用
- ✅ 无需署名
- ✅ 可自由修改和使用

## 图标列表

### 1. 社交媒体类 (social_media/)
| 文件名 | 应用名称 |
|--------|----------|
| facebook.svg | Facebook |
| instagram.svg | Instagram |
| twitter.svg | Twitter/X |
| youtube.svg | YouTube |
| whatsapp.svg | WhatsApp |
| linkedin.svg | LinkedIn |
| tiktok.svg | TikTok |
| pinterest.svg | Pinterest |
| snapchat.svg | Snapchat |
| discord.svg | Discord |

### 2. 办公软件类 (office/)
| 文件名 | 应用名称 |
|--------|----------|
| google.svg | Google |
| googledocs.svg | Google Docs |
| googlesheets.svg | Google Sheets |
| googleslides.svg | Google Slides |
| icloud.svg | iCloud |
| notion.svg | Notion |
| evernote.svg | Evernote |
| trello.svg | Trello |
| zoom.svg | Zoom |

### 3. 工具类 (tools/)
| 文件名 | 应用名称 |
|--------|----------|
| github.svg | GitHub |
| git.svg | Git |
| npm.svg | npm |
| docker.svg | Docker |
| figma.svg | Figma |
| postman.svg | Postman |
| sketch.svg | Sketch |
| axios.svg | Axios |
| android.svg | Android |
| apple.svg | Apple |
| linux.svg | Linux |
| safari.svg | Safari |

### 4. 娱乐类 (entertainment/)
| 文件名 | 应用名称 |
|--------|----------|
| spotify.svg | Spotify |
| netflix.svg | Netflix |
| twitch.svg | Twitch |
| steam.svg | Steam |
| discord.svg | Discord |
| applemusic.svg | Apple Music |
| youtubemusic.svg | YouTube Music |
| soundcloud.svg | SoundCloud |

## SVG转PNG方法

### 方法1: 在线转换 (推荐)
访问以下免费在线转换工具：
- https://convertio.co/svg-png/
- https://www.iconscm.com/svg-to-png
- https://www.pngaaa.com/svg2png

操作步骤：
1. 上传SVG文件
2. 选择输出尺寸 (推荐 512x512 或 1024x1024)
3. 下载PNG文件

### 方法2: 使用Python转换 (本地)
```python
import cairosvg
import os

# 转换单个文件
cairosvg.svg2png(url="icon.svg", write_to="icon.png")

# 批量转换
for file in os.listdir("icons"):
    if file.endswith(".svg"):
        cairosvg.svg2png(url=file, write_to=file.replace(".svg", ".png"))
```

安装依赖：
```bash
pip install cairosvg
```

### 方法3: 使用在线图标生成器
访问 https://www.iconfinder.com/ 可以直接下载PNG格式图标

## 版权说明
- Simple Icons 项目采用 **CC0 1.0** 许可证
- 图标为各品牌官方logo，使用时请注意品牌使用规范
- 更多详情请访问: https://github.com/simple-icons/simple-icons

## 文件信息
- 格式: SVG (可缩放矢量图形)
- 视图框: 24x24
- 用途: 适用于网页、应用、UI设计

---
生成时间: 2026-02-27
