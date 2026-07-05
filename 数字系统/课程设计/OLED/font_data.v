




//å­—ç¬¦æ•°æ®ï¼?16*16ã€? ä¸¿é¡¿16ä¸ªæ•°æ®ï¼Œå…±ä¸¤é¡?

module font_data(
	
	input			clk,
	input			rst,
	
	input[1:0]      sw,
	input[5:0]		select,	//å­—ç¬¦é€‰æ‹©
	input[2:0]		page_cur,	//å½“å‰é¡?
	input[5:0]		index_cur,	//å½“å‰è¡?
	
	output[7:0]		data		//æ•°æ®è¾“å‡º
);

reg[7:0]	data1[31:0]; 
reg[7:0]	data2[31:0]; 
reg[7:0]	data3[31:0]; 
reg[7:0]	data4[31:0]; 
reg[7:0]	data5[31:0]; 
reg[7:0]	data6[31:0]; 
reg[7:0]	data7[31:0];
reg[7:0]	data8[31:0];
reg [7:0]   data9 [31:0];
reg [7:0]   data10 [31:0];
reg [7:0]  data11 [31:0];
reg [7:0]  data12 [31:0];


assign data = 	(select == 'd0) ? data1[(index_cur-'d3) + 'd16 *page_cur] : 
				(select == 'd1) ? data2[(index_cur-'d3) + 'd16 *page_cur] : 
				(select == 'd2) ? data3[(index_cur-'d3) + 'd16 *page_cur] : 
				(select == 'd3) ? data4[(index_cur-'d3) + 'd16 *page_cur] :
				(select == 'd4) ? data5[(index_cur-'d3) + 'd16 *page_cur] : 
				(select == 'd5) ? data6[(index_cur-'d3) + 'd16 *page_cur] : 
				(select == 'd6) ? data7[(index_cur-'d3) + 'd16 *page_cur] :
				(select == 'd7) ? data8[(index_cur-'d3) + 'd16 *page_cur] :
				(select == 'd8) ? data9[(index_cur-'d3) + 'd16 *page_cur] :
				(select == 'd9) ? data10[(index_cur-'d3) + 'd16 *page_cur] :
				(select == 'd10) ? data11[(index_cur-'d3) + 'd16 *page_cur] :
				(select == 'd11) ? data12[(index_cur-'d3) + 'd16 *page_cur] :  'd0;

// data1 (µÚ1¡¢2ĞĞDB)
// ¶¨Òå12¸ö32Éî¶È¡¢8Î»¿íµÄÊı×é


