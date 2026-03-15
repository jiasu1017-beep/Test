const express = require('express');
const cors = require('cors');
const bodyParser = require('body-parser');
const sqlite3 = require('sqlite3').verbose();
const path = require('path');
const fs = require('fs');

const app = express();
const PORT = process.env.PORT || 8080;

app.use(cors());
app.use(bodyParser.json());
app.use(bodyParser.urlencoded({ extended: true }));

const DB_PATH = process.env.DB_PATH || path.join(__dirname, 'data', 'admin.db');
const dataDir = path.dirname(DB_PATH);
if (!fs.existsSync(dataDir)) {
    fs.mkdirSync(dataDir, { recursive: true });
}

const db = new sqlite3.Database(DB_PATH);

db.serialize(() => {
    db.run(`CREATE TABLE IF NOT EXISTS users (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        username TEXT UNIQUE NOT NULL,
        email TEXT UNIQUE NOT NULL,
        password TEXT NOT NULL,
        role TEXT DEFAULT 'user',
        created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
        last_login DATETIME,
        status TEXT DEFAULT 'active'
    )`);

    db.run(`CREATE TABLE IF NOT EXISTS user_sessions (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        user_id INTEGER,
        token TEXT UNIQUE,
        ip_address TEXT,
        user_agent TEXT,
        created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
        expires_at DATETIME,
        FOREIGN KEY (user_id) REFERENCES users(id)
    )`);

    db.run(`CREATE TABLE IF NOT EXISTS login_logs (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        user_id INTEGER,
        username TEXT,
        ip_address TEXT,
        user_agent TEXT,
        action TEXT,
        status TEXT,
        created_at DATETIME DEFAULT CURRENT_TIMESTAMP
    )`);

    db.run(`CREATE TABLE IF NOT EXISTS operation_logs (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        user_id INTEGER,
        username TEXT,
        action TEXT,
        details TEXT,
        ip_address TEXT,
        created_at DATETIME DEFAULT CURRENT_TIMESTAMP
    )`);

    db.run(`CREATE TABLE IF NOT EXISTS app_collections (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        name TEXT NOT NULL,
        description TEXT,
        apps TEXT,
        created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
        updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,
        created_by INTEGER,
        status TEXT DEFAULT 'active'
    )`);

    db.run(`CREATE TABLE IF NOT EXISTS system_config (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        key TEXT UNIQUE NOT NULL,
        value TEXT,
        updated_at DATETIME DEFAULT CURRENT_TIMESTAMP
    )`);
    
    // 用户配置表
    db.run(`CREATE TABLE IF NOT EXISTS user_configs (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        user_id INTEGER NOT NULL,
        key TEXT NOT NULL,
        value TEXT,
        updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,
        UNIQUE(user_id, key),
        FOREIGN KEY (user_id) REFERENCES users(id)
    )`);

    db.run(`CREATE TABLE IF NOT EXISTS admin_users (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        username TEXT UNIQUE NOT NULL,
        password TEXT NOT NULL,
        role TEXT DEFAULT 'admin',
        created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
        last_login DATETIME
    )`);

    const stmt = db.prepare("SELECT * FROM admin_users WHERE username = 'admin'");
    stmt.get((err, row) => {
        if (!row) {
            const bcrypt = require('bcryptjs');
            const hashedPassword = bcrypt.hashSync('admin123', 10);
            db.run("INSERT INTO admin_users (username, password, role) VALUES (?, ?, ?)", 
                ['admin', hashedPassword, 'super_admin']);
            console.log('Default admin user created: admin / admin123');
        }
    });
    stmt.finalize();
});

app.use('/admin', express.static(path.join(__dirname, 'admin-panel')));

app.post('/api/admin/login', (req, res) => {
    const { username, password } = req.body;
    
    if (!username || !password) {
        return res.status(400).json({ success: false, message: '用户名和密码不能为空' });
    }

    const bcrypt = require('bcryptjs');
    db.get("SELECT * FROM admin_users WHERE username = ?", [username], (err, admin) => {
        if (err || !admin) {
            return res.status(401).json({ success: false, message: '用户名或密码错误' });
        }

        if (!bcrypt.compareSync(password, admin.password)) {
            return res.status(401).json({ success: false, message: '用户名或密码错误' });
        }

        const token = require('crypto').randomBytes(32).toString('hex');
        const expiresAt = new Date(Date.now() + 24 * 60 * 60 * 1000).toISOString();
        
        db.run("INSERT INTO user_sessions (user_id, token, ip_address, user_agent, expires_at) VALUES (?, ?, ?, ?, ?)",
            [admin.id, token, req.ip, req.headers['user-agent'], expiresAt]);
        
        db.run("UPDATE admin_users SET last_login = CURRENT_TIMESTAMP WHERE id = ?", [admin.id]);

        res.json({
            success: true,
            message: '登录成功',
            token: token,
            user: {
                id: admin.id,
                username: admin.username,
                role: admin.role
            }
        });
    });
});

