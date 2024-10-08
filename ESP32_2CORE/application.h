#ifndef __CORE_0_H__
#define __CORE_0_H__

#include <DHT.h>
#include <LiquidCrystal_I2C.h>


//==================
//  SENSOR
//==================
int moi1Data = 0;
int moi2Data = 0;
int gasData = 0;
unsigned long getSensorLastTime = millis();

// ************ hàm chính ************ 
void setupSensor() {
    pinMode(LED_PIN, OUTPUT);
    pinMode(FAN_PIN, OUTPUT);
    pinMode(PUMP_PIN, OUTPUT);

    pinMode(GAS_PIN, INPUT);
    pinMode(MOI_1_PIN, INPUT);
    pinMode(MOI_2_PIN, INPUT);
    Serial.println("[Sensor] setup Sensor done!");
}

void loopSensor() {
    if (millis() - getSensorLastTime < GET_SENSOR_DATA_TIME * 1000)
        return;
    getSensorLastTime = millis();

    int moi1Raw = analogRead(MOI_1_PIN);
    int moi2Raw = analogRead(MOI_2_PIN);
    int gasRaw = analogRead(GAS_PIN);

    moi1Data = map(moi1Raw, 0, 4095, 0, 100);
    moi2Data = map(moi2Raw, 0, 4095, 0, 100);
    gasData = map(gasRaw, 0, 4095, 0, 100);
}


//==================
//  DHT 11
//==================
DHT dht(DHT_PIN, DHT11);

String humData = "00.0";
String tempData = "00.0";

unsigned long getDHTLastTime = millis();

// ************ hàm hỗ trợ ************
bool getDHT11Data(){
    // lấy dữ liệu tử DHT11
    float hum = dht.readHumidity();
    float temp = dht.readTemperature();
    // nếu trả về là nan (00.0): báo thiết bị nối sai dây hoặc bị lỗi
    if (isnan(hum) || isnan(temp)){
        tempData = "00.0";
        humData = "00.0";
        Serial.println("[DHT err] DHT can't get data.");
        return false;
    }
    
    // nêu đúng dữ liệu sẽ trả về 23.7 và 70.0 (23.7 độ C và độ ẩm 70% )
    tempData = String(temp);
    humData = String(hum);
    return true;
}

// ************ hàm chính ************ 
void setupDHT() {
    dht.begin();
    Serial.println("[DHT] setup DHT done!");
}

void loopDHT() {
    if (millis() - getDHTLastTime < DHT_GET_DATA_TIME * 1000)
        return;
    getDHTLastTime = millis();

    getDHT11Data();
}

//==================
// INTERRUPT
//==================
unsigned long pressLastTime = millis();
bool isRunningInterrupt = false;

Mode modeStatus = Mode::AUTO;
bool modePressed = false;

bool ledStatus = false;
bool ledPressed = false;

bool fanStatus = false;
bool fanPressed = false;

bool pumpStatus = false;
bool pumpPressed = false;

// ************ hàm hỗ trợ ************
void handlePressed(); // hàm này được viết ở ESP32_2CORE.ino

void IRAM_ATTR pressedModeInterrupt() {
    // trong 500ms thì chỉ nhận 1 lần nhấn
    if (millis() - pressLastTime < PRESS_DEBOUNCE_TIME)
        return;
    pressLastTime = millis();

    // kiểm tra xem có đang sử dụng ngắt chưa?
    if (isRunningInterrupt) return;
    // set trạng thái (đang sử dụng "ngắt")
    isRunningInterrupt = true;  
    // set trạng thái (nút led đã đc nhấn)
    modePressed = true;
}

void IRAM_ATTR pressedLedInterrupt() {
    if (millis() - pressLastTime < PRESS_DEBOUNCE_TIME)
        return;
    pressLastTime = millis();

    if (isRunningInterrupt) return;
    isRunningInterrupt = true;  
    ledPressed = true;
}
void IRAM_ATTR pressedFanInterrupt() {
    if (millis() - pressLastTime < PRESS_DEBOUNCE_TIME)
        return;
    pressLastTime = millis();

    if (isRunningInterrupt) return;
    isRunningInterrupt = true;  
    fanPressed = true;
}
void IRAM_ATTR pressedPumpInterrupt(){
    if (millis() - pressLastTime < PRESS_DEBOUNCE_TIME)
        return;
    pressLastTime = millis();

    if (isRunningInterrupt) return;
    isRunningInterrupt = true;  
    pumpPressed = true;
}

// ************ hàm chính ************ 
void setupInterrupt() {
    pinMode(BTN_MODE_PIN, INPUT);
    pinMode(BTN_LED_PIN, INPUT);
    pinMode(BTN_FAN_PIN, INPUT);
    pinMode(BTN_PUMP_PIN, INPUT);
    attachInterrupt(digitalPinToInterrupt(BTN_MODE_PIN), pressedModeInterrupt, FALLING);
    attachInterrupt(digitalPinToInterrupt(BTN_LED_PIN), pressedLedInterrupt, FALLING);
    attachInterrupt(digitalPinToInterrupt(BTN_FAN_PIN), pressedFanInterrupt, FALLING);
    attachInterrupt(digitalPinToInterrupt(BTN_PUMP_PIN), pressedPumpInterrupt, FALLING);
    
    Serial.println("[Interrupt] setup Interrupt done!");
}

//==================
// LCD
//==================
LiquidCrystal_I2C lcd(LCD_I2C_ADDR, LCD_I2C_NUM_COL, LCD_I2C_NUM_ROW);
unsigned long changeScreenLastTime = millis();
uint8_t currentScreen = 0;

// ************ hàm hỗ trợ ************
void clearLCD() {
    lcd.clear();
}

