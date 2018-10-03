#include <stdio.h>
#include "miniprint.h"

#define POK_LCD_W 220
#define POK_LCD_H 176
#define POK_COLORDEPTH 4
#define width 110
#define height 88

uint8_t drawcolor;

uint16_t palette[16] = {
0x0821,
0x4944,
0x7ac5,
0xbc6a,
0xedce,
0xf6f2,
0xffbb,
0xbdd5,
0x7569,
0x7556,
0x3bf3,
0x328b,
0x2908,
0x79a7,
0xbb4b,
0xe4ad
};

uint8_t screenbuffer[(((POK_LCD_H/2)+1)*POK_LCD_W/2)*POK_COLORDEPTH/8];

#define MODE2_INNER_LOOP_B				\
	       "	str %[c], [%[LCD], 0]"    "\n"	\
	       "	movs %[c], 252"		  "\n"	\
	       "	str %[t], [%[LCD], %[c]]" "\n"	\
	       "	subs %[x], 1"             "\n"	\
	       "	str %[t], [%[LCD], 124]"  "\n"	\
	       "	str %[t], [%[LCD], %[c]]" "\n"  \
    	       "	ldm %[scanline]!, {%[c]}" "\n"	\
	       "	str %[t], [%[LCD], 124]"  "\n"


void lcdRefresh() {
    uint8_t * scrbuf = screenbuffer;
    uint16_t* paletteptr = palette;
    uint32_t x,y,byte,c,t=1<<12;
    uint32_t scanline[110];

    asm volatile(
	".syntax unified"         "\n"

	"mov r10, %[scanline]"    "\n"
	"mov r11, %[t]"           "\n"

	"mode2OuterLoop:"        "\n"

	"movs %[x], 110"          "\n"
	"mode2InnerLoopA:"


	"	ldrb %[byte], [%[scrbuf],0]"   "\n"
	"	lsrs %[c], %[byte], 4"    "\n"

	"	movs %[t], 15" "\n"
	"	ands %[byte], %[t]"    "\n"

	"	lsls %[c], 1"             "\n"
	"	ldrh %[t], [%[paletteptr], %[c]]"      "\n"
	"	lsls %[t], %[t], 3"       "\n"
	"	str %[t], [%[LCD], 0]"    "\n"
	"	stm %[scanline]!, {%[t]}" "\n"
	"	mov %[c], r11" "\n"
	"	movs %[t], 252"   "\n"
	"	str %[c], [%[LCD], %[t]]" "\n"
	"	lsls %[byte], %[byte], 1"             "\n"
	"	str %[c], [%[LCD], 124]"  "\n"
	"	str %[c], [%[LCD], %[t]]" "\n"
	"	ldrh %[t], [%[paletteptr], %[byte]]"      "\n"	
	"	str %[c], [%[LCD], 124]"  "\n"

	"	lsls %[t], %[t], 3"       "\n"
	"	str %[t], [%[LCD], 0]"    "\n"
	"	stm %[scanline]!, {%[t]}" "\n"
	"	mov %[c], r11" "\n"
	"	movs %[t], 252"   "\n"
	"	str %[c], [%[LCD], %[t]]" "\n"
	"	adds %[scrbuf], %[scrbuf], 1" "\n"
	"	str %[c], [%[LCD], 124]"  "\n"
	"	str %[c], [%[LCD], %[t]]" "\n"
	"	subs %[x], 2"          "\n"
	"	str %[c], [%[LCD], 124]"  "\n"
	"	bne mode2InnerLoopA"  "\n"

	"mov %[scanline], r10"    "\n"
	"movs %[x], 110"          "\n"
	"mov %[t], r11"           "\n"
    	       "	ldm %[scanline]!, {%[c]}" "\n"
	"mode2InnerLoopB:"
	MODE2_INNER_LOOP_B
	MODE2_INNER_LOOP_B
	MODE2_INNER_LOOP_B
	MODE2_INNER_LOOP_B
	MODE2_INNER_LOOP_B
	MODE2_INNER_LOOP_B
	MODE2_INNER_LOOP_B
	MODE2_INNER_LOOP_B
	MODE2_INNER_LOOP_B
	MODE2_INNER_LOOP_B
	"	bne mode2InnerLoopB"     "\n"

	"mov %[scanline], r10"    "\n"
	"movs %[t], 1"              "\n"
	"movs %[c], 88"             "\n"
	"add %[y], %[t]"            "\n" // y++... derpy, but it's the outer loop
	"cmp %[y], %[c]"            "\n"
	"bne mode2OuterLoop"       "\n" // if y != 88, loop

	: // outputs
	  [c]"+l" (c),
	  [t]"+l" (t),
	  [x]"+l" (x),
	  [y]"+h" (y),  // +:Read-Write l:lower (0-7) register
	  [scrbuf]"+l" (scrbuf)

	: // inputs
	  [LCD]"l" (0xA0002188),
	  [scanline]"l" (scanline),
	  [paletteptr]"l" (paletteptr),
	  [byte]"l" (byte)
	: // clobbers
	  "cc", "r10", "r11"
	);

}

