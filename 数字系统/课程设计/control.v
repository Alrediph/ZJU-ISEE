module pid_controller #(
    parameter KP = 16'd256,
    parameter KI = 16'd128,
    parameter KD = 16'd64,
    parameter OUT_MAX = 16'd300,
    parameter OUT_MIN = -16'd300
)(
    input  wire        clk,
    input  wire        rst,
    input  wire        update,
    input  wire signed [23:0] target,
    input  wire signed [23:0] actual,
    output reg  signed [15:0] output_ctrl
);

reg signed [23:0] integral;
reg signed [23:0] last_err;
reg signed [31:0] p_term, i_term, d_term;
reg signed [24:0] err;

always @(posedge clk or posedge rst) begin
    if(rst) begin
        integral <= 0;
        last_err <= 0;
        output_ctrl <= 0;
    end else if(update) begin
        err = target - actual;
        p_term = err * $signed(KP);
        integral <= integral + err;
        i_term = integral * $signed(KI);
        d_term = (err - last_err) * $signed(KD);
        last_err <= err;
        output_ctrl <= $signed( (p_term + i_term + d_term) >>> 8 );
        if(output_ctrl > OUT_MAX) output_ctrl <= OUT_MAX;
        if(output_ctrl < OUT_MIN) output_ctrl <= OUT_MIN;
    end
end

endmodule

//=============================================================================
// 顶层模块
//=============================================================================
module car_follow #(
    parameter CLK_FREQ     = 100_000_000,
    parameter DESIRED_CNT  = 294118,
    parameter PWM_FREQ     = 20000
)(
    input  wire       clk,
    input  wire       rst,
    input  wire [1:0] mode,
    output wire       trig_left,
    input  wire       echo_left,
    output wire       trig_right,
    input  wire       echo_right,
    output wire       pwm_left,      // 改为 wire
    output reg        in1_left,
    output reg        in2_left,
    output wire       pwm_right,     // 改为 wire
    output reg        in1_right,
    output reg        in2_right,
    output reg     [1:0] mode_code
);

//-----------------------------------------------------------------------
// 1kHz 时钟分频
//-----------------------------------------------------------------------
reg [16:0] clk_cnt;
reg        clk_1kHz;
always @(posedge clk or posedge rst) begin
    if(rst) begin
        clk_cnt <= 0;
        clk_1kHz <= 0;
    end else if(clk_cnt >= CLK_FREQ/1000 - 1) begin
        clk_cnt <= 0;
        clk_1kHz <= 1;
    end else begin
        clk_cnt <= clk_cnt + 1'b1;
        clk_1kHz <= 0;
    end
end

//-----------------------------------------------------------------------
// 超声波测距状态机
//-----------------------------------------------------------------------
localparam [3:0] 
    IDLE          = 4'd0,
    TRIG_LEFT     = 4'd1,
    WAIT_ECHO_L   = 4'd2,
    MEASURE_L     = 4'd3,
    STORE_L       = 4'd4,
    TRIG_RIGHT    = 4'd5,
    WAIT_ECHO_R   = 4'd6,
    MEASURE_R     = 4'd7,
    STORE_R       = 4'd8,
    WAIT_50MS     = 4'd9;

reg [3:0]  state;
//reg [23:0] echo_cnt;
//reg [23:0] left_cnt, right_cnt;
//reg [11:0] wait_cnt;
reg        trig_left_r, trig_right_r;
reg        echo_left_ff, echo_left_ff2;
reg        echo_right_ff, echo_right_ff2;
wire       echo_left_rise, echo_left_fall;
wire       echo_right_rise, echo_right_fall;

always @(posedge clk or posedge rst) begin
    if(rst) begin
        echo_left_ff  <= 0;
        echo_left_ff2 <= 0;
        echo_right_ff <= 0;
        echo_right_ff2 <= 0;
    end else begin
       
        echo_left_ff  <= echo_left;
        echo_left_ff2 <= echo_left_ff;
       
        echo_right_ff <= echo_right;
        echo_right_ff2 <= echo_right_ff;
    end