const authenticateAdmin = (req, res, next) => {
    const token = req.headers['authorization']?.replace('Bearer ', '');
    
    if (!token) {
        return res.status(401).json({ success: false, message: '未授权访问' });
    }

    db.get("SELECT * FROM user_sessions WHERE token = ? AND expires_at > datetime('now')", [token], (err, session) => {
        if (err || !session) {
            return res.status(401).json({ success: false, message: '登录已过期，请重新登录' });
        }
        req.adminId = session.user_id;
        next();
    });
};

// 验证普通用户 token
const authenticateToken = (req, res, next) => {
    const token = req.headers['authorization']?.replace('Bearer ', '');
    
    if (!token) {
        return res.status(401).json({ success: false, message: '未授权访问' });
    }

    db.get("SELECT user_id FROM user_sessions WHERE token = ? AND expires_at > datetime('now')", [token], (err, session) => {
        if (err || !session) {
            return res.status(401).json({ success: false, message: '登录已过期，请重新登录' });
        }
        req.userId = session.user_id;
        next();
    });
};

app.get('/api/admin/stats', authenticateAdmin, (req, res) => {
    const stats = {};
    
    db.get("SELECT COUNT(*) as count FROM users", (err, row) => {
        stats.totalUsers = row?.count || 0;
        
        db.get("SELECT COUNT(*) as count FROM users WHERE status = 'active'", (err, row) => {
            stats.activeUsers = row?.count || 0;
            
            db.get("SELECT COUNT(*) as count FROM login_logs WHERE created_at > datetime('now', '-24 hours')", (err, row) => {
                stats.logins24h = row?.count || 0;
                
                // 修复：统计有效的在线用户（未过期的 session）
                db.get("SELECT COUNT(DISTINCT user_id) as count FROM user_sessions WHERE expires_at > datetime('now')", (err, row) => {
                    stats.onlineUsers = row?.count || 0;
                    
                    db.get("SELECT COUNT(*) as count FROM app_collections", (err, row) => {
                        stats.totalCollections = row?.count || 0;
                        
                        res.json({ success: true, stats });
                    });
                });
            });
        });
    });
});

app.get('/api/admin/users', authenticateAdmin, (req, res) => {
    const { page = 1, limit = 20, search = '', status = '' } = req.query;
    const offset = (page - 1) * limit;
    
    let whereClause = '1=1';
    const params = [];
    
    if (search) {
        whereClause += ' AND (username LIKE ? OR email LIKE ?)';
        params.push(`%${search}%`, `%${search}%`);
    }
    
    if (status) {
        whereClause += ' AND status = ?';
        params.push(status);
    }

    db.get(`SELECT COUNT(*) as total FROM users WHERE ${whereClause}`, params, (err, row) => {
        const total = row?.total || 0;
        
        db.all(`SELECT id, username, email, role, created_at, last_login, status 
                FROM users WHERE ${whereClause} 
                ORDER BY created_at DESC LIMIT ? OFFSET ?`, 
            [...params, parseInt(limit), parseInt(offset)], (err, users) => {
            if (err) {
                return res.status(500).json({ success: false, message: '获取用户列表失败' });
            }
            
            res.json({
                success: true,
                users: users,
                pagination: {
                    page: parseInt(page),
                    limit: parseInt(limit),
                    total: total,
                    pages: Math.ceil(total / limit)
                }
            });
        });
    });
});

