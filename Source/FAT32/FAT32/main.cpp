#include <windows.h>
#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
using namespace std;

#define SECTOR_SIZE 512
#define COL 100

struct atribute
{
    int read_only;
    int hidden;
    int system;
    int vollabel;
    int directory;
    int archive;
};

struct BootSector {
    unsigned short bytesPerSector; // 0xB
    unsigned short sectorsPerCluster; // 0xD
    unsigned short reservedSectors; // 0xE
    unsigned short numFATs; // 0x10
    unsigned int totalSectors; // 0x20
    unsigned int sectorsPerFAT; // 0x24
    unsigned int clusterNumberRDET; // 0x2C
    unsigned short sectorNumberFileSystem; // 0x30
    unsigned short sectorNumberBackupBoot; // 0x32


    //.......
};
int spc = 0;

atribute readatr(string atri)
{
    atribute temp;
    temp.read_only = atri[7] & 1;
    temp.hidden = atri[6] & 1;
    temp.system = atri[5] & 1;
    temp.vollabel = atri[4] & 1;
    temp.directory = atri[3] & 1;
    temp.archive = atri[2] & 1;
    return temp;
}

string littleEdianConvert(BYTE sector[512], int offset, int byte) {
    stringstream builder;
    for (int i = byte - 1; i >= 0; i--) {
        builder << hex << setfill('0') << setw(2) << static_cast<int>(sector[offset + i]);
    }
    string result = builder.str();
    return result;
}

void getFieldValue(BootSector& curr, BYTE sector[512]) {
    string hexStr;
    hexStr = littleEdianConvert(sector, 0xB, 2);
    curr.bytesPerSector = (int)strtol(hexStr.c_str(), NULL, 16);

    curr.sectorsPerCluster = (int)sector[0xD];

    hexStr = littleEdianConvert(sector, 0xE, 2);
    curr.reservedSectors = (int)strtol(hexStr.c_str(), NULL, 16);

    curr.numFATs = (int)sector[0x10];

    hexStr = littleEdianConvert(sector, 0x20, 4);
    curr.totalSectors = (int)strtol(hexStr.c_str(), NULL, 16);

    hexStr = littleEdianConvert(sector, 0x24, 4);
    curr.sectorsPerFAT = (int)strtol(hexStr.c_str(), NULL, 16);

    hexStr = littleEdianConvert(sector, 0x2C, 4);
    curr.clusterNumberRDET = (int)strtol(hexStr.c_str(), NULL, 16);

    hexStr = littleEdianConvert(sector, 0x2C, 2);
    curr.sectorNumberFileSystem = (int)strtol(hexStr.c_str(), NULL, 16);

    hexStr = littleEdianConvert(sector, 0x32, 2);
    curr.sectorNumberBackupBoot = (int)strtol(hexStr.c_str(), NULL, 16);

}

void printFAT32BootSector(BYTE sector[SECTOR_SIZE]) {
    printf("FAT32 PARTITION Boot Sector:\n");
    printf(" Offset    0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F\n");
    for (int i = 0; i < SECTOR_SIZE; i += 16) {
        cout << hex << setfill('0') << setw(7) << i << "   ";
        for (int j = 0; j < 16; j++) {
            if (i + j < SECTOR_SIZE) {
                cout << hex << setfill('0') << setw(2) << static_cast<int>(sector[i + j]) << " ";
            }
            else {
                cout << "   ";
            }
        }
        cout << "  ";
        for (int j = 0; j < 16; j++) {
            if (i + j < SECTOR_SIZE) {
                BYTE c = sector[i + j];
                if (c >= 32 && c <= 126) {
                    cout << c;
                }
                else {
                    cout << ".";
                }
            }
        }
        cout << "\n";
    }

}

