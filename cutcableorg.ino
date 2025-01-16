#include <Wire.h>               // Library I2C, SDA SCL
#include <LiquidCrystal_I2C.h>  // Library LCD 20x4
#include <Keypad.h>             // Library Keypad 4x4
#include <Arduino.h>            // Library Arduino
#include <EEPROM.h>             // Library EEPROM CPU 2560 = 4 kBa
#define kunci 8

const byte numRows = 4;  // Number of rows on the keypad
const byte numCols = 4;  // Number of columns on the keypad
volatile int lastEncoded = 0;
volatile long temp, counter = 0;
volatile boolean encoderASet = false;
volatile boolean encoderBSet = false;
long lastencoderValue = 0;
float m, m1, m2, sem, hist;
int lastMSB = 0;
int lastLSB = 0;
int encoder_Pin_1 = 3; // encoder output A to Arduino pin 2
int encoder_Pin_2 = 2; // encoder output B to Arduino pin 3
int i, savedValue, l;
int currentValue = 0;
int lock = 0;
int outVal = 0;
int S_Mtr = 5;         // Pengatur speed Motor
int En_Mtr = 6;        // Enable Motor  
int MG = 22;           // Drive relay motor gulung
int MP = 23;           // Drive relay motor potong
int BZ = 24;           // Drive relay Buzzer 
int IC;                // Input Correct
int MA;                // Manual atau Auto baca data di USB
unsigned long displayTime; // Variabel untuk melacak waktu
bool historyDisplayed = false; // Flag untuk mengecek apakah history sedang ditampilkan

char keymap[numRows][numCols] = 
{
  { '1', '2', '3', 'R' },
  { '4', '5', '6', 'S' },
  { '7', '8', '9', 'U' },
  { 'C', '0', 'E', 'D' }
};
// Define the keypad connection pins
byte rowPins[numRows] = { 30, 32, 34, 36 };  // Rows 0 to 3
byte colPins[numCols] = { 31, 33, 35, 37 };  // Columns 0 to 3\
// Create the Keypad objectstore
Keypad keypad = Keypad(makeKeymap(keymap), rowPins, colPins, numRows, numCols); // Inisialisasi keypad

LiquidCrystal_I2C lcd(0x27, 20, 4);  // Inisialisasi LCD

