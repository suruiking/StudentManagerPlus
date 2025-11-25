# StudentManagerPlus - 学生管理系统

一个基于 Qt 框架开发的桌面学生管理系统，提供学生信息管理、课程安排、财务管理、荣誉墙展示等功能。

## 📋 项目简介

StudentManagerPlus 是一个功能完整的学生管理桌面应用程序，采用 Qt Widgets 开发，使用 SQLite 数据库存储数据。系统提供了直观的用户界面和丰富的数据可视化功能，帮助教育机构高效管理学生信息、课程安排和财务记录。

## ✨ 功能特性

### 1. 学员信息管理
- ✅ 学生信息的增删改查（CRUD）
- ✅ 学生照片上传和管理（BLOB 存储）
- ✅ 按姓名/学号搜索学生
- ✅ 表格展示学生信息（学号、姓名、性别、生日、入学日期、学习目标、班级）
- ✅ 学生详细信息查看

### 2. 课程安排
- ✅ 周视图课程表展示
- ✅ 按年份和周数查看课程
- ✅ 课程添加、编辑、删除
- ✅ 支持多个时间段（上午1/2、下午1/2、晚上1/2）
- ✅ 自动计算日期范围

### 3. 财务管理
- ✅ 缴费记录管理（增删改查）
- ✅ 按学生和日期范围筛选记录
- ✅ **支付类型分布饼状图**（数据可视化）
- ✅ **收支趋势折线图**（数据可视化）
- ✅ 支持多种支付方式（微信、支付宝、现金、银行卡等）

### 4. 荣誉墙
- ✅ 荣誉图片展示和管理
- ✅ 图片描述和日期记录

### 5. 系统设置
- ✅ 系统参数配置
- ✅ 数据库路径设置

### 6. 用户认证
- ✅ 登录系统（用户名/密码）
- ✅ SHA256 密码加密
- ✅ 记住密码功能（XOR 加密存储）
- ✅ 默认管理员账户（admin/123456）

## 🛠️ 技术栈

- **开发框架**: Qt 5.15.2
- **编程语言**: C++17
- **数据库**: SQLite
- **UI 框架**: Qt Widgets
- **图表库**: Qt Charts
- **样式**: QSS (Qt Style Sheets)
- **设计模式**: 
  - 单例模式（DataBaseManager、Settings）
  - MVC 模式（QSqlTableModel）

## 📁 项目结构

```
StudentManagerPlus/
├── build/                          # 编译输出目录
├── res/                            # 资源文件（图标、图片）
│   ├── icons8--50.png
│   └── tubiao.webp
├── qss/                            # 样式表文件
│   └── wechat_fresh.qss
├── databasemanager.cpp/h           # 数据库管理（单例模式）
├── logindialog.cpp/h/ui            # 登录对话框
├── mainwindow.cpp/h/ui             # 主窗口
├── studentinfowidget.cpp/h/ui      # 学员信息管理模块
├── schedulewidget.cpp/h/ui         # 课程安排模块
├── finanacialwidget.cpp/h/ui       # 财务管理模块
├── honorwallwidget.cpp/h/ui        # 荣誉墙模块
├── systemsettingswidget.cpp/h/ui   # 系统设置模块
├── settings.cpp/h                  # 系统设置（单例模式）
├── main.cpp                        # 程序入口
├── res.qrc                         # 资源文件配置
└── StudentManagerPlus.pro          # Qt 项目文件
```

## 🗄️ 数据库结构

系统使用 SQLite 数据库，包含以下数据表：

### studentInfo（学生信息表）
- `id` (TEXT PRIMARY KEY) - 学号
- `name` (TEXT) - 姓名
- `gender` (TEXT) - 性别
- `birthday` (TEXT) - 生日
- `join_date` (TEXT) - 入学日期
- `study_goal` (TEXT) - 学习目标
- `progress` (TEXT) - 班级
- `photo` (BLOB) - 照片

### financialRecords（财务记录表）
- `id` (INTEGER PRIMARY KEY AUTOINCREMENT) - 记录ID
- `student_id` (TEXT NOT NULL) - 学生ID
- `payment_date` (TEXT) - 缴费日期
- `amount` (REAL) - 金额
- `payment_type` (TEXT) - 支付类型
- `notes` (TEXT) - 备注

### schedule（课程表）
- `date` (TEXT NOT NULL) - 日期
- `time` (TEXT NOT NULL) - 时间段
- `course_name` (TEXT) - 课程名称
- PRIMARY KEY (date, time)

