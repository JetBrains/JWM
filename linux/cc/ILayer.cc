#include "ILayer.hh"

jwm::ILayer* jwm::ILayer::_ourCurrentLayer = nullptr;


void jwm::ILayer::makeCurrent() {
    if (_ourCurrentLayer != this) {
        makeCurrentForced();
    }
}
void jwm::ILayer::makeCurrentForced() {
    _ourCurrentLayer = this;
}