int ReadSectorToPrint(LPCWSTR  drive, int readPoint, BYTE sector[512]) {
    int retCode = 0;
    DWORD bytesRead;
    HANDLE device = NULL;

    device = CreateFile(drive,    // Drive to open
        GENERIC_READ,           // Access mode
        FILE_SHARE_READ | FILE_SHARE_WRITE,        // Share Mode
        NULL,                   // Security Descriptor
        OPEN_EXISTING,          // How to create
        0,                      // File attributes
        NULL);                  // Handle to template

    if (device == INVALID_HANDLE_VALUE) // Open Error
    {
        cout << "CreateFile: " << GetLastError() << endl;

        return 1;
    }

    SetFilePointer(device, readPoint, NULL, FILE_BEGIN);//Set a Point to Read

    if (!ReadFile(device, sector, 512, &bytesRead, NULL))
    {
        cout << "ReadFile: " << GetLastError() << endl;
        return 1;
    }
    BootSector curr;
    printFAT32BootSector(sector);
    getFieldValue(curr, sector);
    cout << endl << "FAT name: FAT32\n";
    cout << "Bytes Per Sector: " << dec << curr.bytesPerSector << endl;
    cout << "Sectors Per Cluster: " << dec << curr.sectorsPerCluster << endl;
    cout << "Reserved Sectors: " << dec << curr.reservedSectors << endl;
    cout << "Number of file allocation tables: " << dec << curr.numFATs << endl;
    cout << "Number of Sectors in Partition: " << dec << curr.totalSectors << endl;
    cout << "Number of Sectors Per FAT: " << dec << curr.sectorsPerFAT << endl;
    cout << "Cluster Number of RDET: " << dec << curr.clusterNumberRDET << endl;
    cout << "Sector Number of the File System Information Sector: " << dec << curr.sectorNumberFileSystem << endl;
    cout << "Sector Number of the Backup Boot Sector: " << dec << curr.sectorNumberBackupBoot;
    
}

int ReadSector(LPCWSTR  drive, int readPoint, BYTE sector[512])
{
    int retCode = 0;
    DWORD bytesRead;
    HANDLE device = NULL;

    device = CreateFile(drive,    // Drive to open
        GENERIC_READ,           // Access mode
        FILE_SHARE_READ | FILE_SHARE_WRITE,        // Share Mode
        NULL,                   // Security Descriptor
        OPEN_EXISTING,          // How to create
        0,                      // File attributes
        NULL);                  // Handle to template

    if (device == INVALID_HANDLE_VALUE) // Open Error
    {
        printf("CreateFile: %u\n", GetLastError());
        return 1;
    }

    SetFilePointer(device, readPoint * 512, NULL, FILE_BEGIN);//Set a Point to Read

    if (!ReadFile(device, sector, 512, &bytesRead, NULL))
    {
        printf("ReadFile: %u\n", GetLastError());
    }
}

void tohex(BYTE data[][COL], vector<vector<string>>& a)
{
    for (int i(0); i < 32; ++i)
    {
        a.emplace_back();
        for (int j = 0; j < 16; j++)
        {
            stringstream ss;
            ss << hex;
            ss << setw(2) << setfill('0') << (int)data[i][j];
            a.back().push_back(ss.str());
        }
    }
}
       
int toNumber(BYTE* sector, int position, int number)
{
    int k = 0;
    memcpy(&k, sector + position, number);
    return k;
}

string toString(BYTE* sector, int position, int number)
{
    char* tmp = new char[number + 1];
    memcpy(tmp, sector + position, number);
    string s = "";
    for (int i = 0; i < number; i++)
        if (tmp[i] != 0x00 && tmp[i] != char(0xff))
            s += tmp[i];

    return s;
}

string typeofFAT(vector<vector<string>> a)
{
    // initialize the ASCII code string as empty.
    string b = "No";
    if (a[3][6] == "00")
        return b;
    string ascii = "";
    for (int i = 6; i <= 14; i++)
    {
        // extract two characters from hex string
        string part = a[3][i];

        // change it into base 16 and
        // typecast as the character
        char ch = stoul(part, nullptr, 16);

        // add this char to final ASCII string
        ascii += ch;
    }
    return ascii;
}

bool isNumber(const string& str)
{
    for (char const& c : str) {
        if (std::isdigit(c) == 0) return false;
    }
    return true;
}

long int getBytes(vector<vector<string>> a, int n, int m, int numbyte)
{
    long int sum = 0;
    string temp;
    for (int i = numbyte - 1; i >= 0; i--)
    {
     
        temp = temp + a[m][n + i];
    }
   
    return stol(temp, nullptr, 16);
}

int checkEntry(string a)
{
    if (a != "0f" && a != "0F")
        return 1;
    else
        return 2;
}

unsigned char hexval(unsigned char c)
{
    if ('0' <= c && c <= '9')
        return c - '0';
    else if ('a' <= c && c <= 'f')
        return c - 'a' + 10;
    else if ('A' <= c && c <= 'F')
        return c - 'A' + 10;
    else abort();
}

