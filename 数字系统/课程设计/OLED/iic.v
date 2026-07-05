`timescale 1ns / 1ps
//
// Company: 
// Engineer: 
// 
// Create Date:    22:40:45 11/20/2017 
// Design Name: 
// Module Name:    I2C_Master 
// Project Name: 
// Target Devices: 
// Tool versions: 
// Description: 
               /*
	       I2C总线通信协议通用模块：SCL SDA
	       开始信号：SCL高时，SDA拉低
	       结束信号：SCL高时，SDA拉高
	       SDA数据在SCL低电平时置位
	       模块中实际默认开始信号与结束信号在SCL高电平中间产生
	       SDA数据位改变在SCL低电平的中间产生
	       SCL时钟频率为200kHz
	       从机地址可调，模块既支持读也支持写，通过输入管脚控制
		*/
// 
// Dependencies: 
//
// Revision: 
// Revision 0.01 - File Created
// Additional Comments: 
//
//

module I2C_Master(
		I_Clk_in,
		I_Rst_n,
		O_SCL,
		IO_SDA,
		//control_sig
		I_Start,   //一次读/写操作开始信号
		O_Done,    //一次读/写操作结束信号
		I_R_W_SET, //读写控制信号，写为1，读为0
		I_Slave_Addr,//从机地址
		I_R_W_Data,//读写控制字16位I_R_W_Data[15:8]->reg_addr,I_R_W_Data[7:0]->W_data,读状态则可默认为7'b0
            	O_Data,    //读到的数据，当O_Done拉高时数据有效
		O_Error	  //检测传输错误信号，当出现从机未响应，从机不能接收数据等情况时，拉高电平		
 );
 
//I/O
input		I_Clk_in;
input		I_Rst_n;
output		O_SCL;
inout		IO_SDA;
 
input		I_Start;
output		O_Done;
input  [6:0] 	I_Slave_Addr;
input		I_R_W_SET;
input  [15:0]	I_R_W_Data;
output [7:0] 	O_Data;
output      	O_Error;
/******时钟定位模块（测试时时钟为100MHz）,定位SCL的高电平中心，与SCL的低电平中心，产生200kHz的SCL*******/
parameter   Start_Delay=9'd60;//开始时SDA拉低电平持续的时间，共用计数器下应小于SCL_HIGH2LOW-1 60
parameter   Stop_Delay=9'd150;//一次读/写结束后SDA拉高电平的时间，共用计数器下应小于SCL_HIGH2LOW-1 150
parameter   SCL_Period=10'd499;//测试板时钟为50MHz,100KHz为500个Clk 499
parameter   SCL_LOW_Dest=10'd375;//时钟判定高电平在前，低电平在后,低电平中央为3/4个周期，375个Clk
parameter   SCL_HIGH2LOW=9'd249;//电平翻转位置，1/2个SCL周期，250个Clk
parameter   ACK_Dect=9'd124;     //SCL高电平中间位置，用于检测ACK信号124
reg    [8:0]	R_SCL_Cnt;
reg         	R_SCL_En;
 
assign      	O_SCL=(R_SCL_Cnt<=SCL_HIGH2LOW)?1'b1:1'b0;//SCL 时钟输出
 
always @ (posedge I_Clk_in or negedge I_Rst_n)
begin
		if (~I_Rst_n)
		 begin
		  R_SCL_Cnt<=9'b0;
		 end
		else
		 begin
		  if (R_SCL_En)
		   if (R_SCL_Cnt==SCL_Period)
		    R_SCL_Cnt<=9'b0;
		   else
		    R_SCL_Cnt<=R_SCL_Cnt+9'b1;
		  else
		   R_SCL_Cnt<=9'b0;
		 end
end
 
/******SDA读写控制模块******/
reg [5:0]    R_State;
reg          R_SDA_I_O_SET;//SDA双向选择I/O口 1为输出，0为输入
reg          R_SDA_t;      //SDA的输出端口
reg          O_Done;       //结束信号
reg [7:0]    O_Data;       //读到的数据
reg          O_Error;		//传输错误指示信号
 
/****状态定义*****/
parameter    Start=6'd0;  //一次读写开始的状态
parameter    ReStart=6'd34; //读操作入口状态
parameter    Stop=6'd56;    //发送停止位状态
 
always @ (posedge I_Clk_in or negedge I_Rst_n)
begin
		if (~I_Rst_n)
		 begin
		 R_SCL_En<=1'b0;     //计数时钟停止
		 R_State<=6'd0;
		 R_SDA_I_O_SET<=1'b1;//默认设置为输出管脚
		 R_SDA_t<=1'b1;      //SDA输出默认拉高
		 O_Data<=8'b0;
		 O_Done<=1'b0;
		 O_Error<=1'b0;
		 end
		else
		 begin
		  if (I_Start) //当开始信号置高时表示I2C通信开始
		   begin
			case(R_State)
			 Start:   //启动位
			   begin
			   R_SCL_En<=1'b1;
			   O_Error<=1'b0;//每次重新下一次传输时，清除错误标志位
			   if (R_SCL_Cnt==Start_Delay)
			     begin
			      R_SDA_t<=1'b0; //SCL高电平时拉低
			      R_State<=R_State+6'd1;
			     end
			   else
			     begin
			      R_SDA_t<=1'b1;
			      R_State<=R_State;
			     end
			    end
			  6'd1,6'd2,6'd3,6'd4,6'd5,6'd6,6'd7:  //写入7位从机地址
			    begin
			      if (R_SCL_Cnt==SCL_LOW_Dest)
				begin
			         R_SDA_t<=I_Slave_Addr[6'd7-R_State];//从MSB-LSB写入输入端从机地址
				 R_State<=R_State+6'd1;
				end
			      else
                                 R_State<=R_State;		
			     end
			  6'd8: //写入写标志（0）
			    begin
			     if (R_SCL_Cnt==SCL_LOW_Dest)
			      begin
				R_SDA_t<=1'b0;
				R_State<=R_State+6'd1;
			      end
			    else
                               R_State<=R_State;							 
			    end
			  6'd9: //ACK状态 
			    begin
			     if (R_SCL_Cnt==SCL_HIGH2LOW) //在第8个时钟的下降沿释放SDA
			       begin
				R_SDA_I_O_SET<=1'b0;
				R_State<=R_State+6'd1;
			       end
			     else
				R_State<=R_State;
			    end
			  6'd10: //在第9个时钟高电平中心检测ACK信号是否为0，如果为1，则表示从机未应答，进入结束位
			     begin
			       if (R_SCL_Cnt==ACK_Dect)
				 begin
				  O_Error<=IO_SDA;  //检测从机是否响应
				  R_State<=R_State+6'd1;
				 end
			       else
				  R_State<=R_State; 
			     end
			  6'd11:
			     begin
			       if (R_SCL_Cnt==SCL_HIGH2LOW) //在第9个时钟的下降沿重新占用SDA，准备发送从机子寄存器地址
				 begin
				   R_SDA_I_O_SET<=1'b1;
				   R_State<=(O_Error)?Stop:(R_State+6'd1);
				   R_SDA_t<=1'b0;
				 end
				else
				   R_State<=R_State;					  
			      end
			  6'd12,6'd13,6'd14,6'd15,6'd16,6'd17,6'd18,6'd19:  //写入8位寄存器地址
			     begin
			      if (R_SCL_Cnt==SCL_LOW_Dest)
				begin
				 R_SDA_t<=I_R_W_Data[6'd27-R_State];//从MSB-LSB写入寄存器地址 I_R_W_Data[15:8]
				 R_State<=R_State+6'd1;
				end
			      else
                                 R_State<=R_State;							 
			      end			 
			   6'd20: //ACK状态  
			     begin
			       if (R_SCL_Cnt==SCL_HIGH2LOW)//在第8个时钟的下降沿释放SDA
				 begin
				  R_SDA_I_O_SET<=1'b0;
				  R_State<=R_State+6'd1;
				 end
			       else
				 R_State<=R_State;
			     end
			   6'd21: //检测ACK
			     begin
				if (R_SCL_Cnt==ACK_Dect)
				  begin
				   O_Error<=IO_SDA;//检测从机是否响应
				   R_State<=R_State+6'd1;
				  end
				else
				  R_State<=R_State; 
			     end
			   6'd22: 
			      begin
			       if (R_SCL_Cnt==SCL_HIGH2LOW) //在第9个时钟的下降沿重新占用SDA，区分接下来该发送数据还是读数据
			         begin
				  R_SDA_I_O_SET<=1'b1;
				  R_State<=(O_Error)?Stop:((I_R_W_SET)?(R_State+6'd1):ReStart); //从机状态
				  R_SDA_t<=(O_Error|I_R_W_SET)?1'b0:1'b1; //此处拉高SDA信号是为读状态重启开始信号做准备
				 end
			       else
				  R_State<=R_State;							
				 end
			    6'd23,6'd24,6'd25,6'd26,6'd27,6'd28,6'd29,6'd30://写入8位数据地址 
			       begin
				if (R_SCL_Cnt==SCL_LOW_Dest)
				  begin
				   R_SDA_t<=I_R_W_Data[6'd30-R_State];//从MSB-LSB写入8位数据地址
				   R_State<=R_State+6'd1;
				  end
				else
                                   R_State<=R_State;									                                end
			     6'd31: //ACK状态
			        begin
				 if (R_SCL_Cnt==SCL_HIGH2LOW)//在第8个时钟的下降沿释放SDA
				   begin
				    R_SDA_I_O_SET<=1'b0;
				    R_State<=R_State+6'd1;
				   end
				  else
			            R_State<=R_State;					
				 end
			      6'd32://检测ACK
				 begin
				   if (R_SCL_Cnt==ACK_Dect)
				    begin
				     O_Error<=IO_SDA;//检测从机是否响应
				     R_State<=R_State+6'd1;
				    end
				    else
				     R_State<=R_State; 
				  end				 
			       6'd33:
				 begin
				   if (R_SCL_Cnt==SCL_HIGH2LOW)//在第9个时钟的下降沿重新占用SDA，准备发送停止位
				     begin
				      R_SDA_I_O_SET<=1'b1;
				      R_SDA_t<=1'b0;//先拉低SDA信号
				      R_State<=Stop;//跳转到结束位发送状态
				     end
				    else
				      R_State<=R_State;							 
				  end
			       ReStart://主机读状态入口 初始时需要重启开始状态
				 begin
				  if (R_SCL_Cnt==Start_Delay)
				   begin
				    R_SDA_t<=1'b0; //SCL高电平时拉低
				    R_State<=R_State+6'd1;
				   end
				  else
				   begin
				    R_SDA_t<=1'b1;
			            R_State<=R_State;
			           end					  
				 end			
                               6'd35,6'd36,6'd37,6'd38,6'd39,6'd40,6'd41://发送从机7位地址		
                                 begin
			          if (R_SCL_Cnt==SCL_LOW_Dest)
				    begin
				     R_SDA_t<=I_Slave_Addr[6'd41-R_State];//从MSB-LSB写入输入端从机地址
				     R_State<=R_State+6'd1;
				    end
				   else
                                     R_State<=R_State;						
				 end
			        6'd42://写入读标志(1)
				 begin
				   if (R_SCL_Cnt==SCL_LOW_Dest)
				     begin
				      R_SDA_t<=1'b1;//写入读地址标志
				      R_State<=R_State+6'd1;
				     end
				   else
                                      R_State<=R_State;							  
				 end
				6'd43: //ACK状态
				   begin
				     if (R_SCL_Cnt==SCL_HIGH2LOW)//在第8个时钟的下降沿释放SDA
				       begin
				        R_SDA_I_O_SET<=1'b0;
					R_State<=R_State+6'd1;
				       end
				     else
				        R_State<=R_State;							 				            end
			        6'd44://ACK检测
				   begin
				     if (R_SCL_Cnt==ACK_Dect)
				        begin
					 O_Error<=IO_SDA;
					 R_State<=R_State+6'd1;
				        end
				      else
					 R_State<=R_State;
				    end	
				6'd45://之后需要一直读取数据，所以SDA总线这里需要保持输入状态
				    begin
				      if (R_SCL_Cnt==SCL_HIGH2LOW)//在第9个时钟下降沿保持SDA总线的释放状态
					begin
					 R_SDA_I_O_SET<=(O_Error)?1'b1:1'b0;//若前次ACK检测通过，则保持SDA总线释放状态，不                                                                                通过则占用SDA总线用来发送停止位
					 R_State<=(O_Error)?Stop:(R_State+6'd1);
					 R_SDA_t<=1'b0; 
					end
				       else
					 R_State<=R_State;
				     end
				6'd46,6'd47,6'd48,6'd49,6'd50,6'd51,6'd52,6'd53://8个时钟信号高电平中间依次从SDA上读取数据
				     begin
				       if (R_SCL_Cnt==ACK_Dect)
					 begin
					  O_Data<={O_Data[6:0],IO_SDA};//从MSB开始读入数据
					  R_State<=R_State+6'd1;
					 end
				       else
					  R_State<=R_State;
				      end
				6'd54://读入8位数据后,主机需要向外发送一个NACK信号
				    begin
				       if (R_SCL_Cnt==SCL_HIGH2LOW)
					 begin
					  R_SDA_I_O_SET<=1'b1;//主机重新占用SDA
					  R_SDA_t<=1'b1;
					  R_State<=R_State+6'd1;
					 end
				       else
					  R_State<=R_State;
				     end
				6'd55://在第9个时钟下降沿持续占用总线，拉低SDA，开始发送结束位
				    begin
				       if (R_SCL_Cnt==SCL_HIGH2LOW)
					 begin
					  R_SDA_t<=1'b0;
					  R_State<=R_State+6'd1;
					 end
				       else
					 R_State<=R_State;
				    end
				Stop: //发送停止位
				    begin
				       if (R_SCL_Cnt==Stop_Delay)
					 begin
					  R_SDA_t<=1'b1;
					  R_State<=R_State+6'd1;
					 end
				       else
					  R_State<=R_State;
				    end
				6'd57: //停止时钟，同时输出Done信号，表示一次读写操作完成
				    begin
				     R_SCL_En<=1'b0;
				     O_Done<=1'b1;//拉高Done信号
				     R_State<=R_State+6'd1;
				    end
				6'd58:
				    begin
				     O_Done<=1'b0;//拉低Done信号
				     R_State<=Start;
				    end
				default:
				   begin
			             R_SCL_En<=1'b0;//计数时钟停止
			             R_State<=6'd0;
		                     R_SDA_I_O_SET<=1'b1;//默认设置为输出管脚
		                     R_SDA_t<=1'b1;//SDA输出默认拉高
		                     O_Done<=1'b0;			 					
				   end
				endcase			  
			 end		  
		  else         //开始信号无效时，回到初始设置
		   begin
		    R_SCL_En<=1'b0;     //计数时钟停止
		    R_State<=6'd0;
		    R_SDA_I_O_SET<=1'b1;//默认设置为输出管脚
		    R_SDA_t<=1'b1;      //SDA输出默认拉高
		    O_Done<=1'b0;
		   end		 
	       end
end
 
/*******配置三态门信号******/
assign  IO_SDA=(R_SDA_I_O_SET)?R_SDA_t:1'bz;
 
 
endmodule
