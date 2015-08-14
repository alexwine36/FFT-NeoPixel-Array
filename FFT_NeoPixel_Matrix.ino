#include <Adafruit_NeoPixel.h>
#include <fix_fft.h>

//Arduino Uno
#define PIN 6
#define ANI 0

//AtTiny85
//#define PIN 1
//#define ANI 1


#define colors 6

//#define leds 30
#define cols 8
#define rows 15
int leds = cols * rows;
int ppc; //Pixels Per Color

int ColorSelect[][3] = {  {255, 0, 0}, {255, 127, 0}, {255, 255, 0}, {28, 172, 120}, {0, 0, 255}, {159, 0, 255} };
int ColorFreq[][3] = { {2, 0, 0}, {1, 1, 0}, {0, 2, 0}, {0, 1, 1}, {0, 0, 2} };
uint32_t CS[colors];

Adafruit_NeoPixel strip = Adafruit_NeoPixel(leds, PIN, NEO_GRB + NEO_KHZ800);

int mic = ANI;

char im[128];
char data[128];
char data_avgs[128];

//mix max val to map fft
int valMin = 0;
int valMax = 30;
//bias to reduce on low and increase on high
int bias = 0;//+- bias to output

int pixelMatrix[cols][rows];
int columnAverage[cols];

void setup() {
  Serial.begin(115200);
  for (int i = 0; i < 6; i++) {
    CS[i] = strip.Color(ColorSelect[i][0], ColorSelect[i][1], ColorSelect[i][2]);
    //Serial.println(CS[i]);
  }
  matrixInit();

  strip.begin();
  strip.show();
  pinMode(mic, INPUT);
}

void loop() {
  int val;

  randomSeed(analogRead(mic));
  bias = random(70, 150);
  for (int i = 0; i < 128; i++) {
    val = analogRead(mic);
    data[i] = val;
    im[i] = 0;
    i++;
  }

  fix_fft(data, im, 7, 0);
  for (int i = 0; i < 64; i++) {
    data[i] = sqrt(data[i] * data[i] + im[i] * im[i]);
    dataConversion(i);
    strip.clear();
  }
  matrixAverage();
}

void dataConversion(int i) {
  data_avgs[i] = constrain(data[i], 0, rows);
  int dataVar = int(data_avgs[i]);
//  Serial.print(i);
  for (int j = 0; j < dataVar; j++) {
//    Serial.print("-");
  }
//  Serial.println();
}

void matrixPrint() {
  int sampleDist = 64 / cols;
  for (int y = rows; y > 0; y--) {

    for (int x = 0; x < cols; x++) {
      int sampleCols;
      if (x == 0) {
        sampleCols = 1;
      } else {
        sampleCols = sampleDist * x;
      }
      if (y == data_avgs[sampleCols]) {
        Serial.print("^");
      } else {
        Serial.print(" ");
      }
    }
    Serial.println();
  }
}

void matrixAverage() {
  int sampleDist = 64 / cols;
  int columnSum[cols];
  Serial.println("---------------------------------");
  for(int column = 0; column < cols; column++) {
    int freqStart = sampleDist * column;
    int freqEnd = freqStart + sampleDist;
    columnSum[column] = 0;
    
    for(int freq = freqStart; freq < freqEnd; freq++) {
      columnSum[column] += data_avgs[freq];
    }
    columnAverage[column] = columnSum[column] / sampleDist;
    Serial.println(columnAverage[column]);
//    strip.clear();
    matrixData(column, columnAverage[column]);
  }
}

void matrixData(int column, int columnAverage) {
  int pixelCount;
  if(columnAverage > colors){
    pixelCount = columnAverage / colors;
  }else{
    pixelCount = 1;
  }
  int colorShift = columnAverage - pixelCount;
  int colorVal = 5;
  for(int pixel = columnAverage; pixel > 0; pixel--) {
    int pixelNum = pixelMatrix[column][pixel];
    Serial.print("Pixel Number: ");
    Serial.println(pixelNum);
    if(pixel < colorShift) {
      colorShift -= pixelCount;
      colorVal--;
      strip.setPixelColor(pixelNum, CS[colorVal]);
    } else {
      strip.setPixelColor(pixelNum, CS[colorVal]);
    }
    strip.show();
  }
}

void matrixInit() {
  for (int x = 0; x < cols; x++) {
    for (int y = 0; y < rows; y++) {
      int f = x + 1;
      if (f % 2) {
        int columns = rows * x;
        //        Serial.println(columns);
        pixelMatrix[x][y] = rows * x + y;
      } else {
        //        Serial.println("Else");
        pixelMatrix[x][y] = rows * x + rows - 1 - y;
        //        Serial.println(pixelMatrix[x][y]);
      }
    }
  }
  Serial.println();
  Serial.print("Leds: ");
  Serial.println(leds);
  for (int x = 0; x < cols; x++) {
    for (int y = 0; y < rows; y++) {
      Serial.print(pixelMatrix[x][y]);
      Serial.print(" ");
    }
    Serial.println();
  }
}

