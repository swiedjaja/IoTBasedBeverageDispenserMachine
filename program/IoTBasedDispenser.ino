/*
 * Start
 * IoT Based Beverage Dispenser Machine
 * 
 */

/*
 * 
 * Pin Definition
 * 
 */

const int solenoidPin = 17; // OUTPUT
const int peltierPin = 12; // OUTPUT
const int motorPin = 16; // OUTPUT

const int buzzerPin = 2; // OUTPUT

const int waterFlowPin = 33; // INPUT
const int waterLevelPin = 27; // INPUT
const int temperaturePin = 32; // INPUT

const int mediumButtonPin = 34; // INPUT
const int largeButtonPin = 35; // INPUT



/*
 * 
 * 
 * Untuk Google Firebase
 * 
 * 
 */


#include <WiFi.h>
#include <FirebaseESP32.h>

#define FIREBASE_HOST "https://kimmie-24850-default-rtdb.asia-southeast1.firebasedatabase.app/"
#define FIREBASE_AUTH "2pF5Zg84gstDiUQGeu6aFt0xU9XVh1Dc3om2eWit"
#define WIFI_SSID "ZenfoneMax"
#define WIFI_PASSWORD "Deviation"
 
// Firebase ESP32 Data Object
FirebaseData firebaseData;
FirebaseData firebaseDataWater;

FirebaseJson jsonBalance;
FirebaseJson jsonOrder;
FirebaseJson jsonWaterLevel;

/*
 * 
 * 
 * Untuk Fitur Send Email
 * 
 * 
 */


#include <EMailSender.h>
EMailSender emailSend("automaticdispensersk@gmail.com", "automatic1234!");

// Email Penerima
String recipientEmail;

// Struktur Isi Email
String headerHTML;
String drinkSizeHTML; 
String orderTimeHTML;
String updatedBalanceHTML;

// Variabel emailHTML adalah kombinasi keseluruhan string dari isi Email
String emailHTML; 
 
/*
 * 
 * 
 * Untuk RFID
 * 
 * 
 */
#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN 4
#define SS_PIN 5

MFRC522 mfrc522(SS_PIN, RST_PIN);   


/*
 * 
 * Variabel String untuk Firebase
 * 
 * 
 */

 
 // String Directory RFID
  String mainRoot = "/RFID/";
  char cardID[32] = "";
  String balanceRoot = "/Balance";
  String balanceDirectory;
  String balanceParentNode;
  String emailRoot = "/Email";
  String emailDirectory;

// RFID/E72797B4/Balance

 // String Directory ESP32
  uint32_t chipId = 0;
  String espMainRoot = "/SystemID/";
  String espIDNumber;
  String orderRoot = "/Order" ;
  String mediumRoot = "/Medium";
  String largeRoot = "/Large";
  String espParentNode;
  String mediumDirectory;
  String largeDirectory;

// String Directory Water Level ESP32
  String waterLevelRoot = "/Water Level Status";
  String waterLevelInfo;
  String espWaterParentNode;
 
  
 // Variabel Hitungan Balance dan Hitungan Jumlah Pesanan
  int balanceValue;
  int updatedValue;

  int mediumCount;
  int largeCount;

 // Variabel Pengambilan Waktu Real-Time dari NTP Server
  String timePresent;
  char combine[80];

/*
 * 
 * 
 *  Untuk Display
 * 
 * 
 * 
 */


#include "SPI.h"
#include "TFT_22_ILI9225.h"

#ifdef ARDUINO_ARCH_STM32F1
/#define TFT_RST PA1
 #define TFT_RS  PA2
#define TFT_CS  PA0 // SS
 #define TFT_SDI PA7 // MOSI
 #define TFT_CLK PA5 // SCK
 #define TFT_LED 0 // 0 if wired to +5V directly
#elif defined(ESP8266)
#define TFT_RST 4   // D2
#define TFT_RS  5   // D1
#define TFT_CLK 14  // D5 SCK
#define TFT_SDO 12  // D6 MISO
#define TFT_SDI 13  // D7 MOSI
#define TFT_CS  15  // D8 SS
#define TFT_LED 2   // D4     set 0 if wired to +5V directly -> D3=0 is not possible !!
#elif defined(ESP32)
#define TFT_RST 26  // IO 26
#define TFT_RS  25  // IO 25
#define TFT_CLK 14  // HSPI-SCK
#define TFT_SDO 12  // HSPI-MISO
#define TFT_SDI 13  // HSPI-MOSI
#define TFT_CS  15  // HSPI-SS0
#define TFT_LED 0   // 0 if wired to +5V directly
SPIClass hspi(HSPI);
#else
#define TFT_RST 8
#define TFT_RS  9
#define TFT_CS  10  // SS
#define TFT_SDI 11  // MOSI
#define TFT_CLK 13  // SCK
#define TFT_LED 3   // 0 if wired to +5V directly
#endif

#define TFT_BRIGHTNESS 1 // Initial brightness of TFT backlight (optional)

// Use hardware SPI (faster - on Uno: 13-SCK, 12-MISO, 11-MOSI)
TFT_22_ILI9225 tft = TFT_22_ILI9225(TFT_RST, TFT_RS, TFT_CS, TFT_LED, TFT_BRIGHTNESS);
// Use software SPI (slower)
//TFT_22_ILI9225 tft = TFT_22_ILI9225(TFT_RST, TFT_RS, TFT_CS, TFT_SDI, TFT_CLK, TFT_LED, TFT_BRIGHTNESS);