app.put('/api/admin/users/:id', authenticateAdmin, (req, res) => {
    const { id } = req.params;
    const { email, role, status } = req.body;
    
    const updates = [];
    const params = [];
    
    if (email !== undefined) {
        updates.push('email = ?');
        params.push(email);
    }
    if (role !== undefined) {
        updates.push('role = ?');
        params.push(role);
    }
    if (status !== undefined) {
        updates.push('status = ?');
        params.push(status);
    }
    
    if (updates.length === 0) {
        return res.status(400).json({ success: false, message: '没有要更新的字段' });
    }
    
    params.push(id);
    
    db.run(`UPDATE users SET ${updates.join(', ')} WHERE id = ?`, params, function(err) {
        if (err) {
            return res.status(500).json({ success: false, message: '更新用户失败: ' + err.message });
        }
        
        db.run("INSERT INTO operation_logs (user_id, action, details, ip_address) VALUES (?, ?, ?, ?)",
            [req.adminId, 'update_user', `更新用户 ID: ${id}`, req.ip]);
        
        res.json({ success: true, message: '用户更新成功' });
    });
});

app.delete('/api/admin/users/:id', authenticateAdmin, (req, res) => {
    const { id } = req.params;
    
    db.run("DELETE FROM users WHERE id = ?", [id], function(err) {
        if (err) {
            return res.status(500).json({ success: false, message: '删除用户失败' });
        }
        
        db.run("DELETE FROM user_sessions WHERE user_id = ?", [id]);
        
        db.run("INSERT INTO operation_logs (user_id, action, details, ip_address) VALUES (?, ?, ?, ?)",
            [req.adminId, 'delete_user', `删除用户 ID: ${id}`, req.ip]);
        
        res.json({ success: true, message: '用户删除成功' });
    });
});

// 重置用户密码
app.post('/api/admin/users/:id/reset-password', authenticateAdmin, (req, res) => {
    const { id } = req.params;
    const newPassword = '666666'; // 默认重置密码
    
    console.log(`\n[重置密码] 管理员 ID: ${req.adminId}, 用户 ID: ${id}`);
    
    // 使用 SHA-256 哈希密码
    const crypto = require('crypto');
    const hashedPassword = crypto.createHash('sha256').update(newPassword).digest('hex');
    
    db.run("UPDATE users SET password = ? WHERE id = ?", [hashedPassword, id], function(err) {
        if (err) {
            console.log(`  - 失败：${err.message}`);
            return res.status(500).json({ success: false, message: '重置密码失败：' + err.message });
        }
        
        if (this.changes === 0) {
            return res.status(404).json({ success: false, message: '用户不存在' });
        }
        
        // 使所有 session 失效（强制重新登录）
        db.run("DELETE FROM user_sessions WHERE user_id = ?", [id]);
        
        // 记录操作日志
        db.run("INSERT INTO operation_logs (user_id, action, details, ip_address) VALUES (?, ?, ?, ?)",
            [req.adminId, 'reset_password', `重置用户 ID: ${id} 的密码`, req.ip]);
        
        // 记录密码重置日志
        db.run("INSERT INTO login_logs (user_id, action, status, ip_address) VALUES (?, ?, ?, ?)",
            [parseInt(id), 'password_reset', 'admin_reset', req.ip]);
        
        console.log(`  - 成功：用户 ID: ${id} 密码已重置为 ${newPassword}`);
        
        res.json({ 
            success: true, 
            message: `密码已重置为 ${newPassword}`,
            newPassword: newPassword
        });
    });
});

app.get('/api/admin/logs', authenticateAdmin, (req, res) => {
    const { page = 1, limit = 50, type = 'login', startDate = '', endDate = '' } = req.query;
    const offset = (page - 1) * limit;
    
    let table = type === 'login' ? 'login_logs' : 'operation_logs';
    let whereClause = '1=1';
    const params = [];
    
    if (startDate) {
        whereClause += ' AND created_at >= ?';
        params.push(startDate);
    }
    if (endDate) {
        whereClause += ' AND created_at <= ?';
        params.push(endDate);
    }

    db.get(`SELECT COUNT(*) as total FROM ${table} WHERE ${whereClause}`, params, (err, row) => {
        const total = row?.total || 0;
        
        db.all(`SELECT * FROM ${table} WHERE ${whereClause} 
                ORDER BY created_at DESC LIMIT ? OFFSET ?`, 
            [...params, parseInt(limit), parseInt(offset)], (err, logs) => {
            if (err) {
                return res.status(500).json({ success: false, message: '获取日志失败' });
            }
            
            res.json({
                success: true,
                logs: logs,
                pagination: {
                    page: parseInt(page),
                    limit: parseInt(limit),
                    total: total,
                    pages: Math.ceil(total / limit)
                }
            });
        });
    });
});

