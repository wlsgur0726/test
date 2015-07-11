#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QTimer"

#include <stdio.h>
#include <stdint.h>
#include <assert.h>

MainWindow::MainWindow(QWidget *parent) :
QMainWindow(parent),
ui(new Ui::MainWindow),
m_timer(new QTimer(this))
{
	ui->setupUi(this);
	connect(m_timer, SIGNAL(timeout()), this, SLOT(onTimer()));
	m_timer->start(1000);
}

MainWindow::~MainWindow()
{
	delete ui;
	delete m_timer;
}


void MyMemCpy(void* a_dst,
              const void* a_src,
              int a_size)
{
    uint8_t* dst = (uint8_t*)a_dst;
    uint8_t* src = (uint8_t*)a_src;
    int len = a_size;

    const int PtrSize = sizeof(intptr_t);
    while (len >= PtrSize) {
        intptr_t* d = (intptr_t*)dst;
        intptr_t* s = (intptr_t*)src;

        *d = *s;
        dst += PtrSize;
        src += PtrSize;
        len -= PtrSize;
    }

    assert(len >= 0);
    for (; len > 0; --len) {
        *dst = *src;
        ++dst;
        ++src;
    }
}

QString ArrayToString(uint8_t* arr, int size)
{
    QString r;
    for (int i=0; i<size; ++i) {
        char buf[8];
        sprintf(buf, " %02x", arr[i]);
        r += buf;
    }
    return r;
}

void MainWindow::on_pushButton_clicked()
{
    char ptrsize[4];
    sprintf(ptrsize, "%d", sizeof(void*));
	QString s;
	s += QSTR("ㅂㅅ 123 asd 가나다\n");
    s += ptrsize;
    s += QSTR("\n");
    {
        const int SrcSize = 7;
        uint8_t src1[99] = {0xff};
        uint8_t src2[99] = {0xff};
        uint8_t dst[100] = {0xff};

        for (int i=0; i<SrcSize; ++i) {
            src1[i] = i;
            src2[i] = (SrcSize-1) - i;
        }

        s += QSTR("\n src1 : ");
        s += ArrayToString(src1, SrcSize);
        s += QSTR("\n src2 : ");
        s += ArrayToString(src2, SrcSize);

        MyMemCpy(&dst[0],
                 src1,
                 SrcSize);

        MyMemCpy(&dst[SrcSize],
                 src2,
                 SrcSize);

        s += QSTR("\n dst  : ");
        s += ArrayToString(dst, SrcSize * 2 + 1);
        s += QSTR("\n");
    }
	ui->label_1->setText(s);
	
	QString input = ui->textEdit->toPlainText();
	std::wstring wstr;
	wstr.resize(input.length());
	input.toWCharArray((wchar_t*)wstr.c_str());

	ui->textBrowser->setText(QString::fromWCharArray(wstr.c_str()));
}

void MainWindow::onTimer()
{
	static int num = 0;
	++num;
	QString s;
	s.setNum(num);
	ui->label_2->setText(s);
}
