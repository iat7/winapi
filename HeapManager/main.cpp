#include "CHeapManager.h"
#include <iostream>
#include <chrono>
#include <list>

using namespace std;
using namespace std::chrono;

int main() {
	int n = 1000;
	int initialSize = 64 * 1024;
	int maxSize = 64 * 1024 * n;
	high_resolution_clock::time_point startTime, endTime;

	CHeapManager customHeapManager;
	HANDLE libHeapManager;

	vector<void*> customHeapAllocations = vector<void*>();
	vector<LPVOID> libHeapAllocations = vector<LPVOID>();

	startTime = high_resolution_clock::now();
	customHeapManager.Create(initialSize, maxSize);
	for (int i = 0; i < n; ++i) {
		customHeapAllocations.push_back(customHeapManager.Alloc(rand() % (1024 * 1024)));
	}
	for (int i = n-1; i >= 0; --i) {
		customHeapManager.Free(customHeapAllocations[i]);
		customHeapAllocations.erase(customHeapAllocations.begin() + i);
	}
	customHeapManager.Destroy();
	endTime = high_resolution_clock::now();
	cout << "My heap: " << duration_cast<microseconds>(endTime - startTime).count() << " in ms" << endl;

	startTime = high_resolution_clock::now();
	libHeapManager = HeapCreate(0, initialSize, maxSize);
	for (int i = 0; i < n; ++i) {
		libHeapAllocations.push_back(HeapAlloc(libHeapManager, HEAP_NO_SERIALIZE, rand() % (1024 * 1024)));
	}
	for (int i = n - 1; i >= 0; --i) {
		HeapFree(libHeapManager, 0, libHeapAllocations[i]);
		libHeapAllocations.erase(libHeapAllocations.begin() + i);
	}
	HeapDestroy(libHeapManager);
	endTime = high_resolution_clock::now();
	cout << "Standart heap: " << duration_cast<microseconds>(endTime - startTime).count() << " in ms" << endl;

	system("pause");
	return 0;
}