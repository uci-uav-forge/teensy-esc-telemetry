#include <Arduino.h>
#include <SD.h>
#include <sstream>
#include <sys/time.h>
#include <time.h>

using namespace std;

char fileName[13];

char buffer1[10];
int head1 = 0;

char buffer2[10];
int head2 = 0;

char buffer3[10];
int head3 = 0;

char buffer4[10];
int head4 = 0;

struct ESCData {
  byte temp;
  short voltage;
  short current;
  short consumption;
  short rpm;
};

byte update_crc8(int crc, int crc_seed)
{
  byte crc_u = 0;
  crc_u = crc;
  crc_u ^= crc_seed;
  for (int i = 0; i < 8; i++)
  {
    crc_u = (crc_u & 0x80) ? 0x7 ^ (crc_u << 1) : (crc_u << 1);
    crc_u = crc_u & 0xFF;
  }
  return crc_u;
}

bool crc8(char *buff, int len)
{
  byte crc = 0;
  for (int i = 0; i < len; i++)
  {
    crc = update_crc8(buff[i], crc);
  }
  return crc;
}

ESCData readBuffer(char* buff) {
  ESCData data;
  data.temp = buff[0];
  data.voltage = (short)buff[1] << 8 | buff[2];
  data.current = (short)buff[3] << 8 | buff[4];
  data.consumption = (short)buff[5] << 8 | buff[6];
  data.rpm = (short)buff[7] << 8 | buff[8];
  return data;

}

void writeData(const char* fname, ESCData data, int escNo)
{
  digitalWrite(13, HIGH);
  delay(10);
  digitalWrite(13, LOW);
  // if file doesn't exist, then create it. Otherwise, append to it.
  File dataFile = SD.open(fname, FILE_WRITE);

  char buffer[100];
  float volts = data.voltage / 100.0;
  float amps = data.current / 100.0;
  float eRPM = data.rpm / 100.0;
  sprintf(buffer, "%d,%d,%2f,%2f,%2f,%2f", escNo, data.temp, volts, amps, data.consumption, eRPM);
  
  dataFile.println(buffer);

  dataFile.close();
}


void updateBuffer(char* buff, int& head, HardwareSerial serial, const char* fname, int escNo) {
  if (serial.available())
  {
    if (head == 10)
    {
      // shift buffer left 1
      for (int i = 0; i < 9; i++)
      {
        buff[i] = buff[i + 1];
      }
    }
    buff[head] = Serial1.read();
    head++;
    if (head == 10)
    {
      if (crc8(buff, 10))
      {
        ESCData data = readBuffer(buff);
        writeData(fname, data, escNo);
        head = 0;
      }
    }
    else
    {
      head++;
    }
  }
}

void setup()
{

  SD.begin(BUILTIN_SDCARD);
  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);
  delay(1000);
  digitalWrite(13, LOW);
  delay(1000);
  digitalWrite(13, HIGH);
  delay(1000);
  digitalWrite(13, LOW);
  struct timeval tv;
  gettimeofday(&tv, NULL);

  int millisec = lrint(tv.tv_usec/1000.0); // Round to nearest millisec
  if (millisec>=1000) { // Allow for rounding up to nearest second
    millisec -=1000;
    tv.tv_sec++;
  }

  struct tm* tm_info = localtime(&tv.tv_sec);

  strftime(fileName, 13, "%H-%M-%S.txt", tm_info);
  Serial.println(fileName);
  File dataFile = SD.open(fileName, FILE_WRITE);
  if(!dataFile) {
    Serial.println("Error opening file");
  }

  // put your setup code here, to run once:
  // set up rx1
  Serial1.begin(115200);
  Serial2.begin(115200);
  Serial3.begin(115200);
  Serial4.begin(115200);
  Serial.println("Initialized");
}

void loop()
{
  updateBuffer(buffer1, head1, Serial6, fileName, 1);
  updateBuffer(buffer2, head2, Serial7, fileName, 2);
  updateBuffer(buffer3, head3, Serial3, fileName, 3);
  updateBuffer(buffer4, head4, Serial8, fileName, 4);
}