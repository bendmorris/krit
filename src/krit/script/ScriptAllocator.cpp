#include "ScriptAllocator.h"

namespace krit {

BlockAllocator<16> ScriptAllocator::a1(0);
BlockAllocator<64> ScriptAllocator::a2(1);
BlockAllocator<256> ScriptAllocator::a3(2);
BlockAllocator<1024> ScriptAllocator::a4(3);

}
