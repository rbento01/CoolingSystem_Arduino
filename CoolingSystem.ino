#include <dht_nonblocking.h>
#include <SoftwareSerial.h> //Software Serial Port
#define DHT_SENSOR_TYPE DHT_TYPE_11
//bluetooth
#define RxD 10
#define TxD 11

static const int DHT_SENSOR_PIN = 2;
DHT_nonblocking dht_sensor(DHT_SENSOR_PIN, DHT_SENSOR_TYPE );

boolean fan_on = false;

//bluetooth
SoftwareSerial blueToothSerial(RxD, TxD);

#define ENABLE 5
#define DIRA 3
#define DIRB 4

void setup() {
    Serial.begin(9600);

    pinMode(ENABLE, OUTPUT);
    pinMode(DIRA, OUTPUT);
    pinMode(DIRB, OUTPUT);
    digitalWrite(DIRA, HIGH); //one way
    digitalWrite(DIRB, LOW);
    digitalWrite(ENABLE, LOW); // enable off

    //bluetooth
    pinMode(RxD, INPUT);
    pinMode(TxD, OUTPUT);
    blueToothSerial.begin(38400); // Baudrate, you can change the baudrate = 9600
}

/*
 * Poll for a measurement, keeping the state machine alive.  Returns
 * true if a measurement is available.
 */
static bool measure_environment(float* temperature, float* humidity) {
    static unsigned long measurement_timestamp = millis();

    /* Measure once every four seconds. */
    if (millis() - measurement_timestamp > 3000ul) {
        if (dht_sensor.measure(temperature, humidity) == true) {
            measurement_timestamp = millis();
            return (true);
        }
    }

    return (false);
}

void loop() {
    float temperature;
    float humidity;

    //bluetooth
    char recvChar;

    if (measure_environment(&temperature, &humidity) == true) {
        if (temperature >= 25) {
            digitalWrite(ENABLE, HIGH);
            if (!fan_on) {
                Serial.println("High temperature - turning on fan");
                Serial.print("char= ");
                Serial.println(recvChar);
                fan_on = true;
            }
        } else {
            digitalWrite(ENABLE, LOW);
            if (fan_on) {
                Serial.println("Low temperature - turning off fan");
                Serial.print("char= ");
                Serial.println(recvChar);
                fan_on = false;
            }
        }
        Serial.print("T = ");
        Serial.print(temperature, 1);
        Serial.print(" deg. C, H = ");
        Serial.print(humidity, 1);
        Serial.println("%");
        Serial.print("char= ");
        Serial.println(recvChar);

    }
    if (blueToothSerial.available()) { //check if there's any data sent from the remote BT shield
        recvChar = blueToothSerial.read();
        Serial.print(recvChar);
    }
    if (Serial.available()) { //check if there's any data sent from the local serial terminal
                              //you can add the other applications here
        recvChar = Serial.read();
        blueToothSerial.print(recvChar);
    }
        //bluetooth
    switch (recvChar) {
        case 'b':
            digitalWrite(ENABLE, HIGH);
            if (!fan_on) {
                Serial.println("High temperature - turning on fan");
                fan_on = true;
            }
            blueToothSerial.println("High temperature - turning on fan");
            break;
        case 'f':
            digitalWrite(ENABLE, LOW);
            if (fan_on) {
                Serial.println("Low temperature - turning off fan");
                fan_on = false;
            }
            blueToothSerial.println("Low temperature - turning off fan");
            break;
    }
}