string hex2ascii(const string& in, string& out)
{
    out.clear();
    out.reserve(in.length() / 2);
    for (string::const_iterator p = in.begin(); p != in.end(); p++)
    {
        unsigned char c = hexval(*p);
        p++;
        if (p == in.end()) break; // incomplete last digit - should report error
        c = (c << 4) + hexval(*p); // + takes precedence over <<
        out.push_back(c);
    }
    return out;
}

void HexToBin(string& hexdec)
{
    long int i = 0;

    while (hexdec[i]) {

        switch (hexdec[i]) {
        case '0':
            cout << "0000";
            break;
        case '1':
            cout << "0001";
            break;
        case '2':
            cout << "0010";
            break;
        case '3':
            cout << "0011";
            break;
        case '4':
            cout << "0100";
            break;
        case '5':
            cout << "0101";
            break;
        case '6':
            cout << "0110";
            break;
        case '7':
            cout << "0111";
            break;
        case '8':
            cout << "1000";
            break;
        case '9':
            cout << "1001";
            break;
        case 'A':
        case 'a':
            cout << "1010";
            break;
        case 'B':
        case 'b':
            cout << "1011";
            break;
        case 'C':
        case 'c':
            cout << "1100";
            break;
        case 'D':
        case 'd':
            cout << "1101";
            break;
        case 'E':
        case 'e':
            cout << "1110";
            break;
        case 'F':
        case 'f':
            cout << "1111";
            break;
        default:
            cout << "\nInvalid hexadecimal digit "
                << hexdec[i];
        }
        i++;
    }
}

string decToBinary(int n)
{
    string s = "";
    for (int i = 7; i >= 0; i--) {
        int k = n >> i;
        if (k & 1)
            s += "1";
        else
            s += "0";
    }
    return s;
}

void readFile(LPCWSTR drive,unsigned int start, int number)
{
    int k = 0;

    while (true)
    {
        if (number > 0)
        {
            if (number <= 512)
            {
                BYTE sector[512];
                ReadSector(drive, start + k, sector);
                string line = toString(sector, 0x00, number);
                cout << "\t" << line << endl;
                number = number - 512;
            }
            else
            {
                BYTE sector[512];
                ReadSector(drive, start + k, sector);
                string line = toString(sector, 0x00, 512);
                cout << "\t" << line << endl;
                number = number - 512;
                k = k + 1;
            }

        }
        else
        {
            break;
        }
    }


}

