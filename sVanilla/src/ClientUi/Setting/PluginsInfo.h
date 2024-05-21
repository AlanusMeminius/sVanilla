#pragma once
#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui
{
class PluginsInfo;
}
QT_END_NAMESPACE

class PluginsInfo : public QWidget
{
    Q_OBJECT

public:
    explicit PluginsInfo(QWidget* parent = nullptr);
    ~PluginsInfo();

private:
    void setUi();
    void signalsAndSlots();

    void initPluginInfos();

private:
    Ui::PluginsInfo* ui;
};