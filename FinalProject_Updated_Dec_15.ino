#include <Arduino_BMI270_BMM150.h>  // Library for IMU (BMI270 and BMM150 sensors)
#include <ArduinoBLE.h>             // Library for Bluetooth Low Energy (BLE) communication
#include <Arduino_HS300x.h>         // Library for HS300x temperature and humidity sensor

// Global variables
String commands[] = { "Shake it", "Twist it", "Flip it", "Bop it", "Warm it" }; // List of game commands
int currentCommand = -1;    // Index of the current command
unsigned long commandTime;  // Timestamp for when the current command started
const int timeLimit = 5000; // Time limit (in ms) to complete a gesture
bool commandActive = false; // Flag to indicate if a command is currently active
int score = 0;              // Player score

// BLE Service and Characteristics
BLEService gameService("12345678-1234-5678-1234-56789abcdef0");                                            // Define a BLE service for the game
BLECharacteristic commandCharacteristic("abcdef01-1234-5678-1234-56789abcdef0", BLERead | BLENotify, 20);  // BLE characteristic for sending commands
BLECharacteristic scoreCharacteristic("abcdef02-1234-5678-1234-56789abcdef0", BLERead | BLENotify, 20);    // BLE characteristic for sending scores

// Function prototypes
void startNewCommand();
bool detectShake();      // Function to detect "Shake it" (Left-Right motion)
bool detectTwist();      // Function to detect "Twist it" (Rotational motion)
bool detectFlip();       // Function to detect "Flip it" (Flip motion)
bool detectBopIt();      // Function to detect "Bop it" (Up-Down motion)
bool detectWarmIt();     // Function to detect "Warm It" (based on temperature)

void setup() {
  Serial.begin(9600);
  while (!Serial); // Wait for Serial Monitor to open

  // Initialize the IMU (Inertial Measurement Unit)
  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");
    while (1);
  }
  Serial.println("BMI270 IMU initialized!");

  // Initialize the HS300x sensor
  if (!HS300x.begin()) {
    Serial.println("Failed to initialize HS300x sensor!");
    while (1);
  }
  Serial.println("HS300x temperature sensor initialized!");

  // Initialize BLE
  if (!BLE.begin()) {
    Serial.println("Failed to initialize BLE!");
    while (1);
  }
  BLE.setLocalName("BopItGame");
  BLE.setAdvertisedService(gameService);
  gameService.addCharacteristic(commandCharacteristic);
  gameService.addCharacteristic(scoreCharacteristic);
  BLE.addService(gameService);
  BLE.advertise();
  Serial.println("BLE initialized and advertising!");

  // Start the first game command
  startNewCommand();
}

void loop() {
  BLE.poll(); // Handle BLE communication

  // Check if the time limit for the current command has elapsed
  if (commandActive && (millis() - commandTime > timeLimit)) {
    Serial.println("Time's up! You missed the command.");
    commandActive = false;
    startNewCommand();
  }

  // Check for the corresponding gesture if a command is active
  if (commandActive) {
    if ((currentCommand == 0 && detectShake()) || // Detect "Shake it"
        (currentCommand == 1 && detectTwist()) || // Detect "Twist it"
        (currentCommand == 2 && detectFlip()) ||  // Detect "Flip it"
        (currentCommand == 3 && detectBopIt()) || // Detect "Bop it"
        (currentCommand == 4 && detectWarmIt())) { // Detect "Warm It"
      Serial.println("Correct gesture!");
     
      commandActive = false;
      startNewCommand();
    }
  }
}

void startNewCommand() {
  delay(1000); // Pause for 1 second before issuing the next command
  currentCommand = random(0, 5); // Randomly select a command (index 0 to 4)
  String commandText = commands[currentCommand];
  Serial.println("New Command: " + commandText);
  commandCharacteristic.writeValue(commandText.c_str());
  commandTime = millis();
  commandActive = true;
}

// Detect "Shake it" (Left-Right motion)
bool detectShake() {
  float x, y, z;
  if (IMU.accelerationAvailable()) {
    IMU.readAcceleration(x, y, z);
    if (abs(x) > 2.0 && abs(y) < 1.0 && abs(z) < 1.0) {
      Serial.println("Shake detected (Left-Right)!");
      return true;
    }
  }
  return false;
}

// Detect "Twist it" (Rotational motion)
bool detectTwist() {
  float x, y, z;
  if (IMU.gyroscopeAvailable()) {
    IMU.readGyroscope(x, y, z);
    if (abs(z) > 200) {
      Serial.println("Twist detected!");
      return true;
    }
  }
  return false;
}

// Detect "Flip it" (Flip motion)
bool detectFlip() {
  float x, y, z;
  if (IMU.accelerationAvailable()) {
    IMU.readAcceleration(x, y, z);
    if (abs(x) > 1.0 && abs(z) > 1.0) {
      Serial.println("Flip detected!");
      return true;
    }
  }
  return false;
}

// Detect "Bop it" (Up-Down motion)
bool detectBopIt() {
  float x, y, z;
  if (IMU.accelerationAvailable()) {
    IMU.readAcceleration(x, y, z);
    if (abs(z) > 2.0 && abs(x) < 1.0 && abs(y) < 1.0) {
      Serial.println("Bop It detected (Up-Down)!");
      return true;
    }
  }
  return false;
}

// Detect "Warm it" (Temperature threshold)
bool detectWarmIt() {
  float temperature = HS300x.readTemperature(); // Read temperature from HS300x sensor
  Serial.print("Temperature: ");
  Serial.println(temperature); // Debug: Print temperature value

  if (temperature > 27.0) { // Threshold for detecting "Warm It" (adjust as needed)
    Serial.println("Warm It detected!");
    return true;
  }
  return false;
}
