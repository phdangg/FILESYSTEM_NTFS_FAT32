#include<iostream>
#include<cstdlib>
#include<stdio.h>
#include<vector>
#include<windows.h>
#include<iomanip>
#include<sstream>
#include<map>
#include<string>
#include<cstring>
#include<fstream>
#include<io.h>
#include <sstream>

using namespace std;

vector<int> fileID;
vector<int> parentID;
vector<string> nameFile;
bool check = false;

int readSector(LPCWSTR drive, int readPoint, BYTE*& sector) {
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
        cout << "CreateFile : " << GetLastError() << endl;
        cout << endl;
        return 0;
    }

    SetFilePointer(device, readPoint, NULL, FILE_BEGIN);//Set a Point to Read

    if (!ReadFile(device, sector, 512, &bytesRead, NULL))
    {
        cout << "ReadFile : " << GetLastError() << endl;
        return 0;
    }
    else
    {
        cout << "Read successfuly !" << endl;
        cout << endl;
        return 1;
    }
}

void ReadSect(LPCWSTR disk, BYTE*& DATA, unsigned int _nsect) {
    DWORD dwBytesRead(0);

    HANDLE hFloppy = NULL;
    hFloppy = CreateFile(disk,    // Floppy drive to open
        GENERIC_READ,              // Access mode
        FILE_SHARE_READ | FILE_SHARE_WRITE,           // Share Mode
        NULL,                      // Security Descriptor
        OPEN_EXISTING,             // How to create
        0,                         // File attributes
        NULL);                     // Handle to template

    if (hFloppy != NULL)
    {
        LARGE_INTEGER li;
        li.QuadPart = _nsect * 512;
        SetFilePointerEx(hFloppy, li, 0, FILE_BEGIN);

        if (!ReadFile(hFloppy, DATA, 512, &dwBytesRead, NULL))
        {
            printf("Error in reading floppy disk\n");
        }

        CloseHandle(hFloppy);

    }
}

int64_t Get_Bytes(BYTE* sector, int offset, int number) {

    int64_t result = 0;
    memcpy(&result, sector + offset, number);
    return result;
}

string LittleEndianConvert(BYTE* DATA, int offset, int number) {
    char* temp = NULL;
    temp = new char[number + 1];
    memcpy(temp, DATA + offset, number);
    string str = "";
    for (int i = 0; i < number; i++)
        if (temp[i] != 0x00)
            str += temp[i];

    return str;
}

string toBinary(int n) {
    string rs;
    while (n != 0)
    {
        rs = (n % 2 == 0 ? "0" : "1") + rs; n /= 2;
    }
    return rs;
}

string Get_Filename(int id)
{
    string rs = "";
    int pos = -1;
    for (int i = 0; i < fileID.size(); i++)
        if (fileID[i] == id)
        {
            pos = i;
            break;
        }

    if (pos != -1)
        rs = nameFile[pos];
    return rs;
}

int Read_Info_Entry(BYTE* Entry, int start)
{
    int status = Get_Bytes(Entry, start + 56, 4);
    string bin = toBinary(status);
    for (int i = bin.length() - 1; i >= 0; i--)
    {
        int n = bin.length();
        if (bin[i] == '1')
        {
            if (i == n - 2)
            {
                // Hidden
                return -1;
            }
            if (i == n - 3)
            {
                // File System
                return -1;
            }
        }
    }
    cout << "STANDARD_INFORMATION" << endl;

    int size = Get_Bytes(Entry, start + 4, 4);
    cout << "- Length of attribute (header included): " << size << endl;
    cout << "- Status Attribute of File: " << bin << endl;
    for (int i = bin.length() - 1; i >= 0; i--)
    {
        int n = bin.length();
        if (bin[i] == '1')
        {
            if (i == n - 1)
                cout << "\t => Read Only" << endl;
            if (i == n - 4)
                cout << "\t => Vollabel" << endl;
            if (i == n - 5)
                cout << "\t => Directory" << endl;
            if (i == n - 6)
                cout << "\t => Archive" << endl;
        }
    }
    cout << endl;
    return size;
}

