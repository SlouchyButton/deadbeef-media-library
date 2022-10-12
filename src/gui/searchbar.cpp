#include "searchbar.hpp"

#include "utils.hpp"

#include <iostream>

Searchbar::Searchbar() {
    auto primaryIcon = this->property_primary_icon_pixbuf();
    primaryIcon = Utils::getIconByName("system-search", 16);
}

void Searchbar::setTreeModelFilter(FilebrowserFilter* filter) {
    this->mFilebrowserFilter = filter;

    this->signal_changed().connect(sigc::mem_fun(this, &Searchbar::on_change));
}

void Searchbar::on_change() {
    mTimer.disconnect();
    mTimer = Glib::signal_timeout().connect(sigc::mem_fun(*this, &Searchbar::on_timeout), timeoutValue);
}

bool Searchbar::on_timeout() {
    std::cout << "Setting needle: " << this->get_text() << std::endl;
    this->mFilebrowserFilter->setNeedle(this->get_text());
    return false;
}

Searchbar::~Searchbar() {
}