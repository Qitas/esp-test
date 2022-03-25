# UART TEST

在串口收发任务中，接收到启动测试指令，会对storage区域开始擦除操作，然后写入1-250连续数字序列，然后再读出数据进行比对，对比校验成功后返回相应的指令。

目前测试的flash区域为 0x50000 - 0x200000 ，以块为单位操作(起始块地址标号为0)，块大小为 SPI_FLASH_SEC_SIZE (4096)


下载到地址 0x0


1、固件烧录后，串口启动指令（烧写分区1测试10，注意空格将数字分开）
```
test sector 1 10   //测试块1共10遍
test sector a 10   //测试全部块10遍
```

2、接收到启动指令后串口反馈：

```
ready test sector 1:10
```

3、测试完成后串口反馈：
```
[1][0x50000]sector test 10 cycles:pass
[1][0x50000]sector test 10 cycles:write err
[1][0x50000]sector test 10 cycles:erase err
```


在删除和写入发生错误后，就不再继续向下执行，直接报错后等待再次输入测试指令，错误会有相应错误信息输出

