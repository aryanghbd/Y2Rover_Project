module EEE_IMGPROC(
	// global clock & reset
	clk,
	reset_n,
	
	// mm slave
	s_chipselect,
	s_read,
	s_write,
	s_readdata,
	s_writedata,
	s_address,

	// stream sink
	sink_data,
	sink_valid,
	sink_ready,
	sink_sop,
	sink_eop,
	
	// streaming source
	source_data,
	source_valid,
	source_ready,
	source_sop,
	source_eop,
	
	// conduit
	mode
	
);


// global clock & reset
input	clk;
input	reset_n;

// mm slave
input							s_chipselect;
input							s_read;
input							s_write;
output	reg	[31:0]	s_readdata;
input	[31:0]				s_writedata;
input	[2:0]					s_address;

// streaming sink
input	[23:0]            	sink_data;
input								sink_valid;
output							sink_ready;
input								sink_sop;
input								sink_eop;

// streaming source
output	[23:0]			  	   source_data;
output								source_valid;
input									source_ready;
output								source_sop;
output								source_eop;

// conduit export
input                         mode;

////////////////////////////////////////////////////////////////////////
parameter IMAGE_W = 11'd640;
parameter IMAGE_H = 11'd480;
parameter MESSAGE_BUF_MAX = 256;
parameter MSG_INTERVAL = 50;
parameter BB_COL_DEFAULT = 24'h00ff00;


wire [7:0]   red, green, blue, grey;
wire [7:0]   red_out, green_out, blue_out;

wire         sop, eop, in_valid, out_ready;
////////////////////////////////////////////////////////////////////////

// Detect ball colours
wire green_detect;
wire black_detect;
wire pink_detect;
wire blue_detect;
wire orange_detect;

assign green_detect = 	((red <= 8'd150 && red >= 8'd75) && 
								(green <= 8'd255 && green >= 8'd235) && 
								(blue <= 8'd230 && blue >= 8'd95)) ||	//light
								
								((red <= 8'd75 && red >= 8'd30) && 
								(green <= 8'd235 && green >= 8'd125) && 
								(blue <= 8'd95 && blue >= 8'd40)) ||
								
								((red <= 8'd30 && red >= 8'd15) && 
								(green <= 8'd125 && green >= 8'd90) && 
								(blue <= 8'd40 && blue >= 8'd20));	//dark
								
assign black_detect = 	((red <= 8'd50 && red >= 8'd30) && 
								(green <= 8'd110 && green >= 8'd70) && 
								(blue <= 8'd100 && blue >= 8'd65)) ||	//light
								
								((red <= 8'd40 && red >= 8'd20) && 
								(green <= 8'd70 && green >= 8'd45) && 
								(blue <= 8'd60 && blue >= 8'd40)) ||
								
								((red <= 8'd40 && red >= 8'd30) && 
								(green <= 8'd40 && green >= 8'd30) && 
								(blue <= 8'd40 && blue >= 8'd30));	//dark
								
assign pink_detect = 	((red <= 8'd255 && red >= 8'd230) && 
								(green <= 8'd230 && green >= 8'd200) && 
								(blue <= 8'd240 && blue >= 8'd210)) ||	//light
								
								((red <= 8'd230 && red >= 8'd170) && 
								(green <= 8'd200 && green >= 8'd150) && 
								(blue <= 8'd210 && blue >= 8'd140)) ||
								
								((red <= 8'd170 && red >= 8'd130) && 
								(green <= 8'd150 && green >= 8'd110) && 
								(blue <= 8'd140 && blue >= 8'd100));	//dark
								
assign blue_detect = 	((red <= 8'd145 && red >= 8'd80) && 
								(green <= 8'd255 && green >= 8'd220) && 
								(blue <= 8'd255 && blue >= 8'd240)) ||	//light
								
								((red <= 8'd80 && red >= 8'd40) && 
								(green <= 8'd220 && green >= 8'd130) && 
								(blue <= 8'd240 && blue >= 8'd150)) ||
								
								((red <= 8'd50 && red >= 8'd20) && 
								(green <= 8'd130 && green >= 8'd70) && 
								(blue <= 8'd150 && blue >= 8'd80));	//dark
								
assign orange_detect = 	((red <= 8'd255 && red >= 8'd245) && 
								(green <= 8'd255 && green >= 8'd205) && 
								(blue <= 8'd150 && blue >= 8'd65)) ||	//light
								
								((red <= 8'd245 && red >= 8'd160) && 
								(green <= 8'd205 && green >= 8'd110) && 
								(blue <= 8'd85 && blue >= 8'd45)) ||
								
								((red <= 8'd160 && red >= 8'd110) && 
								(green <= 8'd110 && green >= 8'd75) && 
								(blue <= 8'd45 && blue >= 8'd0));	//dark

