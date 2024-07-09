#pragma once
#include "buddy_allocator.h"

void* pseudo_malloc(BuddyAllocator* alloc,int size);

void pseudo_free(BuddyAllocator* alloc,void** block);