int Read_Filename_Entry(BYTE* Entry, int start, int ID)
{

    check = false;

    cout << "Attribute $FILE_NAME" << endl;
    int size = Get_Bytes(Entry, start + 4, 4);
    cout << "- Length of attribute (header included): " << size << endl;
    int parent_file = Get_Bytes(Entry, start + 24, 6);
    cout << "- Parent file: " << parent_file << endl;

    parentID.push_back(parent_file);

    int lengthName = Get_Bytes(Entry, start + 88, 1);
    cout << "- Length of name file: " << lengthName << endl;
    string name = LittleEndianConvert(Entry, start + 90, lengthName * 2);
    cout << "- Name of file: " << name << endl;

    string exts = "";
    size_t pos = name.rfind('.');
    if (pos != string::npos) {
        exts = name.substr(pos + 1);
    }
    if (exts == "jpg" || exts == "bmp" || exts == "png")
        cout << "\t => Use Microsoft Photo to open this file\n";
    if (exts == "doc" || exts == "docx")
        cout << "\t => Use Microsoft Word to open this fille\n";
    if (exts == "ppt" || exts == "pptx")
        cout << "\t => Use Microsoft PowerPoint to open this file\n";
    if (exts == "xls" || exts == "xlsx")
        cout << "\t => Use Microsoft Excel to open this file\n";
    if (exts == "sln" || exts == "cpp")
        cout << "\t => Use Microsoft Visual Studio to open this file\n";
    if (exts == "pdf")
        cout << "\t => Use Foxit PDF Reader to open this fie\n";

    if (exts == "txt") check = true;
    nameFile.push_back(name);
    cout << endl;

    return size;
}

void Read_Data_Entry(BYTE* Entry, int start)
{
    cout << "Attribute $DATA" << endl;
    int size = Get_Bytes(Entry, start + 4, 4);
    cout << "- Length of attribute (header included): " << size << endl;
    int sizeFile = Get_Bytes(Entry, start + 16, 4);
    cout << "- Size of file: " << sizeFile << endl;

    int type = Get_Bytes(Entry, start + 8, 1);
    if (type == 0 && check == true)
    {
        cout << "\t=> Resident" << endl;
        int cont_Size = Get_Bytes(Entry, start + 16, 4);
        int cont_Start = Get_Bytes(Entry, start + 20, 2);
        string content = LittleEndianConvert(Entry, start + cont_Start, cont_Size);
        cout << endl;
        cout << "Content: " << content << endl;
    }
    else
        cout << "\t=> Non-resident" << endl;
    cout << endl;
}

void printSector(BYTE* sector)
{
    int count = 0;
    int num = 0;

    cout << "offset   0  1  2  3  4  5  6  7    8  9  A  B  C  D  E  F" << endl;

    cout << "0x0" << num << "0  ";
    bool flag = 0;
    for (int i = 0; i < 512; i++)
    {
        count++;
        if (i % 8 == 0)
            cout << "  ";
        printf("%02X ", sector[i]);
        if (i == 255)
        {
            flag = 1;
            num = 0;
        }

        if (i == 511) break;
        if (count == 16)
        {
            int index = i;
            cout << endl;
            if (flag == 0)
            {
                num++;
                if (num < 10)
                    cout << "0x0" << num << "0  ";
                else
                {
                    char hex = char(num - 10 + 'A');
                    cout << "0x0" << hex << "0  ";
                }
            }
            else
            {
                if (num < 10)
                    cout << "0x1" << num << "0  ";
                else
                {
                    char hex = char(num - 10 + 'A');
                    cout << "0x1" << hex << "0  ";
                }
                num++;
            }
            count = 0;
        }
    }
    cout << endl;
}

void Print_TreeFolder(int a, int slash, int pos)
{
    slash++;
    for (int i = 0; i < slash; i++) {
        cout << "\t";
    }
    cout << Get_Filename(a) << endl;

    fileID[pos] = -1;
    parentID[pos] = -1;

    vector<int> children;
    vector<int> VT;

    for (int j = 0; j < fileID.size(); j++)
        if (parentID[j] == a)
        {
            children.push_back(fileID[j]);
            VT.push_back(j);
        }

    if (children.size() == 0)
    {
        cout << endl;
        return;
    }

    //recursion
    for (int i = 0; i < children.size(); i++)
    {
        Print_TreeFolder(children[i], slash, VT[i]);

    }

}