void setup() 
{
  //pinMode(LED_BUILTIN, OUTPUT);  //LED_BUILTIN = Pin 13
  lcd.init();
  lcd.backlight();
  pinMode(S_Mtr, OUTPUT); //Drive Dimmer Motor Gulung
  pinMode(En_Mtr, OUTPUT); //Drive En Dimm Motor Potong
  pinMode(MG, OUTPUT);          //Drive relay Motor Gulung
  pinMode(MP, OUTPUT);          //Drive relay Motor Potong
  pinMode(BZ, OUTPUT);          //Drive relay Buzzer
  pinMode(kunci, INPUT_PULLUP); //kunci INPUT_PULLUP = NO INPUT -> HIGH
  
  pinMode(MA, INPUT_PULLUP);    //MA HIGH = Manual ; MA LOW = Auto
  pinMode(encoder_Pin_1, INPUT); 
  pinMode(encoder_Pin_2, INPUT);
  digitalWrite(encoder_Pin_1, HIGH); //turn pullup resistor on
  digitalWrite(encoder_Pin_2, HIGH); //turn pullup resistor on 
  attachInterrupt(0, updateEncoder, CHANGE); 
  attachInterrupt(1, updateEncoder, CHANGE);
  //Serial.begin(9600); // Serial untuk monitoring
  i = 2;              // Awal i = 2 Motor can ON if i = 1 
  m = 0;
  m1 = 0;
  m2 = 0;
  IC = 0;
  MA = 1;
  sem = m1;
  if (sem>=0)
  { 
  sem = hist;
  }

  counter= 0;
  lcd.setCursor(1, 2);
  lcd.print("MOTOR : OFF");
  digitalWrite(MG, LOW);
  digitalWrite(MP, LOW);
  digitalWrite(BZ, LOW);
  digitalWrite(En_Mtr, LOW);
  analogWrite(S_Mtr, 0);
  lcd.setCursor(1, 0);   // Tampilan IN
  lcd.print("IN:");      // Tampilan IN
  lcd.setCursor(4, 0);   // Tampilan masukan manual
  lcd.print("       ");  // Tampilan masukan manual msh null
  lcd.setCursor(10, 0);  // Tampilan awal di LCD 20x4
  lcd.print("SET:");     // Tampilan awal di LCD 20x4
  lcd.setCursor(14, 0);  // Tampilan awal di LCD 20x4
  lcd.print("    ");     // Tampilan awal di LCD 20x4
  lcd.setCursor(15, 0);  // Tampilan awal di LCD 20x4
  lcd.print("0");
  lcd.setCursor(18, 0);  // Tampilan awal di LCD 20x4
  lcd.print("m");        // Tampilan awal di LCD 20x4
  lcd.setCursor(1, 1);   // Tampilan awal di LCD 20x4
  lcd.print("Length:");  // Tampilan awal di LCD 20x4
  lcd.setCursor(16, 1);  // Tampilan awal di LCD 20x4
  lcd.print("m");        // Tampilan awal di LCD 20x4
  lcd.setCursor(19, 2);  // Tampilan awal di LCD 20x4
  lcd.print("M");
  EEPROM.write(0, 0);    // Mengosongkan EEPROM address 0
  EEPROM.write(1, 0);    // Mengosongkan EEPROM address 1
  EEPROM.write(2, 0);    // Mengosongkan EEPROM address 2
  EEPROM.write(3, 0);    // Mengosongkan EEPROM address 4
  EEPROM.write(4, 0);    // Mengosongkan EEPROM address 5
  EEPROM.write(5, 0);    // Mengosongkan EEPROM address 6
  EEPROM.write(7, 0);    // Mengosongkan EEPROM address 7

}

