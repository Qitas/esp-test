# UART TEST

在串口收发任务中，接收到启动测试指令，会对storage

1、固件烧录后，串口启动指令： Flash sector 10 （烧写测试10）

```
Flash sector 10
```

2、接收到启动指令后串口反馈：ready test:10

3、测试完成后串口反馈：TEST OK:10

4、测试过程中数据校验错误反馈：test write err:7/10