/*
 * 
 * 
 * Untuk Sensor Temperature
 * 
 * 
 */
 
#include <OneWire.h>
#include <DallasTemperature.h>

OneWire oneWire(temperaturePin);  
DallasTemperature sensors(&oneWire);

float temperature;
String coldString;
int flagPeltier = 0;

/*
 * 
 * 
 * Untuk Sensor Jarak TOF10120
 * 
 * 
 */
#include <Wire.h>
unsigned char ok_flag;
unsigned char fail_flag;
unsigned short lenth_val = 0;
unsigned char i2c_rx_buf[16];
unsigned char dirsend_flag=0;
unsigned long previousMicros = 0;
int distanceThreshold = 127;
int distance;

/*
 * 
 * 
 * Untuk Sensor Water Flow
 * 
 * 
 */
 
volatile byte pulseCount = 0;  
float calibrationFactor = 140; // Jika air yang mau keluar lebih banyak, besarkan CalibrationFactor
float flowRate = 0.0;
unsigned int flowMilliLitres = 0;
unsigned long totalMilliLitres = 0;
unsigned long oldTime = 0;

/*
 * 
 * 
 * Untuk Buzzer
 * 
 * 
 */
 
#include "EasyBuzzer.h"
unsigned int frequency = 2000;
unsigned int oneBeep = 1;
unsigned int twoBeep = 2;

/*
 * 
 * 
 * Untuk Pengaturan Timing pada Sistem (Timer)
 * 
 * 
 */
 
#include <SimpleTimer.h>
SimpleTimer timer(3000);
SimpleTimer glassTimer(2000);
SimpleTimer debugTimer(1000);
SimpleTimer waterLevelTimer(10000);
SimpleTimer temperatureTimer(10000);
SimpleTimer displayTimer(5000);
SimpleTimer solenoidTimer(500);

/*
 * 
 *  Untuk Push Button Medium dan Push Button Large
 * 
 * 
 */

int flagButton1 = 0;
int flagButton2 = 0;

unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers

int buttonState;
int lastButtonState1 = LOW;   // the previous reading from the input pin
int lastButtonState2 = LOW;   // the previous reading from the input pin

int readMediumButton;
int readLargeButton;

/*
 * 
 * Untuk Variabel Timing 
 * 
 */
unsigned long currentTime = 0;

/*
 * 
 * 
 * Untuk Variabel State Detection
 * 
 * 
 */
 
int flag = 0;                    
int flagDisplay = 0;             
int flagTransition = 0;           
int flagBuzzer = 0;
int flagSolenoid = 0;
int flagThankYou = 0;    
int flagEmail = 0;        

int glassStatus = 0;
int glassTakenStatus = 0;

/*
 * 
 * 
 * Inisialisasi Awal
 * 
 * 
 */
void setup()
{
  Serial.begin(115200);  // Untuk Debug melalui Serial Monitor

/*
 * 
 * Inisialisasi LCD Display
 * 
 */
      #if defined(ESP32)
    hspi.begin();
    tft.begin(hspi);
  #else
    tft.begin();
  #endif

 /*
 * 
 * Inisialisasi Awal Variabel Status Cold
 * 
 */

  sensors.requestTemperatures(); 
  temperature = sensors.getTempCByIndex(0);
  coldString = "NOT READY";
    
 /*
  * 
  * Pin Definition
  * 
  */

  pinMode(peltierPin, OUTPUT); 
  pinMode(motorPin, OUTPUT);
  pinMode(solenoidPin, OUTPUT);
  
  pinMode(waterLevelPin, INPUT);
  pinMode(temperaturePin, INPUT);
  pinMode(waterFlowPin, INPUT);

  pinMode(mediumButtonPin, INPUT);
  pinMode(largeButtonPin, INPUT);

  EasyBuzzer.setPin(buzzerPin);

  /*
   * 
   * Inisialisasi RFID
   * 
   */


   SPI.begin();     
   mfrc522.PCD_Init();   

 /*
  * 
  * Inisialisasi WiFi Connection
  * 
  */
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

 /*
  * 
  * Inisialisasi Google Firebase
  * 
  */
  
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);
  
  Firebase.setReadTimeout(firebaseData, 1000 * 7);
  Firebase.setwriteSizeLimit(firebaseData, "tiny");

  Firebase.setReadTimeout(firebaseDataWater, 1000 * 7);
  Firebase.setwriteSizeLimit(firebaseDataWater, "tiny");

  Serial.println("------------------------------------");
  Serial.println("Connected...");

/*
 * 
 * Inisialisasi Library Wire untuk Sensor Jarak
 * 
 */
 
   Wire.begin();

/*
 * 
 * Inisialisasi Sensor Water Flow
 * 
 */
 
  digitalWrite(waterFlowPin, HIGH);
  attachInterrupt(digitalPinToInterrupt(waterFlowPin), pulseCounter, FALLING);
  
/*
 * 
 * Proses Pengambilan ID ESP32 dalam bentuk String
 * 
 * 
 */

    for(int i = 0; i < 17; i = i + 8) 
   {
     chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;   // Didapat ESP32-ID: 12980728
   }
    espIDNumber = String(chipId); // Isi variabel chipID masih dalam bentuk integer sehingga perlu diubah ke bentuk String
    
}

/*
 * 
 * 
 * Inisialisasi Fungsi Loop pada Sistem
 * 
 * 
 */
 
