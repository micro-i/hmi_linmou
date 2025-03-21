
#include "extension.h"


Extension::Extension() {
	this->rawGrabbingAllowed = true;
	this->processedGrabbingAllowed = true;
	this->extensionWidget = nullptr;
	this->displayStyle = SIDEBAR_TAB;
}

Extension::~Extension() {
}
