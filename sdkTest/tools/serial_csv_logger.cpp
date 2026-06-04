#include <atomic>
#include <chrono>
#include <csignal>
#include <cerrno>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

namespace fs = std::filesystem;

static std::atomic_bool g_running(true);
static void onSignal(int)
{
    g_running = false;
}

static bool fileExists(const fs::path& path)
{
    std::error_code ec;
    return fs::exists(path, ec);
}

static fs::path nextCsvPath(const fs::path& outputDir)
{
    for (int index = 1; index < 10000; ++index)
    {
        std::ostringstream name;
        name << "ccpa_telemetry_" << std::setw(3) << std::setfill('0') << index << ".csv";

        fs::path candidate = outputDir / name.str();
        if (!fileExists(candidate))
        {
            return candidate;
        }
    }

    throw std::runtime_error("too many csv files in output directory");
}

static std::string nowIsoLocal()
{
    const auto now = std::chrono::system_clock::now();
    const std::time_t time = std::chrono::system_clock::to_time_t(now);

    std::tm localTime{};
    localtime_r(&time, &localTime);

    std::ostringstream out;
    out << std::put_time(&localTime, "%Y/%m/%d %H:%M:%S");
    return out.str();
}

static std::string csvEscape(const std::string& value)
{
    std::string escaped = "\"";
    for (char ch : value)
    {
        if (ch == '"')
        {
            escaped += "\"\"";
        }
        else
        {
            escaped += ch;
        }
    }
    escaped += "\"";
    return escaped;
}

static std::string trim(std::string value)
{
    const char* whitespace = " \t\r\n";
    const size_t first = value.find_first_not_of(whitespace);
    if (first == std::string::npos)
    {
        return "";
    }

    const size_t last = value.find_last_not_of(whitespace);
    return value.substr(first, last - first + 1);
}

static std::string assistLevelToMode(int level);

static bool parseRidingModeFromRaw(const std::string& line, std::string& mode)
{
    if (line.find("ID=0x02940015") == std::string::npos) return false;
    std::smatch m;
    if (std::regex_search(line, m, std::regex(R"(DATA=[0-9A-Fa-f]+ ([0-9A-Fa-f]+))")))
    {
        int level = std::stoi(m[1].str(), nullptr, 16) & 0x0F;
        if (level <= 5)
        {
            mode = assistLevelToMode(level);
            return true;
        }
    }
    return false;
}

static std::string formatFloat(float value)
{
    std::ostringstream out;
    out << std::fixed << std::setprecision(2) << value;
    return out.str();
}

static std::string formatVoltageV(uint32_t mv)
{
    std::ostringstream out;
    out << std::fixed << std::setprecision(1) << (static_cast<float>(mv) * 0.001f);
    return out.str();
}

static uint16_t ccpa_u16_le(const std::vector<uint8_t>& data, size_t index)
{
    return static_cast<uint16_t>(data[index]) | (static_cast<uint16_t>(data[index + 1]) << 8);
}

static uint32_t ccpa_u24_le(const std::vector<uint8_t>& data, size_t index)
{
    return static_cast<uint32_t>(data[index])
        | (static_cast<uint32_t>(data[index + 1]) << 8)
        | (static_cast<uint32_t>(data[index + 2]) << 16);
}

static int16_t ccpa_temp_c(uint8_t raw)
{
    return static_cast<int16_t>(raw) - 64;
}

enum class CcpaValueSource : uint8_t
{
    None = 0,
    GeneralInfo = 1,
    Device = 2
};

struct TelemetryRow;

static bool parseRawCanLine(const std::string& line, uint32_t& id, std::vector<uint8_t>& data)
{
    std::smatch m;
    if (!std::regex_search(line, m, std::regex(R"(RAW RX \S+ ID=0x([0-9A-Fa-f]+) DLC=([0-9]+) DATA=([0-9A-Fa-f ]+))")))
    {
        return false;
    }

    id = static_cast<uint32_t>(std::stoul(m[1].str(), nullptr, 16));
    const int dlc = std::stoi(m[2].str());
    data.clear();

    std::istringstream ss(m[3].str());
    std::string byteStr;
    while (data.size() < static_cast<size_t>(dlc) && ss >> byteStr)
    {
        data.push_back(static_cast<uint8_t>(std::stoul(byteStr, nullptr, 16)));
    }

    return static_cast<int>(data.size()) == dlc;
}

