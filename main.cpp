#include <Windows.h>
#include <iostream>

const int game_address = 0x00400000, base_offset = 0x0017E0A8;
const int base_address = game_address + base_offset;

/* leaves artifacts
void clear_screen(){
	HANDLE hOut;
	COORD Position;

	hOut = GetStdHandle(STD_OUTPUT_HANDLE);

	Position.X = 0;
	Position.Y = 0;
	SetConsoleCursorPosition(hOut, Position);
}
*/
struct Player{
	int hp;
	float pos_x;
	float pos_y;
	float pos_z;

	int ar_ammo_clip;
	int ar_ammo_reserve;

	std::string name;
};
template <typename T>
T get_value(HANDLE hProcess, int offset) {
	T value;
	DWORD address = base_address;

	ReadProcessMemory(hProcess, (LPCVOID)address, &address, sizeof(T), NULL);
	address += offset;
	ReadProcessMemory(hProcess, (LPCVOID)address, &value, sizeof(T), NULL);
	
	return value;
}

template <typename T>
T set_value(HANDLE hProcess, int offset, T value) {
	DWORD address = base_address;

	ReadProcessMemory(hProcess, (LPCVOID)address, &address, sizeof(T), NULL);
	address += offset;
	int err = WriteProcessMemory(hProcess, (LPVOID)address, &value, sizeof(T), NULL);
	if (err == 0) {
		std::cout << "Writing to process memory failed... " << GetLastError() << std::endl;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
int main() {
	Player player;

	HWND game_window_handle = FindWindow(NULL, L"AssaultCube");
	DWORD base_address = game_address + base_offset;

	DWORD process_id;
	GetWindowThreadProcessId(game_window_handle, &process_id);

	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, false, process_id);
	if (hProcess == NULL) {
		std::cout << "Retrieving process handle failed... " << std::dec << GetLastError() << std::endl;
		return EXIT_FAILURE;
	}
	int err;
	while(true) {
		//clear_screen();
		std::cout << "------------------------------------------" << std::endl;
		player.ar_ammo_clip = get_value<int>(hProcess, 0x140);
		player.pos_x = get_value<float>(hProcess, 0x28);
		player.pos_y = get_value<float>(hProcess, 0x2C);

		std::cout << "AR Clip:\t"  << std::dec << player.ar_ammo_clip << std::endl;
		std::cout << "Pos X:\t" << player.pos_x << std::endl;
		std::cout << "Pos Y:\t" << player.pos_y << std::endl;

		if (player.ar_ammo_clip <= 20) {
			std::cout << "Ammo below 20, refilling " << std::endl;
			err = set_value<int>(hProcess, 0x140, 50);
			if (err == 1) {
				std::cout << "Failed to refill ammo... " << GetLastError() << std::endl;
			}
			else {
				std::cout << "Successfully refilled ammo..." << std::endl;
			}

		}
		Sleep(500);
	}

	

	BOOL closeHandleReturn = CloseHandle(hProcess);
	if (closeHandleReturn == FALSE)
	{
		std::cout << "CloseHandle failed. GetLastError = " << std::dec << GetLastError() << std::endl;
		system("pause > nul");
		return EXIT_FAILURE;
	}
	std::cout << "CloseHandle succeeded. (No risk of resource leak)" << std::endl;

	return EXIT_SUCCESS;
}