### honorWall（荣誉墙表）
- `id` (INTEGER PRIMARY KEY AUTOINCREMENT) - ID
- `image_data` (BLOB) - 图片数据
- `description` (TEXT) - 描述
- `added_date` (TEXT) - 添加日期

### users（用户表）
- `username` (TEXT PRIMARY KEY) - 用户名
- `password` (TEXT NOT NULL) - 密码（SHA256 哈希）

## 🔧 环境要求

- **Qt 版本**: Qt 5.15.2 或更高版本
- **编译器**: 
  - Windows: MinGW 32-bit 或 MSVC
  - Linux: GCC
  - macOS: Clang
- **C++ 标准**: C++17
- **Qt 模块**: 
  - Qt Core
  - Qt GUI
  - Qt Widgets
  - Qt SQL
  - Qt Charts
  - Qt Network

## 🚀 编译运行

### Windows

1. **安装 Qt**
   - 下载并安装 Qt 5.15.2
   - 确保安装了 MinGW 或 MSVC 编译器

2. **打开项目**
   ```bash
   # 使用 Qt Creator 打开 StudentManagerPlus.pro
   # 或使用命令行
   qmake StudentManagerPlus.pro
   make
   ```

3. **运行程序**
   ```bash
   # 在 build 目录下找到可执行文件
   ./StudentManagerPlus.exe
   ```

### Linux

```bash
# 安装 Qt 开发库
sudo apt-get install qt5-default qtbase5-dev qtcharts5-dev

# 编译
qmake StudentManagerPlus.pro
make

# 运行
./StudentManagerPlus
```

### macOS

```bash
# 使用 Homebrew 安装 Qt
brew install qt@5

# 编译
qmake StudentManagerPlus.pro
make

# 运行
./StudentManagerPlus.app/Contents/MacOS/StudentManagerPlus
```

## 📖 使用说明

### 首次使用

1. **登录系统**
   - 默认管理员账户：`admin`
   - 默认密码：`123456`
   - 可选择"记住密码"功能

2. **添加学生**
   - 进入"学员信息"模块
   - 点击"新增学生"按钮
   - 依次输入学号、姓名、性别、生日、班级等信息
   - 可上传学生照片

3. **安排课程**
   - 进入"课程安排"模块
   - 选择年份和周数
   - 点击表格中的时间格子
   - 点击"添加课程"按钮输入课程名称

4. **记录财务**
   - 进入"财务"模块
   - 选择学生和日期范围
   - 点击"添加"按钮记录缴费信息
   - 查看饼状图和折线图了解收支情况

### 功能操作

- **搜索学生**: 在学员信息模块的搜索框输入姓名或学号
- **删除记录**: 选中表格中的行，点击"删除"按钮
- **修改信息**: 选中记录后点击"修改"按钮
- **查看图表**: 在财务模块自动生成饼状图和折线图

## 🔐 安全特性

- **密码加密**: 使用 SHA256 哈希算法加密存储密码
- **记住密码**: 使用 XOR 加密 + Base64 编码存储到本地配置
- **数据库安全**: SQLite 数据库文件本地存储，支持自定义路径

## 🎨 UI 特性

- **现代化界面**: 使用 QSS 样式表美化界面
- **响应式布局**: 使用 Splitter 实现可调整布局
- **数据可视化**: Qt Charts 提供丰富的图表展示
- **友好交互**: 工具提示、确认对话框等提升用户体验

## 📝 开发说明

### 核心设计模式

1. **单例模式**
   - `DataBaseManager`: 数据库连接管理
   - `Settings`: 系统配置管理

2. **MVC 模式**
   - 使用 `QSqlTableModel` 实现模型-视图分离
   - 表格视图自动同步数据库变化

### 关键功能实现

- **数据库初始化**: 自动创建数据表结构
- **图片存储**: 使用 BLOB 类型存储学生照片
- **日期计算**: ISO 周算法计算周视图日期范围
- **数据统计**: 实时计算支付类型分布和收支趋势

## 🐛 已知问题

- 饼状图标签显示需要优化字体大小（已部分修复）
- 部分错误处理可以进一步完善

## 🔮 未来计划

- [ ] 数据导出功能（Excel/PDF）
- [ ] 多语言支持（国际化）
- [ ] 数据备份和恢复
- [ ] 更多统计图表
- [ ] 操作日志记录
- [ ] 主题切换功能

## 📄 许可证

本项目仅供学习和个人使用。

## 👨‍💻 作者

学生管理系统开发项目

## 🙏 致谢

- Qt 框架
- SQLite 数据库
- Qt Charts 图表库

---

**注意**: 首次运行程序会自动创建数据库文件 `Manager.db`，默认管理员账户为 `admin/123456`，请及时修改密码。