static bool parseTelemetryLine(
    const std::string& line,
    std::string& key,
    std::string& value,
    std::string& unit)
{
    std::smatch match;
    auto matchLine = [&](const char* pattern, const char* nextKey, const char* nextUnit) -> bool
    {
        if (std::regex_search(line, match, std::regex(pattern)))
        {
            key = nextKey;
            value = match[1].str();
            unit = nextUnit;
            return true;
        }
        return false;
    };

    if (matchLine(R"(.*Bike speed = ([0-9.]+) KPH)", "speed", "KPH")) return true;
    if (matchLine(R"(.*Speed = ([0-9.]+), Odo = [0-9]+)", "speed", "KPH")) return true;
    if (matchLine(R"(.*Cadence = ([0-9]+) RPM)", "cadence", "RPM")) return true;
    if (matchLine(R"(.*Rider torque = ([0-9.]+) Nm)", "torque", "Nm")) return true;
    if (matchLine(R"(.*Motor RPM = ([0-9]+) RPM)", "motor_speed", "RPM")) return true;
    if (matchLine(R"(.*motor rpm = ([0-9]+))", "motor_speed", "RPM")) return true;
    if (matchLine(R"(.*Battery SOC = ([0-9]+) %)", "battery_capacity", "%")) return true;
    if (matchLine(R"(.*Battery voltage = ([0-9]+) mV)", "battery_voltage_mv", "mV")) return true;
    if (matchLine(R"(.*Battery current = ([0-9]+) mA)", "battery_current_ma", "mA")) return true;
    if (matchLine(R"(.*Battery temps = ([0-9, \-]+) C)", "battery_temps", "C")) return true;
    if (matchLine(R"(.*Assist mode = (off|eco\+?|normal|sport\+?))", "riding_mode", "")) return true;
    if (matchLine(R"(.*Assist level = ([0-5]))", "assist_level", "")) return true;
    {
        std::smatch m;
        if (std::regex_search(line, m, std::regex(R"(.*Rear gear = ([0-9]+) / ([0-9]+))"))) {
            key = "rear_gear_both";
            value = m[1].str() + "," + m[2].str();
            unit = "";
            return true;
        }
    }

    return false;
}

struct TelemetryRow
{
    std::string speed = "0";
    std::string cadence = "0";
    std::string torque = "0";
    std::string motorSpeed = "0";
    std::string ridingMode = "";
    std::string batteryVoltage = "0";
    std::string batteryCurrent = "0";
    std::string batteryCapacity = "0";
    std::string batteryTemp1 = "";
    std::string batteryTemp2 = "";
    std::string batteryTemp3 = "";
    std::string batteryTemp4 = "";
    std::string rearGear = "";
    std::string timestamp = "";

    CcpaValueSource speedSource = CcpaValueSource::None;
    CcpaValueSource cadenceSource = CcpaValueSource::None;
    CcpaValueSource torqueSource = CcpaValueSource::None;
    CcpaValueSource batteryCapacitySource = CcpaValueSource::None;
};

