#include "ScriptAllocator.h"

namespace krit {

BlockAllocator<8> ScriptAllocator::a8(0);
BlockAllocator<16> ScriptAllocator::a16(1);
BlockAllocator<32> ScriptAllocator::a32(2);
BlockAllocator<64> ScriptAllocator::a64(3);
BlockAllocator<128> ScriptAllocator::a128(4);
BlockAllocator<256> ScriptAllocator::a256(5);
BlockAllocator<512> ScriptAllocator::a512(6);

}
