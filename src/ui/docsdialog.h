#pragma once

#include <QDialog>

class QTabWidget;

class DocsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DocsDialog(QWidget *parent = nullptr);

private:
    void addTab(QTabWidget *tabs, const QString &title, const QString &resource);
};