always@(posedge clk or negedge rst)
begin
	if(rst == 1'b0)
	begin
	if(sw[0] == 1'b0 && sw[1] == 1'b1) //sad 
    begin
        // data1
        data1[0]  = 8'hFF; data1[1]  = 8'hFF; data1[2]  = 8'hFF; data1[3]  = 8'hFF;
        data1[4]  = 8'hFF; data1[5]  = 8'hFF; data1[6]  = 8'hFF; data1[7]  = 8'hFF;
        data1[8]  = 8'hFF; data1[9]  = 8'hFF; data1[10] = 8'hFF; data1[11] = 8'hFF;
        data1[12] = 8'hFF; data1[13] = 8'hFF; data1[14] = 8'hFF; data1[15] = 8'hFF;
        data1[16] = 8'hFF; data1[17] = 8'hFF; data1[18] = 8'hFF; data1[19] = 8'hFF;
        data1[20] = 8'hFF; data1[21] = 8'hFF; data1[22] = 8'hFF; data1[23] = 8'hFF;
        data1[24] = 8'hFF; data1[25] = 8'hFF; data1[26] = 8'hFF; data1[27] = 8'hFF;
        data1[28] = 8'hFF; data1[29] = 8'hFF; data1[30] = 8'hFF; data1[31] = 8'hFF;
    
        // data2
        data2[0]  = 8'hFF; data2[1]  = 8'hFF; data2[2]  = 8'hFF; data2[3]  = 8'h00;
        data2[4]  = 8'h00; data2[5]  = 8'h00; data2[6]  = 8'h00; data2[7]  = 8'h00;
        data2[8]  = 8'h00; data2[9]  = 8'h00; data2[10] = 8'h00; data2[11] = 8'h00;
        data2[12] = 8'h00; data2[13] = 8'h00; data2[14] = 8'h00; data2[15] = 8'h00;
        data2[16] = 8'hFF; data2[17] = 8'hFF; data2[18] = 8'hFF; data2[19] = 8'h00;
        data2[20] = 8'h00; data2[21] = 8'h00; data2[22] = 8'h00; data2[23] = 8'h00;
        data2[24] = 8'h00; data2[25] = 8'h00; data2[26] = 8'h00; data2[27] = 8'h00;
        data2[28] = 8'h00; data2[29] = 8'h00; data2[30] = 8'h00; data2[31] = 8'h00;
    
        // data3
        data3[0]  = 8'h00; data3[1]  = 8'h00; data3[2]  = 8'h00; data3[3]  = 8'h00;
        data3[4]  = 8'h00; data3[5]  = 8'h00; data3[6]  = 8'h00; data3[7]  = 8'h00;
        data3[8]  = 8'h00; data3[9]  = 8'h00; data3[10] = 8'h00; data3[11] = 8'h00;
        data3[12] = 8'h00; data3[13] = 8'hFF; data3[14] = 8'hFF; data3[15] = 8'hFF;
        data3[16] = 8'h00; data3[17] = 8'h00; data3[18] = 8'h00; data3[19] = 8'h00;
        data3[20] = 8'h00; data3[21] = 8'h00; data3[22] = 8'h00; data3[23] = 8'h00;
        data3[24] = 8'h00; data3[25] = 8'h00; data3[26] = 8'h00; data3[27] = 8'h00;
        data3[28] = 8'h00; data3[29] = 8'hFF; data3[30] = 8'hFF; data3[31] = 8'hFF;
    
        // data4
        data4[0]  = 8'hFF; data4[1]  = 8'hFF; data4[2]  = 8'hFF; data4[3]  = 8'hFF;
        data4[4]  = 8'hFF; data4[5]  = 8'hFF; data4[6]  = 8'hFF; data4[7]  = 8'hFF;
        data4[8]  = 8'hFF; data4[9]  = 8'hFF; data4[10] = 8'hFF; data4[11] = 8'hFF;
        data4[12] = 8'hFF; data4[13] = 8'hFF; data4[14] = 8'hFF; data4[15] = 8'hFF;
        data4[16] = 8'hFF; data4[17] = 8'hFF; data4[18] = 8'hFF; data4[19] = 8'hFF;
        data4[20] = 8'hFF; data4[21] = 8'hFF; data4[22] = 8'hFF; data4[23] = 8'hFF;
        data4[24] = 8'hFF; data4[25] = 8'hFF; data4[26] = 8'hFF; data4[27] = 8'hFF;
        data4[28] = 8'hFF; data4[29] = 8'hFF; data4[30] = 8'hFF; data4[31] = 8'hFF;
    
        // data5
        data5[0]  = 8'h1F; data5[1]  = 8'h1F; data5[2]  = 8'h1F; data5[3]  = 8'h1F;
        data5[4]  = 8'h7F; data5[5]  = 8'hFF; data5[6]  = 8'hFF; data5[7]  = 8'hFF;
        data5[8]  = 8'hFF; data5[9]  = 8'hFF; data5[10] = 8'hFF; data5[11] = 8'hFF;
        data5[12] = 8'h1F; data5[13] = 8'h1F; data5[14] = 8'h1F; data5[15] = 8'h1F;
        data5[16] = 8'h00; data5[17] = 8'h00; data5[18] = 8'h00; data5[19] = 8'h00;
        data5[20] = 8'h00; data5[21] = 8'h01; data5[22] = 8'hFF; data5[23] = 8'hFF;
        data5[24] = 8'hFF; data5[25] = 8'hFF; data5[26] = 8'h03; data5[27] = 8'h00;
        data5[28] = 8'h00; data5[29] = 8'h00; data5[30] = 8'h00; data5[31] = 8'h00;
    
        // data6
        data6[0]  = 8'h1F; data6[1]  = 8'h1F; data6[2]  = 8'h1F; data6[3]  = 8'h00;
        data6[4]  = 8'h00; data6[5]  = 8'h00; data6[6]  = 8'h00; data6[7]  = 8'h00;
        data6[8]  = 8'h00; data6[9]  = 8'h00; data6[10] = 8'h00; data6[11] = 8'h00;
        data6[12] = 8'h00; data6[13] = 8'h00; data6[14] = 8'h00; data6[15] = 8'h00;
        data6[16] = 8'h00; data6[17] = 8'h00; data6[18] = 8'h00; data6[19] = 8'h00;
        data6[20] = 8'h00; data6[21] = 8'h00; data6[22] = 8'h00; data6[23] = 8'h00;
        data6[24] = 8'h00; data6[25] = 8'h00; data6[26] = 8'h00; data6[27] = 8'h00;
        data6[28] = 8'h00; data6[29] = 8'h00; data6[30] = 8'h00; data6[31] = 8'h00;
    
        // data7
        data7[0]  = 8'h00; data7[1]  = 8'h00; data7[2]  = 8'h00; data7[3]  = 8'h00;
        data7[4]  = 8'h00; data7[5]  = 8'h00; data7[6]  = 8'h00; data7[7]  = 8'h00;
        data7[8]  = 8'h00; data7[9]  = 8'h00; data7[10] = 8'h00; data7[11] = 8'h00;
        data7[12] = 8'h00; data7[13] = 8'h1F; data7[14] = 8'h1F; data7[15] = 8'h1F;
        data7[16] = 8'h00; data7[17] = 8'h00; data7[18] = 8'h00; data7[19] = 8'h00;
        data7[20] = 8'h00; data7[21] = 8'h00; data7[22] = 8'h00; data7[23] = 8'h00;
        data7[24] = 8'h00; data7[25] = 8'h00; data7[26] = 8'h00; data7[27] = 8'h00;
        data7[28] = 8'h00; data7[29] = 8'h00; data7[30] = 8'h00; data7[31] = 8'h00;
    
        // data8
        data8[0]  = 8'h1F; data8[1]  = 8'h1F; data8[2]  = 8'h3F; data8[3]  = 8'hFF;
        data8[4]  = 8'hFF; data8[5]  = 8'hFF; data8[6]  = 8'hFF; data8[7]  = 8'hFF;
        data8[8]  = 8'hFF; data8[9]  = 8'h7F; data8[10] = 8'h1F; data8[11] = 8'h1F;
        data8[12] = 8'h1F; data8[13] = 8'h1F; data8[14] = 8'h1F; data8[15] = 8'h1F;
        data8[16] = 8'h00; data8[17] = 8'h00; data8[18] = 8'h00; data8[19] = 8'h03;
        data8[20] = 8'hFF; data8[21] = 8'hFF; data8[22] = 8'hFF; data8[23] = 8'hFF;
        data8[24] = 8'h01; data8[25] = 8'h00; data8[26] = 8'h00; data8[27] = 8'h00;
        data8[28] = 8'h00; data8[29] = 8'h00; data8[30] = 8'h00; data8[31] = 8'h00;
    
        // data9
        data9[0]  = 8'h00; data9[1]  = 8'h00; data9[2]  = 8'h00; data9[3]  = 8'h00;
        data9[4]  = 8'h00; data9[5]  = 8'h00; data9[6]  = 8'hFF; data9[7]  = 8'hFF;
        data9[8]  = 8'hFF; data9[9]  = 8'hFF; data9[10] = 8'h00; data9[11] = 8'h00;
        data9[12] = 8'h00; data9[13] = 8'h00; data9[14] = 8'h00; data9[15] = 8'h00;
        data9[16] = 8'h00; data9[17] = 8'h00; data9[18] = 8'h00; data9[19] = 8'h00;
        data9[20] = 8'h00; data9[21] = 8'h00; data9[22] = 8'h0F; data9[23] = 8'h1F;
        data9[24] = 8'h1F; data9[25] = 8'h01; data9[26] = 8'h00; data9[27] = 8'h00;
        data9[28] = 8'h00; data9[29] = 8'h00; data9[30] = 8'h00; data9[31] = 8'h00;
    
        // data10
        data10[0]  = 8'h00; data10[1]  = 8'h00; data10[2]  = 8'h80; data10[3]  = 8'hC0;
        data10[4]  = 8'hE0; data10[5]  = 8'hF0; data10[6]  = 8'h38; data10[7]  = 8'h3C;
        data10[8]  = 8'h1C; data10[9]  = 8'h0C; data10[10] = 8'h0E; data10[11] = 8'h0E;
        data10[12] = 8'h06; data10[13] = 8'h06; data10[14] = 8'h06; data10[15] = 8'h06;
        data10[16] = 8'h00; data10[17] = 8'h00; data10[18] = 8'h00; data10[19] = 8'h01;
        data10[20] = 8'h01; data10[21] = 8'h00; data10[22] = 8'h00; data10[23] = 8'h00;
        data10[24] = 8'h00; data10[25] = 8'h00; data10[26] = 8'h00; data10[27] = 8'h00;
        data10[28] = 8'h00; data10[29] = 8'h00; data10[30] = 8'h00; data10[31] = 8'h00;
    
        // data11
        data11[0]  = 8'h06; data11[1]  = 8'h06; data11[2]  = 8'h0E; data11[3]  = 8'h0E;
        data11[4]  = 8'h0C; data11[5]  = 8'h1C; data11[6]  = 8'h38; data11[7]  = 8'h30;
        data11[8]  = 8'h70; data11[9]  = 8'hE0; data11[10] = 8'hC0; data11[11] = 8'hC0;
        data11[12] = 8'h00; data11[13] = 8'h00; data11[14] = 8'h00; data11[15] = 8'h00;
        data11[16] = 8'h00; data11[17] = 8'h00; data11[18] = 8'h00; data11[19] = 8'h00;
        data11[20] = 8'h00; data11[21] = 8'h00; data11[22] = 8'h00; data11[23] = 8'h00;
        data11[24] = 8'h00; data11[25] = 8'h00; data11[26] = 8'h01; data11[27] = 8'h01;
        data11[28] = 8'h00; data11[29] = 8'h00; data11[30] = 8'h00; data11[31] = 8'h00;
    
        // data12
        data12[0]  = 8'h00; data12[1]  = 8'h00; data12[2]  = 8'h00; data12[3]  = 8'h00;
        data12[4]  = 8'hFF; data12[5]  = 8'hFF; data12[6]  = 8'hFF; data12[7]  = 8'hFF;
        data12[8]  = 8'h00; data12[9]  = 8'h00; data12[10] = 8'h00; data12[11] = 8'h00;
        data12[12] = 8'h00; data12[13] = 8'h00; data12[14] = 8'h00; data12[15] = 8'h00;
        data12[16] = 8'h00; data12[17] = 8'h00; data12[18] = 8'h00; data12[19] = 8'h00;
        data12[20] = 8'h07; data12[21] = 8'h3F; data12[22] = 8'h1F; data12[23] = 8'h03;
        data12[24] = 8'h00; data12[25] = 8'h00; data12[26] = 8'h00; data12[27] = 8'h00;
        data12[28] = 8'h00; data12[29] = 8'h00; data12[30] = 8'h00; data12[31] = 8'h00;
    end
	else if(sw[0] == 1'b1 && sw[1] == 1'b0 ) // normal
	begin
	data1[0]  = 8'hFF; data1[1]  = 8'hFF; data1[2]  = 8'hFF; data1[3]  = 8'hFF;
            data1[4]  = 8'hFF; data1[5]  = 8'hFF; data1[6]  = 8'hFF; data1[7]  = 8'hFF;
            data1[8]  = 8'hFF; data1[9]  = 8'hFF; data1[10] = 8'hFF; data1[11] = 8'hFF;
            data1[12] = 8'hFF; data1[13] = 8'hFF; data1[14] = 8'hFF; data1[15] = 8'hFF;
            data1[16] = 8'hFF; data1[17] = 8'hFF; data1[18] = 8'hFF; data1[19] = 8'hFF;
            data1[20] = 8'hFF; data1[21] = 8'hFF; data1[22] = 8'hFF; data1[23] = 8'hFF;
            data1[24] = 8'hFF; data1[25] = 8'hFF; data1[26] = 8'hFF; data1[27] = 8'hFF;
            data1[28] = 8'hFF; data1[29] = 8'hFF; data1[30] = 8'hFF; data1[31] = 8'hFF;
            
            // data2
            data2[0]  = 8'hFF; data2[1]  = 8'h00; data2[2]  = 8'h00; data2[3]  = 8'h00;
            data2[4]  = 8'h00; data2[5]  = 8'h00; data2[6]  = 8'h00; data2[7]  = 8'h00;
            data2[8]  = 8'h00; data2[9]  = 8'h00; data2[10] = 8'h00; data2[11] = 8'h00;
            data2[12] = 8'h00; data2[13] = 8'h00; data2[14] = 8'h00; data2[15] = 8'h00;
            data2[16] = 8'hFF; data2[17] = 8'h00; data2[18] = 8'h00; data2[19] = 8'h00;
            data2[20] = 8'h00; data2[21] = 8'h00; data2[22] = 8'h00; data2[23] = 8'h00;
            data2[24] = 8'h00; data2[25] = 8'h00; data2[26] = 8'h00; data2[27] = 8'h00;
            data2[28] = 8'h00; data2[29] = 8'h00; data2[30] = 8'h00; data2[31] = 8'h00;
            
            // data3
            data3[0]  = 8'h00; data3[1]  = 8'h00; data3[2]  = 8'h00; data3[3]  = 8'h00;
            data3[4]  = 8'h00; data3[5]  = 8'h00; data3[6]  = 8'h00; data3[7]  = 8'h00;
            data3[8]  = 8'h00; data3[9]  = 8'h00; data3[10] = 8'h00; data3[11] = 8'h00;
            data3[12] = 8'h00; data3[13] = 8'h00; data3[14] = 8'h00; data3[15] = 8'hFF;
            data3[16] = 8'h00; data3[17] = 8'h00; data3[18] = 8'h00; data3[19] = 8'h00;
            data3[20] = 8'h00; data3[21] = 8'h00; data3[22] = 8'h00; data3[23] = 8'h00;
            data3[24] = 8'h00; data3[25] = 8'h00; data3[26] = 8'h00; data3[27] = 8'h00;
            data3[28] = 8'h00; data3[29] = 8'h00; data3[30] = 8'h00; data3[31] = 8'hFF;
            
            // data4
            data4[0]  = 8'hFF; data4[1]  = 8'hFF; data4[2]  = 8'hFF; data4[3]  = 8'hFF;
            data4[4]  = 8'hFF; data4[5]  = 8'hFF; data4[6]  = 8'hFF; data4[7]  = 8'hFF;
            data4[8]  = 8'hFF; data4[9]  = 8'hFF; data4[10] = 8'hFF; data4[11] = 8'hFF;
            data4[12] = 8'hFF; data4[13] = 8'hFF; data4[14] = 8'hFF; data4[15] = 8'hFF;
            data4[16] = 8'hFF; data4[17] = 8'hFF; data4[18] = 8'hFF; data4[19] = 8'hFF;
            data4[20] = 8'hFF; data4[21] = 8'hFF; data4[22] = 8'hFF; data4[23] = 8'hFF;
            data4[24] = 8'hFF; data4[25] = 8'hFF; data4[26] = 8'hFF; data4[27] = 8'hFF;
            data4[28] = 8'hFF; data4[29] = 8'hFF; data4[30] = 8'hFF; data4[31] = 8'hFF;
            
            // data5
            data5[0]  = 8'h3F; data5[1]  = 8'h3F; data5[2]  = 8'h3F; data5[3]  = 8'h3F;
            data5[4]  = 8'h3F; data5[5]  = 8'h3F; data5[6]  = 8'h3F; data5[7]  = 8'h3F;
            data5[8]  = 8'h3F; data5[9]  = 8'h3F; data5[10] = 8'h3F; data5[11] = 8'h3F;
            data5[12] = 8'h3F; data5[13] = 8'h3F; data5[14] = 8'h3F; data5[15] = 8'h3F;
            data5[16] = 8'h00; data5[17] = 8'h00; data5[18] = 8'h00; data5[19] = 8'h00;
            data5[20] = 8'h00; data5[21] = 8'h00; data5[22] = 8'h00; data5[23] = 8'h00;
            data5[24] = 8'h00; data5[25] = 8'h00; data5[26] = 8'h00; data5[27] = 8'h00;
            data5[28] = 8'h00; data5[29] = 8'h00; data5[30] = 8'h00; data5[31] = 8'h00;
            
            // data6
            data6[0]  = 8'h3F; data6[1]  = 8'h00; data6[2]  = 8'h00; data6[3]  = 8'h00;
            data6[4]  = 8'h00; data6[5]  = 8'h00; data6[6]  = 8'h00; data6[7]  = 8'h00;
            data6[8]  = 8'h00; data6[9]  = 8'h00; data6[10] = 8'h00; data6[11] = 8'h00;
            data6[12] = 8'h00; data6[13] = 8'h00; data6[14] = 8'h00; data6[15] = 8'h00;
            data6[16] = 8'h00; data6[17] = 8'h00; data6[18] = 8'h00; data6[19] = 8'h00;
            data6[20] = 8'h00; data6[21] = 8'h00; data6[22] = 8'h00; data6[23] = 8'h00;
            data6[24] = 8'h00; data6[25] = 8'h00; data6[26] = 8'h00; data6[27] = 8'h00;
            data6[28] = 8'h00; data6[29] = 8'h00; data6[30] = 8'h00; data6[31] = 8'h00;
            
            // data7
            data7[0]  = 8'h00; data7[1]  = 8'h00; data7[2]  = 8'h00; data7[3]  = 8'h00;
            data7[4]  = 8'h00; data7[5]  = 8'h00; data7[6]  = 8'h00; data7[7]  = 8'h00;
            data7[8]  = 8'h00; data7[9]  = 8'h00; data7[10] = 8'h00; data7[11] = 8'h00;
            data7[12] = 8'h00; data7[13] = 8'h00; data7[14] = 8'h00; data7[15] = 8'h3F;
            data7[16] = 8'h00; data7[17] = 8'h00; data7[18] = 8'h00; data7[19] = 8'h00;
            data7[20] = 8'h00; data7[21] = 8'h00; data7[22] = 8'h00; data7[23] = 8'h00;
            data7[24] = 8'h00; data7[25] = 8'h00; data7[26] = 8'h00; data7[27] = 8'h00;
            data7[28] = 8'h00; data7[29] = 8'h00; data7[30] = 8'h00; data7[31] = 8'h00;
            
            // data8
            data8[0]  = 8'h3F; data8[1]  = 8'h3F; data8[2]  = 8'h3F; data8[3]  = 8'h3F;
            data8[4]  = 8'h3F; data8[5]  = 8'h3F; data8[6]  = 8'h3F; data8[7]  = 8'h3F;
            data8[8]  = 8'h3F; data8[9]  = 8'h3F; data8[10] = 8'h3F; data8[11] = 8'h3F;
            data8[12] = 8'h3F; data8[13] = 8'h3F; data8[14] = 8'h3F; data8[15] = 8'h3F;
            data8[16] = 8'h00; data8[17] = 8'h00; data8[18] = 8'h00; data8[19] = 8'h00;
            data8[20] = 8'h00; data8[21] = 8'h00; data8[22] = 8'h00; data8[23] = 8'h00;
            data8[24] = 8'h00; data8[25] = 8'h00; data8[26] = 8'h00; data8[27] = 8'h00;
            data8[28] = 8'h00; data8[29] = 8'h00; data8[30] = 8'h00; data8[31] = 8'h00;
            
            // data9
            data9[0]  = 8'h00; data9[1]  = 8'h00; data9[2]  = 8'h00; data9[3]  = 8'h00;
            data9[4]  = 8'h00; data9[5]  = 8'h00; data9[6]  = 8'h00; data9[7]  = 8'h00;
            data9[8]  = 8'h00; data9[9]  = 8'h00; data9[10] = 8'h00; data9[11] = 8'hFF;
            data9[12] = 8'hFF; data9[13] = 8'hFF; data9[14] = 8'hFF; data9[15] = 8'hFF;
            data9[16] = 8'h00; data9[17] = 8'h00; data9[18] = 8'h00; data9[19] = 8'h00;
            data9[20] = 8'h00; data9[21] = 8'h00; data9[22] = 8'h00; data9[23] = 8'h00;
            data9[24] = 8'h00; data9[25] = 8'h00; data9[26] = 8'h00; data9[27] = 8'h07;
            data9[28] = 8'h07; data9[29] = 8'h07; data9[30] = 8'h07; data9[31] = 8'h07;
            
            // data10
            data10[0]  = 8'hFF; data10[1]  = 8'hFF; data10[2]  = 8'hFF; data10[3]  = 8'hFF;
            data10[4]  = 8'hFF; data10[5]  = 8'hFF; data10[6]  = 8'hFF; data10[7]  = 8'hFF;
            data10[8]  = 8'hFF; data10[9]  = 8'hFF; data10[10] = 8'hFF; data10[11] = 8'hFF;
            data10[12] = 8'hFF; data10[13] = 8'hFF; data10[14] = 8'hFF; data10[15] = 8'hFF;
            data10[16] = 8'h07; data10[17] = 8'h07; data10[18] = 8'h07; data10[19] = 8'h07;
            data10[20] = 8'h07; data10[21] = 8'h07; data10[22] = 8'h07; data10[23] = 8'h07;
            data10[24] = 8'h07; data10[25] = 8'h07; data10[26] = 8'h07; data10[27] = 8'h07;
            data10[28] = 8'h07; data10[29] = 8'h07; data10[30] = 8'h07; data10[31] = 8'h07;
            
            // data11
            data11[0]  = 8'hFF; data11[1]  = 8'hFF; data11[2]  = 8'hFF; data11[3]  = 8'hFF;
            data11[4]  = 8'hFF; data11[5]  = 8'hFF; data11[6]  = 8'hFF; data11[7]  = 8'hFF;
            data11[8]  = 8'hFF; data11[9]  = 8'hFF; data11[10] = 8'hFF; data11[11] = 8'hFF;
            data11[12] = 8'hFF; data11[13] = 8'hFF; data11[14] = 8'hFF; data11[15] = 8'hFF;
            data11[16] = 8'h07; data11[17] = 8'h07; data11[18] = 8'h07; data11[19] = 8'h07;
            data11[20] = 8'h07; data11[21] = 8'h07; data11[22] = 8'h07; data11[23] = 8'h07;
            data11[24] = 8'h07; data11[25] = 8'h07; data11[26] = 8'h07; data11[27] = 8'h07;
            data11[28] = 8'h07; data11[29] = 8'h07; data11[30] = 8'h07; data11[31] = 8'h07;
            
            // data12
            data12[0]  = 8'hFF; data12[1]  = 8'hFF; data12[2]  = 8'hFF; data12[3]  = 8'hFF;
            data12[4]  = 8'h00; data12[5]  = 8'h00; data12[6]  = 8'h00; data12[7]  = 8'h00;
            data12[8]  = 8'h00; data12[9]  = 8'h00; data12[10] = 8'h00; data12[11] = 8'h00;
            data12[12] = 8'h00; data12[13] = 8'h00; data12[14] = 8'h00; data12[15] = 8'h00;
            data12[16] = 8'h07; data12[17] = 8'h07; data12[18] = 8'h07; data12[19] = 8'h07;
            data12[20] = 8'h00; data12[21] = 8'h00; data12[22] = 8'h00; data12[23] = 8'h00;
            data12[24] = 8'h00; data12[25] = 8'h00; data12[26] = 8'h00; data12[27] = 8'h00;
            data12[28] = 8'h00; data12[29] = 8'h00; data12[30] = 8'h00; data12[31] = 8'h00;
            end	
        else if (sw[0] == 1'b1 && sw[1] == 1'b1) // happy
        begin
		// data1 (Ä©Î²Êı×Ö1)
        data1[0]  = 8'hFF; data1[1]  = 8'hFF; data1[2]  = 8'hFF; data1[3]  = 8'hFF;
        data1[4]  = 8'hFF; data1[5]  = 8'hFF; data1[6]  = 8'hFF; data1[7]  = 8'hFF;
        data1[8]  = 8'hFF; data1[9]  = 8'hFF; data1[10] = 8'hFF; data1[11] = 8'hFF;
        data1[12] = 8'hFF; data1[13] = 8'hFF; data1[14] = 8'hFF; data1[15] = 8'hFF;
        data1[16] = 8'hFF; data1[17] = 8'hFF; data1[18] = 8'hFF; data1[19] = 8'hFF;
        data1[20] = 8'hFF; data1[21] = 8'hFF; data1[22] = 8'hFF; data1[23] = 8'hFF;
        data1[24] = 8'hFF; data1[25] = 8'hFF; data1[26] = 8'hFF; data1[27] = 8'hFF;
        data1[28] = 8'hFF; data1[29] = 8'hFF; data1[30] = 8'hFF; data1[31] = 8'hFF;
        
        // data2 (Ä©Î²Êı×Ö2)
        data2[0]  = 8'hFF; data2[1]  = 8'hFF; data2[2]  = 8'hFF; data2[3]  = 8'hFF;
        data2[4]  = 8'h00; data2[5]  = 8'h00; data2[6]  = 8'h00; data2[7]  = 8'h00;
        data2[8]  = 8'h00; data2[9]  = 8'h00; data2[10] = 8'h00; data2[11] = 8'h00;
        data2[12] = 8'h00; data2[13] = 8'h00; data2[14] = 8'h00; data2[15] = 8'h00;
        data2[16] = 8'hFF; data2[17] = 8'hFF; data2[18] = 8'hFF; data2[19] = 8'hFF;
        data2[20] = 8'h00; data2[21] = 8'h00; data2[22] = 8'h00; data2[23] = 8'h00;
        data2[24] = 8'h00; data2[25] = 8'h00; data2[26] = 8'h00; data2[27] = 8'h00;
        data2[28] = 8'h00; data2[29] = 8'h00; data2[30] = 8'h00; data2[31] = 8'h00;
        
        // data3 (Ä©Î²Êı×Ö3)
        data3[0]  = 8'h00; data3[1]  = 8'h00; data3[2]  = 8'h00; data3[3]  = 8'h00;
        data3[4]  = 8'h00; data3[5]  = 8'h00; data3[6]  = 8'h00; data3[7]  = 8'h00;
        data3[8]  = 8'h00; data3[9]  = 8'h00; data3[10] = 8'h00; data3[11] = 8'h00;
        data3[12] = 8'hFF; data3[13] = 8'hFF; data3[14] = 8'hFF; data3[15] = 8'hFF;
        data3[16] = 8'h00; data3[17] = 8'h00; data3[18] = 8'h00; data3[19] = 8'h00;
        data3[20] = 8'h00; data3[21] = 8'h00; data3[22] = 8'h00; data3[23] = 8'h00;
        data3[24] = 8'h00; data3[25] = 8'h00; data3[26] = 8'h00; data3[27] = 8'h00;
        data3[28] = 8'hFF; data3[29] = 8'hFF; data3[30] = 8'hFF; data3[31] = 8'hFF;
        
        // data4 (Ä©Î²Êı×Ö4)
        data4[0]  = 8'hFF; data4[1]  = 8'hFF; data4[2]  = 8'hFF; data4[3]  = 8'hFF;
        data4[4]  = 8'hFF; data4[5]  = 8'hFF; data4[6]  = 8'hFF; data4[7]  = 8'hFF;
        data4[8]  = 8'hFF; data4[9]  = 8'hFF; data4[10] = 8'hFF; data4[11] = 8'hFF;
        data4[12] = 8'hFF; data4[13] = 8'hFF; data4[14] = 8'hFF; data4[15] = 8'hFF;
        data4[16] = 8'hFF; data4[17] = 8'hFF; data4[18] = 8'hFF; data4[19] = 8'hFF;
        data4[20] = 8'hFF; data4[21] = 8'hFF; data4[22] = 8'hFF; data4[23] = 8'hFF;
        data4[24] = 8'hFF; data4[25] = 8'hFF; data4[26] = 8'hFF; data4[27] = 8'hFF;
        data4[28] = 8'hFF; data4[29] = 8'hFF; data4[30] = 8'hFF; data4[31] = 8'hFF;
        
        // data5 (Ä©Î²Êı×Ö5)
        data5[0]  = 8'h7F; data5[1]  = 8'h7F; data5[2]  = 8'h7F; data5[3]  = 8'h7F;
        data5[4]  = 8'h7F; data5[5]  = 8'h7F; data5[6]  = 8'h7F; data5[7]  = 8'h7F;
        data5[8]  = 8'h7F; data5[9]  = 8'h7F; data5[10] = 8'h7F; data5[11] = 8'h7F;
        data5[12] = 8'h7F; data5[13] = 8'h7F; data5[14] = 8'h7F; data5[15] = 8'h7F;
        data5[16] = 8'h00; data5[17] = 8'h00; data5[18] = 8'h00; data5[19] = 8'h00;
        data5[20] = 8'h00; data5[21] = 8'h00; data5[22] = 8'h00; data5[23] = 8'h00;
        data5[24] = 8'h00; data5[25] = 8'h00; data5[26] = 8'h00; data5[27] = 8'h00;
        data5[28] = 8'h00; data5[29] = 8'h00; data5[30] = 8'h00; data5[31] = 8'h00;
        
        // data6 (Ä©Î²Êı×Ö6)
        data6[0]  = 8'h7F; data6[1]  = 8'h7F; data6[2]  = 8'h7F; data6[3]  = 8'h7F;
        data6[4]  = 8'h00; data6[5]  = 8'h00; data6[6]  = 8'h00; data6[7]  = 8'h00;
        data6[8]  = 8'h00; data6[9]  = 8'h00; data6[10] = 8'h00; data6[11] = 8'h00;
        data6[12] = 8'h00; data6[13] = 8'h00; data6[14] = 8'h00; data6[15] = 8'h00;
        data6[16] = 8'h00; data6[17] = 8'h00; data6[18] = 8'h00; data6[19] = 8'h00;
        data6[20] = 8'h00; data6[21] = 8'h00; data6[22] = 8'h00; data6[23] = 8'h00;
        data6[24] = 8'h00; data6[25] = 8'h00; data6[26] = 8'h00; data6[27] = 8'h00;
        data6[28] = 8'h00; data6[29] = 8'h00; data6[30] = 8'h00; data6[31] = 8'h00;
        
        // data7 (Ä©Î²Êı×Ö7)
        data7[0]  = 8'h00; data7[1]  = 8'h00; data7[2]  = 8'h00; data7[3]  = 8'h00;
        data7[4]  = 8'h00; data7[5]  = 8'h00; data7[6]  = 8'h00; data7[7]  = 8'h00;
        data7[8]  = 8'h00; data7[9]  = 8'h00; data7[10] = 8'h00; data7[11] = 8'h00;
        data7[12] = 8'h7F; data7[13] = 8'h7F; data7[14] = 8'h7F; data7[15] = 8'h7F;
        data7[16] = 8'h00; data7[17] = 8'h00; data7[18] = 8'h00; data7[19] = 8'h00;
        data7[20] = 8'h00; data7[21] = 8'h00; data7[22] = 8'h00; data7[23] = 8'h00;
        data7[24] = 8'h00; data7[25] = 8'h00; data7[26] = 8'h00; data7[27] = 8'h00;
        data7[28] = 8'h00; data7[29] = 8'h00; data7[30] = 8'h00; data7[31] = 8'hC0;
        
        // data8 (Ä©Î²Êı×Ö8)
        data8[0]  = 8'h7F; data8[1]  = 8'h7F; data8[2]  = 8'h7F; data8[3]  = 8'h7F;
        data8[4]  = 8'h7F; data8[5]  = 8'h7F; data8[6]  = 8'h7F; data8[7]  = 8'h7F;
        data8[8]  = 8'h7F; data8[9]  = 8'h7F; data8[10] = 8'h7F; data8[11] = 8'h7F;
        data8[12] = 8'h7F; data8[13] = 8'h7F; data8[14] = 8'h7F; data8[15] = 8'h7F;
        data8[16] = 8'hC0; data8[17] = 8'h00; data8[18] = 8'h00; data8[19] = 8'h00;
        data8[20] = 8'h00; data8[21] = 8'h00; data8[22] = 8'h00; data8[23] = 8'h00;
        data8[24] = 8'h00; data8[25] = 8'h00; data8[26] = 8'h00; data8[27] = 8'h00;
        data8[28] = 8'h00; data8[29] = 8'h00; data8[30] = 8'h00; data8[31] = 8'h00;
        
        // data9 (Ä©Î²Êı×Ö9)
        data9[0]  = 8'h00; data9[1]  = 8'h00; data9[2]  = 8'h00; data9[3]  = 8'h00;
        data9[4]  = 8'h00; data9[5]  = 8'h00; data9[6]  = 8'h00; data9[7]  = 8'h00;
        data9[8]  = 8'h00; data9[9]  = 8'h00; data9[10] = 8'h00; data9[11] = 8'h00;
        data9[12] = 8'h00; data9[13] = 8'h00; data9[14] = 8'h01; data9[15] = 8'h07;
        data9[16] = 8'h00; data9[17] = 8'h00; data9[18] = 8'h00; data9[19] = 8'h00;
        data9[20] = 8'h00; data9[21] = 8'h00; data9[22] = 8'h00; data9[23] = 8'h00;
        data9[24] = 8'h00; data9[25] = 8'h00; data9[26] = 8'h00; data9[27] = 8'h00;
        data9[28] = 8'h00; data9[29] = 8'h00; data9[30] = 8'h00; data9[31] = 8'h00;
        
        // data10 (Ä©Î²Êı×Ö10)
        data10[0]  = 8'h0F; data10[1]  = 8'h1C; data10[2]  = 8'h3C; data10[3]  = 8'h38;
        data10[4]  = 8'h70; data10[5]  = 8'h60; data10[6]  = 8'hE0; data10[7]  = 8'hC0;
        data10[8]  = 8'hC0; data10[9]  = 8'hC0; data10[10] = 8'h80; data10[11] = 8'h80;
        data10[12] = 8'h80; data10[13] = 8'h80; data10[14] = 8'h80; data10[15] = 8'h80;
        data10[16] = 8'h00; data10[17] = 8'h00; data10[18] = 8'h00; data10[19] = 8'h00;
        data10[20] = 8'h00; data10[21] = 8'h00; data10[22] = 8'h00; data10[23] = 8'h00;
        data10[24] = 8'h01; data10[25] = 8'h01; data10[26] = 8'h01; data10[27] = 8'h01;
        data10[28] = 8'h01; data10[29] = 8'h01; data10[30] = 8'h01; data10[31] = 8'h01;
        
        // data11 (Ä©Î²Êı×Ö11)
        data11[0]  = 8'h80; data11[1]  = 8'h80; data11[2]  = 8'h80; data11[3]  = 8'h80;
        data11[4]  = 8'h80; data11[5]  = 8'h80; data11[6]  = 8'hC0; data11[7]  = 8'hC0;
        data11[8]  = 8'hE0; data11[9]  = 8'hE0; data11[10] = 8'h70; data11[11] = 8'h78;
        data11[12] = 8'h3C; data11[13] = 8'h1E; data11[14] = 8'h0F; data11[15] = 8'h0F;
        data11[16] = 8'h01; data11[17] = 8'h01; data11[18] = 8'h01; data11[19] = 8'h01;
        data11[20] = 8'h01; data11[21] = 8'h01; data11[22] = 8'h01; data11[23] = 8'h01;
        data11[24] = 8'h00; data11[25] = 8'h00; data11[26] = 8'h00; data11[27] = 8'h00;
        data11[28] = 8'h00; data11[29] = 8'h00; data11[30] = 8'h00; data11[31] = 8'h00;
        
        // data12 (Ä©Î²Êı×Ö12)
        data12[0]  = 8'h03; data12[1]  = 8'h00; data12[2]  = 8'h00; data12[3]  = 8'h00;
        data12[4]  = 8'h00; data12[5]  = 8'h00; data12[6]  = 8'h00; data12[7]  = 8'h00;
        data12[8]  = 8'h00; data12[9]  = 8'h00; data12[10] = 8'h00; data12[11] = 8'h00;
        data12[12] = 8'h00; data12[13] = 8'h00; data12[14] = 8'h00; data12[15] = 8'h00;
        data12[16] = 8'h00; data12[17] = 8'h00; data12[18] = 8'h00; data12[19] = 8'h00;
        data12[20] = 8'h00; data12[21] = 8'h00; data12[22] = 8'h00; data12[23] = 8'h00;
        data12[24] = 8'h00; data12[25] = 8'h00; data12[26] = 8'h00; data12[27] = 8'h00;
        data12[28] = 8'h00; data12[29] = 8'h00; data12[30] = 8'h00; data12[31] = 8'h00;
        end 
	end
end
endmodule 