#pragma once

#include <cstdint>
#include <string>
#include <sstream>
#include <vector>
#include <exception>

namespace net
{
    class IpAddress
    {
    private:
        uint32_t maddr_t_;
        std::string maddr_str_;

    public:
        IpAddress(uint32_t addr = 0) : maddr_t_(addr), maddr_str_(ip_uint32_to_string(addr)) {}
        IpAddress(const std::string &addr = "0.0.0.0") : maddr_t_(ip_string_to_uint32(addr)), maddr_str_(addr) {}
        uint32_t get_ip_t() const
        {
            return maddr_t_;
        }
        std::string get_ip_str() const
        {
            return maddr_str_;
        }
        bool operator==(const IpAddress& other)const
        {
            return maddr_t_==other.maddr_t_&&maddr_str_==other.maddr_str_;
        }
    private:
        std::string ip_uint32_to_string(uint32_t addr)
        {
            uint8_t bytes[4];

            bytes[0] = (addr >> 24) & 0xff;
            bytes[1] = (addr >> 16) & 0xff;
            bytes[2] = (addr >> 8) & 0xff;
            bytes[3] = (addr >> 0) & 0xff;

            std::ostringstream oss;
            oss << bytes[0] << "." << bytes[1] << "." << bytes[2] << "." << bytes[3];

            return oss.str();
        }
        uint32_t ip_string_to_uint32(const std::string &addr)
        {
            std::vector<std::string> parts;
            std::string part;
            std::stringstream ss(addr);

            while (std::getline(ss, part, '.'))
            {
                parts.push_back(part);
            }

            if (parts.size() != 4)
                throw std::invalid_argument("Invalid ip address!");

            uint32_t ip_addr = 0;
            for (int i = 0; i < 4; i++)
            {
                int num = std::stoi(parts[i]);

                if (num < 0 || num > 255)
                    throw std::invalid_argument("Invalid ip address!");

                ip_addr |= (num << (3 - i) * 8);
            }

            return ip_addr;
        }
    };
}