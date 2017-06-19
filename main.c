#include <stdio.h>
#include <stdlib.h>

//R1 MASK
#define MASK_CLOCK_R1    0x000100 //BIT 8
#define MASK_BIT_13_R1   0X002000 //BIT 13 
#define MASK_BIT_16_R1   0X010000 //BIT 16
#define MASK_BIT_17_R1   0X020000 //BIT 17
#define MASK_BIT_18_R1   0X040000 //BIT 18

//R2 MASK
#define MASK_CLOCK_R2    0x000400 //BIT 10
#define MASK_BIT_20_R2   0X100000 //BIT 20
#define MASK_BIT_21_R2   0x200000 //BIT 21

//R3 MASK
#define MASK_CLOCK_R3    0x000400 //BIT 10
#define MASK_BIT_7_R3    0X000080 //BIT 7
#define MASK_BIT_20_R3   0X100000 //BIT 20
#define MASK_BIT_21_R3   0x200000 //BIT 21
#define MASK_BIT_22_R3   0x400000 //BIT 22

unsigned long r1 = 0, r2 = 0, r3 = 0;

void shift64Clock();
void shift22Clock();
void shift100Clock();
void generateKeyStream();
void shiftR1WihoutKey();
void shiftR2WihoutKey();
void shiftR3WihoutKey();
void shiftR1(unsigned char key, unsigned char maskKey);
void shiftR2(unsigned char key, unsigned char maskKey);
void shiftR3(unsigned char key, unsigned char maskKey);
unsigned char bitValue(unsigned long r, unsigned long mask);
unsigned char majority();
unsigned char keystream[228];
void encriptedImage();

int main() {
  shift64Clock(); //Revisar el R3
  shift22Clock();
  shift100Clock();
  generateKeyStream();
  encriptedImage();
  return 0;
}

void shift64Clock() {
  unsigned char key64[8] = {0x4E, 0x2F, 0x4D, 0x7C, 0x1E, 0xB8, 0x8B, 0x3A};
  unsigned char p;
  for(int x = 0; x < 64; x++) {
    if(x % 8 == 0) p = key64[x/8];
    shiftR1(p, 128);
    shiftR2(p, 128);
    shiftR3(p, 128);
    p = p << 1;
  }
}

void shift22Clock() {
  unsigned long key22 = 0x3AB3CB;
  for(int x = 0; x < 22; x++) { 
    if(x > 1) {
      shiftR1(key22, 1);
      shiftR2(key22, 1);
      shiftR3(key22, 1);
    }
    key22 = key22 >> 1;
  }
}

void shift100Clock() {
  unsigned char m;
  for(int x = 0; x < 100; x++) { 
    m = majority();
    if(bitValue(r1, MASK_CLOCK_R1) == m) shiftR1WihoutKey();
    if(bitValue(r2, MASK_CLOCK_R2) == m) shiftR2WihoutKey();
    if(bitValue(r3, MASK_CLOCK_R3) == m) shiftR3WihoutKey();
  }
}

void generateKeyStream() {
  unsigned char k, m;
  for(int x = 0; x < 229; x++) {
    unsigned char bit = bitValue(r1, MASK_BIT_18_R1) ^ bitValue(r2, MASK_BIT_21_R2) ^ bitValue(r3, MASK_BIT_22_R3);
    keystream[x] = bit;
    m = majority();
    if(bitValue(r1, MASK_CLOCK_R1) == m) shiftR1WihoutKey();
    if(bitValue(r2, MASK_CLOCK_R2) == m) shiftR2WihoutKey();
    if(bitValue(r3, MASK_CLOCK_R3) == m) shiftR3WihoutKey();
  }
}

void shiftR1(unsigned char key, unsigned char maskKey) {
  unsigned char bit = bitValue(r1, MASK_BIT_18_R1) ^ bitValue(r1, MASK_BIT_17_R1) ^ bitValue(r1, MASK_BIT_16_R1) ^ bitValue(r1, MASK_BIT_13_R1) ^ bitValue(key, maskKey);
  r1 = r1 << 1;
  r1 += bit;
}

void shiftR2(unsigned char key, unsigned char maskKey) {
  unsigned char bit = bitValue(r2, MASK_BIT_21_R2) ^ bitValue(r2, MASK_BIT_20_R2) ^ bitValue(key, maskKey);
  r2 = r2 << 1;
  r2 += bit;
}

void shiftR3(unsigned char key, unsigned char maskKey) {
  unsigned char bit = bitValue(r3, MASK_BIT_22_R3) ^ bitValue(r3, MASK_BIT_21_R3) ^ bitValue(r3, MASK_BIT_20_R3) ^ bitValue(r3, MASK_BIT_7_R3) ^ bitValue(key, maskKey);
  r3 = r3 << 1;
  r3 += bit;
}

void shiftR1WihoutKey() {
  unsigned char bit = bitValue(r1, MASK_BIT_18_R1) ^ bitValue(r1, MASK_BIT_17_R1) ^ bitValue(r1, MASK_BIT_16_R1) ^ bitValue(r1, MASK_BIT_13_R1);
  r1 = r1 << 1;
  r1 += bit;
}

void shiftR2WihoutKey() {
  unsigned char bit = bitValue(r2, MASK_BIT_21_R2) ^ bitValue(r2, MASK_BIT_20_R2);
  r2 = r2 << 1;
  r2 += bit;
}

void shiftR3WihoutKey() {
  unsigned char bit = bitValue(r3, MASK_BIT_22_R3) ^ bitValue(r3, MASK_BIT_21_R3) ^ bitValue(r3, MASK_BIT_20_R3) ^ bitValue(r3, MASK_BIT_7_R3);
  r3 = r3 << 1;
  r3 += bit;
}

unsigned char bitValue(unsigned long r, unsigned long mask) {
  if ((r & mask) != 0) return 1; else return 0;
}

unsigned char majority() {
  unsigned char c1 = bitValue(r1, MASK_CLOCK_R1);
  unsigned char c2 = bitValue(r2, MASK_CLOCK_R2);
  unsigned char c3 = bitValue(r3, MASK_CLOCK_R3);

  if(c1 == c2) return c1;
  if(c1 == c3) return c1;
  return c2;
}

void encriptedImage() {
  FILE *file;
  unsigned char *buffer;
  unsigned long fileLen;

  file = fopen("imagen.bmp", "r");
  
  fseek(file, 0, SEEK_END);
  fileLen=ftell(file);
  fseek(file, 0, SEEK_SET);

  buffer=(unsigned char *)malloc(fileLen+1);
  fread(buffer,fileLen,sizeof(unsigned char),file);
  fclose(file);  
  
  unsigned char v[(fileLen*8)];

  for(int x = 0; x < fileLen; x++) {
  	unsigned char p = buffer[x];
  	for(int y = 7; y >= 0; y--) {
  		v[x*8+y] = bitValue(p, 0x80);
  		p = p << 1;
  	}
  }

  file = fopen("imagen-enc.bmp", "w");

  unsigned char byte;
  int k = 0; 
  for(int x = 0; x < fileLen; x++) {
  	byte = 0;
    if (x > 33)
      for(int y = 0; y < 8; y++) {
        byte = byte >> 1;
        byte += ((v[x*8+y] ^ keystream[k]) * 128);
        k++;
        if(k == 228) k = 0;
      }
    else
      for(int y = 0; y < 8; y++) {
        byte = byte >> 1;	 
        byte += (v[x*8+y] * 128);
      }

    fwrite(&byte, 1, sizeof(byte), file);
  }

  free(buffer);
  
  fclose(file);
}