void setBackLightLCD(bool light) {
    light ? lcd.backlight() : lcd.noBacklight();
}

void printLCD(int col, int row, String mess) {
    String stringPrint = mess;
    // get chiều dài chuỗi
    int len = mess.length();
    // kiểm tra chuỗi có thể in đủ trên màn hình k?
    if (len > LCD_I2C_NUM_COL - col){
        // tách cuỗi có thể in (stringPrint) và chuỗi k thể in (stringCutOff)
        String stringPrint = mess.substring(0, len - 1);
        String stringCutOff = mess.substring(len - 1);
        Serial.printf("[LCD warn] cut-off: %s\n", stringCutOff.c_str());
    }
    // set vị trí con trỏ trên LCD
    lcd.setCursor(col, row);
    // in ra màn hình
    lcd.print(stringPrint);
}

void scrollPrintLCD(int col, int row, String mess, int delayTime, int lenView) {
    String messLong;
    int character = lenView;
    int len = mess.length();
    if (len > character)
    {
        for (int i = 0; i < character; i++)
            messLong = "" + mess + "-" + mess;
        for (int pos = 0; pos <= (len + 1); pos++)
        {
            lcd.setCursor(col, row);
            lcd.print(messLong.substring(pos, pos + character));
            delay(delayTime);
        }
        return;
    }
    lcd.setCursor(col, row);
    lcd.print(mess);
}

void printCenterLCD(int row, String mess)
{
    String stringPrint = "";
    int len = mess.length();
    // nếu mess dài >= LCD_I2C_NUM_COL thì in như bình thường.
    if (len > LCD_I2C_NUM_COL - 2) {
        printLCD(0, row, mess);
        return;
    }

    // tính toán vị trí bắt đầu in giữa dòng.
    int halfLine = floor(LCD_I2C_NUM_COL / 2);
    int startPrintPosition = floor(len / 2);
    int numWhiteSpace = halfLine - startPrintPosition;

    // thêm khoảng trắng đằng trước
    for (size_t i = 0; i < numWhiteSpace; i++)
        stringPrint += " ";

    // thêm mess 
    stringPrint += mess;
    // set vị trí con trỏ trên LCD
    lcd.setCursor(0, row);
    // in ra màn hình
    lcd.print(stringPrint);
}

// show network info
void screenInit(){
    // line 0
    String line0 = "PROJECT: DATN";
    String line1 = "HELLO !!!";
    
    clearLCD();
    delay(10);

    printCenterLCD(0, line0);
    delay(10);
    
    printCenterLCD(1, line1);
    delay(10);
}

void screen0(){
    // line 0
    String line0 = "WF: " + WiFi.SSID();            // WF: WIFI_NAME
    String line1 = WiFi.localIP().toString();       // 192.168.1.123
    
    clearLCD();
    delay(10);

    printCenterLCD(0, line0);
    delay(10);
    
    printCenterLCD(1, line1);
    delay(10);
}

// show DHT11
void screen1(){
    String line0 = tempData + "\xDF" + humData + "%";                           // 32.0°C - 75.0%
    String line1 = "moi: " + String(moi1Data) + "%, " + String(moi2Data) + "%"; // moi: 75% - 75%

    clearLCD();
    delay(10);

    printCenterLCD(0, line0);
    delay(10);

    printLCD(0, 1, line1);
    delay(10);
}

// ************ hàm chính ************ 
void setupLCD() {
    lcd.init();
    // bật sáng màn hình
    setBackLightLCD(1);
    // debug
    Serial.println("[LCD] Setup LCD done!!!");
    // 
    screenInit();
}

void loopLCD() {
    if (millis() - changeScreenLastTime > LCD_CHANGE_SCREEN_TIME * 1000){
        changeScreenLastTime = millis();
        currentScreen++;
        return;
    }

    if(currentScreen == 0) {
        screen0();
        return;
    }

    if(currentScreen == 1) {
        screen1();
        // reset màn hình
        currentScreen = 0;
        return;
    }
}

//==================
// SETUP CORE
//==================
TaskHandle_t ApplicationTask;

// ************ function core ************

void setupApp() {
    // khai báo các chân IO
    setupSensor();
    delay(10);

    // setup DHT11
    setupDHT();
    delay(10);

    // khai báo chân sử dụng ngắt
    setupInterrupt();
    delay(10);

    // setup LCD
    setupLCD();
    delay(10);
}

void loopApp() {
    // sử lý nút nhấn.
    handlePressed();
    // lấy nhiệt độ, độ ẩm từ DHT11 (10s/ lần)
    loopDHT();
    // lấy độ ẩm từ cảm biến, lấy độ sạch không khí từ cb ga (5s/ lần)
    loopSensor();
    // hiển thị 2 màn hình thay phiên nhau (5s/ 1 màn hình)
    loopLCD();
}

void ApplicationFuncCode( void * pvParameters )
{
    delay(100);

    // debug
    Serial.printf("[Multitasking] Running ApplicationFuncCode() on Core %i\n", xPortGetCoreID());

    // setup application
    setupApp();
    
    while (true)
    {
        loopApp();
    }
}

// ************ hàm chính ************
void setupAppCore() {
    int result = xTaskCreate(
                    ApplicationFuncCode,    /* task function */
                    "application",          /* name of task */
                    8192,             /* stack size of task */
                    NULL,             /* parameter of the task */
                    1, /* priority of the task, 0 = tskIDLE_PRIORITY is lowest priority */
                    &ApplicationTask);   /* task handle/pointer to keep track of created task */

    if (result)     Serial.printf("[Multitasking] Task network created: %i\n", result);
    else            Serial.printf("[Multitasking] Failed to create Task 2 %i\n", result);
}
#endif