void keypress()
{ char key = keypad.getKey();  // Baca tombol yang ditekan
  if (key)                     // Pengkondisian tombol terbaca
  {
    if (key >= '0' && key <= '9') 
    {
      if (currentValue < 1000) 
      {
        currentValue = currentValue * 10 + (key - '0');
      }
    }
    if (key == 'S') // = MEN
    {
      if (currentValue >= 0 && currentValue <= 999 && i != 1) 
      {
        EEPROM.write(0, currentValue / 100);
        EEPROM.write(1, (currentValue / 10) % 10);
        EEPROM.write(2, currentValue % 10);
        IC = 1;
      } 
      else 
      {
        lcd.setCursor(14, 0);  // Tampilan awal di LCD 20x4
        lcd.print("    "); 
        lcd.setCursor(15, 0);  // Tampilan awal di LCD 20x4
        lcd.print("Fault"); 
        IC = 0;
        //Serial.println("Invalid value");
      }
      lcd.setCursor(4, 0);
      lcd.print("      ");
      currentValue = 0;
    }

    if (key == 'R') // = COR
    {
      if (lock == HIGH || i != 1) 
      {
        i = 2;
        m1 = 0;
        m = 0;
        counter = 0; // Reset counter seperti biasa
        lcd.setCursor(14, 0);
        lcd.print("      ");
        lcd.setCursor(18, 0);  
        lcd.print("m");
        lcd.setCursor(1, 2);
        lcd.print("MOTOR : OFF");
        lcd.setCursor(9, 1);
        lcd.print("     ");
        lcd.setCursor(15, 2);
        lcd.print("     ");
        EEPROM.write(0, 0);
        EEPROM.write(1, 0);
        EEPROM.write(2, 0);
        // l = saved_l; // Kembalikan nilai 'l' setelah reset
      }
    }
    if (key == 'U') 
    {
      currentValue = currentValue + 1;
    }
    if (key == 'D') 
      {
          currentValue = currentValue --;
      }
    if (key == 'C') // = CAN
    {     
          i=0;
          currentValue = 0;
          lcd.setCursor(15, 0);
          lcd.print("    ");
          lcd.setCursor(1, 2);
          lcd.print("MOTOR : OFF");
          lcd.setCursor(9, 1);
          lcd.print("    ");
    }
    if (key == 'E') 
    {
      if (lock == LOW) i = 1;
    }
    
    if (key == 'D' && 'R' )
    {
     hist == 0;
     lcd.setCursor(15, 0);
     lcd.print("      ");
    }
   if (key == 'U' && 'S') 
   {
      lcd.clear();             // Clear the display
      lcd.setCursor(0, 0);     // Set the cursor position
      lcd.print("HISTORY:");   // Display "history: "
      lcd.setCursor(14, 0 );     // Set position to display 'm'
      lcd.print("m");          // Display 'm'
      lcd.setCursor(9, 0);     // Set position to display 'm'
      lcd.print(hist);         // Display history value 
      delay(6000);             // Delay 6 detik (6000 ms)
      lcd.clear();             // Bersihkan tampilan setelah delay
      lcd.setCursor(1, 2);
      lcd.print("MOTOR : OFF");
      lcd.setCursor(1, 0);
      lcd.print("IN:");
      lcd.setCursor(10, 0);
      lcd.print("SET:");
      lcd.setCursor(1, 1);
      lcd.print("Length:");
      lcd.setCursor(16, 1);
      lcd.print("m");
    }
  }
}
void kontrolMotor ()
{
  if (counter != temp)          
    {
      temp = counter;            
    }
  int value1 = EEPROM.read(0);                       // Membaca byte pertama dari EEPROM
  int value2 = EEPROM.read(1);                       // Membaca byte kedua dari EEPROM
  int value3 = EEPROM.read(2);
  int value4 = EEPROM.read(3);                       // Membaca byte ketiga dari EEPROM
  savedValue = value1 * 100 + value2 * 10 + value3;  // Menggabungkan nilai-nilai untuk mendapatkan angka tiga digit
  m = abs(m1) ;
  if (m < savedValue || m == savedValue + 0.5) 
  {
    lcd.setCursor(15, 2);
    lcd.print("    ");
  }

if (m > savedValue + 0.5) 
  {
    lcd.setCursor(14, 2);
    lcd.print("Over");
    i=0;
  }

if (i == 0) 
  {
    digitalWrite(MG, LOW);
    digitalWrite(BZ, LOW);
    digitalWrite(En_Mtr, LOW);
    analogWrite(S_Mtr, 0);
    lcd.setCursor(1, 2);
    lcd.print("MOTOR : OFF");
    counter = m2;
  }

if (lock == HIGH) 
  {
    i = 0;
    digitalWrite(MG, LOW);
    digitalWrite(BZ, LOW);
    digitalWrite(En_Mtr, LOW);
    analogWrite(S_Mtr, 0);
    lcd.setCursor(1, 2);
    lcd.print("MOTOR : OFF");
  }
if (i == 1 && m < savedValue+1 && lock == LOW) 
  { 
    digitalWrite(MG, HIGH);
    digitalWrite(BZ, HIGH);
    digitalWrite(En_Mtr, HIGH);
    analogWrite(S_Mtr, 255); //duty cycle 100%
    if (m <= savedValue - 0.5)
    {
      analogWrite(S_Mtr, 20); //duty cycle 7,8%
    }
    lcd.setCursor(1, 2);
    lcd.print("MOTOR : ON ");
    currentValue = 0;
    m2 = counter;
    m1 = m2 / 22000;  
    sem  = temp / 22000;
if (m >= savedValue && m < savedValue + 0.5)  
    { i = 0;
      digitalWrite(MG, LOW);
      digitalWrite(BZ, LOW);
      digitalWrite(En_Mtr, LOW);
      analogWrite(S_Mtr, 0);
      lcd.setCursor(14, 2);
      lcd.print("Done");
      lcd.setCursor(1, 2);
      lcd.print("MOTOR : OFF");
      if (lock == HIGH) 
      i = 2;
    }
  }
  if (i == 2) 
  {
    counter = 0;
    digitalWrite(MG, LOW);
    digitalWrite(BZ, LOW);
    digitalWrite(En_Mtr, LOW);
    analogWrite(S_Mtr, 0);
    lcd.setCursor(1, 2);
    lcd.print("MOTOR : OFF");
  }

  if (currentValue < 0) 
  {
    lcd.setCursor(9, 1);
    lcd.print("     ");
  }
  lcd.setCursor(5, 0);
  lcd.print(currentValue);

  if (IC == 1)
  { 
    lcd.setCursor(15, 0);
    lcd.print("   ");
    lcd.setCursor(15, 0);
    lcd.print(savedValue);
  }
  lcd.setCursor(9, 1);
  lcd.print("    ");
  lcd.setCursor(9, 1);
  lcd.print(m); 
}

