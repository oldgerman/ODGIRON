## 固件升级方法

### 【booloader】烧写

如果是全片擦除的芯片，那么请先用ST-LINK烧写“ODGIRON_bootloader_XXX.bin” ，这是booloader文件，只有烧进去了，才能用烙铁自带的USB Type-C口通过DFU升级APP文件，具体操作见下一步【APP】烧写

### 【APP】烧写

1. 安装Dfuse软件，ST的DFU驱动也要安装，win10系统请安装 “Dfuse文件安装路径/bin/Driver/Win7/X86”下的dpinst_x86.exe,[Dfuse下载链接](https://www.st.com/en/development-tools/stsw-stm32080.html)
2. 用USB将至少烧写了booloader程序的烙铁连接电脑，在启动的logo页面时，长按中键，可进入USB DFU升级模式的页面
3. 打开DfuseDemo软件，若检测到处于dfu模式的烙铁芯片，那么就查看“Product ID”栏内的数字，记下来
4. 用Dfuse软件安装时自带的小工具“Dfu file mananger”将“ODGIRON_APP_XXX.hex” 转成“*.dfu”文件升级，
   转之前先填刚才记下的“Product ID”，然后直接刷固件，升级之后请在菜单里 恢复出厂设置