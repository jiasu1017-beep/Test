const sqlite3 = require('sqlite3').verbose();
const bcrypt = require('bcryptjs');
const db = new sqlite3.Database('data/admin.db');

// 测试账号信息
const testAccounts = [
    {
        username: 'testuser',
        email: 'test@ponywork.com',
        password: '123456'
    },
    {
        username: 'demo',
        email: 'demo@ponywork.com',
        password: 'demo123'
    }
];

console.log('🚀 开始创建测试账号...\n');

let created = 0;
let failed = 0;

testAccounts.forEach((account, index) => {
    const hashedPassword = bcrypt.hashSync(account.password, 10);
    
    db.run("INSERT INTO users (username, email, password, role) VALUES (?, ?, ?, 'user')",
        [account.username, account.email, hashedPassword], function(err) {
        if (err) {
            console.error(`❌ 账号 ${account.username} 创建失败：${err.message}`);
            failed++;
        } else {
            console.log(`✅ 账号 ${account.username} 创建成功！`);
            console.log(`   邮箱：${account.email}`);
            console.log(`   密码：${account.password}`);
            console.log(`   用户 ID: ${this.lastID}\n`);
            created++;
        }
        
        // 所有账号处理完毕
        if (created + failed === testAccounts.length) {
            console.log('====================================');
            console.log(`完成！成功：${created}, 失败：${failed}`);
            console.log('====================================\n');
            
            if (created > 0) {
                console.log('📝 测试账号列表：');
                testAccounts.forEach(acc => {
                    console.log(`  - ${acc.email} / ${acc.password}`);
                });
            }
            
            db.close();
        }
    });
});
