#pragma once
#include <QFrame>
#include <QAbstractButton>
#include <QMenuBar>
#include <QLabel>

namespace Ui {
class WindowBarPrivate;
class WindowBar : public QFrame {
    Q_OBJECT
    Q_DECLARE_PRIVATE(WindowBar)
public:
    explicit WindowBar(QWidget *parent = nullptr);
    ~WindowBar();

public:
    QMenuBar *menuBar() const;
    QLabel *titleLabel() const;
    QAbstractButton *iconButton() const;
    QAbstractButton *minButton() const;
    QAbstractButton *maxButton() const;
    QAbstractButton *closeButton() const;

    void setMenuBar(QMenuBar *menuBar);
    void setTitleLabel(QLabel *label);
    void setBarWidget(QWidget *widget);
    void setIconButton(QAbstractButton *btn);
    void setMinButton(QAbstractButton *btn);
    void setMaxButton(QAbstractButton *btn);
    void setCloseButton(QAbstractButton *btn);

    QMenuBar *takeMenuBar();
    QLabel *takeTitleLabel();
    QWidget *takeBarWidget();
    QAbstractButton *takeIconButton();
    QAbstractButton *takeMinButton();
    QAbstractButton *takeMaxButton();
    QAbstractButton *takeCloseButton();

    QWidget *hostWidget() const;
    void setHostWidget(QWidget *w);

    bool titleFollowWindow() const;
    void setTitleFollowWindow(bool value);

    bool iconFollowWindow() const;
    void setIconFollowWindow(bool value);

Q_SIGNALS:
    void minimizeRequested();
    void maximizeRequested(bool max = false);
    void closeRequested();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

    virtual void titleChanged(const QString &text);
    virtual void iconChanged(const QIcon &icon);

protected:
    WindowBar(WindowBarPrivate &d, QWidget *parent = nullptr);

    const std::unique_ptr<WindowBarPrivate> d_ptr;
};

}