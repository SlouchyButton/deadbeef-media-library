#include "iconview.hpp"
#include "plugin.hpp"
#include "settings.hpp"

IconView::IconView() {
}
IconView::~IconView() {
}

void IconView::initialize() {
    //this->mIconView.set_text_column(this->mTreeFilebrowser->mModelColumns.mColumnName);
    
    this->set_pixbuf_column(this->mModelColumns.mColumnIcon);

    char* columnConfigs[6] = {ML_TITLE_COLUMN, ML_SUBTITLE_COLUMN, ML_CUSTOM1_COLUMN, ML_CUSTOM2_COLUMN, ML_CUSTOM3_COLUMN, ML_CUSTOM4_COLUMN};
    std::map<char*, Gtk::TreeModelColumnBase*> columnMap = {
        {(char*)ML_TITLE_COLUMN, &this->mModelColumns.mColumnTitle},
        {(char*)ML_SUBTITLE_COLUMN, &this->mModelColumns.mColumnSubtitle},
        {(char*)ML_CUSTOM1_COLUMN, &this->mModelColumns.mColumnCustom1},
        {(char*)ML_CUSTOM2_COLUMN, &this->mModelColumns.mColumnCustom2},
        {(char*)ML_CUSTOM3_COLUMN, &this->mModelColumns.mColumnCustom3},
        {(char*)ML_CUSTOM4_COLUMN, &this->mModelColumns.mColumnCustom4}
    };

    for (char* columnConfig : columnConfigs) {
        int column = deadbeef->conf_get_int(columnConfig, 0);
        if (column != 0) {
            this->mTextRenderers[columnConfig] = Gtk::CellRendererText();
            this->mTextRenderers[columnConfig].property_ellipsize() = Pango::EllipsizeMode::ELLIPSIZE_END;
            this->mTextRenderers[columnConfig].property_ellipsize_set() = true;
            this->mTextRenderers[columnConfig].property_alignment() = Pango::Alignment::ALIGN_CENTER;
            this->mTextRenderers[columnConfig].property_xalign() = 0.51;

            this->pack_start(this->mTextRenderers[columnConfig], true);

            this->add_attribute(this->mTextRenderers[columnConfig].property_text(), *columnMap[columnConfig]);
        }
    }

    this->mTextRenderers[ML_TITLE_COLUMN].property_weight() = Pango::WEIGHT_BOLD;

    this->set_has_tooltip(true);
    this->set_tooltip_column(1);
    this->set_item_width(deadbeef->conf_get_int(ML_ICON_SIZE, 128)+5);
}