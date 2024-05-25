# Music-Management-System
Fudan 2024 Database PJ1 based on bserv 

![ER图](https://github.com/yinchangguwang/Music-Management-System/assets/145670940/6ce0e273-953e-4860-aea1-5f92ed87fb82)

## 部署方法
### 系统环境要求：
- 操作系统：支持Windows操作系统。
- 数据库：需要安装支持SQL的数据库系统PostgreSQL。
- 编程语言：系统后端使用c++编写，需要安装Visual Studio和c++运行环境。
 -系统框架：bserv。配置方法见https://github.com/JonathanSilver/bserv

### 部署步骤：
- 配置好环境后，下载系统源代码，并解压到本地目录，将WebApp文件夹中文件替换掉同名原文件，将templates文件夹整个替换。
- 配置数据库：创建数据库bserv并连接，并从db.sql导入系统所需的数据表结构。
- 运行系统：在Visual Studio打开WebApp.sln，运行调试命令，启动系统服务。
- 访问系统：打开浏览器，在地址栏输入系统的访问地址localhost:8080，即可访问系统并开始使用。
