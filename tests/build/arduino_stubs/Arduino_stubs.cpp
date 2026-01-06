#include "Arduino_stubs.h"
#include <iostream>
#include <map>
#include <chrono>

static std::map<int, int> pin_states;
static auto start_time = std::chrono::steady_clock::now();

std::vector<DigitalWriteCall> digitalWrite_calls;
std::vector<DelayCall> delay_calls;

void reset_arduino_stubs() {
    Serial.begin_call_count = 0;
    Serial.last_baud_rate = 0;
    Serial.println_call_count = 0;
    Serial.print_call_count = 0;
    digitalWrite_calls.clear();
    delay_calls.clear();
    Serial.outputBuffer.clear();
    pin_states.clear();
}

void digitalWrite(int pin, int value) {
    pin_states[pin] = value;
    digitalWrite_calls.push_back({pin, value});
}

int digitalRead(int pin) {
    return pin_states[pin];
}

void pinMode(int pin, int mode) {
    // Not tracked for testing
}

void delay(int ms) {
    delay_calls.push_back({ms});
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

unsigned long millis() {
    auto now = std::chrono::steady_clock::now();
    auto duration = now - start_time;
    return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
}

SerialClass Serial;

void SerialClass::begin(int baud) {
    begin_call_count++;
    last_baud_rate = baud;
}

void SerialClass::print(const char* str) {
    print_call_count++;
    outputBuffer += str;
}

void SerialClass::println(const char* str) {
    println_call_count++;
    outputBuffer += str;
    outputBuffer += "\n";
}

void SerialClass::print(int val) {
    print_call_count++;
    outputBuffer += std::to_string(val);
}

void SerialClass::println(int val) {
    println_call_count++;
    outputBuffer += std::to_string(val);
    outputBuffer += "\n";
}

void SerialClass::print(const String& str) {
    print_call_count++;
    outputBuffer += str.c_str();
}

void SerialClass::println(const String& str) {
    println_call_count++;
    outputBuffer += str.c_str();
    outputBuffer += "\n";
}

String::String() {}

String::String(const char* str) : data(str) {}

String::String(int val) : data(std::to_string(val)) {}

String& String::operator+=(const char* str) {
    data += str;
    return *this;
}

String String::operator+(const char* str) const {
    String result = *this;
    result.data += str;
    return result;
}

String String::operator+(const String& other) const {
    String result = *this;
    result.data += other.data;
    return result;
}

String operator+(const char* lhs, const String& rhs) {
    String result(lhs);
    result.data += rhs.data;
    return result;
}

const char* String::c_str() const {
    return data.c_str();
}
