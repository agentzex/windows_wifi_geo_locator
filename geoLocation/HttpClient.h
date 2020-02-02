#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include "json.hpp"

using json = nlohmann::json;
using namespace std;


bool make_request(json &wifi_data, string server_address, int server_port, string api_key, wstring &coordinates);
