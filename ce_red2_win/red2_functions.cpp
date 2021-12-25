#include "red2_functions.h"


bool inject_money_decrease(DWORD pid, InjectCodeManager& inject_code_manager_money)
{
    if (inject_code_manager_money.pid == -1 || inject_code_manager_money.is_used==FALSE)
    {
        string inject_module_name = "game.exe";
        uintptr_t inject_code_address = 0xE53AF;
        string inject_code_string = "E9 jmp_offset 90";
        string new_code_string = "89 83 4C 02 00 00 C7 83 4C 02 00 00 40 42 0F 00 E9 jmp_offset";
        inject_code_manager_money = InjectCodeManager(pid, inject_module_name, inject_code_address, inject_code_string, new_code_string);
        inject_code_manager_money.inject();
    }
    else
    {
        inject_code_manager_money.restore();
    }
    return TRUE;
}

bool inject_zero_cooldown_time(DWORD pid, InjectCodeManager& inject_code_manager_zero_cooldown_time)
{
    if (inject_code_manager_zero_cooldown_time.pid == -1 || inject_code_manager_zero_cooldown_time.is_used==FALSE)
    {
        string inject_module_name = "game.exe";
        uintptr_t inject_code_address = 0xB9367;
        string inject_code_string = "E9 jmp_offset 0F 1F 00";
        string new_code_string = "89 56 24 A1 B4 5D A3 00 39 46 68 74 0E 0F 1F 40 00 A1 2C 0D A4 00 E9 jmp_offset C7 46 24 36 00 00 00 A1 2C 0D A4 00 E9 jmp_offset";
        inject_code_manager_zero_cooldown_time = InjectCodeManager(pid, inject_module_name, inject_code_address, inject_code_string, new_code_string);
        inject_code_manager_zero_cooldown_time.inject();
    }
    else
    {
        inject_code_manager_zero_cooldown_time.restore();
    }
    return TRUE;
}

unsigned int get_selected_item_number(DWORD pid)
{
    unsigned int selected_item_number = 0;
    string inject_module_name = "game.exe";
    HMODULE inject_module_base_address = get_inject_module_base_address(pid, inject_module_name);
    uintptr_t selected_item_number_address = (uintptr_t)inject_module_base_address + 0x640C70;
    uintptr_t* selected_item_number_data = (uintptr_t*)read_memory(pid, selected_item_number_address, 4);
    if (selected_item_number_data != NULL)selected_item_number = *selected_item_number_data;
    free(selected_item_number_data);
    return selected_item_number;
}

uintptr_t get_myself_team_id(DWORD pid)
{
    uintptr_t myid = -1;
    //获取自己的队伍id指针
    //game.exe+635DB4
    string inject_module_name = "game.exe";
    HMODULE inject_module_base_address = get_inject_module_base_address(pid, inject_module_name);
    uintptr_t myidaddr = (uintptr_t)inject_module_base_address + 0x635DB4;
    uintptr_t* my_id_data = (uintptr_t*)read_memory(pid, myidaddr, 4);
    if (my_id_data == NULL)printf("read myself_team_id failed!\n");
    else myid = *my_id_data;
    free(my_id_data);
    return myid;
}

vector<uintptr_t> get_selected_items_base_address(DWORD pid)
{
    vector<uintptr_t> selected_items_base_address;
    // 指针指向的第一个对象的地址"game.exe"+00640C64 +0x0
    string inject_module_name = "game.exe";
    HMODULE inject_module_base_address = get_inject_module_base_address(pid, inject_module_name);
    const int max_select_items = 500;
    unsigned int selected_item_number = get_selected_item_number(pid);
    for (int i = 0; i != selected_item_number; i++)
    {
        vector<uintptr_t> offsets;
        offsets.push_back(0x00640C64);
        offsets.push_back(4 * i);
        uintptr_t pointer_to_item_base_address = calc_offsets(pid, (uintptr_t)inject_module_base_address, offsets);
        if (pointer_to_item_base_address == -1)break;
        uintptr_t* pointer_data = (uintptr_t*)read_memory(pid, pointer_to_item_base_address, 4);
        if (pointer_data == NULL)break;
        uintptr_t item_base_address = *pointer_data;
        free(pointer_data);
        selected_items_base_address.push_back(item_base_address);
    }
    return selected_items_base_address;
}

void unlimted_blood(DWORD pid)
{
    uintptr_t myid = get_myself_team_id(pid);
    if (myid == -1)return;
    vector<uintptr_t> selected_items_base_address = get_selected_items_base_address(pid);
    for (int i = 0; i != selected_items_base_address.size(); i++)
    {
        //判断当前选择的对象的队伍id是不是自己的队伍id 
        uintptr_t item_id_address = selected_items_base_address[i] + 0x1B4;
        uintptr_t* item_id_data = (uintptr_t*)read_memory(pid, item_id_address, 4);
        uintptr_t item_id = *item_id_data;
        free(item_id_data);
        // 如果是自己人，那么就增加血量
        if (myid == item_id)
        {
            int blood = 10000;
            uintptr_t blood_address = selected_items_base_address[i] + 0x6C;
            write_memory(pid, blood_address, (char*)&blood, 4);
        }
        else
        {
            // 如果不是自己人，那么可以减少他的血量，但是没有必要 
            int blood = 1;
            /* uintptr_t blood_address = selected_items_base_address[i] + 0x6C;
             write_memory(pid, blood_address, (char*)&blood, 4);*/
        }
    }
}

void upgrade(DWORD pid)
{
    uintptr_t myid = get_myself_team_id(pid);
    if (myid == -1)return;
    vector<uintptr_t> selected_items_base_address = get_selected_items_base_address(pid);

    for (int i = 0; i != selected_items_base_address.size(); i++)
    {
        //判断当前选择的对象的队伍id是不是自己的队伍id 
        uintptr_t item_id_address = selected_items_base_address[i] + 0x1B4;
        uintptr_t* item_id_data = (uintptr_t*)read_memory(pid, item_id_address, 4);
        uintptr_t item_id = *item_id_data;
        free(item_id_data);
        // 如果是自己人，那么就升级
        if (myid == item_id)
        {
            double grade = 2;
            uintptr_t blood_address = selected_items_base_address[i] + 0x118;
            write_memory(pid, blood_address, (char*)&grade, sizeof(double));
        }
        else
        {
            // 如果不是自己人，那么可以减低他的级别，但是没有必要 
            double grade = 0;
            /*uintptr_t blood_address = selected_items_base_address[i] + 0x118;
            write_memory(pid, blood_address, (char*)&grade, sizeof(double));*/
        }
    }
}

void rebellion(DWORD pid)
{
    uintptr_t myid = get_myself_team_id(pid);
    if (myid == -1)return;
    vector<uintptr_t> selected_items_base_address = get_selected_items_base_address(pid);
    //修改当前选中的对象的队伍id指针为自己的id指针
    for (int i = 0; i != selected_items_base_address.size(); i++)
    {
        //判断当前选择的对象的队伍id是不是自己的队伍id 
        uintptr_t item_id_address = selected_items_base_address[i] + 0x1B4;
        uintptr_t* item_id_data = (uintptr_t*)read_memory(pid, item_id_address, 4);
        uintptr_t item_id = *item_id_data;
        free(item_id_data);
        if (item_id != myid)
        {
            write_memory(pid, item_id_address, (char*)&myid, 4);
        }
    }
}