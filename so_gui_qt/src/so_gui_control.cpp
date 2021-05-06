#include "so_gui_control.h"
#include "./ui_so_gui_control.h"

SO_GUI::SO_GUI(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::SO_GUI)
{
    ui->setupUi(this);

    std::string error_msg;

    // the this machines local IP address
    get_local_ip(host_ip_address, error_msg);

    // connect the focus slider and focus spinner
    connect(ui->focus_slider, SIGNAL(valueChanged(int)), this, SLOT(on_focus_slider_change()));
    connect(ui->focus_spinner, SIGNAL(valueChanged(int)), this, SLOT(on_focus_spinner_change()));

    connect(ui->zoom_slider, SIGNAL(valueChanged(int)), this, SLOT(on_zoom_slider_change()));
    connect(ui->zoom_spinner, SIGNAL(valueChanged(int)), this, SLOT(on_zoom_spinner_change()));

    connect(ui->connect_btn, SIGNAL(clicked()), this, SLOT(button_click()));

}

SO_GUI::~SO_GUI()
{
    delete ui;
}


// ----------------------------------------------------------------------------
void SO_GUI::button_click()
{
    uint32_t wait_time = 5;
    uint32_t baud_rate = 115200;

    if(ui->ip_address_tb->text().isEmpty())
    {
        return;
    }

    if(connected)
    {
        // disconnect from the lens
//        sr.close();

        ui->connect_btn->setText("Connect");
        connected = false;
    }
    else
    {
        // connect to the lens
        //sr = stingray_lens(ui->comm_port->text().toStdString(), baud_rate, wait_time);
        //sr.connect();

        ui->focus_spinner->blockSignals(true);
        ui->focus_slider->blockSignals(true);
        ui->zoom_spinner->blockSignals(true);
        ui->zoom_slider->blockSignals(true);

//        ui->focus_slider->setValue(sr.get_focus());
//        ui->focus_spinner->setValue(sr.get_focus());
//        ui->zoom_slider->setValue(sr.get_zoom());
//        ui->zoom_spinner->setValue(sr.get_zoom());

        ui->focus_spinner->blockSignals(false);
        ui->focus_slider->blockSignals(false);
        ui->zoom_spinner->blockSignals(false);
        ui->zoom_slider->blockSignals(false);

        ui->connect_btn->setText("Disconnect");
        connected = true;
    }

}   // end of button_click

// ----------------------------------------------------------------------------
void SO_GUI::on_focus_slider_change()
{
    // disable duplicate event triggers
    ui->focus_spinner->blockSignals(true);

    // update the spin box
    ui->focus_spinner->setValue(ui->focus_slider->value());

    // remove the block
    ui->focus_spinner->blockSignals(false);

    // send focus value to the lens
    if(connected)
    {
//        sr.set_focus((uint16_t)ui->focus_slider->value());
    }

}   // end of on_focus_slider_change

// ----------------------------------------------------------------------------
void SO_GUI::on_focus_spinner_change()
{
    // disable duplicate event triggers
    ui->focus_slider->blockSignals(true);

    // update the slider box
    ui->focus_slider->setValue(ui->focus_spinner->value());

    // remove the block
    ui->focus_slider->blockSignals(false);

    // send focus value to the lens
    if(connected)
    {
//        sr.set_focus((uint16_t)ui->focus_spinner->value());
    }

}   // end of on_focus_spinner_change

// ----------------------------------------------------------------------------
void SO_GUI::on_zoom_slider_change()
{
    // disable duplicate event triggers
    ui->zoom_spinner->blockSignals(true);

    // update the spin box
    ui->zoom_spinner->setValue(ui->zoom_slider->value());

    // remove the block
    ui->zoom_spinner->blockSignals(false);

    // send focus value to the lens
    if(connected)
    {
//        sr.set_zoom((uint16_t)ui->zoom_slider->value());
    }

}   // end of on_zoom_slider_change

// ----------------------------------------------------------------------------
void SO_GUI::on_zoom_spinner_change()
{
    // disable duplicate event triggers
    ui->zoom_slider->blockSignals(true);

    // update the slider box
    ui->zoom_slider->setValue(ui->zoom_spinner->value());

    // remove the block
    ui->zoom_slider->blockSignals(false);

    // send focus value to the lens
    if(connected)
    {
//        sr.set_zoom((uint16_t)ui->zoom_spinner->value());
    }

}   // end of on_zoom_spinner_change


