#include "CurvePreview.h"

#include "core/Processing.h"

#include <QPainter>
#include <QPolygonF>

CurvePreview::CurvePreview(QWidget *parent) :
    QWidget(parent)
{
}

void CurvePreview::setMapping(Mapping const &mapping)
{
    m_mapping = mapping;
    update();
}

QSize CurvePreview::minimumSizeHint() const
{
    return QSize(120, 120);
}

QSize CurvePreview::sizeHint() const
{
    return QSize(160, 160);
}

void CurvePreview::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QRectF const area = QRectF(rect()).adjusted(6, 6, -6, -6);
    painter.fillRect(area, palette().base());

    painter.setPen(QPen(palette().mid().color(), 1, Qt::DotLine));
    painter.drawLine(QPointF(area.center().x(), area.top()), QPointF(area.center().x(), area.bottom()));
    painter.drawLine(QPointF(area.left(), area.center().y()), QPointF(area.right(), area.center().y()));
    painter.setPen(palette().mid().color());
    painter.drawRect(area);

    QPolygonF line;
    int const steps = 200;
    for (int i = 0; i <= steps; ++i)
    {
        float const raw = -1.0f + 2.0f * static_cast<float>(i) / steps;
        float const out = shapeAxis(m_mapping, raw);
        line << QPointF(area.left() + (raw + 1.0f) * 0.5 * area.width(), area.bottom() - out * area.height());
    }

    painter.setPen(QPen(palette().highlight().color(), 2));
    painter.drawPolyline(line);
}
