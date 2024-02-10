#ifndef DIMMINGOVERLAY_H
#define DIMMINGOVERLAY_H

#include <QWidget>
#include <QColor>

class DimmingOverlay : public QWidget {
    Q_OBJECT

public:
    explicit DimmingOverlay(QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    QColor overlayColor;
};

#endif // DIMMINGOVERLAY_H