void setPixel( uint32_t x, uint32_t y, uint32_t color ){
    if( x>=width || y>=height ) return;

    uint32_t i = y*(width>>1) + (x>>1);
    uint8_t pixel = screenbuffer[i];
    if (x&1) pixel = (pixel&0xF0)|(color);
    else pixel = (pixel&0x0F) | (color<<4);
    
    screenbuffer[i] = pixel;
    
}

void vline( uint32_t x, uint32_t y, uint32_t h, uint32_t color ){
    if( x >= width || y >= height || !h ) return;
    if( y+h >= height )
	h = height - y;

    uint8_t *fb = screenbuffer + y * (width>>1) + (x>>1);
    
    color &= 0xF;
    uint8_t mask = 0xF;

    if( x & 1 ){
	mask <<= 4;	
    }else{
	color <<= 4;
    }
    
    while( h-- ){
	*fb = (*fb & mask) | color;	    
	fb += (width>>1);
    }
}

void hline( uint32_t x, uint32_t y, uint32_t w, uint32_t color ){
    if( x >= width || y >= height || !w ) return;
    if( x+w >= width )
	w = width - x;

    uint8_t *fb = screenbuffer + y * (width>>1) + (x>>1);
    if( x&1 ){
	*fb++ = (*fb&0xF0) | (color);
	w--;
    }

    if( w&1 ){
	fb[w>>1] = (fb[w>>1]&0xF) | (color<<4);
	w--;
    }

    color &= 0xF;
    color |= color << 4;
    w >>= 1;
    while( w-- ){
	*fb++ = color;
    }

}

void fillRect( int32_t x, int32_t y, uint32_t w, uint32_t h, uint32_t color ){
    if( x >= width || y >= height ) return;
    if( y < 0 ){
	h += y;
	y = 0;
    }
    if( x < 0 ){
	w += x;
	x = 0;
    }
    if( y+h >= height )
	h = height - y;
    if( x+w >= width )
	w = width - x;
    
    if( x&1 ){
	vline(x, y, h, color);
	x++;
	w--;
    }

    if( w&1 ){
	vline( x+w-1, y, h, color );
	w--;
    }

    color &= 0xF;
    color |= color << 4;
    w >>= 1;
    while( h-- ){
	uint8_t *fb = screenbuffer + (y++ * width>>1) + (x>>1);
	for( uint32_t i=0; i<w; ++i )
	    *fb++ = color;
    }
}

void drawBitmap(int x, int y, int w, int h, const unsigned char *bitmap){
    uint32_t skip = 0;
    if( x <= -w || x >= width || y <= -h || y >= height )
	return;

    x &= ~1;
    
    if( y < 0 ){
	bitmap -= y*(w>>1);
	h += y;
	y = 0;
    }

    if( y+h >= height ){
	h = height - y;
    }

    if( x < 0 ){
	skip = -x/2;
	bitmap += skip;
	w += x;
	x = 0;
    }

    if( x+w >= width ){
	skip += (x+w - width) >> 1;
	w = width - x;
    }

    w >>= 1;
    uint8_t *fb = screenbuffer + (y)*(width>>1) + (x>>1);
    uint32_t stride = (width>>1) - (w);
    while( h-- ){
	for( int i=w; i; --i )
	    *fb++ = *bitmap++;
	bitmap += skip;
	fb += stride;
    }
}

int drawChar(int x, int y, const unsigned char *bitmap, unsigned int index){
    uint8_t w = *bitmap;
    uint8_t h = *(bitmap + 1);
    uint8_t hbytes=0, xtra=1;
    if (h==8 || h==16) xtra=0; //don't add if exactly on byte limit
    hbytes=(h>>3)+xtra; //GLCD fonts are arranged w+1 times h/8 bytes
    //bitmap = bitmap + 3 + index * h * ((w>>3)+xtra); //add an offset to the pointer (fonts !)
    bitmap = bitmap + 4 + index * (w * hbytes + 1); //add an offset to the pointer (fonts !)
    //int8_t i, j, byteNum, bitNum, byteWidth = (w + 7) >> 3;
    int8_t i, j, numBytes;
    numBytes = *bitmap++; //first byte of char is the width in bytes
    // GLCD fonts are arranged LSB = topmost pixel of char, so its easy to just shift through the column
    uint16_t bitcolumn; //16 bits for 2x8 bit high characters

    for (i = 0; i < numBytes; i++) {
	bitcolumn = *bitmap++;
	if (hbytes == 2) bitcolumn |= (*bitmap++)<<8; // add second byte for 16 bit high fonts
	
	for (j = 0; j < h; j++) {
	    if (bitcolumn&0x1) {
		setPixel(x + i, y + j,drawcolor);
	    } 
	    bitcolumn>>=1;
	}
    }
    return numBytes; // for character stepping
}


void ecprintf( int *b ){
    char *str = (char *) *b++;

    while( char c = *str++ ){
	if( c == '%' ){
	    c = *str++;
	    if( !c ) return;
	    if( c == 'd' ){
		print( *b++ );
		continue;
	    }else if( c == 'c' ){
		print( (char) *b++ );
	    }else if( c == 's' ){
		print( (char *) *b++ );
		continue;
	    }
	}
	write( c );
    }

}

void cprintf( const char *str, ... ){
    int *b = (int *) &str;
    ecprintf( b );
}
