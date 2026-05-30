#pragma once
#include <QFrame>

class QKeyEvent;

class AboutDialog : public QFrame
{
    Q_OBJECT
public:
    explicit AboutDialog(QWidget *parent = nullptr);
    void showCentered();
protected:
    void keyPressEvent(QKeyEvent *event) override;
};
