# 电控组第三次培训作业

* 姓名：**毕译文**
* 学号：***20263006495***

一、 参考资料与驱动来源
* OLED 显示驱动及字库均来自群文件

二、 串口反馈格式说明
* 当按键操作或串口指令校验通过并应用后，单片机将通过串口回传更新后的状态。
一、 参考资料与驱动来源
OLED 显示驱动及字库均来自群文件

二、 串口反馈格式说明
 当按键操作或串口指令校验通过并应用后，单片机将通过串口回传更新后的状态。

自定义的反馈文本格式为：
STATE=运行状态,DIR=流水方向,STEP=间隔时间ms,LED=当前亮的灯编号
实际发送示例：STATE=RUN,DIR=FWD,STEP=300ms,LED=0\r\n（和作业要求一致）

三、 选做项说明（DMA 发送反馈）
串口反馈发送模式设为 DMA 发送

uint8_t tx_packet[80];//反馈发送缓冲区  
volatile uint8_t tx_ready = 0;//可以发送标志位
tx_packet用于存储snprintf拼装的字符串
tx_ready用于在按键操作和串口指令校验通过后 自身置1打上标记以用于后续主循环中轮询判断

对拼接功能做了一下封装
void Build_FeedBack_String(void)//拼接发送反馈字符串存到tx_packet中
{
    snprintf(tx_packet, sizeof(tx_packet),"STATE=%s,DIR=%s,STEP=%ums,LED=%u\r\n",
      key2_num? "RUN" : "STOP", key1_num? "REV" : "FWD", run_interval_ms, led_chaser_status);
}

主循环中轮询检测标记 并且还要用过uart1的句柄去判断其是否空闲 等空闲时再去执行发送业务
if (tx_ready)//判断状态是否更新 
    {
      if (huart1.gState == HAL_UART_STATE_READY)//判断uart1句柄是否空闲 防串口占用发送失败
      {
        Build_FeedBack_String();//把数据拼到tx_packet中
        HAL_UART_Transmit_DMA(&huart1, (uint8_t*)tx_packet,strlen((const char*)tx_packet));
        tx_ready = 0;//消费
      }
    }

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)//回调 但不用处理什么业务
{
  if (huart->Instance == USART1);//读一下基地址 无需手动操作
}

在主循环中拼装好反馈字符串后，直接将其首地址和长度交给 DMA 控制器。DMA 在后台自动将存储器中的数据搬运至 USART 的发送寄存器，进行发送，等发送完毕后执行回调通知CPU执行业务

优势：无需像Transmit_IT一样每向发送移位寄存器装字节就TXE一下就打断CPU进入中断搬数据  只在发送完毕后执行回调  解放了CPU

 四、 思考题回答
程序没有出现延迟，按键和串口指令均能无延迟响应。

我选择了时间戳轮询的方式 不会让程序在主循环中阻塞 

我的改进
    1.  非阻塞架构：我使用了HAL_GetTick()  时间戳轮询 。  主循环是一个状态机 无论间隔时间多长，CPU 都在轮询，随时准备响应按键和串口事件。
    2. 中断只打标记：串口和按键的中断回调函数中，只负责修改标志位和记录时间戳，进行标志位的生产。通过标志位，所有的业务执行都交由主循环执行。