app.get('/api/admin/collections', authenticateAdmin, (req, res) => {
    const { page = 1, limit = 20, search = '' } = req.query;
    const offset = (page - 1) * limit;
    
    let whereClause = '1=1';
    const params = [];
    
    if (search) {
        whereClause += ' AND (name LIKE ? OR description LIKE ?)';
        params.push(`%${search}%`, `%${search}%`);
    }

    db.get(`SELECT COUNT(*) as total FROM app_collections WHERE ${whereClause}`, params, (err, row) => {
        const total = row?.total || 0;
        
        db.all(`SELECT * FROM app_collections WHERE ${whereClause} 
                ORDER BY created_at DESC LIMIT ? OFFSET ?`, 
            [...params, parseInt(limit), parseInt(offset)], (err, collections) => {
            if (err) {
                return res.status(500).json({ success: false, message: '获取应用集合列表失败' });
            }
            
            res.json({
                success: true,
                collections: collections,
                pagination: {
                    page: parseInt(page),
                    limit: parseInt(limit),
                    total: total,
                    pages: Math.ceil(total / limit)
                }
            });
        });
    });
});

app.put('/api/admin/collections/:id', authenticateAdmin, (req, res) => {
    const { id } = req.params;
    const { name, description, apps, status } = req.body;
    
    const updates = ['updated_at = CURRENT_TIMESTAMP'];
    const params = [];
    
    if (name !== undefined) {
        updates.push('name = ?');
        params.push(name);
    }
    if (description !== undefined) {
        updates.push('description = ?');
        params.push(description);
    }
    if (apps !== undefined) {
        updates.push('apps = ?');
        params.push(typeof apps === 'string' ? apps : JSON.stringify(apps));
    }
    if (status !== undefined) {
        updates.push('status = ?');
        params.push(status);
    }
    
    params.push(id);
    
    db.run(`UPDATE app_collections SET ${updates.join(', ')} WHERE id = ?`, params, function(err) {
        if (err) {
            return res.status(500).json({ success: false, message: '更新应用集合失败' });
        }
        
        db.run("INSERT INTO operation_logs (user_id, action, details, ip_address) VALUES (?, ?, ?, ?)",
            [req.adminId, 'update_collection', `更新应用集合 ID: ${id}`, req.ip]);
        
        res.json({ success: true, message: '应用集合更新成功' });
    });
});

app.delete('/api/admin/collections/:id', authenticateAdmin, (req, res) => {
    const { id } = req.params;
    
    db.run("DELETE FROM app_collections WHERE id = ?", [id], function(err) {
        if (err) {
            return res.status(500).json({ success: false, message: '删除应用集合失败' });
        }
        
        db.run("INSERT INTO operation_logs (user_id, action, details, ip_address) VALUES (?, ?, ?, ?)",
            [req.adminId, 'delete_collection', `删除应用集合 ID: ${id}`, req.ip]);
        
        res.json({ success: true, message: '应用集合删除成功' });
    });
});

app.get('/api/admin/config', authenticateAdmin, (req, res) => {
    db.all("SELECT * FROM system_config", (err, configs) => {
        if (err) {
            return res.status(500).json({ success: false, message: '获取配置失败' });
        }
        res.json({ success: true, configs });
    });
});

// 用户获取自己的配置
app.get('/api/config/get', authenticateToken, (req, res) => {
    db.get("SELECT id FROM users WHERE id = ?", [req.userId], (err, user) => {
        if (err || !user) {
            return res.status(401).json({ success: false, error: '用户未授权' });
        }
        
        // 从 user_configs 表获取用户配置
        db.all("SELECT * FROM user_configs WHERE user_id = ?", [req.userId], (err, configs) => {
            if (err) {
                return res.status(500).json({ success: false, error: '获取配置失败' });
            }
            
            // 转换为对象格式
            const configObj = {};
            configs.forEach(config => {
                try {
                    configObj[config.key] = JSON.parse(config.value);
                } catch (e) {
                    configObj[config.key] = config.value;
                }
            });
            
            res.json({ success: true, configs: configObj });
        });
    });
});

