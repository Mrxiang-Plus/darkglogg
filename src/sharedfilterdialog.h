#ifndef SHAREDFILTERDIALOG_H
#define SHAREDFILTERDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QScrollArea>


namespace Ui {
class sharedfilterdialog;
}

class sharedfilterdialog : public QDialog
{
    Q_OBJECT

public:
    explicit sharedfilterdialog(QWidget *parent = 0);
    ~sharedfilterdialog();
    QScrollArea* filterScroll;
    QVBoxLayout* mainLayout;

    QWidget* mainWidget;
    QWidget* filterTabTemp;
    int filterCount;


private slots:
    void on_addFilterItem_clicked();

    void on_delFilterItem_clicked();

private:
    Ui::sharedfilterdialog *ui;
    void createFilterHead();
    void addFilterTab();
};

#endif // SHAREDFILTERDIALOG_H
