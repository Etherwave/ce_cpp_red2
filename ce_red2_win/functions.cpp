//
// Created by Amazing on 2021/12/2.
//

#include "functions.h"

#include <Windows.h>
//将string转换成wstring  
wstring string2wstring(string str)
{
    wstring result;
    //获取缓冲区大小，并申请空间，缓冲区大小按字符计算  
    int len = MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.size(), NULL, 0);
    TCHAR* buffer = new TCHAR[len + 1];
    //多字节编码转换成宽字节编码  
    MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.size(), buffer, len);
    buffer[len] = '\0';             //添加字符串结尾  
    //删除缓冲区并返回值  
    result.append(buffer);
    delete[] buffer;
    return result;
}

//将wstring转换成string  
string wstring2string(wstring wstr)
{
    string result;
    //获取缓冲区大小，并申请空间，缓冲区大小事按字节计算的  
    int len = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), wstr.size(), NULL, 0, NULL, NULL);
    char* buffer = new char[len + 1];
    //宽字节编码转换成多字节编码  
    WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), wstr.size(), buffer, len, NULL, NULL);
    buffer[len] = '\0';
    //删除缓冲区并返回值  
    result.append(buffer);
    delete[] buffer;

    return result;
}

inline int Hexchar2int(char c)
{
    int ans = 0;
    if (c >= '0' && c <= '9')ans = c - '0';
    else ans = c - 'A' + 10;
    if (ans > 32)ans -= 32;
    return ans;
}

inline char Hexint2char(int t)
{
    char c = '0';
    if (t >= 0 && t <= 9)c = t + '0';
    else c = t - 10 + 'A';
    return c;
}

uint8_t* HexCodeString2uint8t(string HexCodeString)
{
    int t = 0;
    int cnt = 0;
    int size = HexCodeString.size() / 3 + 1;
    uint8_t* data = (uint8_t*)malloc(sizeof(uint8_t) * size);
    for (int i = 0; i != HexCodeString.size(); i++)
    {
        if (HexCodeString[i] == ' ')
        {
            data[cnt] = t; t = 0; cnt++;
        }
        else
        {
            t <<= 4;
            t += Hexchar2int(HexCodeString[i]);
        }
    }
    data[cnt] = t;
    return data;
}

string Uint8_tData2HexString(uint8_t* data, unsigned int size)
{
    string hexstr = "";
    for (int i = 0; i != size; i++)
    {
        if (i != 0)hexstr += " ";
        hexstr += Hexint2char((data[i] & 0xF0) >> 4);
        hexstr += Hexint2char(data[i] & 0x0F);
    }
    return hexstr;
}

string offset2String(uintptr_t offset)
{
    string ans = "";
    for (int i = 0; i != 4; i++)
    {
        int t = offset & 0xFF;
        if (i != 0)ans += " ";
        ans += Hexint2char(t >> 4);
        ans += Hexint2char(t & 0xF);
        offset >>= 8;
    }
    return ans;
}

int FindPID(string ProcessName)
{
    int pid = -1;
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(pe32);
    HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE) {
        cout << "CreateToolhelp32Snapshot Error!" << endl;;
        return false;
    }
    BOOL bResult = Process32First(hProcessSnap, &pe32);
    int num(0);
    while (bResult)
    {
        if (wstring2string(wstring(pe32.szExeFile)) == ProcessName)
        {
            pid = pe32.th32ProcessID;
            break;
        }
        bResult = Process32Next(hProcessSnap, &pe32);
    }
    CloseHandle(hProcessSnap);
    return pid;
}


HANDLE get_handle(int pid)
{
    HANDLE handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    return handle;
}


uintptr_t calc_offsets(DWORD pid, uintptr_t ptr, vector<uintptr_t> offsets)
{
    uintptr_t addr = ptr;
    uintptr_t t;
    HANDLE handle = get_handle(pid);
    for (unsigned int i = 0; i != offsets.size(); i++)
    {
        addr += offsets[i];
        if (i < offsets.size() - 1)//最后一次只加偏移量，不用读取了
        {
            bool state = ReadProcessMemory(handle, (LPVOID)addr, &t, sizeof(t), 0);
            if (!state)
            {
                //                cout<<"error in reading memory!"<<endl;
                addr = -1;
                break;
            }
            addr = t;
        }
    }
    CloseHandle(handle);
    return addr;
}

