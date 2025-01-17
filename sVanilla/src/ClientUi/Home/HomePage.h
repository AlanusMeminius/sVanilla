#pragma once
#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui
{
class HomePage;
}
QT_END_NAMESPACE

class LoginProxy;

class HomePage : public QWidget
{
    Q_OBJECT

public:
    explicit HomePage(QWidget* parent = nullptr);
    ~HomePage();

    void setWebsiteIcon(const QString& iconPath);

signals:
    void hasAdded(bool hasAdded);
    void updateWebsiteIcon(const std::string& uri);
    void parseUri(const QString& uri);
    void loginSucceed(std::shared_ptr<LoginProxy> loginer);
    void switchAccoutTab();

private:
    void signalsAndSlots();
    void setUi();
    void createHistoryMenu();
    void showLoginDialog(std::shared_ptr<LoginProxy> loginer);

private:
    Ui::HomePage* ui;
    QMenu* m_historyMenu = nullptr;
};