// 用户保存配置
app.post('/api/config/save', authenticateToken, (req, res) => {
    const { configs } = req.body;
    
    if (!configs) {
        return res.status(400).json({ success: false, error: '配置数据不能为空' });
    }
    
    db.serialize(() => {
        for (const [key, value] of Object.entries(configs)) {
            const valueStr = typeof value === 'object' ? JSON.stringify(value) : String(value);
            db.run(`INSERT OR REPLACE INTO user_configs (user_id, key, value, updated_at) VALUES (?, ?, ?, CURRENT_TIMESTAMP)`,
                [req.userId, key, valueStr]);
        }
        
        res.json({ success: true, message: '配置保存成功' });
    });
});

app.put('/api/admin/config/:key', authenticateAdmin, (req, res) => {
    const { key } = req.params;
    const { value } = req.body;
    
    db.run(`INSERT OR REPLACE INTO system_config (key, value, updated_at) VALUES (?, ?, CURRENT_TIMESTAMP)`,
        [key, value], function(err) {
        if (err) {
            return res.status(500).json({ success: false, message: '更新配置失败' });
        }
        
        db.run("INSERT INTO operation_logs (user_id, action, details, ip_address) VALUES (?, ?, ?, ?)",
            [req.adminId, 'update_config', `更新配置: ${key}`, req.ip]);
        
        res.json({ success: true, message: '配置更新成功' });
    });
});

app.post('/api/admin/sync-user', (req, res) => {
    const { username, email, password, action } = req.body;
    
    if (!username || !email) {
        return res.status(400).json({ success: false, message: '用户名和邮箱必填' });
    }
    
    if (action === 'register') {
        const bcrypt = require('bcryptjs');
        const hashedPassword = bcrypt.hashSync(password || 'default123', 10);
        
        db.run("INSERT INTO users (username, email, password) VALUES (?, ?, ?)",
            [username, email, hashedPassword], function(err) {
            if (err) {
                return res.status(500).json({ success: false, message: '注册失败: ' + err.message });
            }
            
            db.run("INSERT INTO login_logs (username, action, status, ip_address) VALUES (?, ?, ?, ?)",
                [username, 'register', 'success', req.ip]);
            
            res.json({ success: true, message: '用户注册成功', userId: this.lastID });
        });
    } else if (action === 'login') {
        db.get("SELECT * FROM users WHERE username = ? OR email = ?", [username, username], (err, user) => {
            if (err || !user) {
                db.run("INSERT INTO login_logs (username, action, status, ip_address) VALUES (?, ?, ?, ?)",
                    [username, 'login', 'failed_not_found', req.ip]);
                return res.status(401).json({ success: false, message: '用户不存在' });
            }
            
            const bcrypt = require('bcryptjs');
            if (!bcrypt.compareSync(password, user.password)) {
                db.run("INSERT INTO login_logs (user_id, username, action, status, ip_address) VALUES (?, ?, ?, ?, ?)",
                    [user.id, username, 'login', 'failed_wrong_password', req.ip]);
                return res.status(401).json({ success: false, message: '密码错误' });
            }
            
            const token = require('crypto').randomBytes(32).toString('hex');
            const expiresAt = new Date(Date.now() + 7 * 24 * 60 * 60 * 1000).toISOString();
            
            db.run("INSERT INTO user_sessions (user_id, token, ip_address, user_agent, expires_at) VALUES (?, ?, ?, ?, ?)",
                [user.id, token, req.ip, req.headers['user-agent'], expiresAt]);
            
            db.run("UPDATE users SET last_login = CURRENT_TIMESTAMP WHERE id = ?", [user.id]);
            
            db.run("INSERT INTO login_logs (user_id, username, action, status, ip_address) VALUES (?, ?, ?, ?, ?)",
                [user.id, username, 'login', 'success', req.ip]);
            
            res.json({
                success: true,
                message: '登录成功',
                token: token,
                user: {
                    id: user.id,
                    username: user.username,
                    email: user.email,
                    role: user.role
                }
            });
        });
    } else {
        res.status(400).json({ success: false, message: '无效的操作' });
    }
});

app.post('/api/admin/validate-token', (req, res) => {
    const { token } = req.body;
    
    if (!token) {
        return res.status(400).json({ success: false, message: 'Token required' });
    }
    
    db.get("SELECT u.* FROM users u JOIN user_sessions s ON u.id = s.user_id WHERE s.token = ? AND s.expires_at > datetime('now')", 
        [token], (err, user) => {
        if (err || !user) {
            return res.status(401).json({ success: false, message: 'Invalid or expired token' });
        }
        
        res.json({
            success: true,
            user: {
                id: user.id,
                username: user.username,
                email: user.email,
                role: user.role
            }
        });
    });
});

