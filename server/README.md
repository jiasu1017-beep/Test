# PonyWork Admin Server 后端管理部署文档

## 📦 核心文件

```
server/
├── admin-server.js      # 后端服务主程序
├── admin-panel/         # 管理后台前端
│   └── index.html
├── package.json         # Node.js 依赖配置
└── README.md           # 本文档
```

---

## 🚀 服务器部署步骤

### 1. SSH 连接服务器
```bash
ssh root@8.163.37.74
# 密码：JIAsu6258586
```

### 2. 创建目录并上传文件
```bash
mkdir -p /var/www/ponywork-admin
cd /var/www/ponywork-admin
mkdir -p admin-panel data
```

在本地 PowerShell 执行上传：
```powershell
# 从 server 目录上传
scp F:\00AI\Test\server\admin-server.js root@8.163.37.74:/var/www/ponywork-admin/
scp F:\00AI\Test\server\package.json root@8.163.37.74:/var/www/ponywork-admin/
scp -r F:\00AI\Test\server\admin-panel root@8.163.37.74:/var/www/ponywork-admin/
```

### 3. 安装依赖并启动
```bash
cd /var/www/ponywork-admin
npm install
nohup node admin-server.js > /var/log/ponywork-admin.log 2>&1 &
```

### 4. 验证服务
```bash
# 检查进程
ps aux | grep admin-server.js

# 检查端口
netstat -tlnp | grep 8080

# 查看日志
tail -f /var/log/ponywork-admin.log
```

---

## 🌐 访问管理后台

**URL**: `http://8.163.37.74:8080/admin`

**默认账号**:
- 用户名：`admin`
- 密码：`admin123`

---

## 📊 API 接口

### 用户认证
- `POST /api/admin/login` - 管理员登录
- `POST /api/admin/validate-token` - 验证 Token

### 用户管理
- `GET /api/admin/users` - 获取用户列表
- `PUT /api/admin/users/:id` - 更新用户
- `DELETE /api/admin/users/:id` - 删除用户

### 日志管理
- `GET /api/admin/logs?type=login|operation` - 获取日志

### 应用集合
- `GET /api/admin/collections` - 获取应用集合
- `PUT /api/admin/collections/:id` - 更新集合
- `DELETE /api/admin/collections/:id` - 删除集合

### 系统配置
- `GET /api/admin/config` - 获取配置
- `PUT /api/admin/config/:key` - 更新配置

### 统计信息
- `GET /api/admin/stats` - 获取系统统计

### 客户端集成
- `POST /api/admin/sync-user` - 用户注册/登录

---

## 🔧 常用命令

### 重启服务
```bash
pkill -f admin-server.js
cd /var/www/ponywork-admin
nohup node admin-server.js > /var/log/ponywork-admin.log 2>&1 &
```

### 查看日志
```bash
tail -f /var/log/ponywork-admin.log
```

### 停止服务
```bash
pkill -f admin-server.js
```

---

## 📝 数据库结构

SQLite 数据库位于：`/var/www/ponywork-admin/data/admin.db`

**主要数据表**:
- `users` - 用户信息
- `admin_users` - 管理员信息
- `user_sessions` - 用户会话
- `login_logs` - 登录日志
- `operation_logs` - 操作日志
- `app_collections` - 应用集合
- `system_config` - 系统配置

---

## 🔐 安全建议

1. **修改默认密码**: 登录后立即修改 admin 账号密码
2. **配置防火墙**: 仅开放必要的端口（8080）
3. **定期备份**: 备份 `/var/www/ponywork-admin/data/admin.db`
4. **使用 HTTPS**: 生产环境建议配置 Nginx 反向代理 + SSL

---

## 🆘 故障排查

### 无法访问 8080 端口
```bash
# 检查防火墙
ufw status
ufw allow 8080/tcp

# 检查安全组（阿里云）
# 在阿里云控制台开放 8080 端口
```

### 服务未启动
```bash
# 查看日志
tail -f /var/log/ponywork-admin.log

# 手动启动
cd /var/www/ponywork-admin
node admin-server.js
```

### Node.js 未安装
```bash
curl -fsSL https://deb.nodesource.com/setup_18.x | bash -
apt-get install -y nodejs
```

---

## 📞 技术支持

如有问题，请提供：
1. 错误日志：`tail -f /var/log/ponywork-admin.log`
2. 服务状态：`ps aux | grep admin-server.js`
3. 端口监听：`netstat -tlnp | grep 8080`
