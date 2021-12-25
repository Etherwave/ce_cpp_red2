#pragma once
#include "functions.h"


bool inject_money_decrease(DWORD pid, InjectCodeManager& inject_code_manager_money);
bool inject_zero_cooldown_time(DWORD pid, InjectCodeManager& inject_code_manager_zero_cooldown_time);
uintptr_t get_myself_team_id(DWORD pid);
vector<uintptr_t> get_selected_items_base_address(DWORD pid);
void unlimted_blood(DWORD pid);
void upgrade(DWORD pid);
void rebellion(DWORD pid);