void readSDET(LPCWSTR drive,int SRDET)
{
    int k = 0;
    string temp = "";
    int count = 0;
    while (true)
    {
        bool checkend = false;
        BYTE sector[512];
        ReadSector(drive, SRDET + k, sector);
        for (int i = 64; i < 512; i = i + 32)
        {
            if (sector[i] == 0xe5)
            {
                continue;
            }
            if (sector[i] == 0x00)
            {
                checkend = true;
                break;
            }
            if (sector[0x0B + i] == 0x0f)
            {
                temp = toString(sector, 0x01 + i, 10) + toString(sector, 0x0E + i, 12) + toString(sector, 0x1C + i, 4) + temp;
                count++;
            }
            else
            {
                if (count != 0)
                {
                    unsigned int high = toNumber(sector, 0x14 + i, 2);
                    unsigned int low = toNumber(sector, 0x1A + i, 2);
                    unsigned int total = high + low;
                    cout << "\tTen tap tin/thu muc: " << temp << endl;
                    cout << "\tKich thuoc: " << dec << toNumber(sector, 0x1C + i, 4) << endl;
                    cout << "\tTrang thai: ";
                    int temp2 = toNumber(sector, 0x0b + i, 1);
                    atribute temp1;
                    string temp3 = decToBinary(temp2);
                    temp1 = readatr(temp3);
                    if (temp1.archive == 1)
                    {
                        cout << "archive" << " ";
                    }
                    if (temp1.hidden == 1)
                    {
                        cout << "hidden" << " ";
                    }
                    if (temp1.system == 1)
                    {
                        cout << "system" << " ";
                    }
                    if (temp1.read_only == 1)
                    {
                        cout << "read only" << " ";
                    }
                    if (temp1.vollabel == 1)
                    {
                        cout << "vollabel" << " ";
                    }
                    if (temp1.directory == 1)
                    {
                        cout << "directory" << " ";
                    }
                    cout << endl;
                    cout << "\tCluster bat dau: " << total << endl;
                    if (total != 0x0)
                    {
                        cout << "\tChiem cac sector: ";
                        for (int i = 0; i < spc; i++)
                        {
                            cout << (total - 2) * spc - SRDET + i << " ";
                        }
                        cout << endl;
                    }
                    if (temp1.directory == 1 && temp1.system != 1)
                    {
                        cout << "\tTap tin/thu muc con: " << endl;
                        readSDET(drive,(total - 2) * spc + SRDET);
                    }
                    if (temp[temp.length() - 1] == 't' && temp[temp.length() - 2] == 'x' && temp[temp.length() - 3] == 't' && temp1.archive == 1)
                    {
                        cout << "Content: " << endl;
                        readFile(drive,(total - 2) * spc + SRDET, toNumber(sector, 0x1C + i, 4));
                    }
                    else if (temp[temp.length() - 1] != 't' && temp[temp.length() - 2] != 'x' && temp[temp.length() - 3] != 't' && temp1.archive == 1)
                    {
                        int pos = 0;
                        for (int i = 0; i < temp.length(); i++) {
                            if (temp[i] == '.')
                                pos = i;
                        }
                        string Type = temp.substr(pos, temp.length() - pos + 1);
                        if (Type == ".jpg" || Type == ".png" || Type == ".bmp")
                            cout << "\t\t\t => Use Microsoft Word to open!\n";
                        if (Type == ".doc" || Type == ".docx")
                            cout << "\t\t\t => Use Microsoft Word to open!\n";
                        else if (Type == ".ppt" || Type == ".pptx")
                            cout << "\t\t\t => Use Microsoft PowerPoint to open!\n";
                        else if (Type == ".xls" || Type == ".xlsx")
                            cout << "\t\t\t => Use Microsoft Excel to open!\n";
                        else if (Type == ".sln" || Type == ".cpp" || Type == ".java" || Type == ".html" || Type == ".css")
                            cout << "\t\t\t => Use Microsoft Visual Studio to open!\n";
                        else if (Type == ".pdf")
                            cout << "\t\t\t => Use Foxit PDF Reader to open!\n";
                    }
                    temp = "";
                    count = 0;
                }
                else
                {
                    if (toString(sector, 0x08 + i, 3) != "   ")
                    {
                        unsigned int high = toNumber(sector, 0x14 + i, 2);
                        unsigned int low = toNumber(sector, 0x1A + i, 2);
                        unsigned int total = high + low;
                        cout << "\tTen tap tin: " << toString(sector, 0x00 + i, 8) + toString(sector, 0x08 + i, 3) << endl;
                        cout << "\tKich thuoc: " << dec << toNumber(sector, 0x1C + i, 4) << endl;
                        cout << "\tTrang thai: ";
                        int temp = toNumber(sector, 0x0b + i, 1);
                        atribute temp1;
                        string temp2 = decToBinary(temp);
                        temp1 = readatr(temp2);
                        if (temp1.archive == 1)
                        {
                            cout << "archive" << " ";
                        }
                        if (temp1.hidden == 1)
                        {
                            cout << "hidden" << " ";
                        }
                        if (temp1.system == 1)
                        {
                            cout << "system" << " ";
                        }
                        if (temp1.read_only == 1)
                        {
                            cout << "read only" << " ";
                        }
                        if (temp1.vollabel == 1)
                        {
                            cout << "vollabel" << " ";
                        }
                        if (temp1.directory == 1)
                        {
                            cout << "directory" << " ";
                        }
                        cout << endl;
                        cout << "\tCluster bat dau: " << total << endl;
                        if (total != 0x0)
                        {
                            cout << "\tChiem cac sector: ";
                            for (int i = 0; i < spc; i++)
                            {
                                cout << (total - 2) * spc - SRDET + i << " ";
                            }
                            cout << endl;
                        }
                        if (temp1.archive == 1 && toString(sector, 0x08 + i, 3) == "TXT")
                        {
                            cout << "Content: " << endl;
                            readFile(drive,(total - 2) * spc + SRDET, toNumber(sector, 0x1C + i, 4));
                        }
                        else
                        {
                            cout << "Dung chuong trinh phu hop khac de doc!" << endl;
                        }
                    }
                    else
                    {
                        unsigned int high = toNumber(sector, 0x14 + i, 2);
                        unsigned int low = toNumber(sector, 0x1A + i, 2);
                        unsigned int total = high + low;
                        cout << "\tTen tap tin/thu muc: " << toString(sector, 0x00 + i, 8) << endl;
                        cout << "\tKich thuoc: " << dec << toNumber(sector, 0x1C + i, 4) << endl;
                        cout << "\tTrang thai: ";
                        int temp = toNumber(sector, 0x0b + i, 1);
                        atribute temp1;
                        string temp2 = decToBinary(temp);
                        temp1 = readatr(temp2);
                        if (temp1.archive == 1)
                        {
                            cout << "archive" << " ";
                        }
                        if (temp1.hidden == 1)
                        {
                            cout << "hidden" << " ";
                        }
                        if (temp1.system == 1)
                        {
                            cout << "system" << " ";
                        }
                        if (temp1.read_only == 1)
                        {
                            cout << "read only" << " ";
                        }
                        if (temp1.vollabel == 1)
                        {
                            cout << "vollabel" << " ";
                        }
                        if (temp1.directory == 1)
                        {
                            cout << "directory" << " ";
                        }
                        cout << endl;
                        cout << "\tCluster bat dau: " << total << endl;
                        if (total != 0x0)
                        {
                            cout << "\tChiem cac sector: ";
                            for (int i = 0; i < spc; i++)
                            {
                                cout << (total - 2) * spc - SRDET + i << " ";
                            }
                            cout << endl;
                        }
                        if (temp1.directory == 1 && temp1.system != 1)
                        {
                            cout << "\tTap tin/thu muc con: " << endl;
                            readSDET(drive,(total - 2) * spc + SRDET);
                        }
                    }
                }
                cout << endl;
            }
        }
        if (checkend == true)
        {
            break;
        }
        else
        {
            k++;

        }
    }
}

