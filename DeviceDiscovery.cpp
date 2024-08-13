#include <iostream>
#include <string>
#include <vector>
#include<thread>
#include <cstdlib>
#include <arpa/inet.h>

// Function to ping an IP address
bool ping(const std::string& ipAddress) {
    std::string command = "ping -n 1 " + ipAddress + " > nul 2>&1";  // Windows ping command
    return system(command.c_str()) == 0;
}

// Function to perform an SNMP query
std::string snmpQuery(const std::string& ipAddress, const std::string& oid) {
    std::string command = "snmpget -v 2c -c public " + ipAddress + " " + oid + " 2>/dev/null";
    char buffer[128];
    std::string result = "";
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) return "ERROR";
    while (fgets(buffer, sizeof buffer, pipe) != NULL) {
        result += buffer;
    }
    pclose(pipe);
    return result;
}

// Function to increment IP address
std::string incrementIpAddress(const std::string& ipAddress) {
    struct sockaddr_in sa;
    inet_pton(AF_INET, ipAddress.c_str(), &(sa.sin_addr));
    sa.sin_addr.s_addr = htonl(ntohl(sa.sin_addr.s_addr) + 1);
    char str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(sa.sin_addr), str, INET_ADDRSTRLEN);
    return std::string(str);
}

// Function to scan IP range and perform SNMP queries
std::vector<std::string> discoverDevices(const std::string& startIp, const std::string& endIp) {
    std::vector<std::string> discoveredDevices;
    std::string currentIp = startIp;

    // Check if startIp is valid
    struct sockaddr_in sa;
    if (inet_pton(AF_INET, startIp.c_str(), &(sa.sin_addr)) != 1) {
        std::cerr << "Invalid start IP address: " << startIp << std::endl;
        return discoveredDevices;
    }

    // Check if endIp is valid
    if (inet_pton(AF_INET, endIp.c_str(), &(sa.sin_addr)) != 1) {
        std::cerr << "Invalid end IP address: " << endIp << std::endl;
        return discoveredDevices;
    }

    while (currentIp <= endIp) {
        std::cout << "Pinging: " << currentIp << std::endl;  // Debug output
        if (int(ping(currentIp))==0) {
            std::cout << "Ping successful: " << currentIp << std::endl;
            std::string sysDescr = snmpQuery(currentIp, "1.3.6.1.2.1.1.1.0");  // OID for system description
            std::cout << "SNMP Query Result: " << sysDescr << std::endl;
            discoveredDevices.push_back(currentIp + " - " + sysDescr);
        } else {
            std::cout << "Ping failed: " << currentIp << std::endl;
        }
        currentIp = incrementIpAddress(currentIp);
    }

    return discoveredDevices;
}

// Test Scenario: Valid IP Range
void testValidIPRange() {
    std::cout << "\nTest Case 1: Valid IP Range\n";
    std::string startIp = "192.168.1.10";
    std::string endIp = "192.168.1.20";

    std::vector<std::string> devices = discoverDevices(startIp, endIp);
    std::cout << "Discovered Devices:\n";
    for (const auto& device : devices) {
        std::cout << device << std::endl;
    }
    if (devices.empty()) {
        std::cout << "No devices found in the range.\n";
    }
}

// Test Scenario: Invalid IP Range
void testInvalidIPRange() {
    std::cout << "\nTest Case 2: Invalid IP Range\n";
    std::string startIp = "192.168.1.256";  // Invalid IP
    std::string endIp = "192.168.1.260";    // Invalid IP

    std::vector<std::string> devices = discoverDevices(startIp, endIp);
    if (devices.empty()) {
        std::cout << "No devices found or invalid IP range provided.\n";
    }
}

// Test Scenario: No Devices Found
void testNoDevicesFound() {
    std::cout << "\nTest Case 3: No Devices Found\n";
    std::string startIp = "192.168.1.289";
    std::string endIp = "192.168.1.299";

    std::vector<std::string> devices = discoverDevices(startIp, endIp);
    std::cout << "Discovered Devices:\n";
    for (const auto& device : devices) {
        std::cout << device << std::endl;
    }
    if (devices.empty()) {
        std::cout << "No devices found in the range.\n";
    }
}

int main() {
    testValidIPRange();
    testInvalidIPRange();
    testNoDevicesFound();
    return 0;
}