LPVOID process_alloc(DWORD pid, unsigned int bytes)
{
    LPVOID virAddr = NULL;
    HANDLE handle = get_handle(pid);
    virAddr = VirtualAllocEx(handle, NULL, bytes, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    CloseHandle(handle);
    return virAddr;
}

void process_free(DWORD pid, LPVOID virAddr, SIZE_T size)
{
    HANDLE handle = get_handle(pid);
    VirtualFreeEx(handle, virAddr, 0, MEM_RELEASE);
    CloseHandle(handle);
}

bool get_all_module_base_address(DWORD pid, map<string, HMODULE>& module_base_address_map)
{
    module_base_address_map.clear();
    bool flag = TRUE;
    const int max_module_number = 1024;
    HMODULE hMods[max_module_number];
    HANDLE handle;
    DWORD cbNeeded;
    unsigned int i;

    // Get a handle to the process.
    handle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
    if (handle == NULL)flag = false;
    if (flag != false)
    {
        // Get a list of all the modules in this process.

        //if (EnumProcessModules(handle, hMods, sizeof(hMods), &cbNeeded))
        //if (EnumProcessModulesEx(handle, hMods, sizeof(hMods), &cbNeeded, LIST_MODULES_32BIT))
        //if (EnumProcessModulesEx(handle, hMods, sizeof(hMods), &cbNeeded, LIST_MODULES_64BIT))
        //if (EnumProcessModulesEx(handle, hMods, sizeof(hMods), &cbNeeded, LIST_MODULES_ALL)!=0)
        if (EnumProcessModulesEx(handle, hMods, sizeof(hMods), &cbNeeded, (DWORD)(0x01 | 0x02)) != 0)
            //        if (EnumProcessModulesEx(handle, hMods, sizeof(hMods), &cbNeeded, LIST_MODULES_32BIT)!=0)
        {
            for (i = 0; i < (cbNeeded / sizeof(HMODULE)); i++)
            {
                TCHAR szModName[MAX_PATH];

                // Get the full path to the module's file.
                if (GetModuleFileNameEx(handle, hMods[i], szModName, sizeof(szModName) / sizeof(TCHAR)))
                {
                    // Print the module name and handle value.                 
                    module_base_address_map[wstring2string(wstring(szModName))] = hMods[i];
                }
            }
        }
        // Release the handle to the process.
        CloseHandle(handle);
    }
    return flag;
}

HMODULE get_inject_module_base_address(DWORD pid, string inject_module_name)
{
    HMODULE inject_module_base_address = NULL;
    map<string, HMODULE> module_base_address_map;
    bool flag = get_all_module_base_address(pid, module_base_address_map);
    if (flag == FALSE)return inject_module_base_address;

    for (auto it = module_base_address_map.begin(); it != module_base_address_map.end(); it++)
    {
        int module_name_start = it->first.rfind("\\") + 1;
        string module_name = it->first.substr(module_name_start, it->first.size() - module_name_start);
        if (module_name == inject_module_name)
        {
            inject_module_base_address = it->second;
            break;
        }
    }
    return inject_module_base_address;
}


int* read_memory(DWORD pid, uintptr_t addr, unsigned int bytes)
{
    HANDLE handle = get_handle(pid);
    int* t = (int*)malloc(bytes);
    bool state = ReadProcessMemory(handle, (LPVOID)addr, t, bytes, NULL);
    if (!state)
    {
        free(t);
        t = NULL;
    }
    CloseHandle(handle);
    return t;
}

bool write_memory(DWORD pid, uintptr_t addr, char* data, int bytes)
{
    HANDLE handle = get_handle(pid);
    bool flag = WriteProcessMemory(handle, (LPVOID)addr, data, bytes, NULL);
    CloseHandle(handle);
    return flag;
}

InjectCodeManager::InjectCodeManager()
{
    this->pid = -1;
}

InjectCodeManager::InjectCodeManager(DWORD pid, string inject_module_name, uintptr_t inject_code_address, string inject_code_string, string new_code_string)
{
    this->is_used = FALSE;
    this->pid = pid;
    this->inject_module_name = inject_module_name;
    this->inject_code_address = inject_code_address;
    this->inject_code_string = inject_code_string;
    this->new_code_string = new_code_string;
}

InjectCodeManager::~InjectCodeManager()
{
    if (this->is_used)this->restore();
}


bool InjectCodeManager::inject()
{
    if (this->is_used)return false;

    this->inject_module_base_address = get_inject_module_base_address(this->pid, this->inject_module_name);
    uintptr_t true_inject_code_address = (uintptr_t)inject_module_base_address + this->inject_code_address;
    // inject_code_string: "E9 jmp_offset 90";
    int jmp_offset_cnt = 0;
    int jmp_offset_index = -1;
    while ((jmp_offset_index = this->inject_code_string.find("jmp_offset", jmp_offset_index + 1)) != -1)jmp_offset_cnt++;
    unsigned int inject_code_size = (this->inject_code_string.size() + jmp_offset_cnt) / 3 + 1;
    jmp_offset_cnt = 0;
    jmp_offset_index = -1;
    while ((jmp_offset_index = this->new_code_string.find("jmp_offset", jmp_offset_index + 1)) != -1)jmp_offset_cnt++;
    unsigned int new_code_size = (this->new_code_string.size() + jmp_offset_cnt) / 3 + 1;

    // 注入跳转到新内存的jmp后边会跟一个偏移，回来也会跟一个偏移
    uintptr_t jmp_offset = 0;
    string jmp_offset_string = "";

    // 申请新的内存
    this->new_memory_address = (uintptr_t)process_alloc(pid, new_code_size);
    //printf("new_memory_address is: %x\n", new_memory_address);

    // 向新内存区写入新代码
    // "89 56 24 A1 B4 5D A3 00 39 46 68 74 0E 0F 1F 40 00 A1 2C 0D A4 00 E9 jmp_offset C7 46 24 36 00 00 00 A1 2C 0D A4 00 E9 jmp_offset"
    // 在一些需要cmp的代码注入的例子中常常会有两个jmp，jmp回去到原来的代码，所以这里的处理要再细致一些
    string formate_new_code_string = new_code_string;
    jmp_offset_index = -1;
    while ((jmp_offset_index = formate_new_code_string.find("jmp_offset", jmp_offset_index+1)) != -1)
    {
        jmp_offset = (true_inject_code_address + inject_code_size) - (this->new_memory_address + jmp_offset_index/3 + 4);
        jmp_offset_string = offset2String(jmp_offset);
        formate_new_code_string.replace(jmp_offset_index, 10, jmp_offset_string.c_str(), jmp_offset_string.size());
    }
    
    uint8_t* new_code_data = HexCodeString2uint8t(formate_new_code_string);
    write_memory(pid, this->new_memory_address, (char*)new_code_data, new_code_size);
    free(new_code_data);

    // 保存旧代码
    uint8_t* old_code_data = (uint8_t*)read_memory(pid, true_inject_code_address, inject_code_size);
    this->old_code_string = Uint8_tData2HexString(old_code_data, inject_code_size);
    free(old_code_data);

    // 注入跳转到新内存的jmp
    jmp_offset = new_memory_address - (true_inject_code_address + 5);
    jmp_offset_string = offset2String(jmp_offset);

    string formate_inject_code_string = inject_code_string;
    formate_inject_code_string.replace(formate_inject_code_string.find("jmp_offset"), 10, jmp_offset_string.c_str(), jmp_offset_string.size());
    uint8_t* inject_code_data = HexCodeString2uint8t(formate_inject_code_string);
    write_memory(pid, true_inject_code_address, (char*)inject_code_data, inject_code_size);
    free(inject_code_data);

    this->is_used = TRUE;
    return TRUE;
}

bool InjectCodeManager::restore()
{
    uint8_t* old_code_data = HexCodeString2uint8t(this->old_code_string);
    uintptr_t true_inject_code_address = (uintptr_t)inject_module_base_address + this->inject_code_address;
    write_memory(pid, true_inject_code_address, (char*)old_code_data, this->old_code_string.size() / 3 + 1);
    process_free(pid, (LPVOID)this->new_memory_address, this->new_code_string.size() / 3 + 1);
    this->is_used = FALSE;
    return TRUE;
}