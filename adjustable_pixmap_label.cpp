/*
 *@author: 缪庆瑞
 *@date: 2021.08.28
 *@brief: 可调整图片位置和大小的label部件
 */
#include "adjustable_pixmap_label.h"
#include <QPainter>
#include <QPixmap>
#include <QDebug>

//图片的最小、最大放缩比以及放缩增量系数
#define PIXMAP_MIN_SCALE 0.05
#define PIXMAP_MAX_SCALE 20
#define PIXMAP_SCALE_DELTA_RATIO 0.05

AdjustablePixmapLabel::AdjustablePixmapLabel(QWidget *parent) : QLabel(parent)
{
    //设置布局策略，避免因图片过大时部件无法调整大小
    this->setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Ignored);
    //开启鼠标跟踪，用于实时跟踪光标所在图像像素点的位置和灰度值(相对影响性能，如不需要屏蔽此处即可)
    this->setMouseTracking(true);
}
/*
 *@brief: 设置label图片，并自动根据当前状态计算图片所在矩形
 *@author: 缪庆瑞
 *@date: 2021.08.31
 *@param: pixmap:图片
 *@param: is_reset:是否重置图片的放缩比例和移动位置，默认true
 */
void AdjustablePixmapLabel::setPixmap(const QPixmap &pixmap, bool is_reset)
{
    QLabel::setPixmap(pixmap);
    if(!this->pixmap()->isNull())
    {
        if(is_reset)
        {
            resetPixmapStatus();
        }
        updatePixmapRectF();

        update();
    }
}
/*
 *@brief: 绘图事件处理函数
 *@author: 缪庆瑞
 *@date: 2021.08.28
 *@param: event:绘图事件
 */
void AdjustablePixmapLabel::paintEvent(QPaintEvent *event)
{
    if(this->pixmap() && !this->pixmap()->isNull())
    {
        //qDebug()<<pixmap_move_point_;

        QPainter painter(this);
        //坐标系平移，以部件中心+移动的增量为原点，即图片的中心
        painter.translate(this->width()/2.0+pixmap_move_pointf_.x(),this->height()/2.0+pixmap_move_pointf_.y());
        //坐标系放缩，以当前原点(图片的中心点)放缩
        painter.scale(pixmap_scale_value_,pixmap_scale_value_);

        //图片矩形区域，以当前坐标系原点为中心。size为图片真实大小，由坐标系放缩比例自动放缩
        QRectF scale_pixmap_rect(QPointF(-this->pixmap()->width()/2.0,-this->pixmap()->height()/2.0),
                                 QSizeF(this->pixmap()->size()));
        painter.drawPixmap(scale_pixmap_rect,*(this->pixmap()),this->pixmap()->rect());
    }
    else
    {
        QLabel::paintEvent(event);
    }
}
/*
 *@brief: 尺寸调整事件处理函数
 *@author: 缪庆瑞
 *@date: 2021.08.28
 *@param: event:尺寸事件
 */
void AdjustablePixmapLabel::resizeEvent(QResizeEvent *event)
{
    QLabel::resizeEvent(event);
    if(this->pixmap() && !this->pixmap()->isNull())
    {
        //当部件大小重新调整时，图片放缩比例和移动位置还原为初始状态，并更新图片所在矩形区域
        resetPixmapStatus();
        updatePixmapRectF();
        //刷新显示
        update();
    }
}
/*
 *@brief: 鼠标按下事件处理函数
 *@author: 缪庆瑞
 *@date: 2021.08.28
 *@param: event:鼠标事件
 */
void AdjustablePixmapLabel::mousePressEvent(QMouseEvent *event)
{
    //记录左键按下的状态与位置
    if(event->button() == Qt::LeftButton)
    {
        mouse_is_pressed_ = true;
        mouse_pre_point_ = event->pos();
    }
    QLabel::mousePressEvent(event);
}
/*
 *@brief: 鼠标移动事件处理函数
 *@author: 缪庆瑞
 *@date: 2021.08.28
 *@param: event:鼠标事件
 */
void AdjustablePixmapLabel::mouseMoveEvent(QMouseEvent *event)
{
    if(this->pixmap() && !this->pixmap()->isNull())
    {
        //鼠标左键按下移动
        if(mouse_is_pressed_)
        {
            //计算鼠标按下移动的距离，增量到图片移动位置上，并更新图片所在矩形
            QPoint tmp_point = event->pos() - mouse_pre_point_;
            pixmap_move_pointf_ += tmp_point;
            mouse_pre_point_ = event->pos();
            updatePixmapRectF();
            //刷新显示
            update();
        }
        //非左键按下移动(没有任何键按下时需要开启鼠标跟踪才能触发)
        else
        {
            //鼠标光标在图片矩形区域内
            if(pixmap_rectf_.contains(event->pos()))
            {
                //计算光标到图片左顶点的距离,除以放缩比例得到光标在原始图像的像素点坐标，以此计算灰度值
                int x_to_left = (event->pos().x() - pixmap_rectf_.x())/pixmap_scale_value_;
                int y_to_top = (event->pos().y() - pixmap_rectf_.y())/pixmap_scale_value_;
                int gray = qGray(this->pixmap()->toImage().pixel(x_to_left,y_to_top));
                //qDebug()<<"gray"<<gray<<"x_to_left:"<<x_to_left<<"y_to_top"<<y_to_top;
                emit grayValueWithPos(gray,x_to_left,y_to_top);
            }
        }
    }
    QLabel::mouseMoveEvent(event);
}
/*
 *@brief: 鼠标释放事件处理函数
 *@author: 缪庆瑞
 *@date: 2021.08.28
 *@param: event:鼠标事件
 */
