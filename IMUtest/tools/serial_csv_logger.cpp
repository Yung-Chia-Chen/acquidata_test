#include <atomic>
#include <cerrno>
#include <chrono>
#include <csignal>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
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
        name << "imu_test_" << std::setw(3) << std::setfill('0') << index << ".csv";

        fs::path candidate = outputDir / name.str();
        if (!fileExists(candidate))
        {
            return candidate;
        }
    }

    throw std::runtime_error("too many csv files in output directory");
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

static std::vector<std::string> splitCsvLine(const std::string& line)
{
    std::vector<std::string> parts;
    std::string current;

    for (char ch : line)
    {
        if (ch == ',')
        {
            parts.push_back(current);
            current.clear();
        }
        else
        {
            current += ch;
        }
    }

    parts.push_back(current);
    return parts;
}

static std::string hostTimeSeconds()
{
    const auto now = std::chrono::system_clock::now();
    const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();

    std::ostringstream out;
    out << (ms / 1000) << "." << std::setw(3) << std::setfill('0') << (ms % 100);
    return out.str();
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

        std::ofstream csv(csvPath, std::ios::binary);
        if (!csv)
        {
            throw std::runtime_error("cannot create csv file");
        }

        csv << "host_time_s,type,t_ms,ax_g,ay_g,az_g,gx_dps,gy_dps,gz_dps,temp_c,accel_norm_g\n";

        const int serial = openSerialPort(portName, baudRate);

        std::cout << "Reading " << portName << " at " << baudRate << " baud\n";
        std::cout << "Saving CSV to: " << fs::absolute(csvPath).string() << "\n";
        std::cout << "Press Ctrl+C to stop.\n";

        std::string lineBuffer;
        char buffer[256];

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

                    if (line.rfind("IMU,", 0) != 0)
                    {
                        continue;
                    }

                    const std::vector<std::string> parts = splitCsvLine(line);
                    if (parts.size() != 10)
                    {
                        continue;
                    }

                    csv << hostTimeSeconds();
                    for (const std::string& part : parts)
                    {
                        csv << ',' << part;
                    }
                    csv << '\n';
                    csv.flush();

                    std::cout << line << "\n";
                }
                else if (ch != '\r')
                {
                    lineBuffer += ch;
                }
            }
        }

        close(serial);
        csv.close();
        std::cout << "\nSaved: " << fs::absolute(csvPath).string() << "\n";
    }
    catch (const std::exception& error)
    {
        std::cerr << "Error: " << error.what() << "\n";
        return 1;
    }

    return 0;
}
