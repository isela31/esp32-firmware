#include <Arduino.h>

// Pins
const int GREEN_LED_PIN     = 18;
const int RED_LED_PIN       = 23;
const int MECH_SIGNAL_PIN   = 19;  // Input from mechanical controller
const int ENABLE_MECH_PIN   = 5;   // Output to enable mechanical system
const int BREAK_PIN         = 4;   // Button to break wait loop
const int HIGH_SIGNAL_PIN    = HIGH;

String incomingMessage;

void setup() {
    Serial.begin(9600);

    pinMode(GREEN_LED_PIN, OUTPUT);
    pinMode(RED_LED_PIN, OUTPUT);

    pinMode(MECH_SIGNAL_PIN, INPUT); // If controller is open collector → change to INPUT_PULLUP
    pinMode(ENABLE_MECH_PIN, OUTPUT);

    pinMode(BREAK_PIN, INPUT_PULLUP); // Safe for button
    pinMode(HIGH_SIGNAL_PIN, OUTPUT);


    digitalWrite(HIGH_SIGNAL_PIN, HIGH);
    digitalWrite(GREEN_LED_PIN, LOW);
    digitalWrite(RED_LED_PIN, LOW);
    digitalWrite(ENABLE_MECH_PIN, HIGH); // Enable mechanical controller


    Serial.println("ESP32 Ready to receive data...");
}

void loop() {
    if (Serial.available()) {
        incomingMessage = Serial.readStringUntil('\n');
        incomingMessage.trim();

        Serial.println("Received: " + incomingMessage);

        // Output incoming data bytes (debug)
        for (char c : incomingMessage) {
            Serial.print("[");
            Serial.print((int)c);
            Serial.print("]");
        }
        Serial.println();

        // LED + action selection
        if (incomingMessage.endsWith("PASS")) {
            digitalWrite(GREEN_LED_PIN, HIGH);
            digitalWrite(RED_LED_PIN, LOW);
            Serial.println("Part accepted.");
        }
        else if (incomingMessage.endsWith("FAIL")) {
            digitalWrite(GREEN_LED_PIN, LOW);
            digitalWrite(RED_LED_PIN, HIGH);
            Serial.println("Part rejected.");
        }
        else {
            digitalWrite(GREEN_LED_PIN, LOW);
            digitalWrite(RED_LED_PIN, LOW);
            Serial.println("Unknown command.");
        }

        //  Wait for mechanical controller ONLY here
        Serial.println("Waiting for mechanical signal...");
        while (digitalRead(MECH_SIGNAL_PIN) == LOW) {
            delay(20);
            if (digitalRead(BREAK_PIN) == LOW) {  // Button pressed?
                Serial.println("Manual break triggered!");
                break;
            }
        }

        Serial.println("ACK"); //  ONLY HERE
        Serial.println("Mechanical action completed.");

        // Reset LEDs
        digitalWrite(GREEN_LED_PIN, LOW);
        digitalWrite(RED_LED_PIN, LOW);
    }
}
