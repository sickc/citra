// Copyright 2019 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <QColorDialog>
#include "citra_qt/configuration/configure_enhancements.h"
#include "core/core.h"
#include "core/settings.h"
#include "ui_configure_enhancements.h"
#include "video_core/renderer_opengl/post_processing_opengl.h"
#include "video_core/renderer_opengl/texture_filters/texture_filterer.h"

ConfigureEnhancements::ConfigureEnhancements(QWidget* parent)
    : QWidget(parent), ui(new Ui::ConfigureEnhancements) {
    ui->setupUi(this);

    for (const auto& filter : OpenGL::TextureFilterer::GetFilterNames())
        ui->texture_filter_combobox->addItem(QString::fromStdString(filter.data()));

    SetConfiguration();

    ui->screen_refresh_rate->setEnabled(Settings::values.custom_refresh_rate);
    connect(ui->custom_refresh_rate, &QCheckBox::toggled, ui->screen_refresh_rate, &QSpinBox::setEnabled);
    connect(ui->custom_refresh_rate, &QCheckBox::toggled, ui->screen_refresh_rate, &QSpinBox::setEnabled);

    ui->layoutBox->setEnabled(!Settings::values.custom_layout);
    connect(ui->custom_layout, &QCheckBox::toggled, ui->custom_layout_group,
            &QWidget::setVisible);
    connect(ui->custom_layout, &QCheckBox::toggled, ui->layout_combobox,
            &QWidget::setDisabled);

    ui->resolution_factor_combobox->setEnabled(Settings::values.use_hw_renderer);

    connect(ui->render_3d_combobox,
            static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
            [this](int currentIndex) {
                updateShaders(static_cast<Settings::StereoRenderOption>(currentIndex));
            });

    connect(ui->bg_button, &QPushButton::clicked, this, [this] {
        const QColor new_bg_color = QColorDialog::getColor(bg_color);
        if (!new_bg_color.isValid()) {
            return;
        }
        bg_color = new_bg_color;
        QPixmap pixmap(ui->bg_button->size());
        pixmap.fill(bg_color);
        const QIcon color_icon(pixmap);
        ui->bg_button->setIcon(color_icon);
    });

    ui->toggle_preload_textures->setEnabled(ui->toggle_custom_textures->isChecked());
    connect(ui->toggle_custom_textures, &QCheckBox::toggled, this, [this] {
        ui->toggle_preload_textures->setEnabled(ui->toggle_custom_textures->isChecked());
        if (!ui->toggle_preload_textures->isEnabled())
            ui->toggle_preload_textures->setChecked(false);
    });

    ui->toggle_disk_shader_cache->setEnabled(Settings::values.use_hw_shader);
}

void ConfigureEnhancements::SetConfiguration() {
    ui->resolution_factor_combobox->setCurrentIndex(Settings::values.resolution_factor);
    ui->render_3d_combobox->setCurrentIndex(static_cast<int>(Settings::values.render_3d));
    ui->factor_3d->setValue(Settings::values.factor_3d);
    updateShaders(Settings::values.render_3d);
    ui->toggle_linear_filter->setChecked(Settings::values.filter_mode);
    int tex_filter_idx = ui->texture_filter_combobox->findText(
        QString::fromStdString(Settings::values.texture_filter_name));
    if (tex_filter_idx == -1) {
        ui->texture_filter_combobox->setCurrentIndex(0);
    } else {
        ui->texture_filter_combobox->setCurrentIndex(tex_filter_idx);
    }
    ui->layout_combobox->setCurrentIndex(static_cast<int>(Settings::values.layout_option));
    ui->custom_layout->setChecked(Settings::values.custom_layout);
    ui->layout_combobox->setEnabled(!Settings::values.custom_layout);
    ui->custom_layout_group->setVisible(Settings::values.custom_layout);
    ui->custom_top_left->setValue(Settings::values.custom_top_left);
    ui->custom_top_top->setValue(Settings::values.custom_top_top);
    ui->custom_top_right->setValue(Settings::values.custom_top_right);
    ui->custom_top_bottom->setValue(Settings::values.custom_top_bottom);
    ui->custom_bottom_left->setValue(Settings::values.custom_bottom_left);
    ui->custom_bottom_top->setValue(Settings::values.custom_bottom_top);
    ui->custom_bottom_right->setValue(Settings::values.custom_bottom_right);
    ui->custom_bottom_bottom->setValue(Settings::values.custom_bottom_bottom);
    ui->swap_screen->setChecked(Settings::values.swap_screen);
    ui->toggle_disk_shader_cache->setChecked(Settings::values.use_hw_shader &&
                                             Settings::values.use_disk_shader_cache);
    ui->custom_refresh_rate->setChecked(Settings::values.custom_refresh_rate);
    ui->screen_refresh_rate->setValue(Settings::values.screen_refresh_rate);
    ui->upright_screen->setChecked(Settings::values.upright_screen);
    ui->toggle_dump_textures->setChecked(Settings::values.dump_textures);
    ui->toggle_custom_textures->setChecked(Settings::values.custom_textures);
    ui->toggle_preload_textures->setChecked(Settings::values.preload_textures);
    bg_color = QColor::fromRgbF(Settings::values.bg_red, Settings::values.bg_green,
                                Settings::values.bg_blue);
    QPixmap pixmap(ui->bg_button->size());
    pixmap.fill(bg_color);
    const QIcon color_icon(pixmap);
    ui->bg_button->setIcon(color_icon);
}