void totalPerm() 
{
  // Salin nilai m1 ke EEPROM jika panjangnya sudah mencapai savedValue
  if (l >= savedValue) {   
    // Simpan nilai m1 dalam format ribuan, ratusan, puluhan, dan satuan
    hist += sem;
    EEPROM.write(3, savedValue / 1000);         // Simpan ribuan
    EEPROM.write(4, (savedValue / 100) % 10);   // Simpan ratusan
    EEPROM.write(5, (savedValue / 10) % 10);    // Simpan puluhan
    EEPROM.write(6, savedValue % 10);           // Simpan satuan

    // Membaca kembali nilai dari EEPROM untuk konfirmasi
    int m1_ribuan = EEPROM.read(3);                
    int m1_ratusan = EEPROM.read(4);                
    int m1_puluhan = EEPROM.read(5);                
    int m1_satuan = EEPROM.read(6);               
    float m1_stored = m1_ribuan * 1000 + m1_ratusan * 100 + m1_puluhan * 10 + m1_satuan;
  }
}

void totalTemp() {
  if (l >= savedValue) {   
    {

    EEPROM.write(3, l / 1000);         // Simpan ribuan
    EEPROM.write(4, (l / 100) % 10);   // Simpan ratusan
    EEPROM.write(5, (l / 10) % 10);    // Simpan puluhan
    EEPROM.write(6, l % 10);           // Simpan satuan

    // Membaca kembali nilai dari EEPROM untuk konfirmasi
    int m1_ribuan  = EEPROM.read(3);                
    int m1_ratusan = EEPROM.read(4);                
    int m1_puluhan = EEPROM.read(5);                
    int m1_satuan  = EEPROM.read(6);               
    // Menggabungkan kembali nilai yang dibaca dari EEPROM
    float m1_stored = m1_ribuan * 1000 + m1_ratusan * 100 + m1_puluhan * 10 + m1_satuan;
    }
  }
}

void loop() 
{
  lock = digitalRead(kunci);   // Ambil data limit switch kabel
  keypress();
  kontrolMotor();
  totalTemp();
  totalPerm();
  updateEncoder();
} 

void updateEncoder()
{ 
  int MSB = digitalRead(encoder_Pin_1); //MSB = most significant bit
  int LSB = digitalRead(encoder_Pin_2); //LSB = least significant bit
  int encoded = (MSB << 1) |LSB; //converting the 2 pin value to single number
  int sum  = (lastEncoded << 2) | encoded; //adding it to the previous encoded value

  if(sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) counter ++;
  if(sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) counter --;
  lastEncoded = encoded;
  Serial.print ("Jumlah putaran=");
  Serial.println (counter);
  delay (500); 
}
