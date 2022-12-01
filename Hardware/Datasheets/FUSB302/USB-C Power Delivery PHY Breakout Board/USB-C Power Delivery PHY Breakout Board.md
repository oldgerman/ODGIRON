# USB-C Power Delivery PHY Breakout Board

[FUSB302BGEVB：C型接口检测解决方案（FUSB302BGEVB）带tssop28 32位 128KB flash 32KB SRAM 50MHz主频位RISC MCU：PIC32MX250F128B-I/SS](https://www.terraelectronica.ru/rd/ONS/FUSB302BGEVB)

QC和PD是主流，特别是PD，后面应该会一统天下，我觉得这种私有协议别浪费时间研究

这个fusb302是不支持pd3.0对吧？手册好像没提

哦，找到了，是只有2.0

对，只支持2.0
你可以看FUSB307，今天问了价格和302基本一样，支持3.0



![USB-C Power Delivery PHY Breakout Board 1](https://cdn.tindiemedia.com/images/resize/5IIKnTYoCHVgKwQHI1qd4ExLwpA=/p/fit-in/1032x688/filters:fill(fff)/i/86153/products/2019-07-22T05%3A44%3A04.861Z-USB-C_PD_Breakout_Top.JPG)



是否想使用新的USB Type-C连接器？您需要一个芯片来管理CC引脚。该板可让您使用I2C和中断线来执行所有必要的USB-C功能。该板的核心是Fairchild FUSB302，C型端口控制器和BMC PHY。您可以使用该芯片执行以下所有操作：
*确定插头方向（正常或反向）
*确定或公布C型功率水平（0.5 V，1.5、1.5或3.0 A时为5 V）
*使用BMC通信进行协商USB供电显式签约高达20 V和5 A（100 W功率）
*协商备用模式，以将Type-C连接器中的引脚复用用于其他目的

所有设计文件均可在GitHub上获得，并且是开源的。原理图，布局，gerber和示例代码在MIT风格的许可下可用。

你可以考虑FUSB307，支持PD3.0，问过价格比FUSB302贵1块钱，当然FUSB302对你来说也够用，PD3.0向下兼容



FUSB302 的目标用户为希望以较少的编程量实施 DRP/SRC/SNK USB Type-C 接头的系统设计人员。FUSB302 实现了 USB Type-C 检测，包括连接和定向。FUSB302 集成了 USB BMC 功率输送协议的物理层，可实现高达 100 W 的电源和角色互换。BMC PD 装置实现了对于符合 C 型规范的其他接口的完全支持。参考法规可用于 FUSB302 在若干嵌入控制器平台之间轻松实施 C 型 和 USB BMC 功率输送协议。单击下面的“软件”链接。

| 特性                                                         |      |
| :----------------------------------------------------------- | ---- |
|                                                              |      |
| 通过自主 DRP 切换实现双角色功能                              |      |
| 能够根据具体的连接，连接主机或设备。                         |      |
| 软件可配置为专用主机、专用设备或双角色                       |      |
| 专用设备可采用具有固定 CC 和 VCONN 通道的 Type-C 插座或 Type-C 插头运行。 |      |
| 完全支持 Type-C 1.1。集成 CC 引脚的以下功能                  |      |
| 作为主机进行连接/断连检测                                    |      |
| 作为主机进行电流性能指示                                     |      |
| 作为设备进行电流性能检测                                     |      |
| 音频适配器附件模式                                           |      |
| 调试附件模式                                                 |      |
| 有源电缆检测                                                 |      |
| 将 CCx 集成到 VCONN 开关中，具有过流限制，支持 USB3.1 全功能数据线。 |      |
| USB 电力传输 (PD) 2.0、版本 1.1 支持                         |      |
| 自动 GoodCRC 报文响应                                        |      |
| 如果需要，自动软复位重新发送的报文                           |      |
| 如果未收到 GoodCRC，自动尝试重发报文                         |      |
| 电池电量耗尽支持（没有加电时的 SNK 模式支持）                |      |
| 自动硬复位发送的顺序集                                       |      |
| 采用 9 焊球 WLCSP (1.215 mm x 1.260 mm) 和 14 引脚 MLP（2.5 mm x 2.5 mm，0.5 mm 节距）封装 |      |
| 低功率运行：ICC = 25 μA（典型值）                            |      |

| 应用                                                         |
| :----------------------------------------------------------- |
| This product is general usage and suitable for many different applications. |





### USB Type-C PD开发板

8个月前

- [电源模块](https://oshwhub.com/explore?filter=2613794836ff4fa2954e4cca5749584a)
- [方案验证板](https://oshwhub.com/explore?filter=90637fd096424071a21927e5cdc436de)
- [消费电子](https://oshwhub.com/explore?filter=e5831c441f25423d88a7311a9b276871)

简介：FUSB302B（C132291）控制器+INA226（C49851）电流检测，实现USB-PD协议，可以DIY线材检测器，充电器输出能力检测器等等。且集成了QC3.0手动诱骗电路，测试QC3.0功能

开源协议: GPL 3.0

https://oshwhub.com/wuziyue/USB-Type-C-PDkai-fa-ban