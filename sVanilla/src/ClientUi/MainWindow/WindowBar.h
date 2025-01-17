#pragma once
#include <QFrame>
#include <QPushButton>
#include <QToolButton>

QT_BEGIN_NAMESPACE
namespace Ui
{
class WindowBar;
}
QT_END_NAMESPACE

class WindowBar : public QFrame
{
    Q_OBJECT

public:
    explicit WindowBar(QWidget* parent = nullptr);
    ~WindowBar();

    [[nodiscard]] const QWidget* getHitWidget() const;

    void setMinButton(QAbstractButton* btn);
    void setMaxButton(QAbstractButton* btn);
    void setCloseButton(QAbstractButton* btn);
    QPushButton* maxButton();

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

private:
    void signalsAndSlots();
    void setUi() const;

signals:
    void barBtnClick(int index);
    void tabChanged(int index);
    void minimizeRequested();
    void maximizeRequested(bool max = false);
    void closeRequested();

private:
    Ui::WindowBar* ui;
};
