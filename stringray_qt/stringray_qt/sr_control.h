#ifndef SR_H
#define SR_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class SR; }
QT_END_NAMESPACE

class SR : public QMainWindow
{
    Q_OBJECT

public:
    SR(QWidget *parent = nullptr);
    ~SR();

public slots:
    // focus events
    void on_focus_slider_change(QEvent *event);
    void on_focus_spinner_change();

    // zoom events
//    void on_zoom_slider_change();
//    void on_zoom_spinner_change();

    // iris events
//    void on_iris_slider_change();
//    void on_iris_spinner_change();

    // connect button
    void button_click();

private:
    Ui::SR *ui;

    bool connected = false;

};
#endif // SR_H
