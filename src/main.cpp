#include <QApplication>
#include <QMainWindow>
#include <QGuiApplication>
#include <QWindow>
#include <QtCore>
#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPalette>
#include <QStackedWidget>
#include <QLabel>
#include <QPushButton>
#include <QButtonGroup>
#include <QAbstractButton>
#include <QPixmap>
#include <QLineEdit>
#include <QProcess>
#include <QTimer>
#include <QRegularExpression>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QSizePolicy>
#include <QClipboard>
#include <QDesktopServices>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>
#include <functional>
#include <memory>

class MainWindow : public QMainWindow
{
private:
    bool isCheckingInternet = false;
    QTimer *refreshTimer = nullptr;
    QNetworkAccessManager *netManager = nullptr;

public:
    MainWindow(QWidget *parent = nullptr) : QMainWindow(parent)
    {
        setWindowTitle("NixlyInstall");
        
        setWindowFlags(Qt::Window | Qt::WindowTitleHint | 
                      Qt::WindowSystemMenuHint | Qt::WindowMinMaxButtonsHint);
        
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        
        setAttribute(Qt::WA_AcceptTouchEvents);
        
        // Set initial size - will be managed by tiling WM
        resize(1000, 700);
        
        QWidget *centralWidget = new QWidget(this);
        QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
        mainLayout->setContentsMargins(0, 0, 0, 0);
        mainLayout->setSpacing(0);
        
        QWidget *topPanel = new QWidget();
        topPanel->setFixedHeight(60);
        topPanel->setAutoFillBackground(true);
        
        QPalette topPalette = topPanel->palette();
        topPalette.setColor(QPalette::Window, QColor(18, 18, 18));
        topPanel->setPalette(topPalette);
        
        QHBoxLayout *topLayout = new QHBoxLayout(topPanel);
        topLayout->setContentsMargins(10, 5, 10, 5);
        topLayout->setSpacing(5);
        
        QButtonGroup *menuButtonGroup = new QButtonGroup(this);
        
        QStringList menuItems = {
            "Welcome to NixlyOS",
            "Internet Connection", 
            "GitHub",
            "Select Drive",
            "Settings",
            "Install"
        };
        
        QList<QPushButton*> menuButtons;
        
        for (int i = 0; i < menuItems.size(); ++i) {
            QPushButton *button = new QPushButton(menuItems[i]);
            button->setCheckable(true);
            button->setMinimumHeight(40);
            button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
            
            button->setStyleSheet(
                "QPushButton {"
                "    background-color: #2A2A2A;"
                "    color: white;"
                "    border: 1px solid #444444;"
                "    border-radius: 5px;"
                "    padding: 8px 16px;"
                "    font-size: 12px;"
                "    font-weight: bold;"
                "}"
                "QPushButton:hover {"
                "    background-color: #3A3A3A;"
                "    border-color: #666666;"
                "}"
                "QPushButton:checked {"
                "    background-color: #0078D4;"
                "    border-color: #106EBE;"
                "}"
                "QPushButton:pressed {"
                "    background-color: #005A9E;"
                "}"
                "QPushButton:disabled {"
                "    background-color: #1A1A1A;"
                "    color: #666666;"
                "    border-color: #333333;"
                "}"
            );
            
            menuButtons.append(button);
            menuButtonGroup->addButton(button, i);
            topLayout->addWidget(button);
        }
        
        QWidget *contentPanel = new QWidget();
        contentPanel->setAutoFillBackground(true);
        
        QPalette contentPalette = contentPanel->palette();
        contentPalette.setColor(QPalette::Window, QColor(26, 26, 26));
        contentPanel->setPalette(contentPalette);
        
        QStackedWidget *contentStack = new QStackedWidget();
        
        auto createPage = [](const QString &title, const QString &description) {
            QWidget *page = new QWidget();
            QVBoxLayout *layout = new QVBoxLayout(page);
            layout->setContentsMargins(40, 40, 40, 40);
            layout->setSpacing(20);
            
            QLabel *titleLabel = new QLabel(title);
            titleLabel->setStyleSheet("color: white; font-size: 28px; font-weight: bold;");
            titleLabel->setAlignment(Qt::AlignCenter);
            layout->addWidget(titleLabel);
            
            QLabel *descLabel = new QLabel(description);
            descLabel->setStyleSheet("color: #cccccc; font-size: 16px; line-height: 1.5;");
            descLabel->setWordWrap(true);
            descLabel->setAlignment(Qt::AlignCenter);
            layout->addWidget(descLabel);
            
            layout->addStretch();
            return page;
        };
        
        QWidget *welcomePage = new QWidget();
        QVBoxLayout *welcomeLayout = new QVBoxLayout(welcomePage);
        welcomeLayout->setContentsMargins(40, 40, 40, 40);
        welcomeLayout->setSpacing(30);
        
        QLabel *logoLabel = new QLabel();
        QStringList logoPaths = {
            "src/images/NixlyOS_logo.png",           // From build directory
            "../src/images/NixlyOS_logo.png",        // From build subdirectory
            "images/NixlyOS_logo.png",               // Relative to src
            "/home/total/nixlyinstall/src/images/NixlyOS_logo.png"  // Absolute path
        };
        
        QPixmap logo;
        bool logoLoaded = false;
        for (const QString &path : logoPaths) {
            logo.load(path);
            if (!logo.isNull()) {
                logoLoaded = true;
                break;
            }
        }
        
        if (logoLoaded) {
            QPixmap scaledLogo = logo.scaled(320, 320, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            logoLabel->setPixmap(scaledLogo);
        } else {
            logoLabel->setText("NixlyOS");
            logoLabel->setStyleSheet("color: #0078D4; font-size: 32px; font-weight: bold;");
        }
        logoLabel->setAlignment(Qt::AlignCenter);
        welcomeLayout->addWidget(logoLabel);
        
        QLabel *welcomeTitleLabel = new QLabel("Welcome to NixlyOS");
        welcomeTitleLabel->setStyleSheet("color: white; font-size: 28px; font-weight: bold;");
        welcomeTitleLabel->setAlignment(Qt::AlignCenter);
        welcomeLayout->addWidget(welcomeTitleLabel);
        
        QLabel *welcomeDescLabel = new QLabel("Welcome to the NixlyOS installation wizard. This installer will guide you through "
            "setting up your new NixlyOS system with all the necessary configurations. ");
        welcomeDescLabel->setStyleSheet("color: #cccccc; font-size: 16px; line-height: 1.5;");
        welcomeDescLabel->setWordWrap(true);
        welcomeDescLabel->setAlignment(Qt::AlignCenter);
        welcomeLayout->addWidget(welcomeDescLabel);

        // Strong security/stability note (bold) under the description
        QLabel *welcomeSecurityNote = new QLabel(
            "<b>Before we begin, we want to be crystal clear: NixlyOS uses every method available to make your system and computer as secure and stable as possible — including Secure Boot, strict control of open ports, strict control of incoming traffic, isolation of every package, limited access to change/edit files, limited information shared through the browser and full encryption of all partitions. If this is not acceptable, you should not start the installation of NixlyOS.</b>");
        welcomeSecurityNote->setTextFormat(Qt::RichText);
        welcomeSecurityNote->setWordWrap(true);
        welcomeSecurityNote->setStyleSheet("color: #e6e6e6; font-size: 16px; line-height: 1.5; font-weight: bold;");
        welcomeSecurityNote->setAlignment(Qt::AlignCenter);
        welcomeLayout->addWidget(welcomeSecurityNote);
        
        QPushButton *letsStartButton = new QPushButton("Let's start!");
        letsStartButton->setMinimumHeight(50);
        letsStartButton->setMaximumWidth(200);
        letsStartButton->setStyleSheet(
            "QPushButton {"
            "    background-color: #0078D4;"
            "    color: white;"
            "    border: none;"
            "    border-radius: 8px;"
            "    padding: 12px 24px;"
            "    font-size: 16px;"
            "    font-weight: bold;"
            "}"
            "QPushButton:hover {"
            "    background-color: #106EBE;"
            "}"
            "QPushButton:pressed {"
            "    background-color: #005A9E;"
            "}"
        );
        
        QHBoxLayout *buttonLayout = new QHBoxLayout();
        buttonLayout->addStretch();
        buttonLayout->addWidget(letsStartButton);
        buttonLayout->addStretch();
        welcomeLayout->addLayout(buttonLayout);
        
        welcomeLayout->addStretch();
        
        QWidget *internetPage = new QWidget();
        QVBoxLayout *internetLayout = new QVBoxLayout(internetPage);
        internetLayout->setContentsMargins(40, 40, 40, 40);
        internetLayout->setSpacing(20);
        
        QLabel *internetTitle = new QLabel("Internet Connection");
        internetTitle->setStyleSheet("color: white; font-size: 28px; font-weight: bold;");
        internetTitle->setAlignment(Qt::AlignCenter);
        internetLayout->addWidget(internetTitle);
        
        QLabel *connectionStatus = new QLabel("No internet access");
        connectionStatus->setStyleSheet("color: #FF6B6B; font-size: 16px;");
        connectionStatus->setAlignment(Qt::AlignCenter);
        internetLayout->addWidget(connectionStatus);
        
        // Removed unused ethernet status label
        
        // Wi‑Fi UI removed; only internet status and continue button are shown
        
        QPushButton *continueButton = new QPushButton("Perfect! Let's continue!");
        continueButton->setMinimumHeight(40);
        continueButton->setMaximumWidth(260);
        continueButton->setStyleSheet(
            "QPushButton {"
            "    background-color: #0078D4;"
            "    color: white;"
            "    border: none;"
            "    border-radius: 8px;"
            "    padding: 12px 24px;"
            "    font-size: 16px;"
            "    font-weight: bold;"
            "}"
            "QPushButton:hover { background-color: #106EBE; }"
            "QPushButton:pressed { background-color: #005A9E; }"
        );
        continueButton->hide();
        
        QHBoxLayout *continueLayout = new QHBoxLayout();
        continueLayout->addStretch();
        continueLayout->addWidget(continueButton);
        continueLayout->addStretch();
        internetLayout->addLayout(continueLayout);
        
        internetLayout->addStretch();

        // network manager instance
        netManager = new QNetworkAccessManager(this);
        
        // Function to check actual internet connectivity (HTTP, multiple endpoints, no TLS)
        std::function<void(QLabel*, QPushButton*)> checkInternetConnectivity;
        checkInternetConnectivity = [this, contentStack](QLabel* statusLabel, QPushButton* contButton) mutable {
            if (isCheckingInternet) return; // Prevent multiple simultaneous checks

            isCheckingInternet = true;

            const QList<QUrl> urls = {
                QUrl("http://connectivitycheck.gstatic.com/generate_204"),
                QUrl("http://clients3.google.com/generate_204"),
                QUrl("http://example.com/")
            };

            // recursive-like sequence using shared lambda
            auto tryIndex = std::make_shared<std::function<void(int)>>();
            *tryIndex = [=, this](int idx) mutable {
                if (idx >= urls.size()) {
                    // Final failure
                    if (contentStack->currentIndex() == 1 && statusLabel && contButton) {
                        statusLabel->setText("No internet access");
                        statusLabel->setStyleSheet("color: #FF6B6B; font-size: 16px;");
                        contButton->hide();
                    }
                    isCheckingInternet = false;
                    return;
                }

                QNetworkRequest request(urls[idx]);
                request.setRawHeader("User-Agent", "NixlyInstall");
                QNetworkReply *reply = netManager->get(request);

                QTimer *to = new QTimer(reply);
                to->setSingleShot(true);
                to->start(2000);
                QObject::connect(to, &QTimer::timeout, reply, &QNetworkReply::abort);

                connect(reply, &QNetworkReply::finished, this, [=, this]() mutable {
                    // If user left the Internet page, just end
                    if (contentStack->currentIndex() != 1 || !statusLabel || !contButton) {
                        reply->deleteLater();
                        isCheckingInternet = false;
                        return;
                    }
                    const int http = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
                    const bool ok = (reply->error() == QNetworkReply::NoError) && (http == 204 || http == 200);
                    reply->deleteLater();
                    if (ok) {
                        statusLabel->setText("✓ Internet access");
                        statusLabel->setStyleSheet("color: #00AA00; font-size: 16px; font-weight: bold;");
                        contButton->show();
                        isCheckingInternet = false;
                    } else {
                        // Try next endpoint
                        (*tryIndex)(idx + 1);
                    }
                });
            };

            // Kick off first endpoint
            (*tryIndex)(0);
        };
        
        // Auto-refresh timer - faster for responsive updates, balanced to avoid races
        refreshTimer = new QTimer(this);
        refreshTimer->setInterval(800); // 0.8s

        /*
        // Switch to wpa_supplicant only (via wpa_cli) for Wi‑Fi scan/connect

        // Wi‑Fi helpers removed

        /*
        // Helper: connect to Wi‑Fi via wpa_cli on a specific interface
        std::function<void(const QString&, const QString&, const QString&, bool)> connectWithWpaCli;
        connectWithWpaCli = [this, connectionStatus, continueButton, checkInternetConnectivity, sudoPath, wpaCliPath, networkctlPath, dhclientPath, udhcpcPath, busyboxPath](const QString &ifname, const QString &ssid, const QString &password, bool secured) mutable {
            auto runCmd = [this, sudoPath](const QString &cmd, const QStringList &args, std::function<void(int, const QString&, const QString&)> cb) {
                std::function<void(const QString&, bool)> startWith;
                startWith = [=, this, &startWith](const QString &pwd, bool allowRetry) mutable {
                    QProcess *p = new QProcess(this);
                    QObject::connect(p, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [=, this](int exitCode, QProcess::ExitStatus){
                        const QString out = QString::fromUtf8(p->readAllStandardOutput());
                        const QString err = QString::fromUtf8(p->readAllStandardError());
                        p->deleteLater();
                        const bool needsPw = err.contains("password", Qt::CaseInsensitive) || err.contains("try again", Qt::CaseInsensitive);
                        if (exitCode != 0 && needsPw && allowRetry) {
                            bool ok = false;
                            QString pw = QInputDialog::getText(nullptr, "Sudo password", "Enter sudo password:", QLineEdit::Password, QString(), &ok);
                            if (ok && !pw.isEmpty()) {
                                const_cast<MainWindow*>(this)->sudoPassword = pw;
                                startWith(pw, false);
                                return;
                            }
                        }
                        cb(exitCode, out, err);
                    });
                    QStringList fullArgs; fullArgs << "-S" << cmd; fullArgs << args;
                    p->start(sudoPath, fullArgs);
                    QByteArray pwb = (pwd + "\n").toUtf8();
                    p->write(pwb);
                    p->closeWriteChannel();
                };
                const QString pw = sudoPassword; // may be empty (press Enter)
                startWith(pw, true);
            };
            
            // Acquire DHCP on the given interface using a few fallbacks
            auto acquireDhcp = [=]() {
                connectionStatus->setText("Obtaining IP via DHCP...");
                connectionStatus->setStyleSheet("color: #FFAA00; font-size: 16px;");
                auto tryNetworkctl = [=](std::function<void(bool)> next){
                    if (!networkctlPath.isEmpty()) {
                        runCmd(networkctlPath, QStringList() << "reload", [=](int, const QString&, const QString&){
                            runCmd(networkctlPath, QStringList() << "renew" << ifname, [=](int rc, const QString&, const QString&){ next(rc == 0); });
                        });
                    } else next(false);
                };
                auto tryDhclient = [=](std::function<void(bool)> next){
                    if (!dhclientPath.isEmpty()) {
                        runCmd(dhclientPath, QStringList() << "-v" << "-r" << ifname, [=](int, const QString&, const QString&){
                            runCmd(dhclientPath, QStringList() << "-v" << ifname, [=](int rc, const QString&, const QString&){ next(rc == 0); });
                        });
                    } else next(false);
                };
                auto tryUdhcpc = [=](std::function<void(bool)> next){
                    if (!udhcpcPath.isEmpty()) {
                        runCmd(udhcpcPath, QStringList() << "-i" << ifname << "-q" << "-t" << "3" << "-n", [=](int rc, const QString&, const QString&){ next(rc == 0); });
                    } else if (!busyboxPath.isEmpty()) {
                        runCmd(busyboxPath, QStringList() << "udhcpc" << "-i" << ifname << "-q" << "-t" << "3" << "-n", [=](int rc, const QString&, const QString&){ next(rc == 0); });
                    } else next(false);
                };
                tryNetworkctl([=](bool ok1){
                    if (ok1) { checkInternetConnectivity(connectionStatus, continueButton); return; }
                    tryDhclient([=](bool ok2){
                        if (ok2) { checkInternetConnectivity(connectionStatus, continueButton); return; }
                        tryUdhcpc([=](bool ok3){
                            if (ok3) { checkInternetConnectivity(connectionStatus, continueButton); return; }
                            connectionStatus->setText("DHCP failed. Checking internet anyway...");
                            connectionStatus->setStyleSheet("color: #FFAA00; font-size: 16px;");
                            checkInternetConnectivity(connectionStatus, continueButton);
                        });
                    });
                });
            };

            // 1) add_network
            runCmd(wpaCliPath, QStringList() << "-i" << ifname << "add_network", [=, this](int rc1, const QString &out1, const QString &){
                if (rc1 != 0) {
                    connectionStatus->setText("wpa_cli failed (add_network). Is wpa_supplicant running?");
                    connectionStatus->setStyleSheet("color: #FF6B6B; font-size: 16px;");
                    return;
                }
                QString netId = out1.trimmed();
                if (netId.isEmpty()) netId = "0"; // best-effort

                // 2) set ssid
                const QString quotedSsid = QString("\"%1\"").arg(ssid);
                runCmd(wpaCliPath, QStringList() << "-i" << ifname << "set_network" << netId << "ssid" << quotedSsid, [=, this](int rc2, const QString &, const QString &){
                    if (rc2 != 0) {
                        connectionStatus->setText("wpa_cli failed (set ssid)");
                        connectionStatus->setStyleSheet("color: #FF6B6B; font-size: 16px;");
                        return;
                    }
                    // 3) security params
                    auto enableSelectSave = [=, this](const QString &nid){
                        runCmd(wpaCliPath, QStringList() << "-i" << ifname << "enable_network" << nid, [=, this](int rc4, const QString &, const QString &){
                            if (rc4 != 0) {
                                connectionStatus->setText("wpa_cli failed (enable network)");
                                connectionStatus->setStyleSheet("color: #FF6B6B; font-size: 16px;");
                                return;
                            }
                            runCmd(wpaCliPath, QStringList() << "-i" << ifname << "select_network" << nid, [=, this](int rc5, const QString &, const QString &){
                                if (rc5 != 0) {
                                    connectionStatus->setText("wpa_cli failed (select network)");
                                    connectionStatus->setStyleSheet("color: #FF6B6B; font-size: 16px;");
                                    return;
                                }
                                runCmd(wpaCliPath, QStringList() << "-i" << ifname << "save_config", [=, this](int, const QString &, const QString &){
                                    connectionStatus->setText("Connected to " + ssid + ".");
                                    connectionStatus->setStyleSheet("color: #00AA00; font-size: 16px;");
                                    acquireDhcp();
                                });
                            });
                        });
                    };

                    if (secured) {
                        const QString quotedPsk = QString("\"%1\"").arg(password);
                        runCmd(wpaCliPath, QStringList() << "-i" << ifname << "set_network" << netId << "psk" << quotedPsk, [=, this](int rc3, const QString &, const QString &){
                            if (rc3 != 0) {
                                connectionStatus->setText("wpa_cli failed (set psk)");
                                connectionStatus->setStyleSheet("color: #FF6B6B; font-size: 16px;");
                                return;
                            }
                            enableSelectSave(netId);
                        });
                    } else {
                        // Open network
                        runCmd(wpaCliPath, QStringList() << "-i" << ifname << "set_network" << netId << "key_mgmt" << "NONE", [=, this](int rc3, const QString &, const QString &){
                            if (rc3 != 0) {
                                connectionStatus->setText("wpa_cli failed (set key_mgmt)");
                                connectionStatus->setStyleSheet("color: #FF6B6B; font-size: 16px;");
                                return;
                            }
                            enableSelectSave(netId);
                        });
                    }
                });
            });
        };
        */

        // Helper to rescan and populate available Wi‑Fi networks safely
        // Wi‑Fi scanning and connection code removed
        /*
            if (wifiScanInProgress) {
                return; // avoid overlapping scans
            }
            wifiScanInProgress = true;
            // Show container and clear previous entries
            wifiContainer->show();
            while (QLayoutItem *child = wifiListLayout->takeAt(0)) {
                if (child->widget()) child->widget()->deleteLater();
                delete child;
            }
            // 1) Gather Wi‑Fi interfaces from the OS
            QStringList wifiIfaces;
            const auto ifaces = QNetworkInterface::allInterfaces();
            for (const QNetworkInterface &iface : ifaces) {
                if (iface.type() == QNetworkInterface::Wifi) {
                    wifiIfaces << iface.name();
                }
            }

            if (wifiIfaces.isEmpty()) {
                connectionStatus->setText("No Wi‑Fi devices found.");
                connectionStatus->setStyleSheet("color: #FF6B6B; font-size: 16px;");
                wifiScanInProgress = false;
                return;
            }

            // 2) Iterate devices: wpa_cli scan + scan_results until we find networks
            std::shared_ptr<std::function<void(int)>> scanNext = std::make_shared<std::function<void(int)>>();
            *scanNext = [=, this](int idx) mutable {
                    if (idx >= wifiIfaces.size()) {
                        connectionStatus->setText("No WiFi networks found. Retrying...");
                        connectionStatus->setStyleSheet("color: #FFAA00; font-size: 16px;");
                        wifiScanInProgress = false;
                        return;
                    }
                    const QString iface = wifiIfaces[idx];
                    // Update status and run scan on this iface
                    connectionStatus->setText(QString("Scanning Wi‑Fi on %1...").arg(iface));
                    connectionStatus->setStyleSheet("color: #FFAA00; font-size: 16px;");
                    // Ensure interface is up before scanning (rfkill unblock + ip link up)
                    QProcess *scanProc = new QProcess(this);
                    QObject::connect(scanProc, &QProcess::started, this, [=, this]() {
                        // First try sending just Enter to sudo -S
                        scanProc->write("\n");
                        scanProc->closeWriteChannel();
                    });
                    QObject::connect(scanProc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [=, this](int exitCode, QProcess::ExitStatus){
                        const QString out = QString::fromUtf8(scanProc->readAllStandardOutput());
                        const QString err = QString::fromUtf8(scanProc->readAllStandardError());
                        scanProc->deleteLater();
                        if (exitCode != 0) {
                            connectionStatus->setText(QString("Scan failed on %1: %2").arg(iface, err.left(120)));
                            connectionStatus->setStyleSheet("color: #FF6B6B; font-size: 16px;");
                            // If sudo password is required, prompt and retry once
                            if (err.contains("password", Qt::CaseInsensitive) || err.contains("try again", Qt::CaseInsensitive)) {
                                bool ok = false;
                                QString pw = QInputDialog::getText(nullptr, "Sudo password", QString("Enter sudo password for scanning %1:").arg(iface), QLineEdit::Password, QString(), &ok);
                                if (ok && !pw.isEmpty()) {
                                    const_cast<MainWindow*>(this)->sudoPassword = pw;
                                    QProcess *scanPw = new QProcess(this);
                                    QObject::connect(scanPw, &QProcess::started, this, [=, this]() {
                                        scanPw->write((pw + "\n").toUtf8());
                                        scanPw->closeWriteChannel();
                                    });
                                    QObject::connect(scanPw, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [=, this](int code2, QProcess::ExitStatus){
                                        const QString out2 = QString::fromUtf8(scanPw->readAllStandardOutput());
                                        const QString err2 = QString::fromUtf8(scanPw->readAllStandardError());
                                        scanPw->deleteLater();
                                        if (code2 != 0) {
                                            connectionStatus->setText(QString("Scan failed on %1: %2").arg(iface, err2.left(120)));
                                            connectionStatus->setStyleSheet("color: #FF6B6B; font-size: 16px;");
                                            QTimer::singleShot(900, this, [=, this]() { (*scanNext)(idx + 1); });
                                            return;
                                        }
                                        // Parse output (out2)
                                        QMap<QString, bool> ssidSecured; QString currentSsid; bool currentSecured = false;
                                        const QStringList lines = out2.split('\n');
                                        auto flushCurrent = [&](){ if (!currentSsid.isEmpty()) { bool prev = ssidSecured.value(currentSsid, false); ssidSecured[currentSsid] = prev || currentSecured; currentSsid.clear(); currentSecured = false; } };
                                        for (QString line : lines) { line = line.trimmed(); if (line.startsWith("BSS ")) { flushCurrent(); continue; } if (line.startsWith("SSID:")) { currentSsid = line.mid(5).trimmed(); continue; } if (line.startsWith("RSN:") || line.startsWith("WPA:")) { currentSecured = true; continue; } }
                                        flushCurrent();
                                        QStringList wifiNetworks; for (auto it = ssidSecured.constBegin(); it != ssidSecured.constEnd(); ++it) { const QString &ssid = it.key(); if (ssid.isEmpty() || ssid == "<hidden>") continue; const bool secured = it.value(); wifiNetworks << (ssid + (secured ? " (Secured)" : " (Open)")); }
                                        if (wifiNetworks.isEmpty()) { connectionStatus->setText(QString("No Wi‑Fi networks found on %1").arg(iface)); connectionStatus->setStyleSheet("color: #FFAA00; font-size: 16px;"); QTimer::singleShot(900, this, [=, this]() { (*scanNext)(idx + 1); }); return; }
                                        connectionStatus->setText(QString("Found %1 WiFi networks:").arg(wifiNetworks.size()));
                                        connectionStatus->setStyleSheet("color: #FFAA00; font-size: 16px;");
                                        for (const QString &network : wifiNetworks) {
                                            QPushButton *wifiButton = new QPushButton(network);
                                            wifiButton->setStyleSheet("QPushButton { background-color: #2A2A2A; color: white; border: 1px solid #444444; border-radius: 5px; padding: 12px; font-size: 14px; text-align: left; }" "QPushButton:hover { background-color: #3A3A3A; }" "QPushButton:pressed { background-color: #1A1A1A; }");
                                            QString ssid = network; bool secured = false; if (ssid.endsWith(" (Secured)")) { ssid.chop(QString(" (Secured)").size()); secured = true; } else if (ssid.endsWith(" (Open)")) { ssid.chop(QString(" (Open)").size()); }
                                            wifiButton->setProperty("ssid", ssid); wifiButton->setProperty("secured", secured); wifiButton->setProperty("ifname", iface);
                                            connect(wifiButton, &QPushButton::clicked, this, [=, this]() { const QString ssidClicked = wifiButton->property("ssid").toString(); const bool needPassword = wifiButton->property("secured").toBool(); const QString ifn = wifiButton->property("ifname").toString(); if (needPassword) { passwordLabel->setText("Enter password for " + ssidClicked + ":"); passwordLabel->setProperty("ssid", ssidClicked); passwordLabel->setProperty("ifname", ifn); passwordContainer->show(); } else { connectionStatus->setText("Connecting to " + ssidClicked + "..."); connectionStatus->setStyleSheet("color: #FFAA00; font-size: 16px;"); connectWithWpaCli(ifn, ssidClicked, QString(), false); } });
                                            wifiListLayout->addWidget(wifiButton);
                                        }
                                        wifiScanInProgress = false;
                                    });
                                    scanPw->start(sudoPath, QStringList() << "-S" << iwPath << "dev" << iface << "scan");
                                    return;
                                }
                            }
                            // Try a fallback without sudo (in case capabilities allow it)
                            QProcess *scan2 = new QProcess(this);
                            QObject::connect(scan2, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [=, this](int ec2, QProcess::ExitStatus){
                                const QString out2 = QString::fromUtf8(scan2->readAllStandardOutput());
                                const QString err2 = QString::fromUtf8(scan2->readAllStandardError());
                                scan2->deleteLater();
                                if (ec2 != 0) {
                                    // Try pkexec as a last resort (may show polkit dialog on Live ISO)
                                    QProcess *scan3 = new QProcess(this);
                                    QObject::connect(scan3, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [=, this](int ec3, QProcess::ExitStatus){
                                        const QString out3 = QString::fromUtf8(scan3->readAllStandardOutput());
                                        const QString err3 = QString::fromUtf8(scan3->readAllStandardError());
                                        scan3->deleteLater();
                                        if (ec3 != 0) {
                                            // Give up on this iface; try next after a short backoff
                                            connectionStatus->setText(QString("Scan failed on %1: %2").arg(iface, err3.left(120)));
                                            connectionStatus->setStyleSheet("color: #FF6B6B; font-size: 16px;");
                                            QTimer::singleShot(900, this, [=, this]() { (*scanNext)(idx + 1); });
                                            return;
                                        }
                                        // Parse out3
                                        QMap<QString, bool> ssidSecured; QString currentSsid; bool currentSecured = false;
                                        const QStringList lines3 = out3.split('\n');
                                        auto flushCurrent3 = [&](){ if (!currentSsid.isEmpty()) { bool prev = ssidSecured.value(currentSsid, false); ssidSecured[currentSsid] = prev || currentSecured; currentSsid.clear(); currentSecured = false; } };
                                        for (QString line : lines3) { line = line.trimmed(); if (line.startsWith("BSS ")) { flushCurrent3(); continue; } if (line.startsWith("SSID:")) { currentSsid = line.mid(5).trimmed(); continue; } if (line.startsWith("RSN:") || line.startsWith("WPA:")) { currentSecured = true; continue; } }
                                        flushCurrent3();
                                        QStringList wifiNetworks; for (auto it = ssidSecured.constBegin(); it != ssidSecured.constEnd(); ++it) { const QString &ssid = it.key(); if (ssid.isEmpty() || ssid == "<hidden>") continue; const bool secured = it.value(); wifiNetworks << (ssid + (secured ? " (Secured)" : " (Open)")); }
                                        if (wifiNetworks.isEmpty()) { connectionStatus->setText(QString("No Wi‑Fi networks found on %1").arg(iface)); connectionStatus->setStyleSheet("color: #FFAA00; font-size: 16px;"); QTimer::singleShot(900, this, [=, this]() { (*scanNext)(idx + 1); }); return; }
                                        connectionStatus->setText(QString("Found %1 WiFi networks:").arg(wifiNetworks.size()));
                                        connectionStatus->setStyleSheet("color: #FFAA00; font-size: 16px;");
                                        for (const QString &network : wifiNetworks) {
                                            QPushButton *wifiButton = new QPushButton(network);
                                            wifiButton->setStyleSheet("QPushButton { background-color: #2A2A2A; color: white; border: 1px solid #444444; border-radius: 5px; padding: 12px; font-size: 14px; text-align: left; }" "QPushButton:hover { background-color: #3A3A3A; }" "QPushButton:pressed { background-color: #1A1A1A; }");
                                            QString ssid = network; bool secured = false; if (ssid.endsWith(" (Secured)")) { ssid.chop(QString(" (Secured)").size()); secured = true; } else if (ssid.endsWith(" (Open)")) { ssid.chop(QString(" (Open)").size()); }
                                            wifiButton->setProperty("ssid", ssid); wifiButton->setProperty("secured", secured); wifiButton->setProperty("ifname", iface);
                                            connect(wifiButton, &QPushButton::clicked, this, [=, this]() { const QString ssidClicked = wifiButton->property("ssid").toString(); const bool needPassword = wifiButton->property("secured").toBool(); const QString ifn = wifiButton->property("ifname").toString(); if (needPassword) { passwordLabel->setText("Enter password for " + ssidClicked + ":"); passwordLabel->setProperty("ssid", ssidClicked); passwordLabel->setProperty("ifname", ifn); passwordContainer->show(); } else { connectionStatus->setText("Connecting to " + ssidClicked + "..."); connectionStatus->setStyleSheet("color: #FFAA00; font-size: 16px;"); connectWithWpaCli(ifn, ssidClicked, QString(), false); } });
                                            wifiListLayout->addWidget(wifiButton);
                                        }
                                        wifiScanInProgress = false;
                                    });
                                    scan3->start(pkexecPath, QStringList() << iwPath << "dev" << iface << "scan");
                                    return;
                                }
                                // Reuse same parsing path with out2
                                QMap<QString, bool> ssidSecured; QString currentSsid; bool currentSecured = false;
                                const QStringList lines2 = out2.split('\n');
                                auto flushCurrent2 = [&](){ if (!currentSsid.isEmpty()) { bool prev = ssidSecured.value(currentSsid, false); ssidSecured[currentSsid] = prev || currentSecured; currentSsid.clear(); currentSecured = false; } };
                                for (QString line : lines2) { line = line.trimmed(); if (line.startsWith("BSS ")) { flushCurrent2(); continue; } if (line.startsWith("SSID:")) { currentSsid = line.mid(5).trimmed(); continue; } if (line.startsWith("RSN:") || line.startsWith("WPA:")) { currentSecured = true; continue; } }
                                flushCurrent2();
                                QStringList wifiNetworks; for (auto it = ssidSecured.constBegin(); it != ssidSecured.constEnd(); ++it) { const QString &ssid = it.key(); if (ssid.isEmpty() || ssid == "<hidden>") continue; const bool secured = it.value(); wifiNetworks << (ssid + (secured ? " (Secured)" : " (Open)")); }
                                if (wifiNetworks.isEmpty()) {
                                    connectionStatus->setText(QString("No Wi‑Fi networks found on %1").arg(iface));
                                    connectionStatus->setStyleSheet("color: #FFAA00; font-size: 16px;");
                                    QTimer::singleShot(900, this, [=, this]() { (*scanNext)(idx + 1); });
                                    return;
                                }
                                connectionStatus->setText(QString("Found %1 WiFi networks:").arg(wifiNetworks.size()));
                                connectionStatus->setStyleSheet("color: #FFAA00; font-size: 16px;");
                                for (const QString &network : wifiNetworks) {
                                    QPushButton *wifiButton = new QPushButton(network);
                                    wifiButton->setStyleSheet("QPushButton { background-color: #2A2A2A; color: white; border: 1px solid #444444; border-radius: 5px; padding: 12px; font-size: 14px; text-align: left; }" "QPushButton:hover { background-color: #3A3A3A; }" "QPushButton:pressed { background-color: #1A1A1A; }");
                                    QString ssid = network; bool secured = false; if (ssid.endsWith(" (Secured)")) { ssid.chop(QString(" (Secured)").size()); secured = true; } else if (ssid.endsWith(" (Open)")) { ssid.chop(QString(" (Open)").size()); }
                                    wifiButton->setProperty("ssid", ssid); wifiButton->setProperty("secured", secured); wifiButton->setProperty("ifname", iface);
                                    connect(wifiButton, &QPushButton::clicked, this, [=, this]() { const QString ssidClicked = wifiButton->property("ssid").toString(); const bool needPassword = wifiButton->property("secured").toBool(); const QString ifn = wifiButton->property("ifname").toString(); if (needPassword) { passwordLabel->setText("Enter password for " + ssidClicked + ":"); passwordLabel->setProperty("ssid", ssidClicked); passwordLabel->setProperty("ifname", ifn); passwordContainer->show(); } else { connectionStatus->setText("Connecting to " + ssidClicked + "..."); connectionStatus->setStyleSheet("color: #FFAA00; font-size: 16px;"); connectWithWpaCli(ifn, ssidClicked, QString(), false); } });
                                    wifiListLayout->addWidget(wifiButton);
                                }
                                wifiScanInProgress = false;
                            });
                            scan2->start(iwPath, QStringList() << "dev" << iface << "scan");
                            return;
                        }
                        // Parse `iw <iface> scan` output
                        QMap<QString, bool> ssidSecured; // ssid -> secured
                        QString currentSsid;
                        bool currentSecured = false;
                        const QStringList lines = out.split('\n');
                        auto flushCurrent = [&](){
                            if (!currentSsid.isEmpty()) {
                                bool prev = ssidSecured.value(currentSsid, false);
                                ssidSecured[currentSsid] = prev || currentSecured;
                                currentSsid.clear();
                                currentSecured = false;
                            }
                        };
                        for (QString line : lines) {
                            line = line.trimmed();
                            if (line.startsWith("BSS ")) { flushCurrent(); continue; }
                            if (line.startsWith("SSID:")) {
                                currentSsid = line.mid(QString("SSID:").size()).trimmed();
                                continue;
                            }
                            if (line.startsWith("RSN:") || line.startsWith("WPA:")) { currentSecured = true; continue; }
                        }
                        flushCurrent();

                        QStringList wifiNetworks;
                        for (auto it = ssidSecured.constBegin(); it != ssidSecured.constEnd(); ++it) {
                            const QString &ssid = it.key();
                            if (ssid.isEmpty() || ssid == "<hidden>") continue;
                            const bool secured = it.value();
                            wifiNetworks << (ssid + (secured ? " (Secured)" : " (Open)"));
                        }

                        if (wifiNetworks.isEmpty()) {
                            connectionStatus->setText(QString("No Wi‑Fi networks found on %1").arg(iface));
                            connectionStatus->setStyleSheet("color: #FFAA00; font-size: 16px;");
                            QTimer::singleShot(900, this, [=, this]() { (*scanNext)(idx + 1); });
                            return;
                        }

                        connectionStatus->setText(QString("Found %1 WiFi networks:").arg(wifiNetworks.size()));
                        connectionStatus->setStyleSheet("color: #FFAA00; font-size: 16px;");
                        for (const QString &network : wifiNetworks) {
                            QPushButton *wifiButton = new QPushButton(network);
                            wifiButton->setStyleSheet(
                                "QPushButton { background-color: #2A2A2A; color: white; border: 1px solid #444444; border-radius: 5px; padding: 12px; font-size: 14px; text-align: left; }"
                                "QPushButton:hover { background-color: #3A3A3A; }"
                                "QPushButton:pressed { background-color: #1A1A1A; }"
                            );
                            QString ssid = network;
                            bool secured = false;
                            if (ssid.endsWith(" (Secured)")) { ssid.chop(QString(" (Secured)").size()); secured = true; }
                            else if (ssid.endsWith(" (Open)")) { ssid.chop(QString(" (Open)").size()); }
                            wifiButton->setProperty("ssid", ssid);
                            wifiButton->setProperty("secured", secured);
                            wifiButton->setProperty("ifname", iface);

                            connect(wifiButton, &QPushButton::clicked, this, [=, this]() {
                                const QString ssidClicked = wifiButton->property("ssid").toString();
                                const bool needPassword = wifiButton->property("secured").toBool();
                                const QString ifn = wifiButton->property("ifname").toString();
                                if (needPassword) {
                                    passwordLabel->setText("Enter password for " + ssidClicked + ":");
                                    passwordLabel->setProperty("ssid", ssidClicked);
                                    passwordLabel->setProperty("ifname", ifn);
                                    passwordContainer->show();
                                } else {
                                    connectionStatus->setText("Connecting to " + ssidClicked + "...");
                                    connectionStatus->setStyleSheet("color: #FFAA00; font-size: 16px;");
                                    connectWithWpaCli(ifn, ssidClicked, QString(), false);
                                }
                            });
                            wifiListLayout->addWidget(wifiButton);
                        }
                        wifiScanInProgress = false;
                    });
                    // Bring interface up (ignore result), then scan
                    QProcess *prep = new QProcess(this);
                    QObject::connect(prep, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [=, this](int, QProcess::ExitStatus){
                        prep->deleteLater();
                        QTimer::singleShot(150, this, [=, this]() {
                            scanProc->start(sudoPath, QStringList() << "-S" << iwPath << "dev" << iface << "scan");
                        });
                    });
                    // Try rfkill unblock and ip link up in one shell run
                    QString cmd = QString("sh");
                    QStringList args; 
                    args << "-c" 
                         << QString("%1 -S rfkill unblock wifi 2>/dev/null; %2 -S %3 link set %4 up 2>/dev/null")
                               .arg(sudoPath, sudoPath, ipPath, iface);
                    prep->start(cmd, args);
                };
                // Start with first interface
                (*scanNext)(0);
        };
        
        // Function to check internet connection only (no Wi‑Fi UI)
        std::function<void()> checkInternetConnection;
        checkInternetConnection = [this, contentStack, connectionStatus, continueButton, checkInternetConnectivity]() mutable {
            if (contentStack->currentIndex() != 1) return; // Only when Internet page visible
            // Keep UI minimal: just check HTTP connectivity continuously
            if (!isCheckingInternet) {
                checkInternetConnectivity(connectionStatus, continueButton);
            }
        }; // minimal internet-only checker

        // Connect refresh timer to check internet connection
        connect(refreshTimer, &QTimer::timeout, this, [this, checkInternetConnection]() {
            checkInternetConnection();
        });
        
        // Connect password input
        connect(connectButton, &QPushButton::clicked, this, [=, this]() {
            if (!passwordInput->text().isEmpty()) {
                QString selectedNetwork = passwordLabel->property("ssid").toString();
                if (selectedNetwork.isEmpty()) {
                    // fallback to previous logic
                    selectedNetwork = passwordLabel->text().split(" ").last().replace(":", "");
                }
                connectionStatus->setText("Connecting to " + selectedNetwork + "...");
                connectionStatus->setStyleSheet("color: #FFAA00; font-size: 16px;");
                
                // Use ifname captured during selection if available
                QString ifn = passwordLabel->property("ifname").toString();
                if (ifn.isEmpty()) {
                    // Try to infer from available Wi‑Fi interfaces
                    const auto ifaces = QNetworkInterface::allInterfaces();
                    for (const QNetworkInterface &iface : ifaces) {
                        if (iface.type() == QNetworkInterface::Wifi) { ifn = iface.name(); break; }
                    }
                }
                passwordContainer->hide();
                connectWithWpaCli(ifn, selectedNetwork, passwordInput->text(), true);
            }
        });
        
        // Connect continue button to enable GitHub page
        connect(continueButton, &QPushButton::clicked, this, [=, this]() {
            menuButtons[2]->setEnabled(true);  // Enable GitHub button
            menuButtons[2]->setChecked(true);  // Select GitHub button
            contentStack->setCurrentIndex(2);  // Navigate to GitHub page
        });
        
        // Start connection check after a short delay - but don't auto-start timer yet
        // QTimer::singleShot(1000, checkInternetConnection);
            
        */

        // Minimal internet-only checker and wiring
        std::function<void()> checkInternetConnection = [this, contentStack, connectionStatus, continueButton, checkInternetConnectivity]() mutable {
            if (contentStack->currentIndex() == 1 && !isCheckingInternet) {
                checkInternetConnectivity(connectionStatus, continueButton);
            }
        };
        connect(refreshTimer, &QTimer::timeout, this, [this, checkInternetConnection]() {
            checkInternetConnection();
        });
        connect(continueButton, &QPushButton::clicked, this, [=, this]() {
            menuButtons[2]->setEnabled(true);
            menuButtons[2]->setChecked(true);
            contentStack->setCurrentIndex(2);
        });

        QWidget *githubPage = new QWidget();
        {
            QVBoxLayout *ghLayout = new QVBoxLayout(githubPage);
            ghLayout->setContentsMargins(40, 40, 40, 40);
            ghLayout->setSpacing(16);

            QLabel *title = new QLabel("GitHub");
            title->setStyleSheet("color: white; font-size: 28px; font-weight: bold;");
            title->setAlignment(Qt::AlignCenter);
            ghLayout->addWidget(title);

            // Intro text under GitHub heading
            QLabel *intro = new QLabel(
                "With NixlyOS, it is a requirement that the entire system is declarative and reproducible. To achieve this, the entire system must be stored in a private github repo that only you have access to.\n\n"
                "This way, you can reinstall your system at any time or on any other PC and you will get exactly the same result, in fact down to every last detail such as package/driver/program versions.\n\n"
                "To ensure that no secrets are stored in your GitHub repo, we do not commit any encrypted secrets; instead, passwords and your GitHub token are configured after your first login.\n\n"
                "Follow the steps below:"
            );
            intro->setWordWrap(true);
            intro->setAlignment(Qt::AlignLeft | Qt::AlignTop);
            intro->setStyleSheet("color: #cccccc; font-size: 15px; line-height: 1.5;");
            ghLayout->addWidget(intro);
            ghLayout->addSpacing(24); // Extra space between intro text and Step 1

            // Step 1 card
            QWidget *step1Card = new QWidget();
            step1Card->setStyleSheet("QWidget { background-color: #232323; border: 1px solid #3A3A3A; border-radius: 8px; }");
            QVBoxLayout *step1Layout = new QVBoxLayout(step1Card);
            step1Layout->setContentsMargins(16, 16, 16, 16);
            step1Layout->setSpacing(10);

            QLabel *step1 = new QLabel("You can skip this step if you already have a GitHub account. If not, create one.");
            step1->setStyleSheet("color: #cccccc; font-size: 15px; line-height: 1.5;");
            step1->setWordWrap(true);
            step1->setAlignment(Qt::AlignLeft | Qt::AlignTop);
            step1Layout->addWidget(step1);

            QHBoxLayout *step1Actions = new QHBoxLayout();
            QPushButton *createAccountBtn = new QPushButton("Create GitHub account");
            createAccountBtn->setStyleSheet(
                "QPushButton { background-color: #2A2A2A; color: white; border: 1px solid #444444; border-radius: 8px; padding: 10px 16px; font-weight: bold; }"
                "QPushButton:hover { background-color: #3A3A3A; }"
            );
            step1Actions->addStretch();
            step1Actions->addWidget(createAccountBtn);
            step1Actions->addStretch();
            step1Layout->addLayout(step1Actions);

            // Row with numeric badge
            QHBoxLayout *step1Row = new QHBoxLayout();
            QLabel *badge1 = new QLabel("1");
            badge1->setFixedSize(32, 32);
            badge1->setAlignment(Qt::AlignCenter);
            badge1->setStyleSheet("QLabel { background-color: #0078D4; color: white; border-radius: 16px; font-size: 16px; font-weight: bold; }");
            step1Row->addWidget(badge1, 0, Qt::AlignTop);
            step1Row->addSpacing(10);
            step1Row->addWidget(step1Card, 1);
            ghLayout->addLayout(step1Row);
            ghLayout->addSpacing(24);

            // Step 2 card
            QWidget *step2Card = new QWidget();
            step2Card->setStyleSheet("QWidget { background-color: #232323; border: 1px solid #3A3A3A; border-radius: 8px; }");
            QVBoxLayout *step2Layout = new QVBoxLayout(step2Card);
            step2Layout->setContentsMargins(16, 16, 16, 16);
            step2Layout->setSpacing(10);

            QLabel *step2 = new QLabel("We now need to go through a Device Activation so that NixlyInstall gets temporary access to view/create your private repository for storing your entire NixlyOS system. When you click ‘Github activation’, we will display your one-time code, copy it to your clipboard, and automatically open the activation page in your browser.");
            step2->setStyleSheet("color: #cccccc; font-size: 15px; line-height: 1.5; margin-top: 2px;");
            step2->setWordWrap(true);
            step2->setAlignment(Qt::AlignLeft | Qt::AlignTop);
            step2Layout->addWidget(step2);

            // Status spinner (shown during login)
            QLabel *status = new QLabel("Klar til å logge inn med GitHub CLI.");
            status->setStyleSheet("color: #cccccc; font-size: 14px;");
            status->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

            QLabel *spinnerLbl = new QLabel("");
            // Make it white, 5x larger, and without any ring/border
            spinnerLbl->setStyleSheet("color: white; font-size: 80px; border: none; background: transparent;");
            spinnerLbl->setFrameStyle(QFrame::NoFrame);
            spinnerLbl->setFocusPolicy(Qt::NoFocus);
            spinnerLbl->setFixedWidth(80);
            spinnerLbl->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            spinnerLbl->hide();
            // Note: spinner is added next to the Avbryt button in the actions row

            // Show the exact one-time code sentence when captured
            QLabel *oneTimeMsg = new QLabel("");
            oneTimeMsg->setStyleSheet("color: #cccccc; font-size: 18px; font-weight: bold; letter-spacing: 2px;");
            oneTimeMsg->setAlignment(Qt::AlignCenter);
            oneTimeMsg->hide();
            step2Layout->addWidget(oneTimeMsg);

            // Dedicated copy button for the one-time code
            QHBoxLayout *oneTimeActions = new QHBoxLayout();
            QPushButton *oneTimeCopyBtn = new QPushButton("Kopier kode");
            oneTimeCopyBtn->setStyleSheet(
                "QPushButton { background-color: #2A2A2A; color: white; border: 1px solid #444444; border-radius: 5px; padding: 8px 12px; font-weight: bold; }"
                "QPushButton:hover { background-color: #3A3A3A; }"
            );
            oneTimeActions->addStretch();
            oneTimeActions->addWidget(oneTimeCopyBtn);
            oneTimeActions->addStretch();
            oneTimeCopyBtn->hide();
            step2Layout->addLayout(oneTimeActions);

            // Device code row
            QLabel *codeLabel = new QLabel("Engangskode (lim inn på github.com/login/device):");
            codeLabel->setStyleSheet("color: white; font-size: 14px; font-weight: bold;");
            ghLayout->addWidget(codeLabel);

            QHBoxLayout *codeRow = new QHBoxLayout();
            QLineEdit *deviceCodeEdit = new QLineEdit();
            deviceCodeEdit->setReadOnly(true);
            deviceCodeEdit->setPlaceholderText("Klikk ‘Generer kode (uten CLI)’ eller ‘Logg inn med GitHub CLI’ for å få koden...");
            deviceCodeEdit->setStyleSheet(
                "QLineEdit { background-color: #2A2A2A; color: #EEEEEE; border: 1px solid #444444; border-radius: 5px; padding: 8px; font-size: 18px; font-weight: bold; text-transform: uppercase; }"
            );
            QPushButton *copyCodeBtn = new QPushButton("Kopier kode");
            copyCodeBtn->setStyleSheet(
                "QPushButton { background-color: #2A2A2A; color: white; border: 1px solid #444444; border-radius: 5px; padding: 8px 12px; font-weight: bold; }"
                "QPushButton:hover { background-color: #3A3A3A; }"
            );
            codeRow->addWidget(deviceCodeEdit, 1);
            codeRow->addWidget(copyCodeBtn);
            ghLayout->addLayout(codeRow);

            QHBoxLayout *actions = new QHBoxLayout();
            QPushButton *openDevicePageBtn = new QPushButton("Åpne github.com/login/device");
            openDevicePageBtn->setStyleSheet(
                "QPushButton { background-color: #2A2A2A; color: white; border: 1px solid #444444; border-radius: 5px; padding: 10px 16px; font-weight: bold; }"
                "QPushButton:hover { background-color: #3A3A3A; }"
            );
            QPushButton *ghLoginBtn = new QPushButton("Github Activation");
            ghLoginBtn->setStyleSheet(
                "QPushButton { background-color: #0078D4; color: white; border: none; border-radius: 8px; padding: 12px 24px; font-size: 16px; font-weight: bold; }"
                "QPushButton:hover { background-color: #106EBE; }"
                "QPushButton:pressed { background-color: #005A9E; }"
            );
            // Success indicator to replace the GitHub Login button after approval
            QLabel *activationOkLabel = new QLabel("✅ Device Activation Successfull!");
            activationOkLabel->setStyleSheet("color: #00AA00; font-size: 16px; font-weight: bold;");
            activationOkLabel->setAlignment(Qt::AlignCenter);
            activationOkLabel->hide();
            QPushButton *ghCancelBtn = new QPushButton("Avbryt");
            ghCancelBtn->setStyleSheet(
                "QPushButton { background-color: #2A2A2A; color: white; border: 1px solid #444444; border-radius: 8px; padding: 12px 18px; font-size: 14px; font-weight: bold; }"
                "QPushButton:hover { background-color: #3A3A3A; }"
            );
            ghCancelBtn->hide();
            actions->addStretch();
            actions->addWidget(openDevicePageBtn);
            actions->addSpacing(12);
            actions->addWidget(ghLoginBtn);
            actions->addWidget(activationOkLabel);
            actions->addSpacing(12);
            actions->addWidget(ghCancelBtn);
            actions->addSpacing(8);
            actions->addWidget(spinnerLbl);
            actions->addStretch();
            step2Layout->addLayout(actions);

            

            // Hide everything except the single login button and status
            codeLabel->hide();
            deviceCodeEdit->hide();
            copyCodeBtn->hide();
            openDevicePageBtn->hide();

            // Info about requirements (moved to Step 2 card below)

            // Alternative: Device Flow uten CLI
            QLabel *altLabel = new QLabel("Alternativ: Generer kode uten CLI (OAuth Device Flow)");
            altLabel->setStyleSheet("color: white; font-size: 14px; font-weight: bold; margin-top: 8px;");
            altLabel->setAlignment(Qt::AlignCenter);
            ghLayout->addWidget(altLabel);

            QHBoxLayout *clientRow = new QHBoxLayout();
            QLineEdit *clientIdEdit = new QLineEdit();
            clientIdEdit->setPlaceholderText("GitHub OAuth Client ID (env: GITHUB_OAUTH_CLIENT_ID)");
            clientIdEdit->setStyleSheet("QLineEdit { background-color: #2A2A2A; color: white; border: 1px solid #444444; border-radius: 5px; padding: 8px; }");
            QString envClient = qEnvironmentVariable("GITHUB_OAUTH_CLIENT_ID");
            if (!envClient.isEmpty()) clientIdEdit->setText(envClient);
            QPushButton *genCodeBtn = new QPushButton("Generer kode (uten CLI)");
            genCodeBtn->setStyleSheet(
                "QPushButton { background-color: #2A2A2A; color: white; border: 1px solid #444444; border-radius: 5px; padding: 10px 16px; font-weight: bold; }"
                "QPushButton:hover { background-color: #3A3A3A; }"
            );
            clientRow->addStretch();
            clientRow->addWidget(clientIdEdit, 1);
            clientRow->addSpacing(8);
            clientRow->addWidget(genCodeBtn);
            clientRow->addStretch();
            // Keep device-flow controls off the page for now
            // ghLayout->addLayout(clientRow);
            // Hide alternative device-flow UI
            altLabel->hide();
            clientIdEdit->hide();
            genCodeBtn->hide();

            // Token output (initially hidden)
            QWidget *tokenContainer = new QWidget();
            QVBoxLayout *tokenLayout = new QVBoxLayout(tokenContainer);
            tokenLayout->setContentsMargins(0, 12, 0, 0);
            QLabel *tokenLabel = new QLabel("GitHub tilgangstoken:");
            tokenLabel->setStyleSheet("color: white; font-size: 14px; font-weight: bold;");
            tokenLayout->addWidget(tokenLabel);
            QHBoxLayout *tokenRow = new QHBoxLayout();
            QLineEdit *tokenEdit = new QLineEdit();
            tokenEdit->setReadOnly(true);
            tokenEdit->setPlaceholderText("Genereres etter at du bekrefter på GitHub...");
            tokenEdit->setStyleSheet(
                "QLineEdit { background-color: #1F1F1F; color: #E6E6E6; border: 1px solid #555555; border-radius: 6px; padding: 10px; font-family: Monospace; font-size: 14px; }"
            );
            QPushButton *copyTokenBtn = new QPushButton("Kopier token");
            copyTokenBtn->setStyleSheet(
                "QPushButton { background-color: #2A2A2A; color: white; border: 1px solid #444444; border-radius: 6px; padding: 10px 12px; font-weight: bold; }"
                "QPushButton:hover { background-color: #3A3A3A; }"
            );
            tokenRow->addWidget(tokenEdit, 1);
            tokenRow->addSpacing(8);
            tokenRow->addWidget(copyTokenBtn);
            tokenLayout->addLayout(tokenRow);
            tokenContainer->hide();
            step2Layout->addWidget(tokenContainer);
            tokenContainer->hide();


            // Row with numeric badge
            QHBoxLayout *step2Row = new QHBoxLayout();
            QLabel *badge2 = new QLabel("2");
            badge2->setFixedSize(32, 32);
            badge2->setAlignment(Qt::AlignCenter);
            badge2->setStyleSheet("QLabel { background-color: #0078D4; color: white; border-radius: 16px; font-size: 16px; font-weight: bold; }");
            step2Row->addWidget(badge2, 0, Qt::AlignTop);
            step2Row->addSpacing(10);
            step2Row->addWidget(step2Card, 1);
            ghLayout->addLayout(step2Row);
            ghLayout->addSpacing(24);

            // Step 3 card (guidance to continue)
            QWidget *step3Card = new QWidget();
            step3Card->setStyleSheet("QWidget { background-color: #232323; border: 1px solid #3A3A3A; border-radius: 8px; }");
            QVBoxLayout *step3Layout = new QVBoxLayout(step3Card);
            step3Layout->setContentsMargins(16, 16, 16, 16);
            step3Layout->setSpacing(10);
            QLabel *step3 = new QLabel("After completing activation, continue to Select Drive to choose the installation target for NixlyOS.");
            step3->setStyleSheet("color: #cccccc; font-size: 15px; line-height: 1.5;");
            step3->setWordWrap(true);
            step3->setAlignment(Qt::AlignLeft | Qt::AlignTop);
            step3Layout->addWidget(step3);
            QHBoxLayout *step3Actions = new QHBoxLayout();
            QPushButton *continueToDriveBtn = new QPushButton("Continue to Select Drive");
            continueToDriveBtn->setStyleSheet(
                "QPushButton { background-color: #0078D4; color: white; border: none; border-radius: 8px; padding: 10px 18px; font-weight: bold; }"
                "QPushButton:hover { background-color: #106EBE; }"
            );
            step3Actions->addStretch();
            step3Actions->addWidget(continueToDriveBtn);
            step3Actions->addStretch();
            step3Layout->addLayout(step3Actions);

            QHBoxLayout *step3Row = new QHBoxLayout();
            QLabel *badge3 = new QLabel("3");
            badge3->setFixedSize(32, 32);
            badge3->setAlignment(Qt::AlignCenter);
            badge3->setStyleSheet("QLabel { background-color: #0078D4; color: white; border-radius: 16px; font-size: 16px; font-weight: bold; }");
            step3Row->addWidget(badge3, 0, Qt::AlignTop);
            step3Row->addSpacing(10);
            step3Row->addWidget(step3Card, 1);
            ghLayout->addLayout(step3Row);

            // Navigation to drive page
            connect(continueToDriveBtn, &QPushButton::clicked, this, [=, this]() {
                if (menuButtons.size() > 3) {
                    menuButtons[3]->setEnabled(true);
                    menuButtons[3]->setChecked(true);
                    contentStack->setCurrentIndex(3);
                }
            });

            // Open signup page
            connect(createAccountBtn, &QPushButton::clicked, this, [=, this]() {
                QDesktopServices::openUrl(QUrl("https://github.com/signup"));
            });

            ghLayout->addStretch();

            // Shared state for login/polling/cancel
            struct GHState {
                QPointer<QProcess> proc;
                QTimer *poll = nullptr;
                bool inProgress = false;
                bool cancelled = false;
                bool statusCheckRunning = false;
                QTimer *spin = nullptr;
                int spinnerIndex = 0;
                bool codeCaptured = false;
                // Device flow state
                QTimer *devicePoll = nullptr;
                bool deviceInProgress = false;
                QString deviceCode;
                QString clientId;
                int deviceIntervalSec = 5;
            };
            auto ghState = std::make_shared<GHState>();
            ghState->poll = new QTimer(githubPage);
            ghState->poll->setInterval(1500);
            ghState->spin = new QTimer(githubPage);
            ghState->spin->setInterval(90);
            ghState->devicePoll = new QTimer(githubPage);
            ghState->devicePoll->setInterval(5000);

            // Spinner frames (braille animation)
            QStringList spinnerFrames = { "⠋","⠙","⠹","⠸","⠼","⠴","⠦","⠧","⠇","⠏" };
            QObject::connect(ghState->spin, &QTimer::timeout, this, [=, this]() mutable {
                ghState->spinnerIndex = (ghState->spinnerIndex + 1) % spinnerFrames.size();
                spinnerLbl->setText(spinnerFrames[ghState->spinnerIndex]);
            });

            // Generate device code without CLI using GitHub OAuth device flow
            connect(genCodeBtn, &QPushButton::clicked, this, [=, this]() mutable {
                QString clientId = clientIdEdit->text().trimmed();
                if (clientId.isEmpty()) {
                    status->setText("Oppgi en GitHub OAuth Client ID.");
                    status->setStyleSheet("color: #FF6B6B; font-size: 14px;");
                    return;
                }
                spinnerLbl->setText("⠋"); spinnerLbl->show(); if (!ghState->spin->isActive()) ghState->spin->start();
                status->setText("Henter aktiveringskode fra GitHub...");
                status->setStyleSheet("color: #FFAA00; font-size: 14px;");

                QUrl url("https://github.com/login/device/code");
                QNetworkRequest req(url);
                req.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
                req.setRawHeader("Accept", "application/json");

                QUrlQuery form;
                form.addQueryItem("client_id", clientId);
                form.addQueryItem("scope", "repo read:org");
                QByteArray body = form.toString(QUrl::FullyEncoded).toUtf8();

                QNetworkReply *rep = netManager->post(req, body);
                connect(rep, &QNetworkReply::finished, this, [=, this]() mutable {
                    if (ghState->spin->isActive()) ghState->spin->stop();
                    spinnerLbl->hide();
                    QByteArray data = rep->readAll();
                    rep->deleteLater();
                    QJsonParseError jerr; QJsonDocument jd = QJsonDocument::fromJson(data, &jerr);
                    if (jerr.error != QJsonParseError::NoError || !jd.isObject()) {
                        status->setText("Kunne ikke tolke svar fra GitHub.");
                        status->setStyleSheet("color: #FF6B6B; font-size: 14px;");
                        return;
                    }
                    QJsonObject obj = jd.object();
                    QString user_code = obj.value("user_code").toString();
                    QString verification_uri = obj.value("verification_uri").toString();
                    QString device_code = obj.value("device_code").toString();
                    int interval = obj.value("interval").toInt(5);
                    if (user_code.isEmpty()) {
                        status->setText("GitHub returnerte ingen kode. Sjekk Client ID.");
                        status->setStyleSheet("color: #FF6B6B; font-size: 14px;");
                        return;
                    }
                    deviceCodeEdit->setText(user_code.toUpper());
                    deviceCodeEdit->setCursorPosition(0);
                    QGuiApplication::clipboard()->setText(deviceCodeEdit->text());
                    status->setText("Koden er generert og kopiert. Åpne lenken under og godkjenn.");
                    status->setStyleSheet("color: #00AA00; font-size: 14px;");
                    if (!verification_uri.isEmpty()) {
                        openDevicePageBtn->setText("Åpne " + verification_uri);
                        openDevicePageBtn->disconnect();
                        connect(openDevicePageBtn, &QPushButton::clicked, this, [=, this]() {
                            if (!deviceCodeEdit->text().isEmpty()) {
                                QGuiApplication::clipboard()->setText(deviceCodeEdit->text());
                                deviceCodeEdit->setFocus();
                                deviceCodeEdit->selectAll();
                                status->setText("Koden er kopiert. Lim den inn på siden.");
                                status->setStyleSheet("color: #00AA00; font-size: 14px;");
                            } else {
                                status->setText("Ingen kode ennå. Klikk ‘Generer kode (uten CLI)’ først.");
                                status->setStyleSheet("color: #FFAA00; font-size: 14px;");
                            }
                            QDesktopServices::openUrl(QUrl(verification_uri));
                        });
                    }
                    // Start polling for access token
                    ghState->clientId = clientIdEdit->text().trimmed();
                    ghState->deviceCode = device_code;
                    ghState->deviceIntervalSec = qMax(1, interval);
                    if (ghState->devicePoll) ghState->devicePoll->setInterval(ghState->deviceIntervalSec * 1000);
                    ghState->deviceInProgress = true;
                    ghCancelBtn->show();
                    if (ghState->spin && !ghState->spin->isActive()) ghState->spin->start();
                    spinnerLbl->show();
                    status->setText("Venter på aktivering... (polling)");
                    status->setStyleSheet("color: #FFAA00; font-size: 14px;");
                    if (ghState->devicePoll && !ghState->devicePoll->isActive()) ghState->devicePoll->start();
                });
            });

            // Copy code
            connect(copyCodeBtn, &QPushButton::clicked, this, [=, this]() {
                if (!deviceCodeEdit->text().isEmpty()) {
                    QClipboard *cb = QGuiApplication::clipboard();
                    cb->setText(deviceCodeEdit->text());
                    status->setText("Koden er kopiert til utklippstavlen.");
                    status->setStyleSheet("color: #cccccc; font-size: 14px;");
                }
            });

            // Open browser to device page; ensure code is visible/copied
            connect(openDevicePageBtn, &QPushButton::clicked, this, [=, this]() {
                if (!deviceCodeEdit->text().isEmpty()) {
                    QGuiApplication::clipboard()->setText(deviceCodeEdit->text());
                    deviceCodeEdit->setFocus();
                    deviceCodeEdit->selectAll();
                    status->setText("Koden er kopiert. Lim den inn på siden.");
                    status->setStyleSheet("color: #00AA00; font-size: 14px;");
                } else {
                    status->setText("Ingen kode ennå. Klikk ‘Generer kode (uten CLI)’ først.");
                    status->setStyleSheet("color: #FFAA00; font-size: 14px;");
                }
                QDesktopServices::openUrl(QUrl("https://github.com/login/device"));
            });

            // Device flow token polling
            QObject::connect(ghState->devicePoll, &QTimer::timeout, this, [=, this]() mutable {
                if (!ghState->deviceInProgress) return;
                QUrl url("https://github.com/login/oauth/access_token");
                QNetworkRequest req(url);
                req.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
                req.setRawHeader("Accept", "application/json");
                QUrlQuery form;
                form.addQueryItem("client_id", ghState->clientId);
                form.addQueryItem("device_code", ghState->deviceCode);
                form.addQueryItem("grant_type", "urn:ietf:params:oauth:grant-type:device_code");
                QByteArray body = form.toString(QUrl::FullyEncoded).toUtf8();
                QNetworkReply *rep = netManager->post(req, body);
                QObject::connect(rep, &QNetworkReply::finished, this, [=, this]() mutable {
                    QByteArray data = rep->readAll();
                    rep->deleteLater();
                    QJsonParseError jerr; QJsonDocument jd = QJsonDocument::fromJson(data, &jerr);
                    if (jerr.error != QJsonParseError::NoError || !jd.isObject()) {
                        return; // keep polling
                    }
                    QJsonObject obj = jd.object();
                    const QString token = obj.value("access_token").toString();
                    const QString error = obj.value("error").toString();
                    if (!token.isEmpty()) {
                        ghState->deviceInProgress = false;
                        if (ghState->devicePoll && ghState->devicePoll->isActive()) ghState->devicePoll->stop();
                        if (ghState->spin && ghState->spin->isActive()) ghState->spin->stop();
                        spinnerLbl->hide();
                        // Show success indicator per new UX
                        ghCancelBtn->hide();
                        ghLoginBtn->hide();
                        oneTimeMsg->hide();
                        activationOkLabel->show();
                        if (menuButtons.size() > 3) menuButtons[3]->setEnabled(true);
                    } else if (!error.isEmpty()) {
                        if (error == "authorization_pending") {
                            // No status text per new UX
                        } else if (error == "slow_down") {
                            ghState->deviceIntervalSec += 5;
                            if (ghState->devicePoll) ghState->devicePoll->setInterval(ghState->deviceIntervalSec * 1000);
                        } else if (error == "expired_token" || error == "access_denied") {
                            ghState->deviceInProgress = false;
                            if (ghState->devicePoll && ghState->devicePoll->isActive()) ghState->devicePoll->stop();
                            if (ghState->spin && ghState->spin->isActive()) ghState->spin->stop();
                            spinnerLbl->hide();
                            ghCancelBtn->hide();
                            // No status text per new UX
                        }
                    }
                });
            });

            // Copy token manually
            connect(copyTokenBtn, &QPushButton::clicked, this, [=, this]() {
                if (!tokenEdit->text().isEmpty()) {
                    QGuiApplication::clipboard()->setText(tokenEdit->text());
                    status->setText("Token kopiert til utklippstavlen.");
                    status->setStyleSheet("color: #cccccc; font-size: 14px;");
                }
            });

            // Cancel ongoing login or device flow
            connect(ghCancelBtn, &QPushButton::clicked, this, [=, this]() mutable {
                bool didCancel = false;
                if (ghState->inProgress) {
                    ghState->cancelled = true;
                    ghState->inProgress = false;
                    if (ghState->poll && ghState->poll->isActive()) ghState->poll->stop();
                    if (ghState->proc) {
                        ghState->proc->kill();
                        ghState->proc->deleteLater();
                        ghState->proc = nullptr;
                    }
                    ghLoginBtn->setEnabled(true);
                    didCancel = true;
                }
                if (ghState->deviceInProgress) {
                    ghState->deviceInProgress = false;
                    if (ghState->devicePoll && ghState->devicePoll->isActive()) ghState->devicePoll->stop();
                    didCancel = true;
                }
                if (didCancel) {
                    ghCancelBtn->hide();
                    if (ghState->spin && ghState->spin->isActive()) ghState->spin->stop();
                    spinnerLbl->hide();
                    // No status text per new UX
                }
            });

            // Poll gh auth status while login is in progress (do not update UI)
            QObject::connect(ghState->poll, &QTimer::timeout, this, [=, this]() mutable {
                if (!ghState->inProgress || ghState->statusCheckRunning) return;
                ghState->statusCheckRunning = true;
                QProcess *chk = new QProcess(githubPage);
                QObject::connect(chk, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this,
                                 [=, this](int exitCode, QProcess::ExitStatus) mutable {
                    if (!ghState->inProgress) { chk->deleteLater(); ghState->statusCheckRunning = false; return; }
                    // Do not show success here; wait for the login process to finish
                    ghState->statusCheckRunning = false;
                    chk->deleteLater();
                });
                chk->start("gh", QStringList() << "auth" << "status" << "--hostname" << "github.com");
            });

            // Trigger gh auth login (device flow) and capture device code
            connect(ghLoginBtn, &QPushButton::clicked, this, [=, this]() mutable {
                if (ghState->inProgress) return;
                // No status text per new UX
                ghState->cancelled = false;
                ghState->inProgress = true;
                ghState->codeCaptured = false;
                ghLoginBtn->setEnabled(false);
                ghCancelBtn->show();
                ghState->spinnerIndex = 0;
                spinnerLbl->setText("⠋");
                spinnerLbl->show();
                if (ghState->spin && !ghState->spin->isActive()) ghState->spin->start();
                deviceCodeEdit->clear();

                QProcess *proc = new QProcess(this);
                QString program = "gh";
                QStringList args;
                args << "auth" << "login"
                     << "--hostname" << "github.com"
                     << "--git-protocol" << "https"
                     << "--scopes" << "repo,read:org"
                     << "--web";

                proc->setProgram(program);
                proc->setArguments(args);
                proc->setProcessChannelMode(QProcess::MergedChannels);
                proc->setReadChannel(QProcess::StandardOutput);
                ghState->proc = proc;

                // Capture and parse output for the one-time code (e.g. XXXX-XXXX)
                auto parseOutput = [=, this]() mutable {
                    if (!proc) return;
                    QString combined = QString::fromUtf8(proc->readAll());

                    // Auto-answer prompts if they appear
                    if (combined.contains("Authenticate Git with your GitHub credentials?") ||
                        combined.contains("Authenticate Git with your GitHub credentials", Qt::CaseInsensitive)) {
                        proc->write("y\n");
                    }
                    if (combined.contains("Press Enter", Qt::CaseInsensitive)) {
                        proc->write("\n");
                    }

                    // 1) Direct code pattern (case-insensitive): XXXX-XXXX
                    QRegularExpression re1("([A-Za-z0-9]{4}-[A-Za-z0-9]{4})", QRegularExpression::CaseInsensitiveOption);
                    QRegularExpressionMatch m1 = re1.match(combined);
                    QString foundCode;
                    if (m1.hasMatch()) {
                        foundCode = m1.captured(1);
                    } else {
                        // 2) Look for phrases like "one-time code: <code>" or "code: <code>"
                        QRegularExpression re2("one[- ]?time code\\s*:\\s*([A-Za-z0-9\\-]{4,})", QRegularExpression::CaseInsensitiveOption);
                        QRegularExpressionMatch m2 = re2.match(combined);
                        if (m2.hasMatch()) {
                            foundCode = m2.captured(1);
                        } else {
                            QRegularExpression re3("code\\s*:\\s*([A-Za-z0-9\\-]{4,})", QRegularExpression::CaseInsensitiveOption);
                            QRegularExpressionMatch m3 = re3.match(combined);
                            if (m3.hasMatch()) foundCode = m3.captured(1);
                        }
                    }

                    if (!foundCode.isEmpty() && !ghState->codeCaptured) {
                        ghState->codeCaptured = true;
                        QString codeUp = foundCode.toUpper();
                        deviceCodeEdit->setText(codeUp);
                        deviceCodeEdit->setCursorPosition(0);
                        // Show the one-time code just above the GitHub Login button
                        oneTimeMsg->setText(codeUp);
                        oneTimeMsg->show();
                        QClipboard *cb = QGuiApplication::clipboard();
                        cb->setText(codeUp);
                        // Confirm prompt to open browser automatically
                        proc->write("\n");
                        // Fallback: open the Device Activation page directly
                        QDesktopServices::openUrl(QUrl("https://github.com/login/device"));
                    }

                // Do not surface additional copy UI per new UX
                };

                QObject::connect(proc, &QProcess::readyRead, this, parseOutput);

                // Some prompts may wait for Enter; send a newline after start
                QObject::connect(proc, &QProcess::started, this, [=, this]() mutable {
                    proc->write("\n");
                    QTimer::singleShot(400, githubPage, [=, this]() mutable { if (proc) proc->write("\n"); });
                });

                QObject::connect(proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this,
                                 [=, this](int exitCode, QProcess::ExitStatus es) {
                    if (exitCode == 0 && es == QProcess::NormalExit) {
                        // Treat as success only when the login process finishes
                        ghState->inProgress = false;
                        if (ghState->poll && ghState->poll->isActive()) ghState->poll->stop();
                        ghCancelBtn->hide();
                        ghLoginBtn->hide();
                        oneTimeMsg->hide();
                        if (ghState->spin && ghState->spin->isActive()) ghState->spin->stop();
                        spinnerLbl->hide();
                        if (ghState->proc) { ghState->proc->deleteLater(); ghState->proc = nullptr; }
                        activationOkLabel->show();
                        if (menuButtons.size() > 3) menuButtons[3]->setEnabled(true);
                    } else if (!ghState->cancelled) {
                        QString errOut = QString::fromUtf8(proc->readAllStandardError());
                        if (errOut.trimmed().isEmpty()) errOut = QString::fromUtf8(proc->readAllStandardOutput());
                        // No status text per new UX
                        ghState->inProgress = false;
                        ghLoginBtn->setEnabled(true);
                        ghCancelBtn->hide();
                        if (ghState->spin && ghState->spin->isActive()) ghState->spin->stop();
                        spinnerLbl->hide();
                    }
                    proc->deleteLater();
                });

                // Pre-check: ensure gh exists
                QProcess *check = new QProcess(this);
                QObject::connect(check, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this,
                                 [=, this](int code, QProcess::ExitStatus) mutable {
                    check->deleteLater();
                    if (code != 0) {
                        // No status text per new UX
                        ghState->inProgress = false;
                        ghLoginBtn->setEnabled(true);
                        ghCancelBtn->hide();
                        if (ghState->spin && ghState->spin->isActive()) ghState->spin->stop();
                        spinnerLbl->hide();
                        return;
                    }
                    // Start the login process; success UI only updates on process finish
                    proc->start();
                });
                check->start("gh", QStringList() << "--version");
            });
        }
            
        QWidget *drivePage = createPage("Select Drive", 
            "Choose the drive where NixlyOS will be installed. WARNING: All data on the "
            "selected drive will be erased during installation.");
            
        QWidget *settingsPage = createPage("Settings", 
            "Configure system settings including timezone, keyboard layout, user accounts, "
            "and other system preferences.");
            
        QWidget *installPage = createPage("Install", 
            "Ready to install NixlyOS! Review your settings and click install to begin "
            "the installation process. This may take several minutes to complete.");
        
        // Add pages to stack
        contentStack->addWidget(welcomePage);
        contentStack->addWidget(internetPage);
        contentStack->addWidget(githubPage);
        contentStack->addWidget(drivePage);
        contentStack->addWidget(settingsPage);
        contentStack->addWidget(installPage);
        
        // Layout for content panel
        QVBoxLayout *contentLayout = new QVBoxLayout(contentPanel);
        contentLayout->setContentsMargins(0, 0, 0, 0);
        contentLayout->addWidget(contentStack);
        
        // Disable all buttons except Welcome initially
        for (int i = 1; i < menuButtons.size(); ++i) {
            menuButtons[i]->setEnabled(false);
        }
        
        // Connect "Let's start" button to enable Internet Connection and navigate to it
        connect(letsStartButton, &QPushButton::clicked, this, [=, this]() {
            menuButtons[1]->setEnabled(true);  // Enable Internet Connection button
            menuButtons[1]->setChecked(true);  // Select Internet Connection button
            contentStack->setCurrentIndex(1);  // Navigate to Internet Connection page
            // Kick off the first check immediately on entering the page
            QTimer::singleShot(200, this, [=, this]() {
                if (refreshTimer && !refreshTimer->isActive()) {
                    refreshTimer->start(); // Start auto-refresh
                }
            });
        });
        
        // Connect menu button selection to content stack
        connect(menuButtonGroup, &QButtonGroup::buttonClicked, this,
                [=, this](QAbstractButton* button) {
                    int index = menuButtonGroup->id(button);
                    contentStack->setCurrentIndex(index);
                    
                    // Start auto-refresh when Internet Connection page is shown
                    if (index == 1) { // Internet Connection page
                        if (!refreshTimer->isActive()) {
                            QTimer::singleShot(500, this, [=, this]() {
                                refreshTimer->start();
                            });
                        }
                    } else {
                        refreshTimer->stop(); // Stop refresh on other pages
                        // Reset state when leaving internet page
                        isCheckingInternet = false;
                    }
                });
        
        // Select first button by default
        menuButtons[0]->setChecked(true);
        contentStack->setCurrentIndex(0);
        
        // Add panels to main layout
        mainLayout->addWidget(topPanel);
        mainLayout->addWidget(contentPanel, 1); // Give content panel expanding space
        
        // Set central widget
        setCentralWidget(centralWidget);
    }
    
    ~MainWindow() override {
        // Ensure timer is stopped and won't fire after destruction
        if (refreshTimer) {
            refreshTimer->stop();
            refreshTimer->disconnect();
        }
    }

public:
    void showEvent(QShowEvent *event) override
    {
        QMainWindow::showEvent(event);
        
        // Set X11/Wayland window properties for tiling window managers
        if (windowHandle()) {
            // Tell the window manager this window should be tiled, not floating
            windowHandle()->setProperty("_NET_WM_WINDOW_TYPE", "_NET_WM_WINDOW_TYPE_NORMAL");
            
            // Disable window decorations if your WM supports it
            windowHandle()->setFlag(Qt::FramelessWindowHint, false);
        }
    }
};

int main(int argc, char *argv[])
{
    // We need to set these environment variables before QApplication is created
    
    // Always use Wayland platform if available; otherwise fall back to XCB
    qInfo("Setting up platform");
    
    // Clear any existing QT_QPA_PLATFORM setting
    qunsetenv("QT_QPA_PLATFORM");
    
    if (qEnvironmentVariableIsSet("WAYLAND_DISPLAY")) {
        qputenv("QT_QPA_PLATFORM", "wayland");
        qputenv("QT_WAYLAND_DISABLE_WINDOWDECORATION", "1");
        qputenv("QT_WAYLAND_SHELL_INTEGRATION", "xdg-shell");
    } else {
        qputenv("QT_QPA_PLATFORM", "xcb");
    }
    
    // Optional: plugin paths; usually not needed if runtime is set up correctly.
    // qputenv("QT_DEBUG_PLUGINS", "1"); // noisy; enable only for debugging
    
    QApplication app(argc, argv);
    
    // Set application metadata
    app.setApplicationName("nixlycc");
    app.setApplicationDisplayName("NixlyCC");
    app.setApplicationVersion("0.1");
    app.setDesktopFileName("nixlycc");
    
    // Create and show the main window
    MainWindow window;
    window.show();
    
    return app.exec();
}