void readRDET(LPCWSTR drive,int SRDET)
{
    int k = 0;
    string temp = "";
    int count = 0;
    while (true)
    {
        bool checkend = false;
        BYTE sector[512];
        ReadSector(drive, SRDET + k, sector);
        for (int i = 0; i < 512; i = i + 32)
        {
            if (sector[i] == 0xe5)
            {
                continue;
            }
            if (sector[i] == 0x00)
            {
                checkend = true;
                break;
            }
            if (sector[0x0B + i] == 0x0f)
            {

                temp = toString(sector, 0x01 + i, 10) + toString(sector, 0x0E + i, 12) + toString(sector, 0x1C + i, 4) + temp;
                count++;

            }
            else
            {
                if (count != 0)
                {
                    unsigned int high = toNumber(sector, 0x14 + i, 2);
                    unsigned int low = toNumber(sector, 0x1A + i, 2);
                    unsigned int total = high + low;
                    cout << "File name: " << temp << endl;
                    cout << "Lenght: " << dec << toNumber(sector, 0x1C + i, 4) << endl;
                    cout << "Status: ";
                    int temp2 = toNumber(sector, 0x0b + i, 1);
                    atribute temp1;
                    string temp3 = decToBinary(temp2);
                    temp1 = readatr(temp3);
                    if (temp1.archive == 1)
                    {
                        cout << "archive" << " ";
                    }
                    if (temp1.hidden == 1)
                    {
                        cout << "hidden" << " ";
                    }
                    if (temp1.system == 1)
                    {
                        cout << "system" << " ";
                    }
                    if (temp1.read_only == 1)
                    {
                        cout << "read only" << " ";
                    }
                    if (temp1.vollabel == 1)
                    {
                        cout << "vollabel" << " ";
                    }
                    if (temp1.directory == 1)
                    {
                        cout << "directory" << " ";
                    }
                    cout << endl;

                    cout << "Cluster bat dau: " << total << endl;
                    if (total != 0x0)
                    {
                        cout << "Chiem cac sector: ";
                        for (int i = 0; i < spc; i++)
                        {
                            cout << (total - 2) * spc - SRDET + i << " ";
                        }
                        cout << endl;
                    }

                    if (temp1.directory == 1 && temp1.system != 1)
                    {
                        cout << "File/sub folder: " << endl;
                        readSDET(drive,(total - 2) * spc + SRDET);
                    }
                    if (temp[temp.length() - 1] == 't' && temp[temp.length() - 2] == 'x' && temp[temp.length() - 3] == 't' && temp1.archive == 1)
                    {
                        cout << "Content: " << endl;
                        readFile(drive,(total - 2) * spc + SRDET, toNumber(sector, 0x1C + i, 4));
                    }
                    else if (temp[temp.length() - 1] != 't' && temp[temp.length() - 2] != 'x' && temp[temp.length() - 3] != 't' && temp1.archive == 1)
                    {
                        int pos = 0;
                        for (int i = 0; i < temp.length(); i++) {
                            if (temp[i] == '.')
                                pos = i;
                        }
                        string Type = temp.substr(pos, temp.length() - pos + 1);

                        if (Type == ".doc" || Type == ".docx")
                            cout << "\t\t\t => Use Microsoft Office Word to open!\n";
                        else if (Type == ".ppt" || Type == ".pptx")
                            cout << "\t\t\t => Use Microsoft Office PowerPoint to open!\n";
                        else if (Type == ".xls" || Type == ".xlsx")
                            cout << "\t\t\t => Use Microsoft Office Excel to open!\n";
                        else if (Type == ".sln" || Type == ".cpp" || Type == ".java" || Type == ".html" || Type == ".css")
                            cout << "\t\t\t => Use Microsoft Visual Studio to open!\n";
                        else if (Type == ".pdf")
                            cout << "\t\t\t => Use Foxit PDF Reader or Web Browers (Edge, Chrome, ...) to open!\n";
                        else
                        {
                            cout << "Can open this file!!!" << endl;
                            return;
                        }
                    }
                    temp = "";
                    count = 0;
                }
                else
                {
                    if (toString(sector, 0x08 + i, 3) != "   ")
                    {
                        unsigned int high = toNumber(sector, 0x14 + i, 2);
                        unsigned int low = toNumber(sector, 0x1A + i, 2);
                        unsigned int total = high + low;
                        cout << "File name: " << toString(sector, 0x00 + i, 8) + toString(sector, 0x08 + i, 3) << endl;
                        cout << "Lenght: " << dec << toNumber(sector, 0x1C + i, 4) << endl;
                        cout << "Status: ";
                        int temp = toNumber(sector, 0x0b + i, 1);
                        atribute temp1;
                        string temp2 = decToBinary(temp);
                        temp1 = readatr(temp2);
                        if (temp1.archive == 1)
                        {
                            cout << "archive" << " ";
                        }
                        if (temp1.hidden == 1)
                        {
                            cout << "hidden" << " ";
                        }
                        if (temp1.system == 1)
                        {
                            cout << "system" << " ";
                        }
                        if (temp1.read_only == 1)
                        {
                            cout << "read only" << " ";
                        }
                        if (temp1.vollabel == 1)
                        {
                            cout << "vollabel" << " ";
                        }
                        if (temp1.directory == 1)
                        {
                            cout << "directory" << " ";
                        }
                        cout << endl;
                        cout << "Cluster bat dau: " << total << endl;
                        if (total != 0x0)
                        {
                            cout << "Chiem cac sector: ";
                            for (int i = 0; i < spc; i++)
                            {
                                cout << (total - 2) * spc - SRDET + i << " ";
                            }
                            cout << endl;
                        }
                        if (temp1.archive == 1 && toString(sector, 0x08 + i, 3) == "TXT")
                        {
                            cout << "Content: " << endl;
                            readFile(drive,(total - 2) * spc + SRDET, toNumber(sector, 0x1C + i, 4));
                        }
                    }
                    else
                    {
                        unsigned int high = toNumber(sector, 0x14 + i, 2);
                        unsigned int low = toNumber(sector, 0x1A + i, 2);
                        unsigned int total = high + low;
                        cout << "File name: " << toString(sector, 0x00 + i, 8) << endl;
                        cout << "Lenght: " << dec << toNumber(sector, 0x1C + i, 4) << endl;
                        cout << "Status: ";
                        int temp = toNumber(sector, 0x0b + i, 1);
                        atribute temp1;
                        string temp2 = decToBinary(temp);
                        temp1 = readatr(temp2);
                        if (temp1.archive == 1)
                        {
                            cout << "archive" << " ";
                        }
                        if (temp1.hidden == 1)
                        {
                            cout << "hidden" << " ";
                        }
                        if (temp1.system == 1)
                        {
                            cout << "system" << " ";
                        }
                        if (temp1.read_only == 1)
                        {
                            cout << "read only" << " ";
                        }
                        if (temp1.vollabel == 1)
                        {
                            cout << "vollabel" << " ";
                        }
                        if (temp1.directory == 1)
                        {
                            cout << "directory" << " ";
                        }
                        cout << endl;
                        cout << "Cluster bat dau: " << total << endl;
                        if (total != 0x0)
                        {
                            cout << "Chiem cac sector: ";
                            for (int i = 0; i < spc; i++)
                            {
                                cout << (total - 2) * spc - SRDET + i << " ";
                            }
                            cout << endl;
                        }
                        if (temp1.directory == 1 && temp1.system != 1)
                        {
                            cout << "File/sub folder :  " << endl;
                            readSDET(drive,(total - 2) * spc + SRDET);
                        }
                    }
                }
                cout << endl;
            }
        }
        if (checkend == true)
        {
            break;
        }
        else
        {
            k++;

        }
    }
}

