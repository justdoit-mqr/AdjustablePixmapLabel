#include "widget.h"
#include "ui_widget.h"
#include <QFileDialog>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);

    adjustable_pixmap_label = new AdjustablePixmapLabel(this);
    ui->verticalLayout_2->addWidget(adjustable_pixmap_label);
}

Widget::~Widget()
{
    delete ui;
}


void Widget::on_pushButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,"打开文件","./","*.png *.jpg *.jpeg *.bmp",nullptr,QFileDialog::DontUseNativeDialog);
    if(!fileName.isEmpty())
    {
        adjustable_pixmap_label->setPixmap(QPixmap(fileName),true);
    }
}
