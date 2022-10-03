#include "container.hpp"

#include "settings.hpp"
#include "plugin.hpp"
#include "filebrowser.hpp"
#include "utils.hpp"

Container::Container() :
    mAddressbox(),
    mScrolledWindow()
{
    this->mTreeFilebrowser = TreeFilebrowser::create();
    this->mMediaLibrary = MediaLibrary();
    mTreeFilebrowser->setIconSize(deadbeef->conf_get_int(ML_ICON_SIZE, 32));

    this->mFilebrowserFilter = FilebrowserFilter::create(mTreeFilebrowser);
    this->mFilebrowserFilter->setModel(mTreeFilebrowser.get());
    this->mSearchbar.setTreeModelFilter(this->mFilebrowserFilter.get());
    this->mAddressbox.setTreeFilebrowser(this->mTreeFilebrowser.get());
    this->buildTreeview();

    this->mScrolledWindow.set_policy(Gtk::PolicyType::POLICY_AUTOMATIC, Gtk::PolicyType::POLICY_AUTOMATIC);
    this->mScrolledWindow.add(mIconView);

    this->mTextRenderer = Gtk::CellRendererText();

    this->pack_start(mSearchbar, false, true);
    this->pack_start(mAddressbox, false, true);
    this->pack_start(mScrolledWindow, true, true);

    this->initialize();
}

void Container::initialize() {
    // Create autofilter
    Filebrowser::VALID_EXTENSIONS = Utils::createValidExtensions();
    std::string defaultDir = Glib::get_user_special_dir(Glib::UserDirectory::USER_DIRECTORY_MUSIC);

    // Set addressbar
    deadbeef->conf_lock();
    auto address = deadbeef->conf_get_str_fast(ML_DEFAULT_PATH, defaultDir.c_str());
    deadbeef->conf_unlock();
    mAddressbox.setAddress(address);
}

void Container::buildTreeview() {
    this->mIconView.set_model(mFilebrowserFilter);
    this->mAddressbox.initialize(&mIconView, mFilebrowserFilter);
    this->mTreeFilebrowser->initialize(&this->mIconView, &this->mAddressbox, &this->mMediaLibrary);

    //this->mTextRenderer.property_wrap_width() = deadbeef->conf_get_int(ML_ICON_SIZE, 32);
    this->mTextRenderer.property_ellipsize() = Pango::EllipsizeMode::ELLIPSIZE_END;
    this->mTextRenderer.property_ellipsize_set() = true;
    this->mTextRenderer.property_alignment() = Pango::Alignment::ALIGN_CENTER;

    //this->mIconView.set_text_column(this->mTreeFilebrowser->mModelColumns.mColumnName);
    this->mIconView.set_pixbuf_column(this->mTreeFilebrowser->mModelColumns.mColumnIcon);
    this->mIconView.pack_end(mTextRenderer, true);
    this->mIconView.add_attribute(mTextRenderer.property_text(), this->mTreeFilebrowser->mModelColumns.mColumnName);

    this->mIconView.set_has_tooltip(true);
    this->mIconView.set_tooltip_column(3);
    this->mIconView.set_item_width(deadbeef->conf_get_int(ML_ICON_SIZE, 32)+5);

    this->mTreePopup.initialize(&this->mIconView, this->mTreeFilebrowser.get(), this->mFilebrowserFilter.get(), &this->mAddressbox);
}

Container::~Container() {
}