// ============================================
// 客户端用户系统 API（新增）
// ============================================

// 用户注册
app.post('/api/auth/register', (req, res) => {
    const { username, email, password } = req.body;
    
    console.log(`\n[注册请求] ${new Date().toISOString()}`);
    console.log(`  - Username: ${username}`);
    console.log(`  - Email: ${email}`);
    console.log(`  - IP: ${req.ip}`);
    
    if (!username || !email || !password) {
        console.log(`  - 失败：缺少必要参数`);
        return res.status(400).json({ success: false, error: '用户名、邮箱和密码必填' });
    }
    
    if (password.length < 6) {
        console.log(`  - 失败：密码长度不足`);
        return res.status(400).json({ success: false, error: '密码长度至少 6 位' });
    }
    
    // 修复：使用 SHA-256 哈希密码（与客户端一致）
    const crypto = require('crypto');
    const hashedPassword = crypto.createHash('sha256').update(password).digest('hex');
    
    db.run("INSERT INTO users (username, email, password, role) VALUES (?, ?, ?, 'user')",
        [username, email, hashedPassword], function(err) {
        if (err) {
            console.log(`  - 失败：${err.message}`);
            if (err.message.includes('username')) {
                return res.status(400).json({ success: false, error: '用户名已存在' });
            }
            if (err.message.includes('email')) {
                return res.status(400).json({ success: false, error: '邮箱已被注册' });
            }
            return res.status(500).json({ success: false, error: '注册失败：' + err.message });
        }
        
        const userId = this.lastID;
        console.log(`  - 成功：用户 ID=${userId}`);
        
        db.run("INSERT INTO login_logs (username, action, status, ip_address) VALUES (?, ?, ?, ?)",
            [username, 'register', 'success', req.ip]);
        
        res.json({ success: true, userId: userId, message: '注册成功' });
    });
});