void loop(){

  currentTime = millis();
  EasyBuzzer.update();
     
  if ((unsigned long) (currentTime - oldTime) > 1000) 
{
     // Disable the interrupt while calculating flow rate and sending the value to the host
     detachInterrupt(digitalPinToInterrupt(waterFlowPin));
    distance = ReadDistance();
    flowRate = ((1000.0 / (currentTime - oldTime)) * pulseCount) / calibrationFactor;
    oldTime = currentTime;
    flowMilliLitres = (flowRate / 60) * 1000;
    totalMilliLitres += flowMilliLitres;

    Serial.print("Distance: ");
    Serial.print(distance);
    Serial.print(" || ");
    Serial.print("Total Volume: ");
    Serial.print(totalMilliLitres);
    Serial.print(" || ");
    Serial.print("Pulse Count: ");
    Serial.print(pulseCount);
    Serial.print(" || ");
    Serial.print("Flag: ");
    Serial.println(flag);
  
    pulseCount = 0;
    // Enable the interrupt again now that we've finished sending output
    attachInterrupt(digitalPinToInterrupt(waterFlowPin), pulseCounter, FALLING);
}

  /*
   * 
   * Keterangan Flag
   * Flag 0: Display Welcome, Menunggu Gelas Diletak (-> Flag 1)
   * Flag 1: Gelas sudah Diletak, Menunggu Push Button Ditekan (-> Flag 2 atau -> Flag 3)
   * Flag 2: Push Button Medium sudah Ditekan, Menunggu Kartu RFID Ditap (-> Flag 4)
   * Flag 3: Push Button Large sudah Ditekan, Menunggu Kartu RFID Ditap (-> Flag 5)
   * Flag 4: Process Firebase untuk Minuman Medium (-> Flag 6)
   * Flag 5: Process Firebase untuk Minuman Large  (-> Flag 7)
   * Flag 6: Display Transaksi Sukses untuk Minuman Medium, kemudian Trigger Water Pump (-> Flag 9)
   * Flag 7: Display Transaksi Sukses untuk Minuman Large, kemudian Trigger Water Pump (-> Flag 10)
   * Flag 8: Balance tidak Cukup, kembali ke Menunggu Push Button Ditekan (-> Flag 1)
   * Flag 9: Water Pump Hidup mengisi Ukuran Minuman Medium, Menunggu Selesai (-> Flag 11)
   * Flag 10: Water Pump Hidup mengisi Ukuran Minuman Large, Menunggu Selesai (-> Flag 11)
   * Flag 11: Water Pump selesai mengisi, Disuruh Ambil Gelas (-> Flag 12)
   * Flag 12: Ucapan Terima Kasih, kembali ke Tahap Awal Menunggu Gelas Diletak Lagi (-> Flag 0)
   * Flag 13: Jika Gelas tiba-tiba diambil di tengah proses, kembali ke Tahap Awal Menunggu Gelas Diletak Lagi (-> Flag 0)
   * 
   */


   if(flag == 0 || flag == 1 || flag == 2 || flag == 3 || flag == 4 || flag == 5 || flag == 6 || flag == 7 || flag == 8 || flag == 11 || flag == 12 || flag == 13)
   {
      checkWaterLevel();
      checkPeltier(); 
   }
   
 if(flag == 0)
  {
    
    if(flagTransition == 0)
    {
      DisplayWelcome();  
    }

    if(flagTransition == 1)
    {
      DisplayColdStatus();
    }
    
    if(displayTimer.isReady())
    {
      if(flagTransition == 0)
      {
        flagTransition = 1;
        displayTimer.reset();
        return;
      }
       if(flagTransition = 1)
      {
        flagTransition = 0;
        displayTimer.reset();
        return;
      }
    }

    if(distance >= distanceThreshold)
      {
        if(glassStatus == 0)
          {
            glassTimer.reset();
            glassStatus = 1; 
          }
        if(glassStatus == 1)
         { 
            if(glassTimer.isReady()) // Apabila Timer sudah berjalan selama 2 detik
            {
              flag = 1; // Pindah ke Tahap berikutnya
            }
         }
      }
          
     if(distance < distanceThreshold)
      {
          glassStatus = 0;
      }       
  }



  if(flag == 1)  
  {
        
    DisplayPushButton(); // Pilihan Push Button ditampilkan setelah Gelas Diletak    
    checkGlassPosition();
  }

//  Untuk send Email Pada saat Letak di Flag 6 Gagal
// Flag 2 atau Flag 3 masih bisa

  if(flag == 2 || flag == 3) // Jika Push Button Medium atau Large sudah ditekan
  {
    
    readRFID(); // RFID baru bisa bekerja apabila push button ditekan untuk pertama kalinya
     checkGlassPosition();
  }


  if(flag == 1 || flag == 2 || flag == 3)
  {
    readPushButton(); // Push Button Hanya bisa ditekan pada saat pemilihan ukuran medium atau large
  }


  if(flag == 4 || flag == 5)
  {
    processFirebase();
    checkGlassPosition();
  }


  // Untuk Keterangan Transaksi Memang tidak ada if-condition untuk Glass Taken Detection.
  if(flag == 6) // Transaksi Sukses Ukuran Medium
  {
      flagEmail = 1; // Email Medium
      
      DisplayMediumPaymentSuccess();

   
      if(flagBuzzer == 0)
      {
        EasyBuzzer.beep(frequency, oneBeep);
        EasyBuzzer.stopBeep();
        flagBuzzer = 1;
        timer.reset();
      }

      if(flagBuzzer == 1)
      {
        if(timer.isReady())
           {
              flag = 9; // Isi Air Medium
           }
      }
      
  }

  
  if(flag == 7) // Transaksi Sukses Ukuran Large
  {
   

     flagEmail = 2; // Email Large
     DisplayLargePaymentSuccess();
     if(flagBuzzer == 0)
     {
        EasyBuzzer.beep(frequency, oneBeep);
        EasyBuzzer.stopBeep();
        flagBuzzer = 1;
        timer.reset();
     }
     if(flagBuzzer == 1)
     {
        if(timer.isReady())
        {
          flag = 10; // Isi Air Large
        }
     }
  }

  
 if(flag == 8) // Transaksi Gagal
  {
    DisplayPaymentFailed();
       
    if(flagBuzzer == 0)
    {
    EasyBuzzer.beep(frequency, twoBeep);
    EasyBuzzer.stopBeep();
    timer.reset();
    flagBuzzer = 1;
 
    }
          
    if(flagBuzzer == 1)
    {
      if(timer.isReady())
         {
             backToFlag1();
         }
    }
  }
  
  if(flag == 9) // Isi Air Medium
  {
    DisplayFilling();

    /*
     * Solenoid Menyala 
     * 500 ms kemudian, Motor Menyala
     * Solenoid Mati
     * 500 ms kemudian, Motor Mati
     *  
     */
   if(flagSolenoid == 0)
    {
       digitalWrite(solenoidPin, HIGH);
       solenoidTimer.reset();
       flagSolenoid = 1;
    }

      if(flagSolenoid == 1)
    {
       if(solenoidTimer.isReady())
       {
          digitalWrite(motorPin, HIGH);
          flagSolenoid = 2;
       }
    }
    
      if(flagSolenoid == 2)
      {
        
          if( totalMilliLitres >= 220)
        {
          digitalWrite(solenoidPin, LOW);
          solenoidTimer.reset();
          flagSolenoid = 3;
        }
        
     }

       if(flagSolenoid == 3)
      {
           if(solenoidTimer.isReady())
         {
           digitalWrite(motorPin, LOW);
           flag = 11; // Ambil Gelas
         }
      }
  }
  
  if(flag == 10)
  { 
    
    DisplayFilling();

    if(flagSolenoid == 0)
    {
       digitalWrite(solenoidPin, HIGH);
       solenoidTimer.reset();
       flagSolenoid = 1;
    }

      if(flagSolenoid == 1)
    {
       if(solenoidTimer.isReady())
       {
          digitalWrite(motorPin, HIGH);
          flagSolenoid = 2;
       }
    }
    
      if(flagSolenoid == 2)
      {
        
          if( totalMilliLitres >= 300)
        {
          digitalWrite(solenoidPin, LOW);
          solenoidTimer.reset();
          flagSolenoid = 3;
        }
        
     
     }

       if(flagSolenoid == 3)
      {
           if(solenoidTimer.isReady())
         {
           digitalWrite(motorPin, LOW);
           flag = 11; // Ambil Gelas
         }
      }
  }

  if(flag == 11)
  {
    Firebase.endStream(firebaseData);
    DisplayTakeGlass();
     if(distance < distanceThreshold)
      {
        flag = 12; // Thank You (Flag Terakhir)
      }
  }

  if(flag == 12)
  {
    DisplayThankYou();    
        if(flagThankYou == 0)
        {
          timer.reset();
          flagThankYou = 1; 
        }
                
        if(flagThankYou == 1)
        {
           if(timer.isReady()) // Jika 3 Detik Sudah Berlalu
           {
              sendEmail(); // Kirim Email
              if(flagEmail == 3) // Jika Email sudah selesai dikirim
              {
                resetFlag();  // Kembali ke Tahap Awal      
              }
           }  
        }
  }  
  

  if(flag == 13)
  {
     DisplayGlassTaken();
     
     if(glassTakenStatus == 0)
      {
        flagDisplay = 7; // Untuk membaca fungsi DisplayGlassTaken()
        timer.reset();
        glassTakenStatus = 1; 
      }
              
      if(glassTakenStatus == 1)
      {
          if(timer.isReady())
         {
           resetFlag();
         }
      }   
  }
}

