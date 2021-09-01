/*
 *@author: 缪庆瑞
 *@date: 2021.08.28
 *@brief: 可调整图片位置和大小的label部件
 *注:该类继承自QLabel，专门用于使用QLabel设置pixmap显示图片的场景，可通过鼠标以及滚轮自由调整图片的位置与大小，并实现了
 *以鼠标光标为中心的放大与缩小。另外因为项目需要，还增加了在鼠标光标移动过程中实时获取图像像素点的灰度值的功能，该功能相对会影响
 *性能，如不需要可在构造函数中关闭鼠标跟踪。
 *
 *移动与放缩实现原理：
 *图片位置移动与放缩的实现基于坐标系的平移与放缩，默认以部件中心点作为坐标系原点，通过移动位置增量在此基础上改变原点进而改变图片的
 *位置。初期还使用过另一个不涉及坐标系变化的方案，即保持部件本身的坐标系，放缩pixmap本身，然后根据部件尺寸和放缩后的图片尺寸计算出
 *绘制矩形，将其绘制到部件上。该方案有一个极大的缺陷就是放大pixmap本身意味着占用的内存也在扩增，容易出现超出QImage的最大限制的问
 *题，所以最终弃用。
 *
 *参考资料:
 *https://blog.csdn.net/weixin_43935474/article/details/118254075
 *https://blog.csdn.net/Viciower/article/details/97648437
 */
#ifndef ADJUSTABLEPIXMAPLABEL_H
#define ADJUSTABLEPIXMAPLABEL_H

#include <QLabel>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QWheelEvent>

class AdjustablePixmapLabel : public QLabel
{
    Q_OBJECT
public:
    explicit AdjustablePixmapLabel(QWidget *parent = nullptr);
    void setPixmap(const QPixmap &pixmap,bool is_reset=true);//对外接口，用来设置图片，并自动计算图片所在矩形

protected:
    //重写绘图与重设部件大小的事件处理函数
    virtual void paintEvent(QPaintEvent *event) override;
    virtual void resizeEvent(QResizeEvent *event) override;

    //重写鼠标与滚轮事件处理函数
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseMoveEvent(QMouseEvent *event) override;
    virtual void mouseReleaseEvent(QMouseEvent *event) override;
    virtual void wheelEvent(QWheelEvent *event) override;


private:
    void resetPixmapStatus(bool match_widget_size=true);//重置pixmap放缩比例和移动位置为初始状态
    void updatePixmapRectF();//更新当前图片所在的矩形区域

    bool mouse_is_pressed_ = false;//鼠标按下标记
    QPoint mouse_pre_point_ = QPoint(0,0);//记录鼠标上一次的位置

    double pixmap_scale_value_ = 1.0;//图片放缩比例
    QPointF pixmap_move_pointf_ = QPointF(0,0);//记录图片移动位置(相对部件中心的移动位置)
    QRectF pixmap_rectf_ = QRectF();//记录当前图片所在的矩形区域(由部件尺寸、图片真实尺寸、放缩比例和移动位置共同决定)

signals:
    void grayValueWithPos(int gray_value,int pos_x,int pos_y);//对外信号，发送当前鼠标光标所在图像像素点的位置和灰度值

public slots: 


};

#endif // ADJUSTABLEPIXMAPLABEL_H