static void updateTelemetryFromCanFrame(uint32_t id, const std::vector<uint8_t>& frameData, TelemetryRow& row)
{
    constexpr uint32_t GENERAL_INFO00ACK = 0x29404FCU;
    constexpr uint32_t GENERAL_INFO00BRO = 0x29404FEU;
    constexpr uint32_t GENERAL_INFO01ACK = 0x29404F8U;
    constexpr uint32_t GENERAL_INFO01BRO = 0x29404FAU;
    constexpr uint32_t CENTRAL_INTERFACE_ASSISTREQ = 0x02940015U;
    constexpr uint32_t CONTROLLER_INFO00ACK = 0x1E942040U;
    constexpr uint32_t CONTROLLER_INFO00BRO = 0x1E942042U;
    constexpr uint32_t CONTROLLER_INFO02ACK = 0x1E942048U;
    constexpr uint32_t CONTROLLER_INFO02BRO = 0x1E94204AU;
    constexpr uint32_t CONTROLLER_INFO03ACK = 0x1E94204CU;
    constexpr uint32_t CONTROLLER_INFO03BRO = 0x1E94204EU;
    constexpr uint32_t BAT1_INFO01ACK = 0x1E942444U;
    constexpr uint32_t BAT1_INFO01BRO = 0x1E942446U;
    constexpr uint32_t BAT1_INFO06ACK = 0x1E942458U;
    constexpr uint32_t BAT1_INFO06BRO = 0x1E94245AU;
    constexpr uint32_t REARDERAILLEUR_INFO00ACK = 0x1E944840U;
    constexpr uint32_t REARDERAILLEUR_INFO00BRO = 0x1E944842U;

    if (id == GENERAL_INFO00ACK || id == GENERAL_INFO00BRO)
    {
        if (frameData.size() >= 6)
        {
            const uint16_t speedRaw = ccpa_u16_le(frameData, 0);
            if (speedRaw != 0xFFFFU)
            {
                row.speed = formatFloat(static_cast<float>(speedRaw) * 0.01f);
                row.speedSource = CcpaValueSource::GeneralInfo;
            }

            row.cadence = std::to_string(frameData[4]);
            row.cadenceSource = CcpaValueSource::GeneralInfo;

            const uint16_t torqueRaw = ccpa_u16_le(frameData, 2);
            if (torqueRaw != 0xFFFFU && torqueRaw != 0x0000U)
            {
                row.torque = formatFloat(static_cast<float>(torqueRaw) * 0.01f);
                row.torqueSource = CcpaValueSource::GeneralInfo;
            }
        }
    }
    else if (id == CONTROLLER_INFO00ACK || id == CONTROLLER_INFO00BRO)
    {
        if (frameData.size() >= 2)
        {
            const uint16_t speedRaw = ccpa_u16_le(frameData, 0);
            if (speedRaw != 0xFFFFU && row.speedSource != CcpaValueSource::GeneralInfo)
            {
                row.speed = formatFloat(static_cast<float>(speedRaw) * 0.01f);
                row.speedSource = CcpaValueSource::Device;
            }
        }
    }
    else if (id == CONTROLLER_INFO02ACK || id == CONTROLLER_INFO02BRO)
    {
        if (frameData.size() >= 7)
        {
            row.motorSpeed = std::to_string(ccpa_u16_le(frameData, 4));
        }
    }
    else if (id == CONTROLLER_INFO03ACK || id == CONTROLLER_INFO03BRO)
    {
        if (frameData.size() >= 4 && row.cadenceSource != CcpaValueSource::GeneralInfo)
        {
            row.cadence = std::to_string(frameData[3]);
            row.cadenceSource = CcpaValueSource::Device;
        }
        if (frameData.size() >= 6 && row.torqueSource != CcpaValueSource::GeneralInfo)
        {
            row.torque = formatFloat(static_cast<float>(ccpa_u16_le(frameData, 4)) * 0.1f);
            row.torqueSource = CcpaValueSource::Device;
        }
    }
    else if (id == GENERAL_INFO01ACK || id == GENERAL_INFO01BRO)
    {
        if (frameData.size() >= 5)
        {
            if (frameData[4] != 0xFFU && frameData[4] != 0x00U)
            {
                row.batteryCapacity = std::to_string(frameData[4]);
                row.batteryCapacitySource = CcpaValueSource::GeneralInfo;
            }
        }
    }
    else if (id == BAT1_INFO01ACK || id == BAT1_INFO01BRO)
    {
        if (frameData.size() >= 7)
        {
            row.batteryVoltage = formatVoltageV(ccpa_u24_le(frameData, 0));
            row.batteryCurrent = std::to_string(ccpa_u16_le(frameData, 4));
            if (frameData[6] != 0xFFU && frameData[6] != 0x00U && row.batteryCapacitySource != CcpaValueSource::GeneralInfo)
            {
                row.batteryCapacity = std::to_string(frameData[6]);
                row.batteryCapacitySource = CcpaValueSource::Device;
            }
        }
    }
    else if (id == BAT1_INFO06ACK || id == BAT1_INFO06BRO)
    {
        if (frameData.size() >= 4)
        {
            row.batteryTemp1 = std::to_string(ccpa_temp_c(frameData[0]));
            row.batteryTemp2 = std::to_string(ccpa_temp_c(frameData[1]));
            row.batteryTemp3 = std::to_string(ccpa_temp_c(frameData[2]));
            row.batteryTemp4 = std::to_string(ccpa_temp_c(frameData[3]));
        }
    }
    else if (id == REARDERAILLEUR_INFO00ACK || id == REARDERAILLEUR_INFO00BRO)
    {
        if (frameData.size() >= 2)
        {
            uint32_t current = frameData[0];
            uint32_t max = frameData[1];
            if (max != 0 && current > max)
            {
                std::swap(current, max);
            }
            row.rearGear = std::to_string(current) + "(" + std::to_string(max) + ")";
        }
    }
}

static std::string assistLevelToMode(int level)
{
    static const char* modes[] = {"off", "eco", "eco+", "normal", "sport", "sport+"};
    if (level >= 0 && level <= 5) return modes[level];
    return "";
}

