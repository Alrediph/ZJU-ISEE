




//oled_init  初始化模�?
module Oled_Init(
	
	input			clk,
	input			rst,
	
	input			init_req,				
	input			write_done,			
	
	output			init_finish,			

	output[23:0]		Init_data		
);

localparam			RST_T			=	1'b0;				

reg[23:0]		Init_data_reg;
reg[4:0]			Init_index;


assign Init_data  = Init_data_reg;
assign init_finish = (Init_index >= 'd26 && write_done == 1'b1) ? 1'b1 : 1'b0;



always@(posedge clk or negedge rst)
begin
	if(rst == RST_T)
		Init_index <= 'd0;
	else if(Init_index == 'd26 && write_done == 1'b1 )
		Init_index <= 'd0;
	else if(write_done == 1'b1 && init_req == 1'b1)
		Init_index <= Init_index + 1'b1;
	else
		Init_index <= Init_index;
end


//always@(posedge clk or negedge rst)
//begin
//	if(rst == RST_T)
//		Init_finish_reg <= 1'b0;
//	else if(Init_index >= 'd26 && write_done == 1'b1)
//		Init_finish_reg <= 1'b1;
//	else
//		Init_finish_reg <= 1'b0;
//end

always@(*)
begin
	case(Init_index)
		'd0:	Init_data_reg <= {8'h78,8'h00,8'hAE};
		'd1:	Init_data_reg <= {8'h78,8'h00,8'h00};
		'd2:	Init_data_reg <= {8'h78,8'h00,8'h10};
		'd3:	Init_data_reg <= {8'h78,8'h00,8'h40};
		'd4:	Init_data_reg <= {8'h78,8'h00,8'hB0};
		'd5:	Init_data_reg <= {8'h78,8'h00,8'h81};
		'd6:	Init_data_reg <= {8'h78,8'h00,8'hFF};
		'd7:	Init_data_reg <= {8'h78,8'h00,8'hA1};
		'd8:	Init_data_reg <= {8'h78,8'h00,8'hA6};
		'd9:	Init_data_reg <= {8'h78,8'h00,8'hA8};
		'd10:	Init_data_reg <= {8'h78,8'h00,8'h3F};
		'd11:	Init_data_reg <= {8'h78,8'h00,8'hC8};
		'd12:	Init_data_reg <= {8'h78,8'h00,8'hD3};
		'd13:	Init_data_reg <= {8'h78,8'h00,8'h00};
		'd14:	Init_data_reg <= {8'h78,8'h00,8'hD5};
		'd15:	Init_data_reg <= {8'h78,8'h00,8'h80};
		'd16:	Init_data_reg <= {8'h78,8'h00,8'hD8};
		'd17:	Init_data_reg <= {8'h78,8'h00,8'h05};
		'd18:	Init_data_reg <= {8'h78,8'h00,8'hD9};
		'd19:	Init_data_reg <= {8'h78,8'h00,8'hF1};
		'd20:	Init_data_reg <= {8'h78,8'h00,8'hDA};
		'd21:	Init_data_reg <= {8'h78,8'h00,8'h12};
		'd22:	Init_data_reg <= {8'h78,8'h00,8'hDB};
		'd23:	Init_data_reg <= {8'h78,8'h00,8'h30};
		'd24:	Init_data_reg <= {8'h78,8'h00,8'h8D};
		'd25:	Init_data_reg <= {8'h78,8'h00,8'h14};
		'd26:	Init_data_reg <= {8'h78,8'h00,8'hAF};
		default:
			Init_data_reg <= {8'h78,8'h00,8'hAE};
		endcase
end








endmodule






