宠物智能管理系统
基于 LVGL 的嵌入式宠物智能管理系统，适用于 Linux 帧缓冲设备（/dev/fb0），提供宠物状态监控、设备控制等功能。

项目信息
项目名称：宠物智能管理系统
作者：Johnson Li
制作时间：2025/11/08
版本号：v1.2
用途：宠物日常管理（喂食、环境监控等）的可视化控制终端

项目简介
本系统基于 LVGL（嵌入式 GUI 库）开发，运行在 Linux 系统的帧缓冲设备（/dev/fb0）上，
提供图形化界面实现宠物相关设备的智能管理。
支持宠物喂食器控制、环境温度 / 湿度监控、设备状态显示等功能，
适合嵌入式 Linux 设备（如开发板）部署。

环境依赖
操作系统：Linux（支持帧缓冲设备/dev/fb0，如 Ubuntu、嵌入式 Linux 发行版）
编译工具：gcc、make
依赖库：libpthread（多线程支持）、LVGL（已通过子模块集成）

安装与编译:

编译项目在当前目录执行 make 命令，自动编译生成可执行文件： 

主目录下输入命令: make

编译完成后，生成的可执行文件为 demo，传输至开发板后 ./demo 直接运行即可（需要将所有图片一同拷入到开发板）：

进入 test1 后 ./start.sh

运行的文件是华为云的通讯文件（需要配置自己的华为云），打开另一个窗口输入: ./server 为本地服务器

led_and_buz 文件中有 led 和 buz 的驱动，如果开发板没有，需要按照教程安装

效果如下:
<p align="center">
  <img src="https://github.com/user-attachments/assets/19362536-16e7-4a36-9d1d-badfe7dd55a2" alt="界面1" width="400">
  <img src="https://github.com/user-attachments/assets/8e1d420d-8a75-4fe9-ac04-c8d446698a96" alt="界面2" width="400">
  <img src="https://github.com/user-attachments/assets/83faf474-333e-469f-b535-6be9c68401be" alt="界面3" width="400">
  <img src="https://github.com/user-attachments/assets/493cba2f-cd11-45f0-9094-b7bb35284596" alt="界面4" width="400">
</p>

注意事项
本项目仅供学习参考，实际部署需根据硬件设备（如传感器、执行器）进行适配
确保运行设备已启用帧缓冲（/dev/fb0 存在），且支持触摸屏（如需交互操作）
若编译失败，检查子模块是否完整初始化，或依赖工具是否安装（如 make、gcc）