/*
 * 
 * Untuk Cek Posisi Gelas 
 * 
 */

 void checkGlassPosition()
 {
    if(distance < distanceThreshold)
      {
        flag = 13;
      }
 }
 
/*
 * 
 * Kembali ke Flag 1 (Disuruh Tekan Push Button kembali) jika Payment Gagal
 * 
 */

 void backToFlag1()
 {
       flag = 1;  
       flagDisplay = 1;
       flagButton1 = 0;
       flagButton2 = 0;
       flagBuzzer = 0;
 }
 
/*
 * 
 * Fungsi resetFlag() untuk kembali ke tahap awal (Flag 0) 
 * 
 */

void resetFlag()
{
  flag = 0;
  
  flagDisplay = 0;
  flagTransition = 0;
  
  flagEmail = 0;

  flagSolenoid = 0;
  flagThankYou = 0;
  
  glassStatus = 0;
  glassTakenStatus = 0;

  
  flagBuzzer = 0;
  flagButton1 = 0;
  flagButton2 = 0;
  
  totalMilliLitres = 0;
  
  displayTimer.reset();
}

/*
 * 
 * 
 * Untuk RFID
 * 
 * 
 */


void array_to_string(byte array[], unsigned int len, char buffer[])
{
   for (unsigned int i = 0; i < len; i++)
   {
      byte nib1 = (array[i] >> 4) & 0x0F;
      byte nib2 = (array[i] >> 0) & 0x0F;
      buffer[i*2+0] = nib1  < 0xA ? '0' + nib1  : 'A' + nib1  - 0xA;
      buffer[i*2+1] = nib2  < 0xA ? '0' + nib2  : 'A' + nib2  - 0xA;
   }
   buffer[len*2] = '\0';
}