void readBootSector(vector<vector<string>> hexarr)
{
    long int SV, NF, SB, SRDET, SF;
        typeofFAT(hexarr);
    if (getBytes(hexarr, 6, 1, 2) != 0)
    {
        SF = getBytes(hexarr, 6, 1, 2);
    }
    else
    {
        SF = getBytes(hexarr, 4, 2, 4);
    }
    getBytes(hexarr, 11, 0, 2);
    spc = getBytes(hexarr, 13, 0, 1);
    getBytes(hexarr, 13, 0, 1);
    getBytes(hexarr, 14, 0, 2);
    SB = getBytes(hexarr, 14, 0, 2);
    getBytes(hexarr, 0, 1, 1);
    NF = getBytes(hexarr, 0, 1, 1);
    cout << "Number of sectors in RDET: " << getBytes(hexarr, 1, 1, 2) * 32 / getBytes(hexarr, 11, 0, 2) << " sector" << endl;
    SRDET = SB + SF * NF;
    if (getBytes(hexarr, 3, 1, 2) != 0)
    {
        cout << "Lenght volume: SV = " << getBytes(hexarr, 3, 1, 2) << " sector = " << getBytes(hexarr, 3, 1, 2) / 2048 << "MB" << endl;
        SV = getBytes(hexarr, 3, 1, 2);
    }
    else
    {
        cout << "Lenght volume: SV = " << getBytes(hexarr, 0, 2, 4) << " sector = " << getBytes(hexarr, 0, 2, 4) / 2048 << "MB" << endl;
        SV = getBytes(hexarr, 0, 2, 4);
    }
    cout << "First sector of FAT1: " << SB << endl;
    cout << "Starting sector of RDET: " << SB + NF * SF << endl;
    cout << "Starting sector of DATA: " << SRDET << endl;
}

