#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <algorithm>

namespace HexTools {
    char const hex_chars[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

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
            
            if (b == '1') {
                c += 1;
            }
            else if (b == '2') {
                c += 2;
            }
            else if (b == '3') {
                c += 3;
            }
            else if (b == '4') {
                c += 4;
            }
            else if (b == '5') {
                c += 5;
            }
            else if (b == '6') {
                c += 6;
            }
            else if (b == '7') {
                c += 7;
            }
            else if (b == '8') {
                c += 8;
            }
            else if (b == '9') {
                c += 9;
            }
            else if (b == 'A') {
                c += 10;
            }
            else if (b == 'B') {
                c += 11;
            }
            else if (b == 'C') {
                c += 12;
            }
            else if (b == 'D') {
                c += 13;
            }
            else if (b == 'E') {
                c += 14;
            }
            else if (b == 'F') {
                c += 15;
            }

            if (a == '1') {
                c += 16;
            }
            else if (a == '2') {
                c += 2 * 16;
            }
            else if (a == '3') {
                c += 3 * 16;
            }
            else if (a == '4') {
                c += 4 * 16;
            }
            else if (a == '5') {
                c += 5 * 16;
            }
            else if (a == '6') {
                c += 6 * 16;
            }
            else if (a == '7') {
                c += 7 * 16;
            }
            else if (a == '8') {
                c += 8 * 16;
            }
            else if (a == '9') {
                c += 9 * 16;
            }
            else if (a == 'A') {
                c += 10 * 16;
            }
            else if (a == 'B') {
                c += 11 * 16;
            }
            else if (a == 'C') {
                c += 12 * 16;
            }
            else if (a == 'D') {
                c += 13 * 16;
            }
            else if (a == 'E') {
                c += 14 * 16;
            }
            else if (a == 'F') {
                c += 15 * 16;
            }

            return (unsigned char)c;
        }

        unsigned int ToUnsignedInt() {
            unsigned int c;
            
            if (b == '1') {
                c += 1;
            }
            else if (b == '2') {
                c += 2;
            }
            else if (b == '3') {
                c += 3;
            }
            else if (b == '4') {
                c += 4;
            }
            else if (b == '5') {
                c += 5;
            }
            else if (b == '6') {
                c += 6;
            }
            else if (b == '7') {
                c += 7;
            }
            else if (b == '8') {
                c += 8;
            }
            else if (b == '9') {
                c += 9;
            }
            else if (b == 'A') {
                c += 10;
            }
            else if (b == 'B') {
                c += 11;
            }
            else if (b == 'C') {
                c += 12;
            }
            else if (b == 'D') {
                c += 13;
            }
            else if (b == 'E') {
                c += 14;
            }
            else if (b == 'F') {
                c += 15;
            }

            if (a == '1') {
                c += 16;
            }
            else if (a == '2') {
                c += 2 * 16;
            }
            else if (a == '3') {
                c += 3 * 16;
            }
            else if (a == '4') {
                c += 4 * 16;
            }
            else if (a == '5') {
                c += 5 * 16;
            }
            else if (a == '6') {
                c += 6 * 16;
            }
            else if (a == '7') {
                c += 7 * 16;
            }
            else if (a == '8') {
                c += 8 * 16;
            }
            else if (a == '9') {
                c += 9 * 16;
            }
            else if (a == 'A') {
                c += 10 * 16;
            }
            else if (a == 'B') {
                c += 11 * 16;
            }
            else if (a == 'C') {
                c += 12 * 16;
            }
            else if (a == 'D') {
                c += 13 * 16;
            }
            else if (a == 'E') {
                c += 14 * 16;
            }
            else if (a == 'F') {
                c += 15 * 16;
            }

            return c;
        }

        char ToChar() {
            int c;
            
            if (b == '1') {
                c += 1;
            }
            else if (b == '2') {
                c += 2;
            }
            else if (b == '3') {
                c += 3;
            }
            else if (b == '4') {
                c += 4;
            }
            else if (b == '5') {
                c += 5;
            }
            else if (b == '6') {
                c += 6;
            }
            else if (b == '7') {
                c += 7;
            }
            else if (b == '8') {
                c += 8;
            }
            else if (b == '9') {
                c += 9;
            }
            else if (b == 'A') {
                c += 10;
            }
            else if (b == 'B') {
                c += 11;
            }
            else if (b == 'C') {
                c += 12;
            }
            else if (b == 'D') {
                c += 13;
            }
            else if (b == 'E') {
                c += 14;
            }
            else if (b == 'F') {
                c += 15;
            }

            if (a == '1') {
                c += 16;
            }
            else if (a == '2') {
                c += 2 * 16;
            }
            else if (a == '3') {
                c += 3 * 16;
            }
            else if (a == '4') {
                c += 4 * 16;
            }
            else if (a == '5') {
                c += 5 * 16;
            }
            else if (a == '6') {
                c += 6 * 16;
            }
            else if (a == '7') {
                c += 7 * 16;
            }
            else if (a == '8') {
                c += 8 * 16;
            }
            else if (a == '9') {
                c += 9 * 16;
            }
            else if (a == 'A') {
                c += 10 * 16;
            }
            else if (a == 'B') {
                c += 11 * 16;
            }
            else if (a == 'C') {
                c += 12 * 16;
            }
            else if (a == 'D') {
                c += 13 * 16;
            }
            else if (a == 'E') {
                c += 14 * 16;
            }
            else if (a == 'F') {
                c += 15 * 16;
            }

            return c;
        }

        int ToInt() {
            int c;
            
            if (b == '1') {
                c += 1;
            }
            else if (b == '2') {
                c += 2;
            }
            else if (b == '3') {
                c += 3;
            }
            else if (b == '4') {
                c += 4;
            }
            else if (b == '5') {
                c += 5;
            }
            else if (b == '6') {
                c += 6;
            }
            else if (b == '7') {
                c += 7;
            }
            else if (b == '8') {
                c += 8;
            }
            else if (b == '9') {
                c += 9;
            }
            else if (b == 'A') {
                c += 10;
            }
            else if (b == 'B') {
                c += 11;
            }
            else if (b == 'C') {
                c += 12;
            }
            else if (b == 'D') {
                c += 13;
            }
            else if (b == 'E') {
                c += 14;
            }
            else if (b == 'F') {
                c += 15;
            }

            if (a == '1') {
                c += 16;
            }
            else if (a == '2') {
                c += 2 * 16;
            }
            else if (a == '3') {
                c += 3 * 16;
            }
            else if (a == '4') {
                c += 4 * 16;
            }
            else if (a == '5') {
                c += 5 * 16;
            }
            else if (a == '6') {
                c += 6 * 16;
            }
            else if (a == '7') {
                c += 7 * 16;
            }
            else if (a == '8') {
                c += 8 * 16;
            }
            else if (a == '9') {
                c += 9 * 16;
            }
            else if (a == 'A') {
                c += 10 * 16;
            }
            else if (a == 'B') {
                c += 11 * 16;
            }
            else if (a == 'C') {
                c += 12 * 16;
            }
            else if (a == 'D') {
                c += 13 * 16;
            }
            else if (a == 'E') {
                c += 14 * 16;
            }
            else if (a == 'F') {
                c += 15 * 16;
            }

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