end
assign echo_left_rise  = echo_left_ff  & ~echo_left_ff2;
assign echo_left_fall  = ~echo_left_ff & echo_left_ff2;
assign echo_right_rise = echo_right_ff & ~echo_right_ff2;
assign echo_right_fall = ~echo_right_ff & echo_right_ff2;
reg [26:0] echo_cnt;        // 从 23位 改为 27位
reg [26:0] left_cnt, right_cnt;
reg [26:0] wait_cnt;        // 从 12位 改为 27位

always @(posedge clk or posedge rst) begin
    if(rst) begin
        state <= IDLE;
        wait_cnt <= 0;
        echo_cnt <= 0;
        trig_left_r <= 0;
        trig_right_r <= 0;
        left_cnt <= 0;
        right_cnt <= 0;
    end else begin
        case(state)
            IDLE: begin
                trig_left_r <= 0;
                trig_right_r <= 0;
                state <= TRIG_LEFT;
            end
            TRIG_LEFT: begin
                if(wait_cnt < 12'd1000) begin
                    trig_left_r <= 1;
                    wait_cnt <= wait_cnt + 1;
                end else begin
                    trig_left_r <= 0;
                    wait_cnt <= 0;
                    state <= WAIT_ECHO_L;
                    echo_cnt <= 0;
                end
            end
            WAIT_ECHO_L: begin
                if(echo_left_rise) begin
                    state <= MEASURE_L;
                    wait_cnt <= 0;
                    echo_cnt <= 0;
                end else if(wait_cnt >= 12'd3000) begin
                    state <= TRIG_RIGHT;
                    wait_cnt <= 0;
                end else begin
                    if(clk_1kHz) wait_cnt <= wait_cnt + 1;
                end
            end
            MEASURE_L: begin
                echo_cnt <= echo_cnt + 1;
                if(echo_left_fall) begin
                    left_cnt <= echo_cnt;
                    state <= STORE_L;
                    wait_cnt <= 0;
                end else if(echo_cnt >= 24'd3_000_000) begin
                    state <= TRIG_RIGHT;
                end
            end
            STORE_L: begin
                state <= TRIG_RIGHT;
            end
            TRIG_RIGHT: begin
                if(wait_cnt < 12'd1000) begin
                    trig_right_r <= 1;
                    wait_cnt <= wait_cnt + 1;
                end else begin
                    trig_right_r <= 0;
                    wait_cnt <= 0;
                    state <= WAIT_ECHO_R;
                    echo_cnt <= 0;
                end
            end
            WAIT_ECHO_R: begin
                if(echo_right_rise) begin
                    state <= MEASURE_R;
                    wait_cnt <= 0;
                    echo_cnt <= 0;
                end else if(wait_cnt >= 12'd3000) begin
                    state <= WAIT_50MS;
                    wait_cnt <= 0;
                end else begin
                    if(clk_1kHz) wait_cnt <= wait_cnt + 1;
                end
            end
            MEASURE_R: begin
                echo_cnt <= echo_cnt + 1;
                if(echo_right_fall) begin
                    right_cnt <= echo_cnt;
                    state <= STORE_R;
                end else if(echo_cnt >= 24'd3_000_000) begin
                    state <= WAIT_50MS;
                end
            end
            STORE_R: begin
                state <= WAIT_50MS;
            end
            WAIT_50MS: begin
                if(wait_cnt >= 12'd50) begin
                    wait_cnt <= 0;
                    state <= IDLE;
                end else if(clk_1kHz) begin
                    wait_cnt <= wait_cnt + 1;
                end
            end
            default: state <= IDLE;
        endcase
    end
end

// 修改信号声明（增大位宽）

/*
// 修改状态机中的计数
always @(posedge clk or posedge rst) begin
    if(rst) begin
        state <= IDLE;
        wait_cnt <= 0;
        echo_cnt <= 0;
        trig_left_r <= 0;
        trig_right_r <= 0;
        left_cnt <= 0;
        right_cnt <= 0;
    end else begin
        case(state)
            IDLE: begin
                trig_left_r <= 0;
                trig_right_r <= 0;
                state <= TRIG_LEFT;
            end
            
            // 修改：TRIG 脉冲 10μs = 1000 个时钟周期
            TRIG_LEFT: begin
                if(wait_cnt < 27'd1000) begin
                    trig_left_r <= 1;
                    wait_cnt <= wait_cnt + 1;
                end else begin
                    trig_left_r <= 0;
                    wait_cnt <= 0;
                    state <= WAIT_ECHO_L;
                    echo_cnt <= 0;
                end
            end
            
            // 修改：超时 30ms = 3,000,000 个时钟周期（去掉 clk_1kHz 条件）
            WAIT_ECHO_L: begin
                if(echo_left_rise) begin
                    state <= MEASURE_L;
                    wait_cnt <= 0;
                    echo_cnt <= 0;
                end else if(wait_cnt >= 27'd3_000_000) begin
                    state <= TRIG_RIGHT;
                    wait_cnt <= 0;
                end else begin
                    wait_cnt <= wait_cnt + 1;  // 去掉 clk_1kHz 条件
                end
            end
            
            // 修改：最大测量 30ms = 3,000,000 个时钟周期
            MEASURE_L: begin
                echo_cnt <= echo_cnt + 1;
                if(echo_left_fall) begin
                    left_cnt <= echo_cnt;
                    state <= STORE_L;
                    wait_cnt <= 0;
                end else if(echo_cnt >= 27'd3_000_000) begin
                    state <= TRIG_RIGHT;
                end
            end
            
            STORE_L: begin
                state <= TRIG_RIGHT;
            end
            
            // 修改：TRIG 脉冲 10μs = 1000 个时钟周期
            TRIG_RIGHT: begin
                if(wait_cnt < 27'd1000) begin
                    trig_right_r <= 1;
                    wait_cnt <= wait_cnt + 1;
                end else begin
                    trig_right_r <= 0;
                    wait_cnt <= 0;
                    state <= WAIT_ECHO_R;
                    echo_cnt <= 0;
                end
            end
            
            // 修改：超时 30ms = 3,000,000 个时钟周期（去掉 clk_1kHz 条件）
            WAIT_ECHO_R: begin
                if(echo_right_rise) begin
                    state <= MEASURE_R;
                    wait_cnt <= 0;
                    echo_cnt <= 0;
                end else if(wait_cnt >= 27'd3_000_000) begin
                    state <= WAIT_50MS;
                    wait_cnt <= 0;
                end else begin
                    wait_cnt <= wait_cnt + 1;  // 去掉 clk_1kHz 条件
                end
            end
            
            // 修改：最大测量 30ms = 3,000,000 个时钟周期
            MEASURE_R: begin
                echo_cnt <= echo_cnt + 1;
                if(echo_right_fall) begin
                    right_cnt <= echo_cnt;
                    state <= STORE_R;
                end else if(echo_cnt >= 27'd3_000_000) begin
                    state <= WAIT_50MS;
                end
            end
            
            STORE_R: begin
                state <= WAIT_50MS;
            end
            
            // 修改：50ms = 5,000,000 个时钟周期（去掉 clk_1kHz 条件）
            WAIT_50MS: begin
                if(wait_cnt >= 27'd5_000_000) begin
                    wait_cnt <= 0;
                    state <= IDLE;
                end else begin
                    wait_cnt <= wait_cnt + 1;  // 去掉 clk_1kHz 条件
                end
            end
            
            default: state <= IDLE;
        endcase
    end
end*/
assign trig_left  = trig_left_r;
assign trig_right = trig_right_r;

//-----------------------------------------------------------------------
// 模式控制状态机（边沿触发，动作执行一次后自动回到跟随）
//-----------------------------------------------------------------------
localparam [2:0]
    MODE_STOP    = 3'd0,//00
    MODE_FOLLOW  = 3'd1,//01 normal
    MODE_FWD_REV = 3'd2,//10 sad
    MODE_SPIN    = 3'd3;//11 happy

reg [2:0] current_mode;
reg [15:0] timer;
reg [1:0] cycle_cnt;
reg fwd_rev_state;
reg action_busy;
reg [1:0] last_mode;

always @(posedge clk or posedge rst) begin
    if(rst) begin
        current_mode <= MODE_FOLLOW;
        mode_code <= current_mode;
        timer <= 0;
        cycle_cnt <= 0;
        fwd_rev_state <= 0;
        action_busy <= 0;
        last_mode <= 2'b11;
    end else begin
        last_mode <= mode;
        if(!action_busy) begin
            case(mode)
                2'b00:  begin current_mode <= MODE_STOP;
                        mode_code <= MODE_STOP;end
                2'b01: 
                        if(last_mode != 2'b01) begin
                            current_mode <= MODE_FWD_REV;
                            mode_code <= MODE_FWD_REV;
                            action_busy <= 1;
                            timer <= 0;
                            cycle_cnt <= 0;
                            fwd_rev_state <= 0;
                        end
                2'b10: if(last_mode != 2'b10) begin
                            current_mode <= MODE_SPIN;
                            mode_code <= MODE_SPIN;
                            action_busy <= 1;
                            timer <= 0;
                        end
                2'b11: begin current_mode <= MODE_FOLLOW;
                       mode_code <= MODE_FOLLOW;end
                default: begin current_mode <= MODE_FOLLOW;
                        mode_code <= MODE_FOLLOW;end
            endcase
        end
        
        case(current_mode)
            MODE_FWD_REV: begin
                if(timer >= 2000) begin
                    timer <= 0;
                    if(fwd_rev_state == 0) fwd_rev_state <= 1;
                    else begin
                        fwd_rev_state <= 0;
                        if(cycle_cnt < 2) cycle_cnt <= cycle_cnt + 1;
                    end
                end else if(clk_1kHz) timer <= timer + 1;
                
                if(cycle_cnt == 2 && fwd_rev_state == 1 && timer >= 2000) begin
                    current_mode <= MODE_FOLLOW;
                    action_busy <= 0;
                    cycle_cnt <= 0;
                    fwd_rev_state <= 0;
                end
            end
            MODE_SPIN: begin
                if(timer >= 10000) begin
                    current_mode <= MODE_FOLLOW;
                    action_busy <= 0;
                    timer <= 0;
                end else if(clk_1kHz) timer <= timer + 1;
            end
            default: ;
        endcase
        
        if(current_mode != MODE_FWD_REV && current_mode != MODE_SPIN)
            action_busy <= 0;
    end
end


//----------------------------------------------------------------------
// PID 相关信号
//-----------------------------------------------------------------------
reg        pid_update;
reg [23:0] avg_cnt;
reg signed [15:0] speed_ctrl, turn_ctrl;
reg signed [23:0] diff_signed;
wire signed [15:0] speed_pid_out, turn_pid_out;

reg old_state;
always @(posedge clk or posedge rst) begin
    if(rst) begin
        pid_update <= 0;
        old_state <= 0;
    end else begin
        old_state <= (state == WAIT_50MS);
        if((state == WAIT_50MS) && !old_state) pid_update <= 1;
        else pid_update <= 0;
    end
end

always @(posedge clk or posedge rst) begin
    if(rst) begin
        avg_cnt <= 0;
        diff_signed <= 0;
    end else begin
        avg_cnt <= (left_cnt + right_cnt) >> 2;
        //avg_cnt <= (left_cnt + right_cnt) >> 1;
        diff_signed <= $signed({1'b0, right_cnt}) - $signed({1'b0, left_cnt});
    end
end

pid_controller #(.KP(512), .KI(128), .KD(256), .OUT_MAX(200), .OUT_MIN(-200)) u_speed_pid (
    .clk(clk), .rst(rst), .update(pid_update),
    .target(DESIRED_CNT), .actual(avg_cnt), .output_ctrl(speed_pid_out)
);

pid_controller #(.KP(384), .KI(64), .KD(192), .OUT_MAX(150), .OUT_MIN(-150)) u_turn_pid (
    .clk(clk), .rst(rst), .update(pid_update),
    .target(24'd0), .actual(diff_signed), .output_ctrl(turn_pid_out)
);

always @(posedge clk or posedge rst) begin
    if(rst) begin
        speed_ctrl <= 0;
        turn_ctrl <= 0;
    end else if(pid_update && current_mode == MODE_FOLLOW) begin
        speed_ctrl <= speed_pid_out;
        turn_ctrl <= turn_pid_out;
    end else if(current_mode != MODE_FOLLOW) begin
        speed_ctrl <= 0;
        turn_ctrl <= 0;
    end
end

//-----------------------------------------------------------------------
// 电机控制：根据模式计算 left_duty, right_duty（带符号）
//-----------------------------------------------------------------------
reg signed [8:0] left_duty, right_duty;
reg [7:0] pwm_left_val, pwm_right_val;
localparam ACT_SPEED = 8'd150;

always @(*) begin
    left_duty = 0;
    right_duty = 0;
    case(current_mode)
        MODE_STOP: ;
        MODE_FOLLOW: begin
           
            left_duty = speed_ctrl + turn_ctrl;
            
            right_duty = speed_ctrl - turn_ctrl;
        end
        MODE_FWD_REV: begin
            if(fwd_rev_state == 0) begin  // 前进
                left_duty = ACT_SPEED;
                right_duty = ACT_SPEED;
            end else begin                // 后退
                left_duty = -ACT_SPEED;
                right_duty = -ACT_SPEED;
            end
        end
        MODE_SPIN: begin
            left_duty = ACT_SPEED;        // 左前进
            right_duty = -ACT_SPEED;      // 右后退 -> 顺时针转圈
        end
        default: ;
    endcase
end

// 将 duty 转换为 PWM 占空比和 IN1/IN2 信号（单一驱动）
always @(*) begin
    pwm_left_val = 0;
    pwm_right_val = 0;
    in1_left = 0;  in2_left = 0;
    in1_right = 0; in2_right = 0;
    
    // 左侧电机
    if(left_duty > 8'd255) begin
        pwm_left_val = 8'd255;
        in1_left = 1; in2_left = 0;
    end else if(left_duty < -8'd255) begin
        pwm_left_val = 8'd255;
        in1_left = 0; in2_left = 1;
    end else if(left_duty > 0) begin
        pwm_left_val = left_duty[7:0];
        in1_left = 1; in2_left = 0;
    end else if(left_duty < 0) begin
        pwm_left_val = -left_duty[7:0];
        in1_left = 0; in2_left = 1;
    end else begin
        pwm_left_val = 0;
        in1_left = 0; in2_left = 0;
    end
    
    // 右侧电机
    if(right_duty > 8'd255) begin
        pwm_right_val = 8'd255;
        in1_right = 1; in2_right = 0;
    end else if(right_duty < -8'd255) begin
        pwm_right_val = 8'd255;
        in1_right = 0; in2_right = 1;
    end else if(right_duty > 0) begin
        pwm_right_val = right_duty[7:0];
        in1_right = 1; in2_right = 0;
    end else if(right_duty < 0) begin
        pwm_right_val = -right_duty[7:0];
        in1_right = 0; in2_right = 1;
    end else begin
        pwm_right_val = 0;
        in1_right = 0; in2_right = 0;
    end
end

//-----------------------------------------------------------------------
// PWM 生成
//-----------------------------------------------------------------------
reg [11:0] pwm_cnt;
wire [11:0] pwm_period = CLK_FREQ / PWM_FREQ;
reg pwm_left_r, pwm_right_r;

always @(posedge clk or posedge rst) begin
    if(rst) begin
        pwm_cnt <= 0;
        pwm_left_r <= 0;
        pwm_right_r <= 0;
    end else begin
        if(pwm_cnt >= pwm_period - 1)
            pwm_cnt <= 0;
        else
            pwm_cnt <= pwm_cnt + 1;
        pwm_left_r  <= (pwm_cnt < {4'b0, pwm_left_val}) ? 1'b1 : 1'b0;
        pwm_right_r <= (pwm_cnt < {4'b0, pwm_right_val}) ? 1'b1 : 1'b0;
    end
end

assign pwm_left  = pwm_left_r;
assign pwm_right = pwm_right_r;

endmodule