// Find boundary of cursor box

// Highlight detected areas
wire [23:0] colour_high;
assign grey = green[7:1] + red[7:2] + blue[7:2]; //Grey = green/2 + red/4 + blue/4
assign colour_high  = 	green_detect ? {8'h0, 8'hff, 8'h0} : 
							black_detect ? {8'h0, 8'h0, 8'h0} : 
							pink_detect ? {8'hff, 8'hc0, 8'hcb} : 
							blue_detect ? {8'h0, 8'h0, 8'hff} : 
							orange_detect ? {8'hff, 8'ha5, 8'h0} : 
							{grey, grey, grey};

// Show bounding boxes
wire [23:0] new_image;
wire green_bb_active, black_bb_active, pink_bb_active, blue_bb_active, orange_bb_active;
assign green_bb_active = (x == green_left) | (x == green_right) | (y == green_top) | (y == green_bottom);
assign black_bb_active = (x == black_left) | (x == black_right) | (y == black_top) | (y == black_bottom);
assign pink_bb_active = (x == pink_left) | (x == pink_right) | (y == pink_top) | (y == pink_bottom);
assign blue_bb_active = (x == blue_left) | (x == blue_right) | (y == blue_top) | (y == blue_bottom);
assign orange_bb_active = (x == orange_left) | (x == orange_right) | (y == orange_top) | (y == orange_bottom);

assign new_image = 	green_bb_active ? 24'h00ff00 : 
							black_bb_active ? 24'h000000 : 
							pink_bb_active ? 24'hffc0cb : 
							blue_bb_active ? 24'h0000ff : 
							orange_bb_active ? 24'hffa500 : 
							colour_high;

// Switch output pixels depending on mode switch
// Don't modify the start-of-packet word - it's a packet discriptor
// Don't modify data in non-video packets
assign {red_out, green_out, blue_out} = (mode & ~sop & packet_video) ? new_image : {red,green,blue};

//Count valid pixels to get the image coordinates. Reset and detect packet type on Start of Packet.
reg [10:0] x, y;
reg packet_video;
always@(posedge clk) begin
	if (sop) begin
		x <= 11'h0;
		y <= 11'h0;
		packet_video <= (blue[3:0] == 3'h0);
	end
	else if (in_valid) begin
		if (x == IMAGE_W-1) begin
			x <= 11'h0;
			y <= y + 11'h1;
		end
		else begin
			x <= x + 11'h1;
		end
	end
end

//Find first and last coloured pixels
reg [10:0] green_x_min, green_y_min, green_x_max, green_y_max;
reg [10:0] black_x_min, black_y_min, black_x_max, black_y_max;
reg [10:0] pink_x_min, pink_y_min, pink_x_max, pink_y_max;
reg [10:0] blue_x_min, blue_y_min, blue_x_max, blue_y_max;
reg [10:0] orange_x_min, orange_y_min, orange_x_max, orange_y_max;
always@(posedge clk) begin
	if (green_detect & in_valid) begin	//Update bounds when the pixel is orange
		if (x < green_x_min) green_x_min <= x;
		if (x > green_x_max) green_x_max <= x;
		if (y < green_y_min) green_y_min <= y;
		green_y_max <= y;
	end
		if (black_detect & in_valid) begin	//Update bounds when the pixel is black
		if (x < black_x_min) black_x_min <= x;
		if (x > black_x_max) black_x_max <= x;
		if (y < black_y_min) black_y_min <= y;
		black_y_max <= y;
	end
		if (pink_detect & in_valid) begin	//Update bounds when the pixel is pink
		if (x < pink_x_min) pink_x_min <= x;
		if (x > pink_x_max) pink_x_max <= x;
		if (y < pink_y_min) pink_y_min <= y;
		pink_y_max <= y;
	end
		if (blue_detect & in_valid) begin	//Update bounds when the pixel is blue
		if (x < blue_x_min) blue_x_min <= x;
		if (x > blue_x_max) blue_x_max <= x;
		if (y < blue_y_min) blue_y_min <= y;
		blue_y_max <= y;
	end
		if (orange_detect & in_valid) begin	//Update bounds when the pixel is orange
		if (x < orange_x_min) orange_x_min <= x;
		if (x > orange_x_max) orange_x_max <= x;
		if (y < orange_y_min) orange_y_min <= y;
		orange_y_max <= y;
	end
	if (sop & in_valid) begin	//Reset bounds on start of packet
		green_x_min <= IMAGE_W-11'h1;
		green_x_max <= 0;
		green_y_min <= IMAGE_H-11'h1;
		green_y_max <= 0;
		
		black_x_min <= IMAGE_W-11'h1;
		black_x_max <= 0;
		black_y_min <= IMAGE_H-11'h1;
		black_y_max <= 0;
		
		pink_x_min <= IMAGE_W-11'h1;
		pink_x_max <= 0;
		pink_y_min <= IMAGE_H-11'h1;
		pink_y_max <= 0;
		
		blue_x_min <= IMAGE_W-11'h1;
		blue_x_max <= 0;
		blue_y_min <= IMAGE_H-11'h1;
		blue_y_max <= 0;
		
		orange_x_min <= IMAGE_W-11'h1;
		orange_x_max <= 0;
		orange_y_min <= IMAGE_H-11'h1;
		orange_y_max <= 0;
	end
end

//Process bounding box at the end of the frame.
reg [1:0] msg_state;
reg [10:0] green_left, green_right, green_top, green_bottom;
reg [10:0] black_left, black_right, black_top, black_bottom;
reg [10:0] pink_left, pink_right, pink_top, pink_bottom;
reg [10:0] blue_left, blue_right, blue_top, blue_bottom;
reg [10:0] orange_left, orange_right, orange_top, orange_bottom;
reg [7:0] frame_count;
always@(posedge clk) begin
	if (eop & in_valid & packet_video) begin  //Ignore non-video packets
		
		//Latch edges for display overlay on next frame
		green_left <= green_x_min;
		green_right <= green_x_max;
		green_top <= green_y_min;
		green_bottom <= green_y_max;
		
		black_left <= black_x_min;
		black_right <= black_x_max;
		black_top <= black_y_min;
		black_bottom <= black_y_max;
		
		pink_left <= pink_x_min;
		pink_right <= pink_x_max;
		pink_top <= pink_y_min;
		pink_bottom <= pink_y_max;
		
		blue_left <= blue_x_min;
		blue_right <= blue_x_max;
		blue_top <= blue_y_min;
		blue_bottom <= blue_y_max;
		
		orange_left <= orange_x_min;
		orange_right <= orange_x_max;
		orange_top <= orange_y_min;
		orange_bottom <= orange_y_max;
		
		//Start message writer FSM once every MSG_INTERVAL frames, if there is room in the FIFO
		frame_count <= frame_count - 1;
		
		if (frame_count == 0 && msg_buf_size < MESSAGE_BUF_MAX - 3) begin
			msg_state <= 2'b01;
			frame_count <= MSG_INTERVAL-1;
		end
	end
	
	//Cycle through message writer states once started
	if (msg_state != 2'b00) msg_state <= msg_state + 2'b01;

end

reg [15:0] green_d, black_d, pink_d, blue_d, orange_d, hundred, ten;
always@(posedge clk) begin

	green_d = 16'd2460/(green_x_max-green_x_min);
	black_d = 16'd2460/(black_x_max-black_x_min);
	pink_d = 16'd2460/(pink_x_max-pink_x_min);
	blue_d = 16'd2460/(blue_x_max-blue_x_min);
	orange_d = 16'd2460/(orange_x_max-orange_x_min);
	
	hundred = 16'd100;
	ten = 16'd10;
	
	if (green_d < ten) green_d = 16'd100;	//filter out glitches
	if (black_d < ten) black_d = 16'd100;
	if (pink_d < ten) pink_d = 16'd100;
	if (blue_d < ten) blue_d = 16'd100;
	if (orange_d < ten) orange_d = 16'd100;
	
	if (green_d > hundred) green_d = 16'd100;	//filter out glitches
	if (black_d > hundred) black_d = 16'd100;
	if (pink_d > hundred) pink_d = 16'd100;
	if (blue_d > hundred) blue_d = 16'd100;
	if (orange_d > hundred) orange_d = 16'd100;

	if (green_d < black_d) black_d = 16'd100;	
	else if (green_d > black_d) green_d = 16'd100;
	if (green_d < pink_d) pink_d = 16'd100;
	else if (green_d > pink_d) green_d = 16'd100;
	if (green_d < blue_d) blue_d = 16'd100;
	else if (green_d > blue_d) green_d = 16'd100;
	if (green_d < orange_d) orange_d = 16'd100;
	else if (green_d > orange_d) green_d = 16'd100;
	if (black_d < pink_d) pink_d = 16'd100;
	else if (black_d > pink_d) black_d = 16'd100;
	if (black_d < blue_d) blue_d = 16'd100;
	else if (black_d > blue_d) black_d = 16'd100;
	if (black_d < orange_d) orange_d = 16'd100;
	else if (black_d > orange_d) black_d = 16'd100;
	if (pink_d < blue_d) blue_d = 16'd100;
	else if (pink_d > blue_d) pink_d = 16'd100;
	if (pink_d < orange_d) orange_d = 16'd100;
	else if (pink_d > orange_d) pink_d = 16'd100;
	if (blue_d < orange_d) orange_d = 16'd100;
	else if (blue_d > orange_d) blue_d = 16'd100;
	
	if (green_d == hundred) green_d = 16'd0;
	if (black_d == hundred) black_d = 16'd0;
	if (pink_d == hundred) pink_d = 16'd0;
	if (blue_d == hundred) blue_d = 16'd0;
	if (orange_d == hundred) orange_d = 16'd0;
end

//Generate output messages for CPU
reg [31:0] msg_buf_in; 
wire [31:0] msg_buf_out;
reg msg_buf_wr;
wire msg_buf_rd, msg_buf_flush;
wire [7:0] msg_buf_size;
wire msg_buf_empty;

`define RED_BOX_MSG_ID "RBB"

always@(*) begin	//Write words to FIFO as state machine advances
	case(msg_state)
		2'b00: begin
			msg_buf_in = 32'b0;
			msg_buf_wr = 1'b0;
		end
		2'b01: begin
			msg_buf_in = {green_d, black_d};	//Message ID
			msg_buf_wr = 1'b1;
		end
		2'b10: begin
			msg_buf_in = {pink_d, blue_d};	//Top left coordinate
			msg_buf_wr = 1'b1;
		end
		2'b11: begin
			msg_buf_in = {orange_d,16'd0}; //Bottom right coordinate
			msg_buf_wr = 1'b1;
		end
	endcase
end


//Output message FIFO
MSG_FIFO	MSG_FIFO_inst (
	.clock (clk),
	.data (msg_buf_in),
	.rdreq (msg_buf_rd),
	.sclr (~reset_n | msg_buf_flush),
	.wrreq (msg_buf_wr),
	.q (msg_buf_out),
	.usedw (msg_buf_size),
	.empty (msg_buf_empty)
	);


//Streaming registers to buffer video signal
STREAM_REG #(.DATA_WIDTH(26)) in_reg (
	.clk(clk),
	.rst_n(reset_n),
	.ready_out(sink_ready),
	.valid_out(in_valid),
	.data_out({red,green,blue,sop,eop}),
	.ready_in(out_ready),
	.valid_in(sink_valid),
	.data_in({sink_data,sink_sop,sink_eop})
);

STREAM_REG #(.DATA_WIDTH(26)) out_reg (
	.clk(clk),
	.rst_n(reset_n),
	.ready_out(out_ready),
	.valid_out(source_valid),
	.data_out({source_data,source_sop,source_eop}),
	.ready_in(source_ready),
	.valid_in(in_valid),
	.data_in({red_out, green_out, blue_out, sop, eop})
);


/////////////////////////////////
/// Memory-mapped port		 /////
/////////////////////////////////

// Addresses
`define REG_STATUS    			0
`define READ_MSG    				1
`define READ_ID    				2
`define REG_BBCOL					3

//Status register bits
// 31:16 - unimplemented
// 15:8 - number of words in message buffer (read only)
// 7:5 - unused
// 4 - flush message buffer (write only - read as 0)
// 3:0 - unused


// Process write

reg  [7:0]   reg_status;
reg	[23:0]	bb_col;

always @ (posedge clk)
begin
	if (~reset_n)
	begin
		reg_status <= 8'b0;
		bb_col <= BB_COL_DEFAULT;
	end
	else begin
		if(s_chipselect & s_write) begin
		   if      (s_address == `REG_STATUS)	reg_status <= s_writedata[7:0];
		   if      (s_address == `REG_BBCOL)	bb_col <= s_writedata[23:0];
		end
	end
end


//Flush the message buffer if 1 is written to status register bit 4
assign msg_buf_flush = (s_chipselect & s_write & (s_address == `REG_STATUS) & s_writedata[4]);


// Process reads
reg read_d; //Store the read signal for correct updating of the message buffer

// Copy the requested word to the output port when there is a read.
always @ (posedge clk)
begin
   if (~reset_n) begin
	   s_readdata <= {32'b0};
		read_d <= 1'b0;
	end
	
	else if (s_chipselect & s_read) begin
		if   (s_address == `REG_STATUS) s_readdata <= {16'b0,msg_buf_size,reg_status};
		if   (s_address == `READ_MSG) s_readdata <= {msg_buf_out};
		if   (s_address == `READ_ID) s_readdata <= 32'h1234EEE2;
		if   (s_address == `REG_BBCOL) s_readdata <= {8'h0, bb_col};
	end
	
	read_d <= s_read;
end

//Fetch next word from message buffer after read from READ_MSG
assign msg_buf_rd = s_chipselect & s_read & ~read_d & ~msg_buf_empty & (s_address == `READ_MSG);
						


endmodule

