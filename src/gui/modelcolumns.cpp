#include "modelcolumns.hpp"

ModelColumns::ModelColumns() {
    this->add(this->mColumnIcon);
    this->add(this->mColumnTooltip);
    this->add(this->mColumnAlbumPointer);
    this->add(this->mColumnTitle);
    this->add(this->mColumnSubtitle);
    // This is absurdly stupid. There surely has to be a better way to do this.
    this->add(this->mColumnCustom1);
    this->add(this->mColumnCustom2);
    this->add(this->mColumnCustom3);
    this->add(this->mColumnCustom4);
    this->add(this->mColumnVisibility);
}