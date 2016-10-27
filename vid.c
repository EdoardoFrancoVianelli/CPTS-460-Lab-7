//display driver

#define VDC_INDEX (0x3D4)
#define VDC_DATA  (0x3D5)
#define CUR_SIZE  (10) //cursor size register
#define VID_ORG   (12) // start address register
#define CURSOR (14)
#define LINE_WIDTH (80)
#define SCR_LINES  (25)
#define SCR_BYTES (4000)
#define CURSOR_SHAPE (15)

u16 base = 0xB800; //VRAM base address
u16 vid_mask = 0x3FFF; //mask = video ram size - 1
u16 offset; //offset from vram segment base
int color; // attribute type
int org; //current display origin
int row, column; //logical row, column position

int getRow(){
	return row;
}
int getCol(){
	return column;
}

int vid_init() {
	int i, w;
	printf("Initializing video driver\n");
	org = row = column = 0;
	color = 0x0A;
	set_VDC(CUR_SIZE, CURSOR_SHAPE);
	set_VDC(VID_ORG, 0);
	set_VDC(CURSOR, 0);
	w = 0x0700;
	for (i = 0; i <  25 * 80; i++){
		put_word(w, base, 0+2*i);
	}
}



int scroll(){
	u16 i, w, bytes;
	offset = org + SCR_BYTES + 2 * LINE_WIDTH;
	if (offset <= vid_mask){
		org += 2*LINE_WIDTH;	
	}else{
		for (i = 0; i < 24*80; i++){
			w = get_word(base, org+160+2*i);
			put_word(w, base, 0+2*i);
		}
		org = 0;
	}
	
	offset = org + 2 * 24 * 80;
	w = 0x0C00;
	for (i = 0; i < 80; i++){
		put_word(w, base, offset + 2*i);
	}
	set_VDC(VID_ORG, org >> 1);
}



int move_cursor(){
	int pos = 2 * (row * 80 + column);
	offset  = (org + pos) & vid_mask;
	set_VDC(CURSOR, offset >> 1);
}

int putc(char c){
	u16 w, pos;
	int temp_row = row, temp_col = column, i;
	
	if (c == '\n'){
		row++;
		if (row >= 25){
			row = 24;
			scroll();
		}
		move_cursor();
		return;
	}
	if (c == '\r'){
		column = 0;
		move_cursor();
		return;
	}
	if (c == '\b'){
		if (column > 0){
			column--;
			move_cursor();
			put_word(0x0700, base, offset);
		}
		return;
	}
	
	pos = 2 * (row * 80 + column);
	offset = (org + pos) & vid_mask;
	w = (color << 8) + c;
	put_word(w, base, offset);
	column++;
	if (column >= 80){
		column = 0;
		row++;
		if (row >= 25){
			row = 24;
			scroll();
		}	
	}
	move_cursor();
}

void SetRow(int rw){
	row = rw;
}

void SetColumn(int col){
	column = col;
}

int display_timer_bottom_right (int seconds, 
				int minutes, 
				int hours, 
				int *old_row,
				int *old_column){
	int i;
	*old_row    =    row;
	*old_column = column; 
	
	ClearTimer();
	
	row    = 24;
	column = 68;
	
	if (hours   < 10) putc('0');
	printf("%d:", hours);
	if (minutes < 10) putc('0');
	printf("%d:", minutes);
	if (seconds < 10) putc('0');
	printf("%d", seconds);	
	
	
}

int ClearTimer(){
	int i, old_r = row, old_c = column;
	row = 23;
	column = 79;
	
	for (i = 0; i < 11; i ++){
		putc('\b');
	}
	for (i = 0; i < 11; i++){
		putc(' ');
	}
	
	row = 24;
	column = 79;
	
	for (i = 0; i < 11; i ++){
		putc('\b');
	}
	for (i = 0; i < 12; i++){
		putc(' ');
	}
}

int set_VDC(u16 reg, u16 val){
	lock();
	out_byte(VDC_INDEX, reg);
	out_byte(VDC_DATA, (val >> 8) & 0xFF);
	out_byte(VDC_INDEX, reg+1);
	out_byte(VDC_DATA, val & 0xFF);
	unlock();
}











































