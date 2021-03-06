#define WIN32_LEAN_AND_MEAN

#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <fstream>
#include <Windows.h>
#include <wincrypt.h>
#include <AtlBase.h>
#include <atlconv.h>
#include <thread>
#include <string>
#include <map>
#include <tlhelp32.h>
#include <wtsapi32.h>
#include <Wlanapi.h>
#include "json.hpp"
#include "HttpClient.h"


using namespace std;
using json = nlohmann::json;



bool get_networks(json &data){

	HANDLE hClient = NULL;
	DWORD dwMaxClient = 2;   
	DWORD dwCurVersion = 0;
	DWORD dwResult = 0;

	unsigned int i, j;
	/* variables used for WlanEnumInterfaces  */
	PWLAN_INTERFACE_INFO_LIST pIfList = NULL;
	PWLAN_INTERFACE_INFO pIfInfo = NULL;
	PWLAN_BSS_LIST pBssListMac = NULL;
	PWLAN_BSS_ENTRY  pBssEntryMac = NULL;
	int iRSSI = 0;

	dwResult = WlanOpenHandle(dwMaxClient, NULL, &dwCurVersion, &hClient);
	if (dwResult != ERROR_SUCCESS) {
		//wprintf(L"WlanOpenHandle failed with error: %u\n", dwResult);
		return false;
		// You can use FormatMessage here to find out why the function failed
	}

	dwResult = WlanEnumInterfaces(hClient, NULL, &pIfList);
	if (dwResult != ERROR_SUCCESS) {
		if (pIfList != NULL) {
			WlanFreeMemory(pIfList);
			pIfList = NULL;
		}
		//wprintf(L"WlanEnumInterfaces failed with error: %u\n", dwResult);
		return false;
		// You can use FormatMessage here to find out why the function failed
	}

	for (i = 0; i < (int)pIfList->dwNumberOfItems; i++) {
		pIfInfo = (WLAN_INTERFACE_INFO *)&pIfList->InterfaceInfo[i];

		dwResult = WlanGetNetworkBssList(hClient, &pIfInfo->InterfaceGuid, NULL, dot11_BSS_type_any, NULL, NULL, &pBssListMac);
		if (dwResult != ERROR_SUCCESS){
			return false;
		}
		for (int j = 0; j < pBssListMac->dwNumberOfItems; j++) {
			json new_entry;
			// Get the RSSI value
			pBssEntryMac = &pBssListMac->wlanBssEntries[j];
			char macStr[18];
			snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
				pBssEntryMac->dot11Bssid[0], pBssEntryMac->dot11Bssid[1], pBssEntryMac->dot11Bssid[2], pBssEntryMac->dot11Bssid[3], pBssEntryMac->dot11Bssid[4], pBssEntryMac->dot11Bssid[5]);
			string mac_address = macStr;

			new_entry["mac_address"] = mac_address;
			new_entry["signalStrength"] = pBssEntryMac->lRssi;
			data.push_back(new_entry);
		}
	}

	if (pBssListMac != NULL) {
		WlanFreeMemory(pBssListMac);
		pBssListMac = NULL;
	}

	if (pIfList != NULL) {
		WlanFreeMemory(pIfList);
		pIfList = NULL;
	}

	if (data.empty()) { //checking that at least 1 bssid was found
		return false;
	}

	return true;
}



int WINAPI WinMain(HINSTANCE inst, HINSTANCE prev, LPSTR cmd, int show) {

	json data;
	if (!get_networks(data)) {
		MessageBoxW(NULL, L"No WIFI networks were found! check if your WNIC is working properly", L"Error", MB_ICONEXCLAMATION | MB_OK);
		return -1;
	}


	//exmaple coordindates from combain API

	//json net1;
	//net1["macAddress"] = "00:26:3e:0c:71:45";
	//net1["signalStrength"] = -61;

	//json net2;
	//net2["macAddress"] = "00:26:3e:0c:71:44";
	//net2["signalStrength"] = -62;
	//data.push_back(net1);
	//data.push_back(net2);


	// you can now send an API request to geo-location API services like Google, unwiredlabs, mylnikov, combain and others.
	// here's an example with combain API:	
	json wifi_data;
	wifi_data["wifiAccessPoints"] = data;

	string address = "cps.combain.com";
	int port = 443;
	string api_key = "0000000000";
	wstring coordinates = L"";

	if (!make_request(wifi_data, address, port, api_key, coordinates)) {
		MessageBoxW(NULL, L"API request to Geolocation service failed or location wasn't found!", L"Error", MB_ICONEXCLAMATION | MB_OK);
		return -1;
	}

	// use google maps with coordinates - 	//https://www.google.com/maps/search/?api=1&query=<lat>,<lng>
	wstring google_maps_url = L"https://www.google.com/maps/search/?api=1&query=" + coordinates;
	
	//open google maps with defualt web browser configured to show the found coordinates
	STARTUPINFO si = { 0 };
	PROCESS_INFORMATION pi = { 0 };
	wstring cmdline = L"explorer \"" + google_maps_url + L"\"";
	CreateProcess(NULL, &cmdline[0], NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi);
	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);

	return 0;

}