int main(int argc, char** argv)
{

    wstring name;
    cout << "Enter drive name: ";
    wcin >> name;
    name = L"\\\\.\\" + name + L":";
    LPCWSTR drive = name.c_str();
    BYTE sector[512];
    BYTE temp[100][100];
    vector<vector<string>> hexarr;  
            ReadSectorToPrint(drive, 0, sector);
            for (int i = 0; i < 32; i++)
            {
                for (int j = 0; j < 16; j++)
                {
                    temp[i][j] = sector[i * 16 + j];
                }
            }
            tohex(temp, hexarr);
            cout << endl;

            readBootSector(hexarr);

            cout << "_________________________________________________________" << endl;
            cout << "\t \t \t TREE FOLDER" << endl;          
            long int SB = getBytes(hexarr, 14, 0, 2);
            long int SF;
            long int NF = getBytes(hexarr, 0, 1, 1);
            if (getBytes(hexarr, 6, 1, 2) != 0)
            {
                SF = getBytes(hexarr, 6, 1, 2);
            }
            else
            {
                SF = getBytes(hexarr, 4, 2, 4);
            }
            long int SRDET = SB + SF * NF;

            BYTE sector1[512];
            BYTE temp1[100][100];
            vector<vector<string>> hexarr1;

            ReadSector(L"\\\\.\\A:", SB, sector1);
            for (int i = 0; i < 32; i++)
            {
                for (int j = 0; j < 16; j++)
                {
                    temp1[i][j] = sector1[i * 16 + j];
                }
            }
            cout << endl;
            tohex(temp1, hexarr1);
            cout << endl;
            readRDET(drive,SRDET);
    return 0;
}