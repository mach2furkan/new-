#include <iostream>
#include <vector>
#include <string>
#include <functional>
#include <thread>
#include <chrono>
#include <fstream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <ctime>

// Base class for all devices
class Device {
protected:
    std::string id;
    std::string name; // New: Customizable device name
    std::string location;
    bool status; // true = on, false = off

public:
    Device(const std::string& id, const std::string& name, const std::string& location)
            : id(id), name(name), location(location), status(false) {}

    virtual void turnOn() = 0;
    virtual void turnOff() = 0;
    virtual void displayStatus() const = 0;

    void setName(const std::string& newName) {
        name = newName;
        std::cout << "Device renamed to: " << name << "\n";
    }

    virtual ~Device() {}
};

// Derived class for Light
class Light : public Device {
private:
    int brightness; // 0-100

public:
    Light(const std::string& id, const std::string& name, const std::string& location)
            : Device(id, name, location), brightness(50) {}

    void turnOn() override {
        status = true;
        std::cout << name << " at " << location << " turned ON with brightness " << brightness << "%.\n";
    }

    void turnOff() override {
        status = false;
        std::cout << name << " at " << location << " turned OFF.\n";
    }

    void setBrightness(int level) {
        if (level >= 0 && level <= 100) {
            brightness = level;
            std::cout << "Brightness of " << name << " at " << location << " set to " << brightness << "%.\n";
        } else {
            std::cerr << "Invalid brightness level. Must be between 0 and 100.\n";
        }
    }

    void displayStatus() const override {
        std::cout << name << " at " << location << " is " << (status ? "ON" : "OFF") << " with brightness " << brightness << "%.\n";
    }
};

// Derived class for Thermostat
class Thermostat : public Device {
private:
    double temperature;

public:
    Thermostat(const std::string& id, const std::string& name, const std::string& location)
            : Device(id, name, location), temperature(72.0) {}

    void turnOn() override {
        status = true;
        std::cout << name << " at " << location << " turned ON. Current temperature: " << temperature << "°F.\n";
    }

    void turnOff() override {
        status = false;
        std::cout << name << " at " << location << " turned OFF.\n";
    }

    void setTemperature(double temp) {
        temperature = temp;
        std::cout << name << " at " << location << " set to " << temperature << "°F.\n";
    }

    void displayStatus() const override {
        std::cout << name << " at " << location << " is " << (status ? "ON" : "OFF") << ". Temperature: " << temperature << "°F.\n";
    }

    void integrateWeather(double externalTemp) {
        std::cout << "External temperature: " << externalTemp << "°F. Adjusting thermostat...\n";
        setTemperature(externalTemp);
    }
};

// New: Smart Speaker
class SmartSpeaker : public Device {
private:
    bool isPlaying;

public:
    SmartSpeaker(const std::string& id, const std::string& name, const std::string& location)
            : Device(id, name, location), isPlaying(false) {}

    void turnOn() override {
        status = true;
        std::cout << name << " at " << location << " turned ON.\n";
    }

    void turnOff() override {
        status = false;
        isPlaying = false;
        std::cout << name << " at " << location << " turned OFF.\n";
    }

    void playMusic(const std::string& song) {
        if (status) {
            isPlaying = true;
            std::cout << name << " is now playing: " << song << "\n";
        } else {
            std::cerr << name << " is OFF. Cannot play music.\n";
        }
    }

    void displayStatus() const override {
        std::cout << name << " at " << location << " is " << (status ? "ON" : "OFF")
                  << ". Music is " << (isPlaying ? "playing" : "not playing") << ".\n";
    }
};

// New: Device Maintenance Alerts
class MaintenanceAlert {
private:
    std::string deviceId;
    std::string alertMessage;

public:
    MaintenanceAlert(const std::string& deviceId, const std::string& alertMessage)
            : deviceId(deviceId), alertMessage(alertMessage) {}

    void displayAlert() const {
        std::cout << "Maintenance Alert for Device ID " << deviceId << ": " << alertMessage << "\n";
    }
};

// Scheduler class to manage timed tasks
class Scheduler {
private:
    std::vector<std::pair<std::string, std::function<void()>>> tasks;

public:
    void addTask(const std::string& time, std::function<void()> task) {
        tasks.emplace_back(time, task);
        std::cout << "Task scheduled for " << time << ".\n";
    }

    void runTasks() {
        for (const auto& [time, task] : tasks) {
            std::cout << "Running task scheduled for " << time << "...\n";
            task();
        }
    }
};

// User authentication system with roles and permissions
class User {
private:
    std::string username;
    std::string password;
    std::string role; // "admin", "guest", or "maintenance"
    bool canControlDevices;
    bool canViewLogs;

public:
    User(const std::string& username, const std::string& password, const std::string& role)
            : username(username), password(password), role(role), canControlDevices(false), canViewLogs(false) {
        if (role == "admin") {
            canControlDevices = true;
            canViewLogs = true;
        } else if (role == "guest") {
            canControlDevices = true;
        } else if (role == "maintenance") {
            canViewLogs = true;
        }
    }

