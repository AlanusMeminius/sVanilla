#include <QClipboard>
#include <QTimer>
#include <QMenu>
#include <QPushButton>
#include <QStandardPaths>
#include <QDesktopServices>
#include <QFileDialog>
#include <QProcess>
#include <QMessageBox>

#include "HomePage.h"
#include "ui_HomePage.h"
#include "BaseQt/Utility.h"
#include "Plugin/PluginManager.h"
#include "Storage/SearchHistoryStorage.h"
#include "Storage/StorageManager.h"
#include "Login/LoginDialog.h"
#include "MainWindow/SApplication.h"
#include "ClientLog.h"
#include "const_string.h"
#include "Login/LoginBubble.h"

inline const std::string mainPage = "https://svanilla.app/";
constexpr char userfaceDir[] = "userface";

bool copyWithAdminPrivileges(const QString& source, const QString& destination)
{
    QStringList arguments;
    QString program;

#ifdef _WIN32
    program = "powershell";
    QString command = QString("Copy-Item -Path '%1' -Destination '%2' -Force").arg(source).arg(destination);
    arguments << QString("Start-Process powershell -ArgumentList '-NoProfile', '-ExecutionPolicy Bypass', '-Command', \"%1\" -Verb RunAs").arg(command);
#elif __linux__
    program = "sudo";
    arguments << "-S" << QString("cp \"%1\" \"%2\"").arg(source).arg(destination);
#elif __APPLE__
    program = "osascript";
    arguments << "-e" << QString("do shell script \"cp '%1' '%2'\" with administrator privileges").arg(source).arg(destination);
#endif

    MLogI(svanilla::cHomeModule, "copyWithAdmin started! command: {}, arguments: {}", program.toStdString(), arguments.join(" ").toStdString());

    CLog_Unique_TimerK(copyWithAdmin_process_start);
    QProcess process;
    process.setProgram(program);
    process.setArguments(arguments);
    process.start();
    process.waitForFinished();
    CLog_Unique_TimerK_END(copyWithAdmin_process_start);

    if (process.exitStatus() == QProcess::NormalExit && process.exitCode() == 0)
    {
        return true;
    }
    else
    {
        MLogE(svanilla::cHomeModule, "Failed to copy file with administrator privileges, source: {}, destination: {}", source.toStdString(),
              destination.toStdString());
        QMessageBox::critical(nullptr, "Error", "Failed to copy file with administrator privileges.");
        return false;
    }
}

HomePage::HomePage(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::HomePage)
{
    ui->setupUi(this);
    signalsAndSlots();
    setUi();
}

HomePage::~HomePage()
{
    delete ui;
}

void HomePage::setWebsiteIcon(const QString& iconPath)
{
    ui->lineEditHome->setWebsiteIcon(iconPath);
}

