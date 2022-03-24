# UART TEST

在串口收发任务中，接收到启动测试指令，会对storage区域开始擦除操作，然后写入1-250连续数字序列，然后再读出数据进行比对，对比校验成功后返回相应的指令。

目前测试的flash区域为 0x50000 - 0x200000 ，以块为单位操作(起始块地址标号为0)，块大小为 SPI_FLASH_SEC_SIZE (4096)


融合后固件大小2M 可以直接下载到地址 0x0

编译固件-默认控制串口配置为：
```
RX  5
TX  4
115200
```


1、固件烧录后，串口启动指令： Flash sector 1 10 （烧写分区1测试10，注意空格将数字分开）
```
Flash sector 1 10   //测试块1共10遍
Flash sector a 10   //测试全部块10遍
```

2、接收到启动指令后串口反馈：

```
ready test sector 1:10
```

3、测试完成后串口反馈：
```
TEST sector 1 OK:10
```

4、测试过程中数据校验错误反馈：
```
test sector 1:erase err
test sector 1:write err
```

在删除和写入发生错误后，就不再继续向下执行，直接报错后等待再次输入测试指令


```
Flash sector 20 100

ready test sector 20:100
TEST sector 20 OK:100
```