static std::string normalizeBatteryTemp(std::string value)
{
    value = trim(value);
    if (value.empty()) return "";
    try {
        if (std::stoi(value) <= -999) return "";
    } catch (...) {
        return "";
    }
    return value;
}

static void updateTelemetry(TelemetryRow& row, const std::string& key, const std::string& value)
{
    if (key == "speed") row.speed = value;
    else if (key == "cadence") row.cadence = value;
    else if (key == "torque") row.torque = value;
    else if (key == "motor_speed") row.motorSpeed = value;
    else if (key == "riding_mode") row.ridingMode = value;
    else if (key == "assist_level") row.ridingMode = assistLevelToMode(std::stoi(value));
    else if (key == "battery_capacity") row.batteryCapacity = value;
    else if (key == "battery_voltage_mv") row.batteryVoltage = formatVoltageV(static_cast<uint32_t>(std::stoul(value)));
    else if (key == "battery_current_ma") row.batteryCurrent = value;
    else if (key == "battery_temps") {
        std::istringstream ss(value);
        std::string token;
        std::vector<std::string> temps;
        while (std::getline(ss, token, ',')) {
            token.erase(0, token.find_first_not_of(" "));
            temps.push_back(token);
        }
        if (temps.size() >= 1) row.batteryTemp1 = normalizeBatteryTemp(temps[0]);
        if (temps.size() >= 2) row.batteryTemp2 = normalizeBatteryTemp(temps[1]);
        if (temps.size() >= 3) row.batteryTemp3 = normalizeBatteryTemp(temps[2]);
        if (temps.size() >= 4) row.batteryTemp4 = normalizeBatteryTemp(temps[3]);
    }
    else if (key == "rear_gear_both") {
        auto comma = value.find(',');
        if (comma != std::string::npos) {
            row.rearGear = value.substr(0, comma) + "(" + value.substr(comma + 1) + ")";
        }
    }
}

static void writeTelemetryRow(std::ofstream& csv, const TelemetryRow& row)
{
    csv << row.speed << ','
        << row.cadence << ','
        << row.torque << ','
        << row.motorSpeed << ','
        << row.ridingMode << ','
        << row.batteryCapacity << ','
        << row.batteryVoltage << ','
        << row.batteryCurrent << ','
        << row.batteryTemp1 << ','
        << row.batteryTemp2 << ','
        << row.batteryTemp3 << ','
        << row.batteryTemp4 << ',';

    csv << csvEscape(row.rearGear) << ','
        << csvEscape("'" + row.timestamp) << '\n';
}

static speed_t baudToTermios(unsigned int baudRate)
{
    switch (baudRate)
    {
        case 9600: return B9600;
        case 19200: return B19200;
        case 38400: return B38400;
        case 57600: return B57600;
        case 115200: return B115200;
        case 230400: return B230400;
        case 460800: return B460800;
        case 921600: return B921600;
        default: throw std::runtime_error("unsupported baud rate");
    }
}

