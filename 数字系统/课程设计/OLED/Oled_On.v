





module Oled_On(
	input		clk,
	input		rst,
	
	input		write_done,		
	
	input		On_req,		
	output		On_ack,		
	
	output[23:0]	 On_data		
);	

localparam			RST_T			=	1'b0;				

reg[23:0]		On_data_reg;


reg[3:0]	On_page;
reg[7:0]	On_index;

assign On_data  = On_data_reg;
assign On_ack = (On_index >= 'd130 && On_page >= 'd7 && write_done == 1'b1) ? 1'b1 : 1'b0;

always@(posedge clk or negedge rst)
begin
	if(rst == RST_T)
		On_index <= 'd0;
	else if(On_index == 'd130 && write_done == 1'b1 )
		On_index <= 'd0;
	else if(write_done == 1'b1 && On_req == 1'b1)
		On_index <= On_index + 1'b1;
	else
		On_index <=On_index;
end

//设置�?
always@(posedge clk or negedge rst)
begin
	if(rst == RST_T)
		On_page <= 'd0;
	else if(On_index == 'd130 && write_done == 1'b1 && On_page == 'd7)
		On_page <= 'd0;
	else if(On_index == 'd130 && write_done == 1'b1)
		On_page <=On_page + 1'b1;
	else
		On_page <= On_page;
end
always@(*)
begin
	case(On_index)
		'd0:On_data_reg <= {8'h78,8'h00,8'hb0 + On_page};
		'd1:On_data_reg <= {8'h78,8'h00,8'h00};
		'd2:On_data_reg <= {8'h78,8'h00,8'h10};
	default:On_data_reg <= {8'h78,8'h40,8'hFF};
	endcase
	
end


endmodule 