void readRFID()
{
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
      return;
   }
    
   // Select one of the cards
   if ( ! mfrc522.PICC_ReadCardSerial()) {
      return;
   }

   array_to_string(mfrc522.uid.uidByte, 4, cardID);

   Serial.print("UID: ");
   Serial.print(cardID); //Print the output uid string
   // Kartu terdeteksi, send email, baru masuk ke Firebase
   if(flag == 2) // Flag = 2 didapat jika Push Button Medium ditekan
   {
      flag = 4; // Medium
   }

   if(flag == 3) // Flag = 3 didapat jika Push Button Large ditekan
   {
      flag = 5; // Large
   }
   
                         
 
    
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();

}

/*
 * 
 * Untuk Pemrosesan Firebase
 * 
 * 
 */

void processFirebase()
{
     balanceDirectory = mainRoot + cardID + balanceRoot;
     balanceParentNode = mainRoot + cardID;
     emailDirectory = mainRoot + cardID + emailRoot;
  
      mediumDirectory = espMainRoot + espIDNumber + orderRoot + mediumRoot;
      largeDirectory = espMainRoot + espIDNumber + orderRoot + largeRoot;
      espParentNode = espMainRoot + espIDNumber + orderRoot;   //  SystemID/12980728/Order/Medium

     Serial.print(" || ESP32 ID: ");
     Serial.print(espIDNumber);

     Firebase.beginStream(firebaseData, "");
       
     if (Firebase.getString(firebaseData, emailDirectory)) {
        if (firebaseData.dataType() == "string")
        {
           recipientEmail = firebaseData.stringData();
           Serial.print(" || Email: ");
           Serial.print(recipientEmail);
        }
    }
    else 
    {
      Serial.print(firebaseData.errorReason());
    }

    if(flag == 4) // Jika Push Button Medium sudah ditekan dan Kartu RFID sudah di-tap
    {
        if (Firebase.getInt(firebaseData, balanceDirectory)) { // Baca Data Balance skrg ada berapa
            if (firebaseData.dataType() == "int")
            {
               balanceValue = firebaseData.intData();
              Serial.print(" || Balance: ");
              Serial.print(balanceValue);         
            }
        }
        else 
        {
          Serial.print(firebaseData.errorReason());
        }
            
// Misalnya sekarang Balance yang dibaca ternyata ada 50000
           if(balanceValue >= 10000)
         {
               if (Firebase.getInt(firebaseData, mediumDirectory)) {
                    if (firebaseData.dataType() == "int")
                    {
                      mediumCount = firebaseData.intData();
                      mediumCount = mediumCount + 1;
                      jsonOrder.set(mediumRoot, mediumCount);
                      Firebase.updateNode(firebaseData, espParentNode, jsonOrder);  
                      
                      Serial.print(" || Medium Count: ");
                      Serial.print(mediumCount); 
                    }
               }
               else 
                {
                  Serial.print(firebaseData.errorReason());
                }
         
            updatedValue = balanceValue - 10000;
            jsonBalance.set(balanceRoot, updatedValue);
            Firebase.updateNode(firebaseData, balanceParentNode, jsonBalance); 
 
            flag = 6; // Transaksi Sukses, Pengisian Minuman Ukuran Medium
         }
                   
         if(balanceValue < 10000)
          {
            flag = 8; // Insufficient Balance
          }
    }
        

     if(flag == 5) // Jika Push Button Large sudah ditekan dan Kartu RFID sudah di-tap
    {
   
        if (Firebase.getInt(firebaseData, balanceDirectory)) { // Baca Data Balance skrg ada berapa
            if (firebaseData.dataType() == "int")
            {
               balanceValue = firebaseData.intData();
              Serial.print(" || Balance: ");
              Serial.print(balanceValue);
            }
        }
        else 
        {
          Serial.print(firebaseData.errorReason());
        }

       if(balanceValue >= 20000) // Baca Apakah Balancenya lebih besar dari 20000
             {
                   if (Firebase.getInt(firebaseData, largeDirectory)) {
                        if (firebaseData.dataType() == "int")
                        {
                          largeCount = firebaseData.intData();
                          largeCount = largeCount + 1;
                          jsonOrder.set(largeRoot, largeCount);
                          Firebase.updateNode(firebaseData, espParentNode, jsonOrder);  

                          Serial.print(" || Large Count: ");
                          Serial.print(largeCount);
                        }
                   }
                   else 
                    {
                      Serial.print(firebaseData.errorReason());
                    }
                updatedValue = balanceValue - 20000;
                jsonBalance.set(balanceRoot, updatedValue);
                Firebase.updateNode(firebaseData, balanceParentNode, jsonBalance);  

                  // Balance sudah dipotong sebelum Buzzer berbunyi

                flag = 7;  // Transaksi Sukses, Pengisian Minuman Ukuran Large
             }
             
         if(balanceValue < 20000)
        { 
          flag = 8;   // Insufficient Balance
        }
    }
}

/*
 * 
 * 
 * Untuk Push Button
 * 
 * 
 */
