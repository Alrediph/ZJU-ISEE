module Oled_Top_Wrapper(
    input           clk,
    input           rst_n,        // 外部复位
    input  [1:0]    sw,           // 开关
    output          vcc,
    output          scl,
    inout           sda
);

// ====================== SW 变化检测 + 自动复位 ======================
reg [1:0]  sw_reg;
reg        sw_change;
reg        rst_sync;
reg [19:0] rst_cnt;
reg        clear;
// 检测 sw 变化
always @(posedge clk or negedge rst_n) begin
    if(!rst_n) begin
        sw_reg    <= 2'd0;
        sw_change <= 1'b0;
    end else begin
        sw_reg    <= sw;
        sw_change <= (sw_reg != sw && sw != 2'd0);
    end
end

// 复位生成：sw 变化 → 拉低复位一次
always @(posedge clk or negedge rst_n) begin
    if(!rst_n) begin
        rst_sync <= 1'b1;
        rst_cnt  <= 20'd0;
    end
    else if(sw_change) begin
        rst_sync <= 1'b0;
        rst_cnt  <= 20'd0;
    end
    else if(rst_cnt < 20'd100_000) begin
        rst_cnt  <= rst_cnt + 1'b1;
        rst_sync <= 1'b0;
    end
    else begin
        rst_sync <= 1'b1;
    end
end

// ====================== 例化子模块 ======================
Oled_Top Oled_Top_u(
    .clk        (clk),
    .rst        (rst_sync),    // sw 变化会复位
    .sw         (sw),
    .clear_key  (sw[1] | sw[0]),
    .on_key     (1'b0),        // 直接给低电平
    .vcc        (vcc),
    .scl        (scl),
    .sda        (sda)
);

endmodule