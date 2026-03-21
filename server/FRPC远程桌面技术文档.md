# FRPC 远程桌面技术原理与部署说明

## 一、技术原理

### 1.1 整体架构

```
┌─────────────────┐          ┌─────────────────┐          ┌─────────────────┐
│   用户电脑A      │          │   FRP服务器      │          │   用户电脑B      │
│  (被控制端)      │          │  (中转服务器)    │          │  (控制端)        │
│                 │          │                 │          │                 │
│  frpc.exe ─────┼─────────►│   frps.ini     │◄─────────┼─ RDP客户端     │
│  本地:3389      │          │  端口:7000      │          │                 │
│  远程:20xxx     │          │  端口:20xxx     │          │                 │
└─────────────────┘          └─────────────────┘          └─────────────────┘
```

### 1.2 FRP 工作原理

FRP (Fast Reverse Proxy) 是一个反向代理工具，用于实现内网穿透。

**核心组件：**
- **FRPS (FRP Server)**: 部署在公网服务器上，监听端口 7000
- **FRPC (FRP Client)**: 运行在用户电脑上，连接 FRPS

**端口映射流程：**

1. **FRPC 启动**：连接到 FRPS 服务器 (8.163.37.74:7000)
2. **建立隧道**：FRPC 将本地 RDP 端口 (3389) 映射到 FRPS 的端口
3. **端口分配**：
   - 每个用户有唯一的 userId
   - 每个设备有唯一的 deviceName (电脑名)
   - 端口 = 20000 + hash(userId + deviceName) % 30000
   - 端口范围：20000-49999
4. **远程连接**：外部电脑连接到 FRPS 的映射端口，即可访问内网的 RDP 服务

### 1.3 RDP 文件

RDP (Remote Desktop Protocol) 文件是 Windows 远程桌面的配置文件：

```ini
full address:s:8.163.37.74:20123
username:s:Administrator
screen mode id:i:2
```

**关键参数：**
- `full address`: FRPS服务器地址:映射端口
- `username`: Windows 用户名
- `screen mode id`: 1=窗口模式, 2=全屏模式

### 1.4 数据同步

用户数据存储结构：
```
server/data/
├── admin.db          # 主数据库（用户账号）
├── user_1.db         # 用户1的独立数据库
│   └── user_configs  # 包含 FRPC 配置
├── user_2.db         # 用户2的独立数据库
└── ...
```

**同步内容：**
- FRPC 配置（服务器地址、端口、设备名）
- 远程桌面连接列表
- 应用收藏等

---

## 二、部署说明

### 2.1 服务器端部署

#### 2.1.1 环境要求

- Linux 服务器（测试用 Ubuntu/CentOS）
- 公网 IP 或域名
- 开放端口：7000, 20000-50000

#### 2.1.2 部署步骤

**步骤1：下载 FRP**

```bash
cd /opt
wget https://github.com/fatedier/frp/releases/download/v0.51.3/frp_0.51.3_linux_amd64.tar.gz
tar -xzf frp_0.51.3_linux_amd64.tar.gz
cd frp_0.51.3_linux_amd64
```

**步骤2：配置 FRPS**

创建 `frps.ini`：

```ini
[common]
bind_port = 7000           # FRPC 连接端口
token = ponywork2024      # 认证令牌
allow_ports = 20000-50000 # 允许分配的端口范围
```

**步骤3：启动 FRPS**

```bash
# 前台运行测试
./frps -c frps.ini

# 后台运行
nohup ./frps -c frps.ini > frps.log 2>&1 &

# 设置开机自启
chmod +x /etc/init.d/frps
```

**步骤4：开放防火墙端口**

```bash
# Ubuntu
sudo ufw allow 7000/tcp
sudo ufw allow 20000:50000/tcp

# CentOS
sudo firewall-cmd --permanent --add-port=7000/tcp
sudo firewall-cmd --permanent --add-port=20000-50000/tcp
sudo firewall-cmd --reload
```

#### 2.1.3 阿里云安全组配置