void readPushButton()
{

  unsigned long currentMillis = millis();

  readMediumButton = digitalRead(mediumButtonPin);
  readLargeButton = digitalRead(largeButtonPin);


  if (readMediumButton != lastButtonState1) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }
  
  if (readLargeButton != lastButtonState2) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }


  if ((millis() - lastDebounceTime) > debounceDelay) {
          if( readMediumButton == HIGH && flagButton1 == 0 ) 
          {
              flagButton1 = 1;
              flagButton2 = 0;
              
          }
          if( readMediumButton == LOW && flagButton1 == 1 ) 
          {
          
              Serial.println();
              Serial.println("Medium Selected");
              Serial.println("Place your Card");
              flagDisplay = 2;
              flagButton1 = 2;
              
          }
          
           if( readLargeButton == HIGH && flagButton2 == 0 ) 
          {
              flagButton1 = 0;
              flagButton2 = 1;
              
          }
          if( readLargeButton == LOW && flagButton2 == 1 ) 
          { 
             
              Serial.println();
              Serial.println("Large Selected");
              Serial.println("Place your Card");
              flagDisplay = 2;
              flagButton2 = 2;
          }
          
  }

    if(flagButton1 == 2)
    { 
        flag = 2; // Flag untuk size Medium
        DisplayMedium();
    }
    
    if(flagButton2 == 2)
    {
        flag = 3; // Flag untuk size Large
        DisplayLarge();
    }    

      lastButtonState1 = readMediumButton;
      lastButtonState2 = readLargeButton;
  
}

/*
 * 
 * 
 * Untuk Fitur Display
 * 
 * 
 */
 
void DisplayWelcome()
{
if(flagDisplay == 0)
 {
    tft.fillRectangle(0, 0, 175, 219, COLOR_WHITE); // (x-INITIAL, y-INITIAL, x-LAST, y-LAST, color). Ini bisa dimanfaatkan untuk clear tampilan
  
    // Urutan strukturnya harus seperti ini: setFont, setBackgroundColor, drawText
    tft.setFont(Terminal12x16);
    tft.setBackgroundColor(COLOR_WHITE);
    tft.drawText(45, 80, "WELCOME", COLOR_BLACK);

    // Urutan strukturnya harus seperti ini: setFont, setBackgroundColor, drawText
    tft.setFont(Terminal12x16);
    tft.setBackgroundColor(COLOR_WHITE);
    tft.drawText(15, 120, "PUT THE GLASS", COLOR_BLACK);
    flagDisplay = 1;
 }
}


void DisplayColdStatus()
{
    if(flagDisplay == 1)
   {
      tft.fillRectangle(0, 0, 175, 219, COLOR_WHITE);
    
        tft.setFont(Terminal12x16);
        tft.setBackgroundColor(COLOR_WHITE);
        tft.drawText(60, 60, "COLD:", COLOR_BLACK);
    
        tft.setFont(Terminal12x16);
        tft.setBackgroundColor(COLOR_WHITE);
        tft.drawText(60, 90, coldString, COLOR_BLACK);
        
     flagDisplay = 0;
   }
}

void DisplayPushButton()
{
if(flagDisplay == 0 || flagDisplay == 1)
 {
    tft.fillRectangle(0, 0, 175, 219, COLOR_WHITE); // (x-INITIAL, y-INITIAL, x-LAST, y-LAST, color). Ini bisa dimanfaatkan untuk clear tampilan

    // Urutan strukturnya harus seperti ini: setFont, setBackgroundColor, drawText
    tft.setFont(Terminal12x16);
    tft.setBackgroundColor(COLOR_WHITE);
    tft.drawText(25, 80, "PLEASE PUSH", COLOR_BLACK);

    // Urutan strukturnya harus seperti ini: setFont, setBackgroundColor, drawText
    tft.setFont(Terminal12x16);
    tft.setBackgroundColor(COLOR_WHITE);
    tft.drawText(30, 120, "THE BUTTON", COLOR_BLACK);
    flagDisplay = 2;
 }
}


void DisplayMedium()
{
if(flagDisplay == 2)
 {
    tft.fillRectangle(0, 0, 175, 219, COLOR_WHITE); // (x-INITIAL, y-INITIAL, x-LAST, y-LAST, color). Ini bisa dimanfaatkan untuk clear tampilan

    // Urutan strukturnya harus seperti ini: setFont, setBackgroundColor, drawText
    tft.setFont(Terminal12x16);
    tft.setBackgroundColor(COLOR_WHITE);
    tft.drawText(30, 60, "MEDIUM SIZE", COLOR_BLACK);

    // Urutan strukturnya harus seperti ini: setFont, setBackgroundColor, drawText
    tft.setFont(Terminal12x16);
    tft.setBackgroundColor(COLOR_WHITE);
    tft.drawText(30, 100, "PUT CARD", COLOR_BLACK);
     flagDisplay = 3;
 }
}


void DisplayLarge()
{
if(flagDisplay == 2)
 {
  tft.fillRectangle(0, 0, 175, 219, COLOR_WHITE); // (x-INITIAL, y-INITIAL, x-LAST, y-LAST, color). Ini bisa dimanfaatkan untuk clear tampilan

  // Urutan strukturnya harus seperti ini: setFont, setBackgroundColor, drawText
    tft.setFont(Terminal12x16);
    tft.setBackgroundColor(COLOR_WHITE);
    tft.drawText(30, 60, "LARGE SIZE", COLOR_BLACK);

    // Urutan strukturnya harus seperti ini: setFont, setBackgroundColor, drawText
    tft.setFont(Terminal12x16);
    tft.setBackgroundColor(COLOR_WHITE);
    tft.drawText(30, 100, "PUT CARD", COLOR_BLACK);

 flagDisplay = 3;
 }
}