void HomePage::signalsAndSlots()
{
    connect(ui->btnIcon, &QPushButton::clicked, this, [this] {});

    connect(ui->lineEditHome, &AddLinkLineEdit::Complete, this, [this] {
        parseUri(ui->lineEditHome->text());
        ui->lineEditHome->clear();
    });

    connect(ui->lineEditHome, &AddLinkLineEdit::textChanged, this, [this](const QString& text) {
        if (!text.isEmpty() && text.length() > 1)
        {
            emit updateWebsiteIcon(text.toStdString());
        }
    });

    connect(ui->btnLearn, &QPushButton::clicked, this, [this] {
        MLogI(svanilla::cHomeModule, "btnLearn clicked");
        QDesktopServices::openUrl(QUrl(QString::fromStdString(mainPage)));
    });
    connect(ui->btnLoadPlugin, &QPushButton::clicked, this, [this] {
        const QString pluginDir = QDir(QString::fromStdString(plugin::PluginManager::m_pluginDir)).absolutePath();
        const QString fileName =
            QFileDialog::getOpenFileName(this, tr("Import Plugin"), {}, QString("*") + QString::fromStdString(plugin::PluginManager::m_dynamicExtension));
        QString newPlugin = pluginDir + "/" + QFileInfo(fileName).fileName();
        {
            if (QFile::exists(newPlugin))
            {
                MLogW(svanilla::cHomeModule, "import existed plugin, filename: {}", fileName.toStdString());
                return;
            }

            plugin::DynamicLibLoader dynamicLibLoader(fileName.toStdString());
            dynamicLibLoader.loadLibrary();
            auto plugin = dynamicLibLoader.loadPluginSymbol();
            if (!plugin)
            {
                MLogW(svanilla::cHomeModule, "load plugin filed, filename: {}", fileName.toStdString());
                return;
            }
        }

        MLogI(svanilla::cHomeModule, " import plugin, source: {},destination: {} ", fileName.toStdString(), newPlugin.toStdString());
        if (sApp->isInstalled())
        {
            copyWithAdminPrivileges(fileName, newPlugin);
        }
        else
        {
            QFile::copy(fileName, newPlugin);
        }
    });
    connect(ui->btnLoginWebsite, &QPushButton::clicked, this, [this] {
        auto& plugins = sApp->pluginManager().plugins();
        if (plugins.size() > 1)
        {
            QMenu menu(this);
            for (auto& [_, plugin] : plugins)
            {
                auto action = new QAction(QString::fromStdString(plugin->pluginMessage().name), &menu);
                QIcon icon(LoginDialog::binToImage(plugin->websiteIcon(), QSize(24, 24)));
                action->setIcon(icon);
                menu.addAction(action);
                connect(action, &QAction::triggered, &menu, [this, plugin]() {
                    auto loginer = std::make_shared<LoginProxy>(plugin->loginer());
                    showLoginDialog(loginer);
                });
            }
            const QPoint pos = ui->btnLoginWebsite->mapToGlobal(QPoint(0, ui->btnLoginWebsite->sizeHint().height()));
            menu.exec(pos);
        }
        else if (plugins.size() == 1)
        {
            for (auto& [_, plugin] : plugins)
            {
                auto loginer = std::make_shared<LoginProxy>(plugin->loginer());
                showLoginDialog(loginer);
            }
        }
        else
        {
        }
    });

    connect(ui->btnClipBoard, &QPushButton::clicked, this, [this] {
        const QClipboard* clipboard = QGuiApplication::clipboard();
        ui->lineEditHome->setText(clipboard->text());
        MLogI(svanilla::cHomeModule, " btnClipBoard, search: {}", clipboard->text().toStdString());
        emit parseUri(clipboard->text());
    });
    connect(ui->btnHistory, &QPushButton::clicked, this, [this] {
        MLogI(svanilla::cHomeModule, " btnHistory clicked");
        createHistoryMenu();
        const QPoint pos = ui->btnHistory->mapToGlobal(QPoint(0, -m_historyMenu->sizeHint().height()));
        m_historyMenu->exec(pos);
    });
}

void HomePage::setUi()
{
    constexpr int iconSize = 40;
    ui->btnIcon->setIconSize({iconSize, iconSize});
    constexpr int homeLineEditHeight = 30;
    ui->lineEditHome->setFixedHeight(homeLineEditHeight);
}

void HomePage::createHistoryMenu()
{
    if (m_historyMenu == nullptr)
    {
        m_historyMenu = new QMenu(this);
    }
    else
    {
        m_historyMenu->clear();
    }

    const auto actionCallback = [this](const QString& text) {
        ui->lineEditHome->setText(text);
        ui->lineEditHome->setFocus();
    };
    auto historyStorage = sqlite::StorageManager::intance().searchHistoryStorage();
    auto history = historyStorage->allItems();
    util::createMenu(m_historyMenu, width() / 3, history, actionCallback);
}

void HomePage::showLoginDialog(std::shared_ptr<LoginProxy> loginer)
{
    if (!loginer)
    {
        return;
    }

    if (loginer->isLogin())
    {
        emit switchAccoutTab();
    }
    else
    {
        MLogI(svanilla::cHomeModule, " LoginWebsite ");
        LoginDialog login(loginer);
        if (QDialog::Accepted == login.exec())
        {
            MLogI(svanilla::cHomeModule, " LoginWebsite succeed");
            emit switchAccoutTab();
            emit loginSucceed(loginer);
        }
    }

    // const auto loginBubble = new LoginBubble(loginer);
    // const auto globalPos = mapToGlobal(QPoint(0, 0));
    // loginBubble->showCenter(QRect(globalPos, QSize(width(), height())));
}