在阿里云控制台添加安全组规则：
| 协议 | 端口 | 源 |
|------|------|-----|
| TCP | 7000 | 0.0.0.0/0 |
| TCP | 20000-50000 | 0.0.0.0/0 |

### 2.2 客户端配置

#### 2.2.1 FRPC 二进制文件

将 `frpc.exe` 放在程序同一目录下：

```
PonyWork/
├── PonyWork.exe
└── frpc.exe
```

#### 2.2.2 配置文件

客户端自动生成 `frpc.ini`：

```ini
[common]
server_addr = 8.163.37.74
server_port = 7000
protocol = tcp
token = ponywork2024

[rdp_DESKTOP-XXXX]
type = tcp
local_ip = 127.0.0.1
local_port = 3389
remote_port = 20123    # 固定端口
```

### 2.3 使用流程

#### 2.3.1 发起端（本机）

1. 打开 PonyWork 应用
2. 进入"远程桌面"页面
3. 在 FRPC 中继区域，点击"一键设置"或"开启"
4. FRPC 启动成功后，显示分配的端口（如 20123）
5. 点击"添加到列表"，将本机添加到远程桌面列表

#### 2.3.2 控制端（其他电脑）

1. 在其他电脑登录同一账号
2. 同步后会看到刚才添加的远程桌面
3. 双击连接，输入 Windows 密码即可远程控制

---

## 三、关键代码说明

### 3.1 端口分配算法

```cpp
// 结合 userId + deviceName 生成唯一端口
QString combined = QString::number(userId) + "_" + deviceName;
int hash = qHash(combined) % 30000;
int fixedPort = 20000 + qAbs(hash);
```

**特点：**
- 同一用户不同设备：端口不同
- 不同用户同一设备：端口不同
- 端口固定，重启不变

### 3.2 RDP 文件生成

```cpp
// 生成 RDP 文件内容
out << "full address:s:" << serverAddr << ":" << remotePort << "\n";
out << "username:s:" << username << "\n";
out << "screen mode id:i:" << (fullScreen ? 2 : 1) << "\n";
```

### 3.3 FRPC 进程管理

使用 Qt QProcess 管理 FRPC 进程：
- 启动：`m_process->start()`
- 停止：`m_process->terminate()` / `m_process->kill()`
- 状态监听：通过信号槽处理启动、错误、退出事件

---

## 四、常见问题

### 4.1 连接失败

**检查项：**
1. FRPS 服务器是否运行：`netstat -tlnp | grep 7000`
2. 防火墙是否开放端口
3. 阿里云安全组是否配置

### 4.2 端口冲突

如果多台设备分配到相同端口：
- 检查设备名是否相同
- 确保不同设备使用不同电脑名

### 4.3 RDP 连接不上

**排查步骤：**
1. 确认本机远程桌面已开启
2. 检查防火墙是否阻止 3389 端口
3. 确认用户名密码正确

---

## 五、API 参考

### 5.1 后端管理接口

**获取所有用户端口**
```
GET /api/admin/frpc/ports
```

响应：
```json
{
  "success": true,
  "ports": [
    {
      "user_id": 1,
      "username": "testuser",
      "remote_port": 20123,
      "device_name": "DESKTOP-A",
      "is_enabled": true
    }
  ]
}
```

### 5.2 用户配置接口

**保存 FRPC 配置**
```
POST /api/config/frpc/save
Body: { "frpc": { "remotePort": 20123, "deviceName": "DESKTOP-A" } }
```

**获取 FRPC 配置**
```
GET /api/config/frpc/get
```

---

## 六、总结

本系统通过 FRPC 实现内网穿透，使外部电脑可以通过 RDP 协议远程控制内网 Windows 电脑。核心优势：

1. **无需公网 IP**：利用 FRPS 中转，无需用户拥有公网 IP
2. **自动端口分配**：基于用户ID和设备名自动计算端口
3. **账号同步**：配置随账号云端同步，多设备管理方便
4. **一键配置**：UI 简化操作，用户体验友好
