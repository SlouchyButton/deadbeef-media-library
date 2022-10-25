#include "iconview.hpp"
#include "plugin.hpp"
#include "settings.hpp"

IconView::IconView() {
}
IconView::~IconView() {
}

void IconView::initialize() {
    //this->mIconView.set_text_column(this->mTreeFilebrowser->mModelColumns.mColumnName);
    this->mTextRenderers["name"] = Gtk::CellRendererText();
    this->mTextRenderers["name"].property_ellipsize() = Pango::EllipsizeMode::ELLIPSIZE_END;
    this->mTextRenderers["name"].property_ellipsize_set() = true;
    this->mTextRenderers["name"].property_alignment() = Pango::Alignment::ALIGN_CENTER;
    this->mTextRenderers["name"].property_xalign() = 0.51;
    this->mTextRenderers["artist"] = Gtk::CellRendererText();
    this->mTextRenderers["artist"].property_ellipsize() = Pango::EllipsizeMode::ELLIPSIZE_END;
    this->mTextRenderers["artist"].property_ellipsize_set() = true;
    this->mTextRenderers["artist"].property_alignment() = Pango::Alignment::ALIGN_CENTER;
    this->mTextRenderers["artist"].property_xalign() = 0.51;

    this->set_pixbuf_column(this->mModelColumns.mColumnIcon);
    this->pack_start(this->mTextRenderers["name"], true);
    this->pack_end(this->mTextRenderers["artist"], true);
    this->add_attribute(this->mTextRenderers["name"].property_text(), this->mModelColumns.mColumnTitle);
    this->add_attribute(this->mTextRenderers["artist"].property_text(), this->mModelColumns.mColumnSubtitle);

    this->set_has_tooltip(true);
    this->set_tooltip_column(4);
    this->set_item_width(deadbeef->conf_get_int(ML_ICON_SIZE, 128)+5);
}