void DisplayMediumPaymentSuccess()
{
if(flagDisplay == 3)
 {
  tft.fillRectangle(0, 0, 175, 219, COLOR_WHITE); 

  // Urutan strukturnya harus seperti ini: setFont, setBackgroundColor, drawText
    tft.setFont(Terminal12x16);
    tft.setBackgroundColor(COLOR_WHITE);
    tft.drawText(50, 60, "PAYMENT", COLOR_BLACK);

    // Urutan strukturnya harus seperti ini: setFont, setBackgroundColor, drawText
    tft.setFont(Terminal12x16);
    tft.setBackgroundColor(COLOR_WHITE);
    tft.drawText(50, 80, "SUCCESS", COLOR_BLACK);

    // Urutan strukturnya harus seperti ini: setFont, setBackgroundColor, drawText
    tft.setFont(Terminal12x16);
    tft.setBackgroundColor(COLOR_WHITE);
    tft.drawText(50, 100, "BALANCE", COLOR_BLACK);

    // Urutan strukturnya harus seperti ini: setFont, setBackgroundColor, drawText
    tft.setFont(Terminal12x16);
    tft.setBackgroundColor(COLOR_WHITE);
    tft.drawText(50, 120, "-10000", COLOR_BLACK);

 flagDisplay = 4;
 }
}

void DisplayLargePaymentSuccess()
{
if(flagDisplay == 3)
 {
  tft.fillRectangle(0, 0, 175, 219, COLOR_WHITE); 

  // Urutan strukturnya harus seperti ini: setFont, setBackgroundColor, drawText
    tft.setFont(Terminal12x16);
    tft.setBackgroundColor(COLOR_WHITE);
    tft.drawText(50, 60, "PAYMENT", COLOR_BLACK);

    // Urutan strukturnya harus seperti ini: setFont, setBackgroundColor, drawText
    tft.setFont(Terminal12x16);
    tft.setBackgroundColor(COLOR_WHITE);
    tft.drawText(50, 80, "SUCCESS", COLOR_BLACK);

    // Urutan strukturnya harus seperti ini: setFont, setBackgroundColor, drawText
    tft.setFont(Terminal12x16);
    tft.setBackgroundColor(COLOR_WHITE);
    tft.drawText(50, 100, "BALANCE", COLOR_BLACK);

    // Urutan strukturnya harus seperti ini: setFont, setBackgroundColor, drawText
    tft.setFont(Terminal12x16);
    tft.setBackgroundColor(COLOR_WHITE);
    tft.drawText(50, 120, "-20000", COLOR_BLACK);

 flagDisplay = 4;
 }
}

void DisplayPaymentFailed()
{
if(flagDisplay == 3)
 {
  tft.fillRectangle(0, 0, 175, 219, COLOR_WHITE); // (x-INITIAL, y-INITIAL, x-LAST, y-LAST, color). Ini bisa dimanfaatkan untuk clear tampilan

  // Urutan strukturnya harus seperti ini: setFont, setBackgroundColor, drawText
    tft.setFont(Terminal12x16);
    tft.setBackgroundColor(COLOR_WHITE);
    tft.drawText(20, 60, "INSUFFICIENT", COLOR_BLACK);

    // Urutan strukturnya harus seperti ini: setFont, setBackgroundColor, drawText
    tft.setFont(Terminal12x16);
    tft.setBackgroundColor(COLOR_WHITE);
    tft.drawText(50, 100, "BALANCE", COLOR_BLACK);

 flagDisplay = 4;
 }
}

void DisplayFilling()
{
if(flagDisplay == 4)
 {
  tft.fillRectangle(0, 0, 175, 219, COLOR_WHITE); // (x-INITIAL, y-INITIAL, x-LAST, y-LAST, color). Ini bisa dimanfaatkan untuk clear tampilan

  // Urutan strukturnya harus seperti ini: setFont, setBackgroundColor, drawText
    tft.setFont(Terminal12x16);
    tft.setBackgroundColor(COLOR_WHITE);
    tft.drawText(30, 60, "FILLING...", COLOR_BLACK);
 flagDisplay = 5;
 }
}

void DisplayTakeGlass()
{
if(flagDisplay == 5)
 {
  tft.fillRectangle(0, 0, 175, 219, COLOR_WHITE); // (x-INITIAL, y-INITIAL, x-LAST, y-LAST, color). Ini bisa dimanfaatkan untuk clear tampilan

  // Urutan strukturnya harus seperti ini: setFont, setBackgroundColor, drawText
    tft.setFont(Terminal12x16);
    tft.setBackgroundColor(COLOR_WHITE);
    tft.drawText(30, 60, "TAKE GLASS", COLOR_BLACK);
 flagDisplay = 6;
 }
}

void DisplayThankYou()
{
if(flagDisplay == 6)
 {
  tft.fillRectangle(0, 0, 175, 219, COLOR_WHITE); // (x-INITIAL, y-INITIAL, x-LAST, y-LAST, color). Ini bisa dimanfaatkan untuk clear tampilan

  // Urutan strukturnya harus seperti ini: setFont, setBackgroundColor, drawText
    tft.setFont(Terminal12x16);
    tft.setBackgroundColor(COLOR_WHITE);
    tft.drawText(30, 60, "THANK YOU", COLOR_BLACK);
 flagDisplay = 0;
 }
}

