#include "CHeapManager.h"

using namespace std;

CHeapManager::CHeapManager()
{
	SYSTEM_INFO info;
	GetSystemInfo(&info);
	smallPageSize = 4 * 1024;
	bigPageSize = 128 * 1024; // Static const int + initialization
	pageBlocksUsed = vector<int>();
	registeredBlocks = new set< pair<LPVOID, int> >[3];
	for (int i = 0; i < 3; i++) {
		registeredBlocks[i] = set< pair<LPVOID, int> >();
	}
}

void CHeapManager::Create(int initialSize, int maxSize)
{
	initialSize = (initialSize / smallPageSize) * smallPageSize + smallPageSize;
	maxSize = (maxSize / smallPageSize) * smallPageSize + smallPageSize;
	start = ::VirtualAlloc(NULL, maxSize, MEM_RESERVE, PAGE_READWRITE);
	LPVOID commitedSpace = ::VirtualAlloc(start, initialSize, MEM_COMMIT, PAGE_READWRITE);

	pageBlocksUsed.resize(maxSize / smallPageSize, 0);
	for (int i = 0; i <= initialSize / smallPageSize; ++i) {
		pageBlocksUsed[i]++;
	}
	registerBlock(start, maxSize);
}

void* CHeapManager::Alloc(int size) {
	size = (size / sizeof(int)) * sizeof(int) + sizeof(int);
	pair<LPVOID, int> freeBlock = findFreeBlock(size);
	LPVOID blockPointer = freeBlock.first;
	int blockSize = freeBlock.second;
	int offset = static_cast<byte*>(blockPointer) - static_cast<byte*>(start);
	int startPage = offset / smallPageSize;
	int endPage = (offset + sizeof(int) + size - 1) / smallPageSize;
	LPVOID allocated = ::VirtualAlloc(blockPointer, size + sizeof(int), MEM_COMMIT, PAGE_READWRITE);
	memcpy(blockPointer, &size, sizeof(int));
	for (int i = startPage; i <= endPage; i++) pageBlocksUsed[i] += 1;
	if (size + sizeof(int) < blockSize) {
		registerBlock(static_cast<byte*>(blockPointer) + sizeof(int) + size, blockSize - size - sizeof(int));
	}
	return static_cast<byte*>(blockPointer) + sizeof(int);
}

// Comments, function from bound search, consts for 3  and other
void CHeapManager::Free(void *mem) {
	mem = static_cast<byte*>(mem) - sizeof(int);
	int blockSize;
	memcpy(&blockSize, mem, sizeof(int));
	int offset = static_cast<byte*>(mem) - static_cast<byte*>(start);
	int startPage = offset / smallPageSize;
	int endPage = (offset + sizeof(int) + blockSize - 1) / smallPageSize;
	vector<int> freePages;
	for (int i = startPage; i <= endPage; i++) {
		pageBlocksUsed[i]--;
		if (pageBlocksUsed[i] == 0) {
			freePages.push_back(i);
		}
	}
	pair<LPVOID, int> leftBound = make_pair(mem, 0);
	pair<LPVOID, int> rightBound;
	if (mem != start) {
		pair<LPVOID, int> leftBound = make_pair(mem, 0);
		for (int i = 0; i < 3; i++) {
			if (!registeredBlocks[i].empty()) {
				set <pair<LPVOID, int>>::iterator it = registeredBlocks[i].upper_bound(leftBound);
				if (it == registeredBlocks[i].begin()) {
					break;
				}
				it--;
				LPVOID freeBlockPtr = it->first;
				int registeredBlocksize = it->second;
				if (static_cast<byte*>(freeBlockPtr) + registeredBlocksize + sizeof(int) == static_cast<byte*>(mem)) {
					leftBound.first = freeBlockPtr;
					leftBound.second = registeredBlocksize;
					registeredBlocks[i].erase(it);
					break;
				}
			}
		}
	}
	if (static_cast<byte*>(mem)-static_cast<byte*>(start)+blockSize < maxSize) {
		pair<LPVOID, int> rightBound = make_pair(static_cast<LPVOID>(0), 0);
		for (int i = 0; i < 3; i++) {
			if (!registeredBlocks[i].empty()) {
				set <pair<LPVOID, int>>::iterator it = registeredBlocks[i].upper_bound(rightBound);
				if (it != registeredBlocks[i].end()) {
					LPVOID freeBlockPtr = it->first;
					int registeredBlocksize = it->second;
					if (static_cast<byte*>(mem)+blockSize + sizeof(int) == static_cast<byte*>(freeBlockPtr)) {
						rightBound.first = freeBlockPtr;
						rightBound.second = registeredBlocksize;
						registeredBlocks[i].erase(it);
						break;
					}
				}
			}
		}
	}
	registerBlock(leftBound.first, leftBound.second + blockSize + rightBound.second);
	if (!freePages.empty()) {
		startPage = freePages[0];
		endPage = freePages[freePages.size() - 1];
		VirtualFree(static_cast<byte*>(start) + startPage * smallPageSize, (endPage - startPage + 1) * smallPageSize, MEM_DECOMMIT);
	}
}

void CHeapManager::Destroy() {
	VirtualFree(start, maxSize, MEM_RELEASE);
	start = 0;
	pageBlocksUsed = vector <int>();
	for (int i = 0; i < 3; i++) {
		registeredBlocks[i] = set<pair<LPVOID, int>>();
	}
}

void CHeapManager::registerBlock(LPVOID start, int size) {
	if (size < smallPageSize) { ::memcpy(start, &size, sizeof(int)); registeredBlocks[0].insert(make_pair(start, size + sizeof(int))); }
	else if (size < bigPageSize) registeredBlocks[1].insert(make_pair(start, size + sizeof(int)));
	else registeredBlocks[2].insert(make_pair(start, size + sizeof(int)));
}

pair<LPVOID, int> CHeapManager::findFreeBlock(int size) {
	pair<LPVOID, int> result = make_pair(nullptr, -1);
	int startIndex = 3;
	if (size + sizeof(int) < smallPageSize) startIndex = 0;
	if (smallPageSize <= size + sizeof(int) && size + sizeof(int) <	bigPageSize) startIndex = 1;
	if (bigPageSize <= size + sizeof(int)) startIndex = 2;
	for (int i = startIndex; i < 3; i++) {
		if (registeredBlocks[i].size() > 0) {
			for (auto it = registeredBlocks[i].begin(); it != registeredBlocks[i].end(); it++) {
				int freeMemory = it->second;
				if (freeMemory >= size + sizeof(int)) {
					result = *it;
					registeredBlocks[i].erase(it);
					return result;
				}
			}
		}
	}
	return result;
}