#include "Zone.h"
#include <algorithm>

Zone::~Zone() {
	deleteAll();
}

void Zone::addObject(ZoneObject *obj) {
	objects_.push_back(obj);
}

void Zone::deleteAll() {
	for (ZoneObject *obj : objects_) {
		delete obj;
	}
	objects_.clear();
}


Zone zoneInstance;

// static
void *ZoneObject::operator new(std::size_t size) {
	void *obj = malloc(size);
	zoneInstance.addObject(static_cast<ZoneObject *>(obj));
	return obj;
}

// static
void ZoneObject::operator delete(void *block) noexcept {
	free(block);
}
