#include "plot.h"
#include "curvedata.h"
#include "sensordata.h"

#include <qwt_plot_grid.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_layout.h>
#include <qwt_scale_widget.h>
#include <qwt_legend.h>
#include <qwt_system_clock.h>
#include <qwt_plot_directpainter.h>

#include <QTimerEvent>
#include <QMouseEvent>
#include <QDebug>

Plot::Plot(QWidget *parent) : QwtPlot(parent)
{
    // Set title
    setTitle(tr("Sensor Data Window"));

    // Set background
    setCanvasBackground(Qt::white);

    // Set axis
    setAxisTitle(QwtPlot::xBottom, tr("Time(s)"));
    setAxisScale(QwtPlot::xBottom, 0, 10.0);

    setAxisTitle(QwtPlot::yLeft, tr("Sensor Value(v)"));
    setAxisScale(QwtPlot::yLeft, 0, 50.0);

    // Legend
    QwtLegend *legend = new QwtLegend;
    legend->setItemMode(QwtLegend::CheckableItem);
    insertLegend(legend, QwtPlot::RightLegend);

    // Insert grid
    m_grid = new QwtPlotGrid;
    m_grid->enableXMin(true);
    m_grid->enableYMin(true);    
    m_grid->setMajPen(QPen(Qt::gray, 0.0, Qt::DotLine));
    m_grid->setMinPen(QPen(Qt::gray, 0.0, Qt::DotLine));
    m_grid->attach(this);

    // Insert markers
    QwtPlotMarker *mY = new QwtPlotMarker();
    mY->setLabel(QString::fromLatin1("y = 0.0"));
    mY->setLabelAlignment(Qt::AlignRight | Qt::AlignTop);
    mY->setLineStyle(QwtPlotMarker::HLine);
    mY->setYValue(25.0);
    mY->attach(this);

    QwtPlotCurve *curve;
    // Insert curves
    curve = new QwtPlotCurve(tr("Accelerometer_X"));
    curve->setPen(QPen(Qt::red));
    curve->setStyle(QwtPlotCurve::Lines);
    curve->setData(new CurveData());
    curve->attach(this)
    m_curve[SENSOR_ACCL_X] = curve;

    curve = new QwtPlotCurve(tr("Accelerometer_Y"));
    curve->setPen(QPen(Qt::green));
    curve->setStyle(QwtPlotCurve::Lines);
    curve->setData(new CurveData());
    curve->attach(this);
    m_curve[SENSOR_ACCL_Y] = curve;

    curve = new QwtPlotCurve(tr("Accelerometer_Z"));
    curve->setPen(QPen(Qt::blue));
    curve->setStyle(QwtPlotCurve::Lines);
    curve->setData(new CurveData());
    curve->attach(this);
    m_curve[SENSOR_ACCL_Z] = curve;

    curve = new QwtPlotCurve(tr("Gyroscope_X"));
    curve->setPen(QPen(Qt::blue));
    curve->setStyle(QwtPlotCurve::Lines);
    curve->setData(new CurveData());
    curve->attach(this);
    m_curve[SENSOR_GYRO_X] = curve;

    curve = new QwtPlotCurve(tr("Gyroscope_Y"));
    curve->setPen(QPen(Qt::blue));
    curve->setStyle(QwtPlotCurve::Lines);
    curve->setData(new CurveData());
    curve->attach(this);
    m_curve[SENSOR_GYRO_Y] = curve;

    curve = new QwtPlotCurve(tr("Gyroscope_Z"));
    curve->setPen(QPen(Qt::blue));
    curve->setStyle(QwtPlotCurve::Lines);
    curve->setData(new CurveData());
    curve->attach(this);
    m_curve[SENSOR_GYRO_Z] = curve;

    setAutoFillBackground(true);
    setAutoReplot(false);

    m_directPainter = new QwtPlotDirectPainter(this);
    m_serialSamplingThread.start();
    installEventFilter(this);
}

Plot::~Plot()
{
}

bool Plot::eventFilter(QObject *object, QEvent *event)
{
    if (!canvas())
        return false;

    switch (event->type()) {
    case QEvent::MouseMove:
        widgetMouseMoveEvent((QMouseEvent *)event);
        break;

    default:
        break;
    }

    return QwtPlot::eventFilter(object, event);
}

void Plot::widgetMouseMoveEvent(QMouseEvent *mouseEvent)
{
    QPoint pos = mouseEvent->pos();
    qDebug() << "widgetMouseMoveEvent" << pos;
    //  rescale
    /*for (int i; i < QwtPlot::axisCnt; i++) {
        QwtScaleWidget *scaleWidget = axisWidget(i);
        if (scaleWidget)
            scaleWidget->setMargin(0);

        QwtScaleDraw *scaleDraw = axisScaleDraw(i);
        if (scaleDraw)
            scaleDraw->enableComponent(QwtAbstractScaleDraw::Backbone, false);
    }
    plotLayout()->setAlignCanvasToScales(true);*/
    //replot();
}

void Plot::paintEvent(QPaintEvent *paintEvent)
{
    QwtPlot::paintEvent(paintEvent);
}

void Plot::replot()
{
    QwtPlot::replot();
}
/*
void Plot::start()
{
    //m_serialSamplingThread.start();
    m_clock.start();
    m_timerId = startTimer(10);   // 0.01-second timer

    emit started();
}
*/
/*
void Plot::running(bool on)
{
}
*/
void Plot::pause()
{
    emit paused();
}

void Plot::stop()
{
    //m_clock

    emit stopped();
}

void Plot::updateCurve(int index)
{
    CurveData *data = (CurveData *)m_curve[index]->data();

    data->values().lock();

    const int numPoints = data->size();
    if (numPoints > m_paintedPoints) {
        const bool doClip = !canvas()->testAttribute(Qt::WA_PaintOnScreen);
        if (doClip) {

        }
        m_directPainter->drawSeries(m_curve[index], m_paintedPoints - 1, m_paintedPoints);
        m_paintedPoints += 1;
    }

    data->values().unlock();
}

void Plot::timerEvent(QTimerEvent *te)
{
    // update curve
    if (m_timerId == te->timerId()) {
        CurveData *data = (CurveData *)m_curve[SENSOR_GYRO_Z]->data();

        data->values().lock();
        const int numPoints = data->size();
        if (numPoints > m_paintedPoints) {
            const bool doClip = !canvas()->testAttribute(Qt::WA_PaintOnScreen);
            if (doClip) {

            }
            m_directPainter->drawSeries(m_curve, m_paintedPoints - 1, m_paintedPoints);
            m_paintedPoints += 1;
        }

        data->values().unlock();
    }
    QwtPlot::timerEvent(te);
}