void ConfigureEnhancements::updateShaders(Settings::StereoRenderOption stereo_option) {
    ui->shader_combobox->clear();

    if (stereo_option == Settings::StereoRenderOption::Anaglyph)
        ui->shader_combobox->addItem(QStringLiteral("dubois (builtin)"));
    else if (stereo_option == Settings::StereoRenderOption::Interlaced)
        ui->shader_combobox->addItem(QStringLiteral("horizontal (builtin)"));
    else
        ui->shader_combobox->addItem(QStringLiteral("none (builtin)"));

    ui->shader_combobox->setCurrentIndex(0);

    for (const auto& shader : OpenGL::GetPostProcessingShaderList(
             stereo_option == Settings::StereoRenderOption::Anaglyph)) {
        ui->shader_combobox->addItem(QString::fromStdString(shader));
        if (Settings::values.pp_shader_name == shader)
            ui->shader_combobox->setCurrentIndex(ui->shader_combobox->count() - 1);
    }
}

void ConfigureEnhancements::RetranslateUI() {
    ui->retranslateUi(this);
}

void ConfigureEnhancements::ApplyConfiguration() {
    Settings::values.resolution_factor =
        static_cast<u16>(ui->resolution_factor_combobox->currentIndex());
    Settings::values.render_3d =
        static_cast<Settings::StereoRenderOption>(ui->render_3d_combobox->currentIndex());
    Settings::values.factor_3d = ui->factor_3d->value();
    Settings::values.pp_shader_name =
        ui->shader_combobox->itemText(ui->shader_combobox->currentIndex()).toStdString();
    Settings::values.filter_mode = ui->toggle_linear_filter->isChecked();
    Settings::values.texture_filter_name = ui->texture_filter_combobox->currentText().toStdString();
    Settings::values.layout_option =
        static_cast<Settings::LayoutOption>(ui->layout_combobox->currentIndex());
    Settings::values.custom_layout = ui->custom_layout->isChecked();
    Settings::values.custom_top_left = ui->custom_top_left->value();
    Settings::values.custom_top_top = ui->custom_top_top->value();
    Settings::values.custom_top_right = ui->custom_top_right->value();
    Settings::values.custom_top_bottom = ui->custom_top_bottom->value();
    Settings::values.custom_bottom_left = ui->custom_bottom_left->value();
    Settings::values.custom_bottom_top = ui->custom_bottom_top->value();
    Settings::values.custom_bottom_right = ui->custom_bottom_right->value();
    Settings::values.custom_bottom_bottom = ui->custom_bottom_bottom->value();
    Settings::values.swap_screen = ui->swap_screen->isChecked();
    Settings::values.use_disk_shader_cache =
        Settings::values.use_hw_shader && ui->toggle_disk_shader_cache->isChecked();
    Settings::values.custom_refresh_rate = ui->custom_refresh_rate->isChecked();
    Settings::values.screen_refresh_rate = ui->screen_refresh_rate->value();
    Settings::values.upright_screen = ui->upright_screen->isChecked();
    Settings::values.dump_textures = ui->toggle_dump_textures->isChecked();
    Settings::values.custom_textures = ui->toggle_custom_textures->isChecked();
    Settings::values.preload_textures = ui->toggle_preload_textures->isChecked();
    Settings::values.bg_red = static_cast<float>(bg_color.redF());
    Settings::values.bg_green = static_cast<float>(bg_color.greenF());
    Settings::values.bg_blue = static_cast<float>(bg_color.blueF());
}

ConfigureEnhancements::~ConfigureEnhancements() {
    delete ui;
}
