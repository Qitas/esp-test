# UART TEST

在串口收发任务中，接收到启动测试指令，会对storage区域开始擦除操作，然后写入1-128连续序列，然后读出数据进行对比，对比校验成功后返回相应的指令。

1、固件烧录后，串口启动指令： Flash sector 1 10 （烧写分区1测试10，注意空格将数字分开）

```
Flash sector 1 10
```

2、接收到启动指令后串口反馈：ready test:10

```
ready test sector 1:10
```

3、测试完成后串口反馈：TEST sector 1 OK:10
```
TEST sector 1 OK:10
```

4、测试过程中数据校验错误反馈：test sector 1 write err:7/10
```
test sector 1:erase err
test sector 1:write err
```
