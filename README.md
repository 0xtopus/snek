# 61C Spring 2023 Project 1: snek
一个简单的贪吃蛇小游戏项目，更多信息请见: [https://cs61c.org/sp23/projects/proj1/](https://cs61c.org/sp23/projects/proj1/)

## 使用方法
在linux环境下：
1. 运行下列命令将仓库克隆到本地：
```bash
clone git@github.com:0xtopus/snek.git
```

2. 切换到仓库目录，运行下列命令生成可执行文件：
```bash
make interactive-snake
```

3. 运行可执行文件，使用wasd这四个按键操控蛇的移动：
```bash
./interactive-snake
```

如果你希望调整蛇运动速度的快慢，你可以输入类似下面的命令（0.5表示蛇每步之间的间隔时间为0.5s）：
```bash
./interactive-snake -d 0.5
```
你也可以在游戏中按下`]`键或者`[`键来加快或减慢蛇的移速。

按下`Ctrl` + `C`组合键可以退出游戏。
