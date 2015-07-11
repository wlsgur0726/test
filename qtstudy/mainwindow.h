#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#define QSTR(str) (QString::fromWCharArray(L##str))

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
	void onTimer();
    void on_pushButton_clicked();
	
private:
    Ui::MainWindow *ui;
	QTimer* m_timer;
};

#endif // MAINWINDOW_H
