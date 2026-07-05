//显示控制模块
module Oled_Show_control(
	
	input			clk,
	input			rst,
	input			show_ack,
	input			show_req,
	output			show_end,
	output[6:0]		start_x,
	output[3:0]		start_y,	
	output[5:0]	    show_select		//显示数据索引
);

//==================== 【参数定义】 ====================
localparam			STR_ONE_X_STR 		=7'd32;   //第1行字符的x起始位置
localparam			STR_ONE_Y_STR 		=4'd1;    //第1行字符的y起始位置
localparam			STR_TWO_X_STR 		=7'd32;   //第2行字符的x起始位置
localparam			STR_TWO_Y_STR 		=4'd3;    //第2行字符的y起始位置 16/8=2
localparam			STR_THREE_X_STR 	=7'd32;   //第3行字符的x起始位置 【新增】
localparam			STR_THREE_Y_STR 	=4'd5;    //第3行字符的y起始位置 【新增】
localparam			STR_TO_STR    		=7'd0;    //字符间隔
localparam			STR_Y_TO_STR_Y 		=8'd0;    //字符行间距
localparam			STR_WIDTH     		=7'd16;   //字符宽度大小
localparam			STR_NUM				=6'd12;   //总字符数（3行×4个） 【修改】
localparam			STR_TWO_INDEX		=6'd3;    //第2行起始索引（第4个字符）
localparam			STR_THREE_INDEX		=6'd7;    //第3行起始索引（第8个字符）【新增】

//==================== 寄存器定义 ====================
reg[6:0]	start_x_reg;
reg[3:0]	start_y_reg;
reg[5:0]	show_select_reg;

//==================== 输出赋值 ====================
assign start_x = start_x_reg;
assign start_y = start_y_reg;
assign show_select = show_select_reg;

//显示完成标志
assign show_end = (show_ack == 1'b1 && show_select_reg == STR_NUM) ? 1'b1 : 1'b0;

//==================== 字符索引计数 ====================
always@(posedge clk or negedge rst)
begin
	if(rst == 1'b0)
		show_select_reg <= 'd0;
	else if(show_req == 1'b1 && show_ack == 1'b1)
		show_select_reg <= show_select_reg + 1'b1;
	else if(show_req == 1'b1)
		show_select_reg <= show_select_reg;
	else
		show_select_reg <= 'd0;
end

//==================== 坐标控制（核心：3行切换） ====================
always@(posedge clk or negedge rst)
begin
	if(rst == 1'b0)
	begin
		//默认第1行起始位置
		start_x_reg <= STR_ONE_X_STR;
		start_y_reg <= STR_ONE_Y_STR;
	end
	else if(show_req == 1'b1 && show_ack == 1'b1)
	begin
		if(show_select_reg == STR_TWO_INDEX)  //切换到第2行
		begin
			start_x_reg <= STR_TWO_X_STR;
			start_y_reg <= STR_TWO_Y_STR;
		end
		else if(show_select_reg == STR_THREE_INDEX)  //切换到第3行 【新增】
		begin
			start_x_reg <= STR_THREE_X_STR;
			start_y_reg <= STR_THREE_Y_STR;
		end
		else  //同一行继续写下一个字符
		begin
			start_x_reg <= start_x_reg + STR_WIDTH + STR_TO_STR;
			start_y_reg <= start_y_reg;
		end
	end
	else if(show_req == 1'b1)	
	begin
		start_x_reg <= start_x_reg;
		start_y_reg <= start_y_reg;
	end
	else begin
		//空闲状态回到第1行
		start_x_reg <= STR_ONE_X_STR;
		start_y_reg <= STR_ONE_Y_STR;
	end
end

endmodule