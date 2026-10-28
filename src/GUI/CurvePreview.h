#ifndef CURVEPREVIEW_H_
#define CURVEPREVIEW_H_

#include "core/Mapping.h"

#include <QWidget>

// Plots the transfer function of an axis mapping: raw input -1..1 against output 0..1.
class CurvePreview : public QWidget
{
    Q_OBJECT

public:
    explicit CurvePreview(QWidget *parent = nullptr);

    void setMapping(Mapping const &mapping);

    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    Mapping m_mapping;
};

#endif /* CURVEPREVIEW_H_ */
