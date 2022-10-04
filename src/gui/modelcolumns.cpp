#include "modelcolumns.hpp"

ModelColumns::ModelColumns() {
    this->add(this->mColumnIcon);
    this->add(this->mColumnName);
    this->add(this->mColumnAlbumPointer);
    this->add(this->mColumnTooltip);
    this->add(this->mColumnVisibility);
}