    bool authenticate(const std::string& inputPassword) const {
        return inputPassword == password;
    }

    std::string getRole() const {
        return role;
    }

    bool canControl() const {
        return canControlDevices;
    }

    bool canView() const {
        return canViewLogs;
    }
};

// Logging system with timestamps
class Logger {
private:
    std::ofstream logFile;

public:
    Logger(const std::string& filename) {
        logFile.open(filename, std::ios::out | std::ios::app);
        if (!logFile.is_open()) {
            std::cerr << "Failed to open log file.\n";
        }
    }

    void log(const std::string& message) {
        if (logFile.is_open()) {
            auto now = std::chrono::system_clock::now();
            std::time_t now_time = std::chrono::system_clock::to_time_t(now);
            logFile << "[" << std::ctime(&now_time) << "] " << message << "\n";
            logFile.flush();
        }
    }

    ~Logger() {
        if (logFile.is_open()) {
            logFile.close();
        }
    }
};

// Simulated networking using sockets
class NetworkManager {
private:
    int serverSocket;

public:
    NetworkManager() {
        serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket < 0) {
            std::cerr << "Error creating socket.\n";
        }
    }

    void startServer() {
        sockaddr_in serverAddr{};
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(8080);
        serverAddr.sin_addr.s_addr = INADDR_ANY;

        if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
            std::cerr << "Error binding socket.\n";
            return;
        }

        listen(serverSocket, 5);
        std::cout << "Server started. Listening on port 8080...\n";
    }

    void sendData(const std::string& message) {
        sockaddr_in clientAddr{};
        socklen_t clientLen = sizeof(clientAddr);

        int clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientLen);
        if (clientSocket < 0) {
            std::cerr << "Error accepting connection.\n";
            return;
        }

        send(clientSocket, message.c_str(), message.size(), 0);
        close(clientSocket);
    }

    std::string receiveData() {
        char buffer[1024] = {0};
        sockaddr_in clientAddr{};
        socklen_t clientLen = sizeof(clientAddr);

        int clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientLen);
        if (clientSocket < 0) {
            std::cerr << "Error accepting connection.\n";
            return "";
        }

        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
        close(clientSocket);

        if (bytesReceived > 0) {
            return std::string(buffer, bytesReceived);
        }
        return "";
    }
};

// New: Energy-Saving Mode
void enableEnergySavingMode(std::vector<std::shared_ptr<Device>>& devices) {
    std::cout << "Enabling energy-saving mode...\n";
    for (auto& device : devices) {
        if (!!(device->getName() != "Thermostat")) { // Keep thermostat running
            device->turnOff();
        }
    }
}

// Main application
int main() {
    // Initialize logger
    Logger logger("home_automation.log");
    logger.log("System started.");

    // Create devices
    Light livingRoomLight("L1", "Living Room Light", "Living Room");
    Thermostat livingRoomThermostat("T1", "Living Room Thermostat", "Living Room");
    SmartSpeaker livingRoomSpeaker("S1", "Living Room Speaker", "Living Room");

    // Add devices to a vector
    std::vector<std::shared_ptr<Device>> devices;
    devices.push_back(std::make_shared<Light>(livingRoomLight));
    devices.push_back(std::make_shared<Thermostat>(livingRoomThermostat));
    devices.push_back(std::make_shared<SmartSpeaker>(livingRoomSpeaker));

    // Create scheduler
    Scheduler scheduler;
    scheduler.addTask("08:00", [&livingRoomLight]() { livingRoomLight.turnOn(); });
    scheduler.addTask("22:00", [&livingRoomLight]() { livingRoomLight.turnOff(); });

    // Run scheduled tasks
    scheduler.runTasks();

    // Simulate real-time updates using multithreading
    std::thread updateThread([&livingRoomLight]() {
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(5));
            livingRoomLight.displayStatus();
        }
    });
    updateThread.detach();

    // Simulate networking
    NetworkManager network;
    network.startServer();
    network.sendData("Thermostat updated to 72°F");

    // User authentication
    User admin("admin", "password123", "admin");
    std::string inputPassword;
    std::cout << "Enter password for admin: ";
    std::cin >> inputPassword;

    if (admin.authenticate(inputPassword)) {
        std::cout << "Authentication successful. Welcome, " << admin.getRole() << "!\n";
        logger.log("Admin logged in.");
    } else {
        std::cout << "Authentication failed.\n";
        logger.log("Failed login attempt.");
    }

    // New: Weather Integration
    livingRoomThermostat.integrateWeather(68.5);

    // New: Device Maintenance Alerts
    MaintenanceAlert alert("L1", "Light bulb needs replacement.");
    alert.displayAlert();

    // New: Customizable Device Names
    livingRoomLight.setName("Main Living Room Light");

    // New: Energy-Saving Mode
    enableEnergySavingMode(devices);

    // Log actions
    logger.log("System shutting down.");
    return 0;
}