void AdjustablePixmapLabel::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        mouse_is_pressed_ = false;
    }
    else if(event->button() == Qt::RightButton)
    {
        if(this->pixmap() && !this->pixmap()->isNull())
        {
            //当鼠标右键释放时，图片放缩比例和移动位置还原为初始状态，并更新图片所在矩形区域
            resetPixmapStatus();
            updatePixmapRectF();
            //刷新显示
            update();
        }
    }
    QLabel::mouseReleaseEvent(event);
}
/*
 *@brief: 鼠标滚轮事件处理函数
 *@author: 缪庆瑞
 *@date: 2021.08.28
 *@param: event:滚轮事件
 */
void AdjustablePixmapLabel::wheelEvent(QWheelEvent *event)
{
    QLabel::wheelEvent(event);
    if(this->pixmap() && !this->pixmap()->isNull())
    {
        //获取滚动角度与方向，用来增减放缩比例
        QPoint angle_point = event->angleDelta()/8;
        if(!angle_point.isNull())
        {
            //目前的鼠标滚轮精度大多为15度，高精度暂不考虑
            int delta = angle_point.y()/15;
            double tmp_pixmap_scale_value_ = pixmap_scale_value_;//缓存当前的放缩比例
            //增减放缩比例，并对范围校验
            pixmap_scale_value_ += delta*PIXMAP_SCALE_DELTA_RATIO;
            if(pixmap_scale_value_ < PIXMAP_MIN_SCALE)
            {
                pixmap_scale_value_ = PIXMAP_MIN_SCALE;
            }
            if(pixmap_scale_value_ > PIXMAP_MAX_SCALE)
            {
                pixmap_scale_value_ = PIXMAP_MAX_SCALE;
            }
            //根据当前放缩比例刷新显示
            //repaint();
            update();

            //如果鼠标在当前(本次放缩之前)图片区域内，则通过移动原点位置实现以光标为中心放缩
            if(pixmap_rectf_.contains(event->pos()))
            {
                //计算鼠标光标位置与坐标原点的距离
                double x_to_center = event->pos().x() - (this->width()/2.0+pixmap_move_pointf_.x());
                double y_to_center = event->pos().y() - (this->height()/2.0+pixmap_move_pointf_.y());
                //计算经过放缩后，之前鼠标光标位置对应的点要回到光标位置应该移动的距离
                double x_move = x_to_center/tmp_pixmap_scale_value_*pixmap_scale_value_ - x_to_center;
                double y_move = y_to_center/tmp_pixmap_scale_value_*pixmap_scale_value_ - y_to_center;
                //qDebug()<<"X move:"<<x_move<<"Y move:"<<y_move;
                //修改移动位置，重新刷新显示
                pixmap_move_pointf_.setX(pixmap_move_pointf_.x() - x_move);
                pixmap_move_pointf_.setY(pixmap_move_pointf_.y() - y_move);
                //repaint();
                update();
            }

            updatePixmapRectF();
        }
    }
}
/*
 *@brief: 重置pixmap放缩比例和移动位置为初始状态
 *@author: 缪庆瑞
 *@date: 2021.09.01
 *@param: match_widget_size:是否根据部件大小自动调整到合适的比例
 */
void AdjustablePixmapLabel::resetPixmapStatus(bool match_widget_size)
{
    if(this->pixmap() && !this->pixmap()->isNull())
    {
        //根据部件大小自动调整到合适的放缩比例
        if(match_widget_size)
        {
            double width_scale = this->width()/(double)this->pixmap()->width();
            double height_scale = this->height()/(double)this->pixmap()->height();
            pixmap_scale_value_ = qMin(width_scale,height_scale);
        }
        //不进行放缩，显示图片真实大小
        else
        {
          pixmap_scale_value_ = 1.0;
        }
        pixmap_move_pointf_ = QPointF(0,0);
    }
}
/*
 *@brief: 更新当前图片所在的矩形区域
 *注:该区域由部件尺寸、图片真实尺寸、放缩比例和移动位置共同决定，其中任意元素发生更改时都要调用
 *该方法更新矩形。
 *@author: 缪庆瑞
 *@date: 2021.08.31
 */
void AdjustablePixmapLabel::updatePixmapRectF()
{
    double left_x = (this->width()/2.0+pixmap_move_pointf_.x()) - this->pixmap()->width()*pixmap_scale_value_/2.0;
    double top_y = (this->height()/2.0+pixmap_move_pointf_.y()) - this->pixmap()->height()*pixmap_scale_value_/2.0;
    double width = this->pixmap()->width()*pixmap_scale_value_;
    double height = this->pixmap()->height()*pixmap_scale_value_;
    pixmap_rectf_ = QRectF(left_x,top_y,width,height);
}


