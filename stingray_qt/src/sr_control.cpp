#include "sr_control.h"
#include "./ui_sr_control.h"

SR::SR(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::SR)
{
    ui->setupUi(this);

//    ui->statusbar->hide();

    // connect the focus slider and focus spinner
    connect(ui->focus_slider, SIGNAL(valueChanged(int)), this, SLOT(on_focus_slider_change()));
    connect(ui->focus_spinner, SIGNAL(valueChanged(int)), this, SLOT(on_focus_spinner_change()));

    connect(ui->zoom_slider, SIGNAL(valueChanged(int)), this, SLOT(on_zoom_slider_change()));
    connect(ui->zoom_spinner, SIGNAL(valueChanged(int)), this, SLOT(on_zoom_spinner_change()));

    connect(ui->iris_slider, SIGNAL(valueChanged(int)), this, SLOT(on_iris_slider_change()));
    connect(ui->iris_spinner, SIGNAL(valueChanged(int)), this, SLOT(on_iris_spinner_change()));

    connect(ui->connect_btn, SIGNAL(clicked()), this, SLOT(button_click()));


}

SR::~SR()
{
    delete ui;
}


// ----------------------------------------------------------------------------
void SR::button_click()
{
    uint32_t wait_time = 5;
    uint32_t baud_rate = 115200;

    if(ui->comm_port->text().isEmpty())
    {
        return;
    }

    if(connected)
    {
        // disconnect from the lens
        sr.close();

        ui->connect_btn->setText("Connect");
        connected = false;
    }
    else
    {
        // connect to the lens
        sr = stingray_lens(ui->comm_port->text().toStdString(), baud_rate, wait_time);
        sr.connect();

        ui->focus_spinner->blockSignals(true);
        ui->focus_slider->blockSignals(true);
        ui->zoom_spinner->blockSignals(true);
        ui->zoom_slider->blockSignals(true);
        ui->iris_spinner->blockSignals(true);
        ui->iris_slider->blockSignals(true);

        ui->focus_slider->setValue(sr.get_focus());
        ui->focus_spinner->setValue(sr.get_focus());
        ui->zoom_slider->setValue(sr.get_zoom());
        ui->zoom_spinner->setValue(sr.get_zoom());
        ui->iris_slider->setValue(sr.get_iris());
        ui->iris_spinner->setValue(sr.get_iris());

        ui->focus_spinner->blockSignals(false);
        ui->focus_slider->blockSignals(false);
        ui->zoom_spinner->blockSignals(false);
        ui->zoom_slider->blockSignals(false);
        ui->iris_spinner->blockSignals(false);
        ui->iris_slider->blockSignals(false);

        ui->connect_btn->setText("Disconnect");
        connected = true;
    }

}   // end of button_click

// ----------------------------------------------------------------------------
void SR::on_focus_slider_change()
{
    // disable duplicate event triggers
    ui->focus_spinner->blockSignals(true);

    // update the spin box
    ui->focus_spinner->setValue(ui->focus_slider->value());

    // remove the block
    ui->focus_spinner-> blockSignals(false);

    // send focus value to the lens
    if(connected)
    {
        sr.set_focus((uint16_t)ui->focus_slider->value());
    }

}   // end of on_focus_slider_change

// ----------------------------------------------------------------------------
void SR::on_focus_spinner_change()
{
    // disable duplicate event triggers
    ui->focus_slider->blockSignals(true);

    // update the slider box
    ui->focus_slider->setValue(ui->focus_spinner->value());

    // remove the block
    ui->focus_slider-> blockSignals(false);

    // send focus value to the lens
    if(connected)
    {
        sr.set_focus((uint16_t)ui->focus_spinner->value());
    }

}   // end of on_focus_spinner_change

// ----------------------------------------------------------------------------
void SR::on_zoom_slider_change()
{
    // disable duplicate event triggers
    ui->zoom_spinner->blockSignals(true);

    // update the spin box
    ui->zoom_spinner->setValue(ui->zoom_slider->value());

    // remove the block
    ui->zoom_spinner-> blockSignals(false);

    // send focus value to the lens
    if(connected)
    {
        sr.set_zoom((uint16_t)ui->zoom_slider->value());
    }

}   // end of on_zoom_slider_change

// ----------------------------------------------------------------------------
void SR::on_zoom_spinner_change()
{
    // disable duplicate event triggers
    ui->zoom_slider->blockSignals(true);

    // update the slider box
    ui->zoom_slider->setValue(ui->zoom_spinner->value());

    // remove the block
    ui->zoom_slider-> blockSignals(false);

    // send focus value to the lens
    if(connected)
    {
        sr.set_zoom((uint16_t)ui->zoom_spinner->value());
    }

}   // end of on_zoom_spinner_change

// ----------------------------------------------------------------------------
void SR::on_iris_slider_change()
{
    // disable duplicate event triggers
    ui->iris_spinner->blockSignals(true);

    // update the spin box
    ui->iris_spinner->setValue(ui->iris_slider->value());

    // remove the block
    ui->iris_spinner-> blockSignals(false);

    // send focus value to the lens
    if(connected)
    {
        sr.set_iris((uint8_t)ui->iris_slider->value());
    }

}   // end of on_iris_slider_change

// ----------------------------------------------------------------------------
void SR::on_iris_spinner_change()
{
    // disable duplicate event triggers
    ui->iris_slider->blockSignals(true);

    // update the slider box
    ui->iris_slider->setValue(ui->iris_spinner->value());

    // remove the block
    ui->iris_slider-> blockSignals(false);

    // send focus value to the lens
    if(connected)
    {
        sr.set_iris((uint8_t)ui->iris_spinner->value());
    }

}   // end of on_iris_spinner_change







