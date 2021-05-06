#include "so_gui_control.h"
#include "./ui_so_gui_control.h"

SO_GUI::SO_GUI(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::SO_GUI)
{
    ui->setupUi(this);

    // the this machines local IP address
    result = get_local_ip(host_ip_address, error_msg);

    // connect the focus slider and focus spinner
    connect(ui->focus_slider, SIGNAL(valueChanged(int)), this, SLOT(on_focus_slider_change()));
    connect(ui->focus_spinner, SIGNAL(valueChanged(int)), this, SLOT(on_focus_spinner_change()));

    connect(ui->zoom_slider, SIGNAL(valueChanged(int)), this, SLOT(on_zoom_slider_change()));
    connect(ui->zoom_spinner, SIGNAL(valueChanged(int)), this, SLOT(on_zoom_spinner_change()));

    connect(ui->connect_btn, SIGNAL(clicked()), this, SLOT(button_click()));
    connect(ui->af_btn, SIGNAL(clicked()), this, SLOT(af_btn_click()));

    statusBar()->showMessage("Ready", 2000);
}

SO_GUI::~SO_GUI()
{
    delete ui;
}


// ----------------------------------------------------------------------------
void SO_GUI::button_click()
{

    if(ui->ip_address_tb->text().isEmpty())
    {
        statusBar()->showMessage("Enter an IP Address!", 10000);
        return;
    }

    camera_ip_address = ui->ip_address_tb->text().toStdString();

    if(connected)
    {
        // turn streaming off
        so_cam.config_streaming_control(SO::STREAM_OFF);

        // disconnect from the camera
        so_cam.close();

        ui->connect_btn->setText("Connect");
        connected = false;
        statusBar()->showMessage("Disconnected", 2000);

    }
    else
    {
        // configure the UDP info
        so_cam.udp_camera_info = udp_info(write_port, read_port);

        // initialize camera
        result = so_cam.init_camera(camera_ip_address, error_msg);

        // turn the video output on and stream
        result = so_cam.set_ethernet_display_parameter(inet_addr(host_ip_address.c_str()), video_port);
        result = so_cam.config_streaming_control(SO::STREAM_ON);

        if(result != 0)
        {
            statusBar()->showMessage(QString::fromStdString(error_msg), 10000);
            return;
        }

        ui->focus_spinner->blockSignals(true);
        ui->focus_slider->blockSignals(true);
        ui->zoom_spinner->blockSignals(true);
        ui->zoom_slider->blockSignals(true);

        ui->focus_slider->setValue(so_cam.lens.focus_position);
        ui->focus_spinner->setValue(so_cam.lens.focus_position);
        ui->zoom_slider->setValue(so_cam.lens.zoom_index);
        ui->zoom_spinner->setValue(so_cam.lens.zoom_index);

        ui->focus_spinner->blockSignals(false);
        ui->focus_slider->blockSignals(false);
        ui->zoom_spinner->blockSignals(false);
        ui->zoom_slider->blockSignals(false);

        ui->connect_btn->setText("Disconnect");
        connected = true;

        statusBar()->showMessage("Connected: SN-" + QString::fromStdString(so_cam.get_sn()));
    }

}   // end of button_click

// ----------------------------------------------------------------------------
void SO_GUI::af_btn_click()
{
    if(connected)
    {
        uint16_t value;

        statusBar()->showMessage("Starting Auto Focus...");

        result = so_cam.start_auto_focus(1);

        QThread::msleep(5000);

        result = so_cam.get_focus_position(value);

        ui->focus_spinner->blockSignals(true);
        ui->focus_slider->blockSignals(true);

        ui->focus_slider->setValue(so_cam.lens.focus_position);
        ui->focus_spinner->setValue(so_cam.lens.focus_position);

        ui->focus_spinner->blockSignals(false);
        ui->focus_slider->blockSignals(false);

        statusBar()->showMessage("Auto Focus Complete.", 5000);
    }

}   // end of af_btn_click

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
        so_cam.set_focus_position((uint16_t)ui->focus_slider->value());
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
        so_cam.set_focus_position((uint16_t)ui->focus_spinner->value());
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
        so_cam.set_zoom_index((uint16_t)ui->zoom_slider->value());
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
        so_cam.set_zoom_index((uint16_t)ui->zoom_spinner->value());
    }

}   // end of on_zoom_spinner_change


