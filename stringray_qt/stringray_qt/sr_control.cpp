#include "sr_control.h"
#include "./ui_sr_control.h"

SR::SR(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::SR)
{
    ui->setupUi(this);

    // connect the focus slider and focus spinner
    connect(ui->focus_slider, SIGNAL(sliderReleased()), this, SLOT(on_focus_slider_change(event)));
    connect(ui->focus_spinner, SIGNAL(valueChanged(int)), this, SLOT(on_focus_spinner_change()));
   // connect(ui->focus_slider, &QSlider::valueChanged, this, SLOT(on_slider_change()));

    connect(ui->connect_btn, SIGNAL(clicked()), this, SLOT(button_click()));


}

SR::~SR()
{
    delete ui;
}

void SR::on_focus_spinner_change()
{
    // disable duplicate event triggers
    ui->focus_slider->blockSignals(true);

    // send focus value to the lens



    // update the slider box
    ui->focus_slider->setValue(ui->focus_spinner->value());

    // remove the block
    ui->focus_slider-> blockSignals(false);
}

void SR::button_click()
{
    int bp = 1;

    if(ui->comm_port->text().isEmpty())
    {

        return;
    }

    if(connected)
    {
        // disconnect from the lens

        ui->connect_btn->setText("Connect");
        connected = false;
    }
    else
    {
        // connect to the lens

        ui->connect_btn->setText("Disconnect");
        connected = true;
    }


}

void SR::on_focus_slider_change(QEvent *event)
{



    // disable duplicate event triggers
    ui->focus_spinner->blockSignals(true);

    // send focus value to the lens


    // update the spin box
    ui->focus_spinner->setValue(ui->focus_slider->value());

    // remove the block
    ui->focus_spinner-> blockSignals(false);
}

