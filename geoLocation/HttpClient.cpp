#include <iostream>
#include "httplib.h"
#include "HttpClient.h"
#include "json.hpp"


using json = nlohmann::json;
using namespace std;


bool make_request(json &wifi_data, string server_address, int server_port, string api_key, wstring &coordinates) {
	httplib::SSLClient cli(server_address.c_str(), server_port, 600); //https

	string post = wifi_data.dump();
	string url = "/?key=" + api_key;

	auto res = cli.Post(url.c_str(), post, "application/json");
	if (res && res->status == 200) {
		auto response = json::parse(res->body);
		float lat = response["location"]["lat"];
		float lng = response["location"]["lng"];
		wstring s_lat = to_wstring(lat);
		wstring s_lng = to_wstring(lng);
		coordinates = s_lat + L',' + s_lng;
		return true;
	}
	return false;
}


