#define _CRT_SECURE_NO_WARNINGS

#ifndef SO_GUI_H
#define SO_GUI_H

#include <cstdio>
#include <iostream>

#include <QMainWindow>
#include <Qthread>

#include "ip_helper.h"

// include lfor the Sierra-Olympic Camera code
#include "sierra_olympic_camera.h"

QT_BEGIN_NAMESPACE
namespace Ui { class SO_GUI; }
QT_END_NAMESPACE

class SO_GUI : public QMainWindow
{
    Q_OBJECT

public:
    SO_GUI(QWidget *parent = nullptr);
    ~SO_GUI();

public slots:
    // focus events
    void on_focus_slider_change();
    void on_focus_spinner_change();

    // zoom events
    void on_zoom_slider_change();
    void on_zoom_spinner_change();

    // connect button
    void button_click();

    // autofocus buttun
    void af_btn_click();

private:
    Ui::SO_GUI *ui;

    std::string host_ip_address;

    int32_t result = 0;
    uint16_t read_port = 14002;                 // c2replyPort
    uint16_t write_port = 14001;                // listenPort1
    uint16_t c2_inbound2 = 14003;               // listenPort2
    uint16_t video_port = 15004;

    // Sierra-Olympic specific variables
    SO::camera so_cam;
    std::string camera_ip_address;
    std::string error_msg;

    bool connected = false;

};
#endif // SR_H
