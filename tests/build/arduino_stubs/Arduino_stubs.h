#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <chrono>
#include <thread>

void digitalWrite(int pin, int value);
int digitalRead(int pin);
void pinMode(int pin, int mode);
void delay(int ms);
unsigned long millis();
void reset_arduino_stubs();

class String {
private:
    std::string data;

public:
    String();
    String(const char* str);
    String(int val);
    String& operator+=(const char* str);
    String operator+(const char* str) const;
    String operator+(const String& other) const;
    const char* c_str() const;
    
    friend String operator+(const char* lhs, const String& rhs);
};

struct DigitalWriteCall {
    int pin;
    int value;
};

struct DelayCall {
    int ms;
};

class SerialClass {
public:
    void begin(int baud);
    void print(const char* str);
    void println(const char* str);
    void print(int val);
    void println(int val);
    void print(const String& str);
    void println(const String& str);
    
    int begin_call_count = 0;
    int last_baud_rate = 0;
    int println_call_count = 0;
    int print_call_count = 0;
    
    std::string outputBuffer;
};

extern SerialClass Serial;
extern std::vector<DigitalWriteCall> digitalWrite_calls;
extern std::vector<DelayCall> delay_calls;

#define HIGH 1
#define LOW 0
#define INPUT 0
#define OUTPUT 1
#define LED 13
