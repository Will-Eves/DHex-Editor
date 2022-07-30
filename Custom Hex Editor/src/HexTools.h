#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <map>

namespace HexTools {
    char const hex_chars[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

    std::map<char, int> hex_chars_as_int = {
        {'0', 0},
        {'1', 1},
        {'2', 2},
        {'3', 3},
        {'4', 4},
        {'5', 5},
        {'6', 6},
        {'7', 7},
        {'8', 8},
        {'9', 9},
        {'A', 10},
        {'B', 11},
        {'C', 12},
        {'D', 13},
        {'E', 14},
        {'F', 15},
    };

    struct HexByte {
        unsigned char a = '0';
        unsigned char b = '0';

        HexByte() {
            return;
        }

        HexByte(const unsigned char& data) {
            a = hex_chars[(data & 0xF0) >> 4];
            b = hex_chars[(data & 0x0F) >> 0];
        }

        HexByte(const unsigned int& data) {
            a = hex_chars[(data & 0xF0) >> 4];
            b = hex_chars[(data & 0x0F) >> 0];
        }

        HexByte(const char& data) {
            a = hex_chars[((unsigned char)data & 0xF0) >> 4];
            b = hex_chars[((unsigned char)data & 0x0F) >> 0];
        }

        HexByte(const int& data) {
            a = hex_chars[((unsigned int)data & 0xF0) >> 4];
            b = hex_chars[((unsigned int)data & 0x0F) >> 0];
        }

        HexByte(const char* data) {
            std::string newData = data;
            std::transform(newData.begin(), newData.end(), newData.begin(), toupper);
            a = newData[0];
            b = newData[1];
        }

        HexByte(const HexByte& data) {
            a = data.a;
            b = data.b;
        }

        unsigned char ToUnsignedChar() {
            unsigned int c = 0;

            c += hex_chars_as_int[b];
            c += hex_chars_as_int[a] * 16;

            return (unsigned char)c;
        }

        unsigned int ToUnsignedInt() {
            unsigned int c = 0;
            
            c += hex_chars_as_int[b];
            c += hex_chars_as_int[a] * 16;

            return c;
        }

        char ToChar() {
            int c = 0;
            
            c += hex_chars_as_int[b];
            c += hex_chars_as_int[a] * 16;

            return c;
        }

        int ToInt() {
            int c = 0;
            
            c += hex_chars_as_int[b];
            c += hex_chars_as_int[a] * 16;

            return c;
        }
    };

    std::ostream& operator<<(std::ostream& os, const HexByte& data)
    {
        os << data.a << data.b;
        return os;
    }

    struct HexData {
        std::vector<HexByte> data;

        HexData() {
            return;
        }

        HexData(std::string filename) {
            std::ifstream input(filename, std::ios::binary);
            std::vector<unsigned char> buffer(std::istreambuf_iterator<char>(input), {});

            for (unsigned char c : buffer) {
                data.push_back(c);
            }
        }

        HexData(std::vector<unsigned char> buffer) {
            for (unsigned char c : buffer) {
                data.push_back(c);
            }
        }

        HexData(const HexData& hexData) {
            data = hexData.data;
        }

        std::string Format(int horizontalSpacing = 0, int verticalSpacing = 0, int breakEvery = -1, int startByte=0, int endByte=-1) {
            if (endByte == -1) endByte = data.size();
            
            std::stringstream stream;
            int index = 0;
            int charNum = 0;
            for (int i = startByte; i < endByte; i++) {
                stream << data[i];
                for (int i = 0; i < horizontalSpacing; i++) {
                    stream << " ";
                }
                index++;
                if (index == breakEvery) {
                    stream << "\n";
                    for (int i = 0; i < verticalSpacing; i++) {
                        stream << "\n";
                    }
                    index = 0;
                }
            }
            stream << "\n";
            return stream.str();
        }

        void Save(std::string filename) {
            std::ofstream output(filename, std::ios::binary);
            std::string buffer;

            for (HexByte byte : data) {
                buffer += byte.ToUnsignedChar();
            }

            output.write((const char*)buffer.c_str(), buffer.size());
        }

        HexByte GetByteAt(long long position) {
            return data[position];
        }

        void SetByteAt(HexByte byte, long long position) {
            data[position] = byte;
        }

        std::string ToString() {
            std::string str;
            for (HexByte byte : data) {
                str += byte.ToUnsignedChar();
            }
            return str;
        }

        HexByte& operator[](int i) {
            if (i > data.size()) {
                return data[0];
            }

            return data[i];
        }
    };
}
