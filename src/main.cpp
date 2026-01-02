#include <Arduino.h>

// Pins
const int GREEN_LED_PIN   = 18;
const int RED_LED_PIN     = 23;
const int MECH_SIGNAL_PIN_PASSED = 19;
const int MECH_SIGNAL_PIN_FAILED = 21;
const int MECH_SIGNAL_PIN_INPUT  = 22;
const int ENABLE_MECH_PIN = 5;
const int BREAK_PIN       = 4;

String incomingMessage;

int useMechanicalController = 1; // 1 = yes, 0 = no 
int mechanicalCompletionSignal = 0; // signal from mech controller (not used in this simulation)
bool mechDone = false;

void setup() {
    Serial.begin(9600);
    delay(2000);  // allow UART to stabilize

    pinMode(GREEN_LED_PIN, OUTPUT);
    pinMode(RED_LED_PIN, OUTPUT);
    pinMode(MECH_SIGNAL_PIN_PASSED, OUTPUT);       // or INPUT_PULLUP later
    pinMode(MECH_SIGNAL_PIN_FAILED, OUTPUT);
    pinMode(MECH_SIGNAL_PIN_INPUT, INPUT);
    pinMode(ENABLE_MECH_PIN, OUTPUT);
    
    pinMode(BREAK_PIN, INPUT_PULLUP);

    digitalWrite(GREEN_LED_PIN, LOW);
    digitalWrite(RED_LED_PIN, LOW);
    digitalWrite(ENABLE_MECH_PIN, HIGH);
    
    Serial.println("ESP READY");

    if (Serial.available() > 0){
        Serial.flush(); 
        incomingMessage = Serial.readStringUntil('\n');
        incomingMessage.trim();

        // --- Optional mech config handshake (PC sends 2 lines):
        // MECH_CONFIG
        // USE_MECH   or   NO_MECH
        unsigned long t0 = millis();
        while (millis() - t0 < 3000) {                 // wait up to 3s for config
            if (Serial.available() > 0) {
                String line = Serial.readStringUntil('\n');
                line.trim();
                if (line == "MECH_CONFIG") {
                    // read next line as value
                    while (Serial.available() == 0) delay(10);
                    String val = Serial.readStringUntil('\n');
                    val.trim();
                    useMechanicalController = (val == "USE_MECH") ? 1 : 0;
                    Serial.println("MECH_CONFIG_ACK");
                    Serial.flush();
                    break;
                }
            }
        }

        delay(100);

        if (incomingMessage.endsWith("USE_MECH")){
            useMechanicalController = 1;    
        }
        else{
            useMechanicalController = 0;    
        }
    }



}

void loop() {
    if (Serial.available() > 0) {
        incomingMessage = Serial.readStringUntil('\n');
        incomingMessage.trim();

        if (incomingMessage == "MECH_CONFIG") {
            // wait next line (USE_MECH / NO_MECH)
            unsigned long t0 = millis();
            while (Serial.available() == 0 && millis() - t0 < 1000) delay(1);

            if (Serial.available() > 0) {
                String val = Serial.readStringUntil('\n');
                val.trim();
                useMechanicalController = (val == "USE_MECH") ? 1 : 0;
                Serial.println("MECH_CONFIG_ACK");
                Serial.flush();
            }
            return; // done handling this message
        }


        if (incomingMessage.endsWith("PASS")) {

            // indicate success and send ACK
            digitalWrite(GREEN_LED_PIN, HIGH);
            digitalWrite(RED_LED_PIN, LOW);
            Serial.println("ACK");
            Serial.flush();

            if(useMechanicalController){
                digitalWrite(MECH_SIGNAL_PIN_PASSED, HIGH);
                delay(200);

                mechanicalCompletionSignal = digitalRead(MECH_SIGNAL_PIN_INPUT);
                unsigned long startTime = millis();
                unsigned long elapsedTime = 0;

                while (mechanicalCompletionSignal == 0) {
                    mechanicalCompletionSignal = digitalRead(MECH_SIGNAL_PIN_INPUT);
                    elapsedTime = millis() - startTime;

                if (digitalRead(BREAK_PIN) == LOW || elapsedTime > 5000) { // 5-second timeout
                    break; // exit loop if break pin is pressed or timeout occurs
                    }
                }
                if(useMechanicalController && mechanicalCompletionSignal == 1){
                    mechDone = true;
                }
                digitalWrite(MECH_SIGNAL_PIN_PASSED, LOW); 
                digitalWrite(GREEN_LED_PIN, LOW);            
            }
        }


        else if (incomingMessage.endsWith("FAIL")) {

            // indicate failure and send ACK
            digitalWrite(GREEN_LED_PIN, LOW);
            digitalWrite(RED_LED_PIN, HIGH);
            Serial.println("ACK");
            Serial.flush();


            if(useMechanicalController){
                digitalWrite(MECH_SIGNAL_PIN_FAILED, HIGH);
                delay(200);
            
                mechanicalCompletionSignal = digitalRead(MECH_SIGNAL_PIN_INPUT);
                unsigned long startTime = millis();
                unsigned long elapsedTime = 0;

                while (mechanicalCompletionSignal == 0) {
                    mechanicalCompletionSignal = digitalRead(MECH_SIGNAL_PIN_INPUT);
                    elapsedTime = millis() - startTime;

                    if (digitalRead(BREAK_PIN) == LOW || elapsedTime > 5000) { // 5-second timeout
                        break; // exit loop if break pin is pressed or timeout occurs
                        }
                    }

                if(useMechanicalController && mechanicalCompletionSignal == 1){
                    mechDone = true;
                }
                digitalWrite(MECH_SIGNAL_PIN_FAILED, LOW);
                digitalWrite(RED_LED_PIN, LOW);
            }
        }

        if (incomingMessage.endsWith("CHECK_MECH")) {
            // Simulate mechanical completion
            int startTime = millis();
            while(!mechDone){
                // wait
                if (millis() - startTime > 5000) { // timeout after 5 seconds
                    break;
                }
            }
            if(mechDone){
                Serial.println("MECH_DONE");
                Serial.flush();
            }
            else{
                Serial.println("MECH_TIMEOUT");
                Serial.flush();
            }
        }
    }
}