void TreeFolder(unsigned int len_MFT, unsigned int MFTStart, LPCWSTR disk)
{

    for (int i = 2; i < len_MFT - MFTStart; i += 2)
    {
        int currentSector = MFTStart + i;
        BYTE* currentEntry = NULL;
        currentEntry = new BYTE[512];
        ReadSect(disk, currentEntry, currentSector);
        if (LittleEndianConvert(currentEntry, 0x00, 4) == "FILE")
        {
            check = false;
            int ID = Get_Bytes(currentEntry, 0x02C, 4);
            if (ID > 38)
            {
                cout << endl;
                cout << endl;
                cout << "File ID: " << ID << endl;
                int startInfor = Get_Bytes(currentEntry, 0x014, 2);
                int sizeInfor = Read_Info_Entry(currentEntry, startInfor);
                if (sizeInfor == -1)
                    continue;
                int startName = sizeInfor + 56;
                int sizeName = Read_Filename_Entry(currentEntry, startName, ID);
                int startData = startName + sizeName;
                if (Get_Bytes(currentEntry, startData, 4) == 64)
                {
                    int len_obj = Get_Bytes(currentEntry, startData + 4, 4);
                    startData += len_obj;
                    Read_Data_Entry(currentEntry, startData);
                }
                fileID.push_back(ID);
            }
        }
        delete currentEntry;
    }

    cout << "_________________________________________________________" << endl;
    cout << "\t \t \t TREE FOLDER" << endl;
    for (int i = 0; i < fileID.size(); i++)
        if (fileID[i] != -1 && parentID[i] != -1)
            Print_TreeFolder(fileID[i], -1, i);
}

void read_MFT(unsigned int MFTStart, unsigned int sectors_per_cluster, LPCWSTR disk)
{
    BYTE* MFT = new BYTE[512];
    MFTStart *= sectors_per_cluster;
    ReadSect(disk, MFT, MFTStart);

    int Entry_in4 = Get_Bytes(MFT, 0x014, 2);
    cout << "$INFORMATION Entry starts at: " << Entry_in4 << endl;
    int len_in4 = Get_Bytes(MFT, 0x048, 4);
    cout << "Length of INFORMATION Entry: " << len_in4 << endl;
    int Entry_Name = Entry_in4 + len_in4;
    cout << "$FILENAME Entry starts at: " << Entry_Name << endl;
    int len_Name = Get_Bytes(MFT, 0x09C, 4);
    cout << "Length of $FILENAME Entry: " << len_Name << endl;
    int tmp = Get_Bytes(MFT, 0x108, 4);
    int Entry_Data = 0;
    if (tmp == 64) {
        Entry_Data = Entry_Name + len_Name + Get_Bytes(MFT, 0x10C, 4);
        cout << "$DATA Entry starts at: " << Entry_Data << endl;
        int len_data = Get_Bytes(MFT, 0x134, 4);
        cout << "Length of DATA Entry : " << len_data << endl;
    }
    else {
        Entry_Data = Entry_Name + len_Name;
        cout << "DATA Entry starts at: " << Entry_Data << endl;
        int len_data = Get_Bytes(MFT, 0x10C, 4);
        cout << "Length of DATA Entry: " << len_data << endl;
    }

    unsigned int len_MFT = MFTStart + (Get_Bytes(MFT, Entry_Data + 24, 8) + 1) * 8;
    cout << "Number of sectors in MFT is: " << len_MFT - MFTStart << endl;
    cout << endl << endl;

    TreeFolder(len_MFT, MFTStart, disk);
}

void Read_BPB(BYTE* sector, LPCWSTR disk)
{
    unsigned int bytes_per_sector = Get_Bytes(sector, 0x0B, 2); // Bytes Per Sector
    unsigned int sectors_per_cluster = Get_Bytes(sector, 0x0D, 1); // Sectors Per Cluster
    unsigned int sectors_per_track = Get_Bytes(sector, 0x18, 2); // Sectors Per Track
    unsigned int total_sectors = Get_Bytes(sector, 0x28, 8); // Total Sectors
    unsigned int MFTStart = Get_Bytes(sector, 0x30, 8); // Cluster start of MFT
    unsigned int MFTMirrorStart = Get_Bytes(sector, 0x38, 8); // Cluster start of MFTMirror
    cout << endl;
    cout << endl << endl << endl;
    cout << "|Bytes Per Sector : " << bytes_per_sector << endl;
    cout << "|Sectors Per Cluster : " << sectors_per_cluster << endl;
    cout << "|Sectors Per Track : " << sectors_per_track << endl;
    cout << "|Total Sectors : " << total_sectors << endl;
    cout << "|Cluster start of MFT : " << MFTStart << endl;
    cout << "|Cluster start of MFTMirror : " << MFTMirrorStart << endl;
    cout << endl << endl << endl;

    read_MFT(MFTStart, sectors_per_cluster, disk);
}

int main(int argc, char** argv)
{
    wstring name;
    cout << "Enter drive name: ";
    wcin >> name;
    name = L"\\\\.\\" + name + L":";
    LPCWSTR drive = name.c_str();
    BYTE* sector = new BYTE[512];
    bool check = readSector(drive, 0, sector);
    if (check == true)
    {
        printSector(sector);
        Read_BPB(sector, drive);
        delete[] sector;
    }
    else
    {
        cout << "Error reading this drive !" << endl;
        return 0;
    }
}

