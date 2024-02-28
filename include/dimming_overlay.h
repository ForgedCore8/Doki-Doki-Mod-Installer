#ifndef DIMMINGOVERLAY_H
#define DIMMINGOVERLAY_H

// qt.
#include <QWidget>
#include <QColor>

class DimmingOverlay : public QWidget
{
    Q_OBJECT

    QColor overlayColor;

public:
    explicit DimmingOverlay(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;
};

#endif // DIMMINGOVERLAY_H