static int openSerialPort(const std::string& portName, unsigned int baudRate)
{
    const int serial = open(portName.c_str(), O_RDONLY | O_NOCTTY);
    if (serial < 0)
    {
        throw std::runtime_error("cannot open serial port " + portName + ": " + std::strerror(errno));
    }

    termios tty{};
    if (tcgetattr(serial, &tty) != 0)
    {
        close(serial);
        throw std::runtime_error("cannot read serial settings: " + std::string(std::strerror(errno)));
    }

    tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
    tty.c_oflag &= ~OPOST;
    tty.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
    tty.c_cflag &= ~(CSIZE | PARENB);
    tty.c_cflag |= CS8;

    const speed_t speed = baudToTermios(baudRate);
    cfsetispeed(&tty, speed);
    cfsetospeed(&tty, speed);

    tty.c_cflag |= CLOCAL | CREAD;
    tty.c_cflag &= ~PARENB;
    tty.c_cflag &= ~CSTOPB;
#ifdef CRTSCTS
    tty.c_cflag &= ~CRTSCTS;
#endif
    tty.c_cc[VMIN] = 0;
    tty.c_cc[VTIME] = 5;

    if (tcsetattr(serial, TCSANOW, &tty) != 0)
    {
        close(serial);
        throw std::runtime_error("cannot apply serial settings: " + std::string(std::strerror(errno)));
    }

    return serial;
}

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cout << "Usage: ./serial_csv_logger /dev/ttyUSB0 [baudRate] [outputDir]\n";
        std::cout << "Example: ./serial_csv_logger /dev/ttyACM0 115200 csv_logs\n";
        return 1;
    }

    const std::string portName = argv[1];
    const unsigned int baudRate = argc >= 3 ? static_cast<unsigned int>(std::stoul(argv[2])) : 115200;
    const fs::path outputDir = argc >= 4 ? fs::path(argv[3]) : fs::path("csv_logs");

    std::signal(SIGINT, onSignal);

    try
    {
        fs::create_directories(outputDir);
        const fs::path csvPath = nextCsvPath(outputDir);

        const int serial = openSerialPort(portName, baudRate);

        std::cout << "Reading " << portName << " at " << baudRate << " baud\n";
        std::cout << "Collecting data every 500 ms. No CSV will be written until you stop.\n";
        std::cout << "CSV will be saved to: " << fs::absolute(csvPath).string() << " when collection stops.\n";
        std::cout << "Press Ctrl+C to stop.\n";

        std::string lineBuffer;
        char buffer[256];
        TelemetryRow telemetry;
        std::vector<TelemetryRow> collectedRows;
        auto nextWriteTime = std::chrono::steady_clock::now() + std::chrono::milliseconds(500);

        while (g_running)
        {
            const ssize_t bytesRead = read(serial, buffer, sizeof(buffer));
            if (bytesRead < 0)
            {
                std::cerr << "Serial read failed: " << std::strerror(errno) << "\n";
                break;
            }

            for (ssize_t i = 0; i < bytesRead; ++i)
            {
                const char ch = buffer[i];
                if (ch == '\n')
                {
                    const std::string line = trim(lineBuffer);
                    lineBuffer.clear();

                    std::string key;
                    std::string value;
                    std::string unit;

                    if (!line.empty())
                    {
                        if (parseTelemetryLine(line, key, value, unit))
                        {
                            updateTelemetry(telemetry, key, value);
                        }
                        else
                        {
                            uint32_t canId;
                            std::vector<uint8_t> frameData;
                            if (parseRawCanLine(line, canId, frameData))
                            {
                                updateTelemetryFromCanFrame(canId, frameData, telemetry);
                                std::cout << "RAW CAN: " << line << "\n";
                                if (canId == 0x1E944840U || canId == 0x1E944842U)
                                {
                                    std::cout << "DECODED rear_gear: " << static_cast<int>(frameData[0])
                                              << " / " << static_cast<int>(frameData[1]) << "\n";
                                }
                            }
                        }

                        std::string newMode;
                        if (parseRidingModeFromRaw(line, newMode))
                        {
                            telemetry.ridingMode = newMode;
                        }
                    }
                }
                else if (ch != '\r')
                {
                    lineBuffer += ch;
                }
            }

            const auto now = std::chrono::steady_clock::now();
            if (now >= nextWriteTime)
            {
                telemetry.timestamp = nowIsoLocal();
                collectedRows.push_back(telemetry);
                nextWriteTime = now + std::chrono::milliseconds(500);

                std::cout << "sample=" << collectedRows.size()
                          << ", speed=" << telemetry.speed
                          << ", cadence=" << telemetry.cadence
                          << ", torque=" << telemetry.torque
                          << ", motor=" << telemetry.motorSpeed
                          << ", mode=" << telemetry.ridingMode
                          << ", battery=" << telemetry.batteryCapacity
                          << ", voltage=" << telemetry.batteryVoltage
                          << ", current=" << telemetry.batteryCurrent
                          << ", rear_gear=" << telemetry.rearGear
                          << ", time=" << telemetry.timestamp << "\n";
            }
        }

        close(serial);

        std::ofstream csv(csvPath, std::ios::binary);
        if (!csv)
        {
            throw std::runtime_error("cannot create csv file");
        }

        csv << "\xEF\xBB\xBF";
        csv << "speed(kph),cadence(rpm),torque(nm),motor_speed(rpm),riding_mode,battery_capacity(%),battery_voltage(v),battery_current(ma),battery_temp1(c),battery_temp2(c),battery_temp3(c),battery_temp4(c),rear_gear,time\n";
        for (const auto& row : collectedRows)
        {
            writeTelemetryRow(csv, row);
        }
        csv.close();

        std::cout << "\nSaved: " << fs::absolute(csvPath).string() << " (" << collectedRows.size() << " rows)\n";
    }
    catch (const std::exception& error)
    {
        std::cerr << "Error: " << error.what() << "\n";
        return 1;
    }

    return 0;
}