// 用户登录
app.post('/api/auth/login', (req, res) => {
    const { email, username, password } = req.body;
    
    console.log(`\n[登录请求] ${new Date().toISOString()}`);
    console.log(`  - Email: ${email || 'N/A'}`);
    console.log(`  - Username: ${username || 'N/A'}`);
    console.log(`  - IP: ${req.ip}`);
    
    if ((!email && !username) || !password) {
        console.log(`  - 失败：缺少必要参数`);
        return res.status(400).json({ success: false, error: '邮箱/用户名和密码必填' });
    }
    
    const identifier = email || username;
    const isEmail = email ? true : false;
    
    const query = isEmail ? "SELECT * FROM users WHERE email = ?" : "SELECT * FROM users WHERE username = ?";
    
    db.get(query, [identifier], (err, user) => {
        if (err || !user) {
            console.log(`  - 失败：用户不存在`);
            db.run("INSERT INTO login_logs (username, action, status, ip_address) VALUES (?, ?, ?, ?)",
                [identifier, 'login', 'failed_not_found', req.ip]);
            return res.status(401).json({ success: false, error: '邮箱/用户名或密码错误' });
        }
        
        // 修复：支持 SHA-256 和 bcrypt 两种密码格式
        const crypto = require('crypto');
        const sha256Hash = crypto.createHash('sha256').update(password).digest('hex');
        
        console.log(`  - 密码验证调试:`);
        console.log(`    收到密码长度：${password.length}`);
        console.log(`    收到密码前 20 位：${password.substring(0, 20)}...`);
        console.log(`    数据库密码长度：${user.password.length}`);
        console.log(`    数据库密码前 20 位：${user.password.substring(0, 20)}...`);
        console.log(`    SHA256 哈希：${sha256Hash}`);
        
        let passwordValid = false;
        
        // 检查数据库密码格式并验证
        if (user.password.length === 64 && /^[a-f0-9]{64}$/i.test(user.password)) {
            // 数据库密码是 SHA-256 格式，直接比较
            console.log(`    检测到数据库密码为 SHA-256 格式`);
            passwordValid = (user.password === sha256Hash);
            console.log(`    SHA-256 验证结果：${passwordValid ? '成功' : '失败'}`);
        } else if (user.password.startsWith('$2a$') || user.password.startsWith('$2b$')) {
            // 数据库密码是 bcrypt 格式
            console.log(`    检测到数据库密码为 bcrypt 格式`);
            
            // 尝试 1: 直接用收到的密码（可能是 SHA-256）进行 bcrypt 验证
            const bcrypt = require('bcryptjs');
            try {
                passwordValid = bcrypt.compareSync(password, user.password);
                if (passwordValid) {
                    console.log(`    bcrypt 验证（使用收到的密码）结果：成功`);
                } else {
                    console.log(`    bcrypt 验证（使用收到的密码）结果：失败`);
                    // 尝试 2: 如果失败，说明收到的是 SHA-256，需要用明文验证
                    // 但客户端已经发送哈希，所以这种情况无法处理
                    console.log(`    提示：客户端发送的是 SHA-256 哈希，但数据库需要 bcrypt 验证`);
                    console.log(`    建议：重新注册该用户或使用明文密码登录`);
                }
            } catch (e) {
                console.log(`    bcrypt 验证异常：${e.message}`);
                passwordValid = false;
            }
        } else {
            console.log(`    未知密码格式，尝试验证`);
            // 其他格式，直接比较
            passwordValid = (user.password === password || user.password === sha256Hash);
        }
        
        if (!passwordValid) {
            console.log(`  - 失败：密码错误`);
            db.run("INSERT INTO login_logs (user_id, username, action, status, ip_address) VALUES (?, ?, ?, ?, ?)",
                [user.id, user.username, 'login', 'failed_wrong_password', req.ip]);
            return res.status(401).json({ success: false, error: '邮箱/用户名或密码错误' });
        }
        
        console.log(`  - 密码验证通过`);
        
        const token = require('crypto').randomBytes(32).toString('hex');
        const expiresAt = new Date(Date.now() + 7 * 24 * 60 * 60 * 1000).toISOString();
        
        db.run("INSERT INTO user_sessions (user_id, token, ip_address, expires_at) VALUES (?, ?, ?, ?)",
            [user.id, token, req.ip, expiresAt]);
        
        // 修复：使用 ISO 格式记录登录时间
        const loginTime = new Date().toISOString();
        db.run("UPDATE users SET last_login = ? WHERE id = ?", [loginTime, user.id]);
        
        db.run("INSERT INTO login_logs (user_id, username, action, status, ip_address) VALUES (?, ?, ?, ?, ?)",
            [user.id, user.username, 'login', 'success', req.ip]);
        
        console.log(`  - 成功：用户 ID=${user.id}, Token=${token.substring(0, 20)}...`);
        
        res.json({
            success: true,
            token: token,
            user: {
                id: user.id,
                email: user.email,
                vipLevel: 0
            }
        });
    });
});

// 检查邮箱是否存在
app.get('/api/auth/check-email', (req, res) => {
    const email = req.query.email;
    
    if (!email) {
        return res.status(400).json({ success: false, error: '邮箱必填' });
    }
    
    db.get("SELECT id FROM users WHERE email = ?", [email], (err, user) => {
        if (err) {
            return res.status(500).json({ success: false, error: '查询失败' });
        }
        
        res.json({ exists: !!user });
    });
});

// 获取用户资料
app.get('/api/auth/profile', (req, res) => {
    const token = req.headers['authorization']?.replace('Bearer ', '');
    
    if (!token) {
        return res.status(401).json({ success: false, error: '未授权访问' });
    }
    
    db.get("SELECT u.* FROM users u JOIN user_sessions s ON u.id = s.user_id WHERE s.token = ? AND s.expires_at > datetime('now')", 
        [token], (err, user) => {
        if (err || !user) {
            return res.status(401).json({ success: false, error: '登录已过期' });
        }
        
        res.json({
            success: true,
            user: {
                id: user.id,
                email: user.email,
                username: user.username,
                vip_level: 0,
                created_at: user.created_at,
                last_login: user.last_login
            }
        });
    });
});

app.get('/admin/*', (req, res) => {
    res.sendFile(path.join(__dirname, 'admin-panel', 'index.html'));
});

app.listen(PORT, '0.0.0.0', () => {
    console.log(`====================================`);
    console.log(`   Admin Server Running`);
    console.log(`   Port: ${PORT}`);
    console.log(`   Admin Panel: http://localhost:${PORT}/admin`);
    console.log(`   Default Login: admin / admin123`);
    console.log(`====================================`);
});
