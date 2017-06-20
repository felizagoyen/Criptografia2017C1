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

unsigned long r1, r2, r3;
unsigned long key22 = 0x3AB3CB;

void encryptImage(char *sourceFile, char *destinationFile);
FILE *openFile(char *path, const char *mode);
unsigned long getFileLength(FILE *file);
void convertFileInBits(FILE *file, unsigned long fileLength, unsigned char *fileInBits);
void writeEncriptedFile(FILE *file, unsigned long fileLength, unsigned char *fileInBits);
void runA5(int frameCounter);
void shift64Clock();
void shift22Clock(int frameCounter);
void shiftAllWithKey(unsigned long key, unsigned char maskKey);
void shift100Clock();
void generateKeyStream();
void shiftAllWithoutKey();
void shiftR1WihoutKey();
void shiftR2WihoutKey();
void shiftR3WihoutKey();
void shiftR1(unsigned char key, unsigned char maskKey);
void shiftR2(unsigned char key, unsigned char maskKey);
void shiftR3(unsigned char key, unsigned char maskKey);
unsigned char bitValue(unsigned long r, unsigned long mask);
unsigned char majority();
unsigned char keystream[228];

int main(int argc, char *argv[]) {
  if (argc != 3) {
    printf("Missing arguments.\n\nUsage %s sourceFile.bmp destinationFile.bmp\n", argv[0]);
    return -1;
  }

  encryptImage(argv[1], argv[2]);

  return 0;
}

void encryptImage(char *sourceFile, char *destinationFile) {
  FILE *file = openFile(sourceFile, "r");
  
  unsigned long fileLength = getFileLength(file);
  unsigned char *fileInBits = (unsigned char *) malloc(fileLength*8);
  convertFileInBits(file, fileLength, fileInBits);

  fclose(file);

  file = openFile(destinationFile, "w");
  writeEncriptedFile(file, fileLength, fileInBits);

  free(fileInBits);
  fclose(file);
}

FILE *openFile(char *path, const char *mode) {
  FILE *file = fopen(path, mode);
  if(!file) {
    printf("Could not open %s file.\n", path);
    exit(-1);
  }
  return file;
}

void convertFileInBits(FILE *file, unsigned long fileLength, unsigned char *fileInBits) {
  unsigned char *buffer = (unsigned char *) malloc(fileLength + 1);
  fread(buffer, fileLength, sizeof(unsigned char), file);

  for(int x = 0; x < fileLength; x++) {
    for(int y = 7; y >= 0; y--) {
      fileInBits[x*8+y] = bitValue(buffer[x], 0x80);
      buffer[x] = buffer[x] << 1;
    }
  }

  free(buffer);
}

unsigned long getFileLength(FILE *file) {
  fseek(file, 0, SEEK_END);
  unsigned long fileLength = ftell(file);
  fseek(file, 0, SEEK_SET);
  return fileLength;
}

void writeEncriptedFile(FILE *file, unsigned long fileLength, unsigned char *fileInBits) {
  unsigned char byte;
  int frameCounter = 0, bitCounter = 0;
  runA5(frameCounter);
 
  for(int x = 0; x < fileLength; x++) {
  	byte = 0;
    if (x > 33)
      for(int y = 0; y < 8; y++) {
        byte = byte >> 1;
        byte += ((fileInBits[x*8+y] ^ keystream[bitCounter]) * 128);
        bitCounter++;
        if(bitCounter == 228) {
          runA5(++frameCounter);
          bitCounter = 0;
        }
      }
    else
      for(int y = 0; y < 8; y++) {
        byte = byte >> 1;	 
        byte += (fileInBits[x*8+y] * 128);
      }

    fwrite(&byte, 1, sizeof(byte), file);
  }
}

void runA5(int frameCounter) {
  shift64Clock();
  shift22Clock(frameCounter);
  shift100Clock();
  generateKeyStream();
}

void shift64Clock() {
  r1 = r2 = r3 = 0;
  unsigned char key64[8] = {0x4E, 0x2F, 0x4D, 0x7C, 0x1E, 0xB8, 0x8B, 0x3A};
  for(int x = 0; x < 64; x++) {
    shiftAllWithKey(key64[x/8], 0x80);
    key64[x/8] = key64[x/8] << 1;
  }
}

void shift22Clock(int frameCounter) {
  key22+=frameCounter;
  for(int x = 0; x < 22; x++) { 
    shiftAllWithKey(key22, 0x01);
    key22 = key22 >> 1;
  }
}

void shiftAllWithKey(unsigned long key, unsigned char maskKey) {
  shiftR1(key, maskKey);
  shiftR2(key, maskKey);
  shiftR3(key, maskKey);
}

void shift100Clock() {
  for(int x = 0; x < 100; x++) shiftAllWithoutKey();
}

void generateKeyStream() {
  for(int x = 0; x < 229; x++) {
    unsigned char bit = bitValue(r1, MASK_BIT_18_R1) ^ bitValue(r2, MASK_BIT_21_R2) ^ bitValue(r3, MASK_BIT_22_R3);
    keystream[x] = bit;
    shiftAllWithoutKey();
  }
}

void shiftAllWithoutKey() {
  unsigned char m = majority();
  if(bitValue(r1, MASK_CLOCK_R1) == m) shiftR1WihoutKey();
  if(bitValue(r2, MASK_CLOCK_R2) == m) shiftR2WihoutKey();
  if(bitValue(r3, MASK_CLOCK_R3) == m) shiftR3WihoutKey();
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