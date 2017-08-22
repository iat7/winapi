#include <windows.h>
#include <set>
#include <vector>

using namespace std;

class CHeapManager {
public:
	CHeapManager();
	void Create(int initialSize, int maxSize);
	void* Alloc(int size);
	void Free(void* mem);
	void Destroy();
private:
	int smallPageSize;
	int bigPageSize;
	vector <int> pageBlocksUsed;
	int maxSize;
	int initialSize;
	LPVOID start;
	set<pair<LPVOID, int>> * registeredBlocks;
	void registerBlock(LPVOID start, int size);
	pair<LPVOID, int> findFreeBlock(int size);
};