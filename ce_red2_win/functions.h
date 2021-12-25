#pragma once

//
// Created by Amazing on 2021/12/2.
//

#ifndef CE_INJECT_FUNCTIONS_H
#define CE_INJECT_FUNCTIONS_H

#include <iostream>
#include <Windows.h>
#include <Tlhelp32.h>
#include <stdio.h>
#include <time.h>
#include <tchar.h>
#include <psapi.h>
#include <vector>
#include <map>
#include <conio.h>
#include <process.h>


using namespace std;

class InjectCodeManager
{
public:
    DWORD pid = -1;
    bool is_used = FALSE;

    string old_code_string = "";
    string inject_code_string = "";
    string new_code_string = "";

    uintptr_t inject_code_address;
    uintptr_t new_memory_address;

    string inject_module_name = "";
    HMODULE inject_module_base_address = NULL;

    InjectCodeManager();
    InjectCodeManager(DWORD pid, string inject_module_name, uintptr_t inject_code_address, string inject_code_string, string new_code_string);
    ~InjectCodeManager();

    bool inject();
    bool restore();
};

wstring string2wstring(string str);
string wstring2string(wstring wstr);

HANDLE get_handle(int pid);
int FindPID(string ProcessName);
uintptr_t calc_offsets(DWORD pid, uintptr_t ptr, vector<uintptr_t> offsets);
LPVOID process_alloc(DWORD pid, unsigned int bytes);
bool get_all_module_base_address(DWORD pid, map<string, HMODULE>& module_base_address_map);
HMODULE get_inject_module_base_address(DWORD pid, string inject_module_name);
int* read_memory(DWORD pid, uintptr_t addr, unsigned int bytes);
bool write_memory(DWORD pid, uintptr_t addr, char* data, int bytes);
inline int Hexchar2int(char c);
inline char Hexint2char(int t);
uint8_t* HexCodeString2uint8t(string HexCodeString);
string Uint8_tData2HexString(uint8_t* data, unsigned int size);
string offset2String(uintptr_t offset);




#endif //CE_INJECT_FUNCTIONS_H