void DisplayGlassTaken()
{
if(flagDisplay == 7)
 {
  tft.fillRectangle(0, 0, 175, 219, COLOR_WHITE);

    tft.setFont(Terminal12x16);
    tft.setBackgroundColor(COLOR_WHITE);
    tft.drawText(30, 60, "GLASS TAKEN", COLOR_BLACK);

    tft.setFont(Terminal12x16);
    tft.setBackgroundColor(COLOR_WHITE);
    tft.drawText(1, 100, "ORDER CANCELLED", COLOR_BLACK);
    
 flagDisplay = 0;
 }
}

/*
 * 
 * 
 * Untuk Sensor Jarak
 * 
 * 
 * 
 */
 
int serial_putc( char c, struct __file * )
{
  Serial.write( c );
  return c;
}

void SensorRead(unsigned char addr,unsigned char* datbuf,unsigned char cnt) 
{
  unsigned short result=0;
  Wire.beginTransmission(82);
  Wire.write(byte(addr));      
  Wire.endTransmission();     
  unsigned long currentMicros = micros();
   if((unsigned long)(currentMicros - previousMicros) >= 1000) // Tunggu 1000 us ( Tunggu 1 ms)
   {
      Wire.requestFrom(82, cnt);
      if (cnt <= Wire.available()) 
      { 
        *datbuf++ = Wire.read();  
        *datbuf++ = Wire.read(); 
      }
      previousMicros = currentMicros;
   }
}

int ReadDistance(){
    SensorRead(0x00,i2c_rx_buf,2);
    lenth_val=i2c_rx_buf[0];
    lenth_val=lenth_val<<8;
    lenth_val|=i2c_rx_buf[1];
    return lenth_val;
}

/*
 * 
 * 
 * 
 * Untuk Sensor Water Flow
 * 
 * 
 * 
 */

void pulseCounter()
{
  pulseCount++;
}


/*
 * 
 * 
 * Untuk Pengiriman Email
 * 
 * 
 */
 
void sendEmail()
{
  if(flagEmail == 1) // Medium
  {
     headerHTML = " <h1>Automatic Dispenser Order</h1> ";
     drinkSizeHTML = "<p>Size: Medium</p> <p>Balance -10000</p>";
     updatedBalanceHTML = "<p></p> <span>Your Balance: </span>" + String(updatedValue);
     
      emailHTML = headerHTML + drinkSizeHTML + updatedBalanceHTML;
    
      EMailSender::EMailMessage orderMessage;
      orderMessage.subject = "Automatic Dispenser Order Receipt";
      orderMessage.message = emailHTML;
      EMailSender::Response resp = emailSend.send(recipientEmail, orderMessage);
    
      Serial.println("Sending status: ");
      Serial.println(resp.status);
      Serial.println(resp.code);
      Serial.println(resp.desc);
      flagEmail = 3;
  }

   if(flagEmail == 2) // Large
  {
      headerHTML = " <h1>Automatic Dispenser Order</h1> ";
      drinkSizeHTML = "<p>Size: Large</p> <p>Balance -20000</p>";
      updatedBalanceHTML = "<p></p> <span>Your Balance: </span>" + String(updatedValue);
     
      emailHTML = headerHTML + drinkSizeHTML + updatedBalanceHTML;
    
      EMailSender::EMailMessage orderMessage;
      orderMessage.subject = "Automatic Dispenser Order Receipt";
      orderMessage.message = emailHTML;
      EMailSender::Response resp = emailSend.send(recipientEmail, orderMessage);
  
      Serial.println("Sending status: ");
      Serial.println(resp.status);
      Serial.println(resp.code);
      Serial.println(resp.desc);
      flagEmail = 3;
  }
}

/*
 * 
 * Untuk mengecek Water Level
 * 
 * 
 */
 
void checkWaterLevel()
{
  espWaterParentNode = espMainRoot + espIDNumber;
     if(waterLevelTimer.isReady())
       {
        
          int waterLevelValue = digitalRead(waterLevelPin);
        
          if(waterLevelValue == HIGH)
          {
              waterLevelInfo = "BELUM HABIS";
              jsonWaterLevel.set(waterLevelRoot, waterLevelInfo);
              Firebase.updateNode(firebaseDataWater, espWaterParentNode, jsonWaterLevel);  
          }
          
          if(waterLevelValue == LOW)
          {
              waterLevelInfo = "HAMPIR HABIS";
              jsonWaterLevel.set(waterLevelRoot, waterLevelInfo);
              Firebase.updateNode(firebaseDataWater, espWaterParentNode, jsonWaterLevel);  
          }
              Serial.print(" || Water Level: ");
              Serial.print(waterLevelInfo);
              Serial.print(" || ");
              waterLevelTimer.reset();
       }
  
}


/*
 * 
 * Untuk Monitoring Temperature di Wadah Pendingin dan Switching Peltier
 * 
 */
 
void checkPeltier()
{
 
  if(temperatureTimer.isReady())
   {
      sensors.requestTemperatures(); 
      temperature = sensors.getTempCByIndex(0);
      Serial.println();
      Serial.print(" || Temperature: ");
      Serial.print(temperature);
      Serial.print(" || ");
      Serial.println();
      temperatureTimer.reset();
   }
  
      if(temperature <= 16 && flagPeltier == 0)
      {
        digitalWrite(peltierPin, LOW);
        flagPeltier = 1;
        coldString = "READY";
      }
      if(temperature >= 17 && flagPeltier == 1)
      {
        digitalWrite(peltierPin, HIGH);
        flagPeltier = 0;
      }
  
      if(temperature >= 20)
      {
        digitalWrite(peltierPin, HIGH);
        coldString = "NOT READY";
      }  
}


/*
 * 
 * 
 * End
 * 
 * 
 */