#include <QApplication>
#include <QMainWindow>
#include <QScreen>
#include <QGuiApplication>
#include <QWindow>
#include <QtCore>
#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPalette>
#include <QStackedWidget>
#include <QLabel>
#include <QFont>
#include <QPushButton>
#include <QButtonGroup>
#include <QAbstractButton>
#include <QPixmap>
#include <QLineEdit>
#include <QScrollArea>
#include <QProcess>
#include <QTimer>
#include <QNetworkInterface>
#include <QRegularExpression>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QSizePolicy>
#include <functional>

class MainWindow : public QMainWindow
{
private:
    bool lastEthernetState = false;
    bool lastInternetState = false;
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
        
        QLabel *connectionStatus = new QLabel("Checking internet connection...");
        connectionStatus->setStyleSheet("color: #cccccc; font-size: 16px;");
        connectionStatus->setAlignment(Qt::AlignCenter);
        internetLayout->addWidget(connectionStatus);
        
        QLabel *ethernetStatus = new QLabel();
        ethernetStatus->setStyleSheet("font-size: 14px; color: #FF6B6B; margin-top: 10px;");
        ethernetStatus->setAlignment(Qt::AlignCenter);
        internetLayout->addWidget(ethernetStatus);
        
        QWidget *wifiContainer = new QWidget();
        QVBoxLayout *wifiLayout = new QVBoxLayout(wifiContainer);
        
        QScrollArea *wifiScrollArea = new QScrollArea();
        wifiScrollArea->setMinimumHeight(450);
        wifiScrollArea->setMaximumHeight(450);
        wifiScrollArea->setWidgetResizable(true);
        wifiScrollArea->setStyleSheet(
            "QScrollArea { background-color: #2A2A2A; border: 1px solid #444444; border-radius: 5px; }"
            "QScrollBar:vertical { background-color: #1A1A1A; width: 12px; }"
            "QScrollBar::handle:vertical { background-color: #444444; border-radius: 6px; }"
        );
        
        QWidget *wifiListWidget = new QWidget();
        QVBoxLayout *wifiListLayout = new QVBoxLayout(wifiListWidget);
        wifiScrollArea->setWidget(wifiListWidget);
        wifiLayout->addWidget(wifiScrollArea);
        
        QWidget *passwordContainer = new QWidget();
        QVBoxLayout *passwordLayout = new QVBoxLayout(passwordContainer);
        
        QLabel *passwordLabel = new QLabel("Enter WiFi Password:");
        passwordLabel->setStyleSheet("color: white; font-size: 16px; font-weight: bold;");
        passwordLayout->addWidget(passwordLabel);
        
        QLineEdit *passwordInput = new QLineEdit();
        passwordInput->setEchoMode(QLineEdit::Password);
        passwordInput->setStyleSheet(
            "QLineEdit {"
            "    background-color: #2A2A2A;"
            "    color: white;"
            "    border: 1px solid #444444;"
            "    border-radius: 5px;"
            "    padding: 8px;"
            "    font-size: 14px;"
            "}"
            "QLineEdit:focus {"
            "    border-color: #0078D4;"
            "}"
        );
        passwordLayout->addWidget(passwordInput);
        
        QPushButton *connectButton = new QPushButton("Connect");
        connectButton->setStyleSheet(
            "QPushButton {"
            "    background-color: #0078D4;"
            "    color: white;"
            "    border: none;"
            "    border-radius: 5px;"
            "    padding: 8px 16px;"
            "    font-size: 14px;"
            "    font-weight: bold;"
            "}"
            "QPushButton:hover { background-color: #106EBE; }"
            "QPushButton:pressed { background-color: #005A9E; }"
        );
        passwordLayout->addWidget(connectButton);
        passwordContainer->hide();
        
        wifiLayout->addWidget(passwordContainer);
        wifiContainer->hide();
        internetLayout->addWidget(wifiContainer);
        
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
        
        // Function to check actual internet connectivity
        std::function<void(QLabel*, QPushButton*)> checkInternetConnectivity;
        checkInternetConnectivity = [this, contentStack](QLabel* statusLabel, QPushButton* contButton) mutable {
            if (isCheckingInternet) return; // Prevent multiple simultaneous checks
            
            isCheckingInternet = true;
            QNetworkRequest request(QUrl("https://connectivitycheck.gstatic.com/generate_204"));
            request.setRawHeader("User-Agent", "NixlyInstall");
            
            QNetworkReply *reply = netManager->get(request);
            // timeout
            QTimer *to = new QTimer(reply);
            to->setSingleShot(true);
            to->start(2000);
            QObject::connect(to, &QTimer::timeout, reply, &QNetworkReply::abort);
            
            connect(reply, &QNetworkReply::finished, this, [this, contentStack, statusLabel, contButton, reply]() mutable {
                isCheckingInternet = false; // Reset flag when check completes
                // If user left the Internet page, do not touch UI
                if (contentStack->currentIndex() != 1) {
                    reply->deleteLater();
                    return;
                }
                if (!statusLabel || !contButton) {
                    reply->deleteLater();
                    return;
                }
                
                const int http = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
                const bool ok = (reply->error() == QNetworkReply::NoError) && (http == 204 || http == 200);
                if (ok) {
                    if (!lastInternetState) { // Only update if state changed
                        lastInternetState = true;
                        statusLabel->setText("✓ Internet connection verified!");
                        statusLabel->setStyleSheet("color: #00AA00; font-size: 16px; font-weight: bold;");
                        contButton->show();
                    }
                } else {
                    // Always reflect failure immediately so it doesn't get stuck on "Checking..."
                    lastInternetState = false;
                    statusLabel->setText("No internet access detected");
                    statusLabel->setStyleSheet("color: #FF6B6B; font-size: 16px;");
                    contButton->hide();
                }
                reply->deleteLater();
            });
        };
        
        // Auto-refresh timer - faster for responsive updates, balanced to avoid races
        refreshTimer = new QTimer(this);
        refreshTimer->setInterval(800); // 0.8s
        
        // Function to check internet connection
        std::function<void()> checkInternetConnection;
        checkInternetConnection = [this, contentStack, ethernetStatus, connectionStatus, continueButton, wifiContainer, checkInternetConnectivity, passwordLabel, passwordContainer, wifiListLayout]() mutable {
            // Only operate when Internet page is visible
            if (contentStack->currentIndex() != 1) return;
            // Check for active ethernet/cable connection and WiFi connection
            QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();
            bool hasEthernet = false;
            bool hasWifiLink = false;
            
            for (const QNetworkInterface &interface : interfaces) {
                if (interface.flags().testFlag(QNetworkInterface::IsUp) &&
                    interface.flags().testFlag(QNetworkInterface::IsRunning) &&
                    !interface.flags().testFlag(QNetworkInterface::IsLoopBack) &&
                    (interface.type() == QNetworkInterface::Ethernet || interface.type() == QNetworkInterface::Wifi)) {
                    if (interface.type() == QNetworkInterface::Ethernet) {
                        hasEthernet = true;
                    }
                    if (interface.type() == QNetworkInterface::Wifi) {
                        // Consider WiFi "linked" if it has at least one non-loopback IPv4/IPv6 address
                        const auto addrs = interface.addressEntries();
                        for (const QNetworkAddressEntry &ae : addrs) {
                            if (!ae.ip().isNull()) { hasWifiLink = true; break; }
                        }
                    }
                }
            }
            
            // Always check internet connectivity, but only update UI if state changes
            if (hasEthernet) {
                if (!lastEthernetState) {
                    lastEthernetState = true;
                    ethernetStatus->setText("✓ Ethernet connection detected");
                    ethernetStatus->setStyleSheet("font-size: 14px; color: #00AA00; margin-top: 10px;");
                    connectionStatus->setText("Checking internet...");
                    connectionStatus->setStyleSheet("color: #FFAA00; font-size: 16px;");
                    wifiContainer->hide();
                    continueButton->hide();
                }
                if (!isCheckingInternet) {
                    checkInternetConnectivity(connectionStatus, continueButton);
                }
            } else if (hasWifiLink) {
                // Wi-Fi link present but not necessarily internet
                ethernetStatus->setText("✓ Wi-Fi link detected");
                ethernetStatus->setStyleSheet("font-size:14px; color:#00AA00; margin-top:10px;");
                connectionStatus->setText("Checking internet...");
                connectionStatus->setStyleSheet("color:#FFAA00; font-size:16px;");
                wifiContainer->hide();
                continueButton->hide();
                if (!isCheckingInternet) {
                    checkInternetConnectivity(connectionStatus, continueButton);
                }
            } else {
                // Transition from ethernet -> no ethernet
                if (lastEthernetState) {
                    lastEthernetState = false;
                    lastInternetState = false; // Reset internet state when ethernet disconnects
                    isCheckingInternet = false; // Reset checking flag
                }

                // Always update UI when no ethernet is present
                ethernetStatus->setText("✗ No ethernet or Wi-Fi connection available");
                ethernetStatus->setStyleSheet("font-size: 14px; color: #FF6B6B; margin-top: 10px;");
                connectionStatus->setText("Please select a WiFi network:");
                connectionStatus->setStyleSheet("color: #FFAA00; font-size: 16px;");
                continueButton->hide();
                wifiContainer->show();

                // Clear existing WiFi entries to avoid duplicates on refresh
                while (QLayoutItem *child = wifiListLayout->takeAt(0)) {
                    if (child->widget()) child->widget()->deleteLater();
                    delete child;
                }

                // Scan for actual WiFi networks using nmcli (async)
                QProcess *nmcliProcess = new QProcess(this);
                QStringList args{ "device", "wifi", "list", "-t", "-f", "IN-USE,SSID,SECURITY" };
                connect(nmcliProcess, &QProcess::finished, this, [=](int exitCode){
                    QStringList wifiNetworks;
                    if (exitCode == 0) {
                        QString out = QString::fromUtf8(nmcliProcess->readAllStandardOutput());
                        // Lines: "*:SSID:WPA2" or ":SSID:--"
                        for (const QString &line : out.split('\n', Qt::SkipEmptyParts)) {
                            QStringList parts = line.split(':');
                            if (parts.size() >= 3) {
                                const QString ssid = parts[1].trimmed();
                                const QString security = parts[2].trimmed();
                                if (!ssid.isEmpty() && ssid != "--") {
                                    const bool secured = (security != "--");
                                    wifiNetworks << (ssid + (secured ? " (Secured)" : " (Open)"));
                                }
                            }
                        }
                    }
                    nmcliProcess->deleteLater();

                    if (wifiNetworks.isEmpty()) {
                        connectionStatus->setText("No WiFi networks found. Retrying...");
                        connectionStatus->setStyleSheet("color: #FFAA00; font-size: 16px;");
                        wifiContainer->hide();
                    } else {
                        connectionStatus->setText(QString("Found %1 WiFi networks:").arg(wifiNetworks.size()));
                        connectionStatus->setStyleSheet("color: #FFAA00; font-size: 16px;");
                        
                        for (const QString &network : wifiNetworks) {
                            QPushButton *wifiButton = new QPushButton(network);
                            wifiButton->setStyleSheet(
                                "QPushButton {"
                                "    background-color: #2A2A2A;"
                                "    color: white;"
                                "    border: 1px solid #444444;"
                                "    border-radius: 5px;"
                                "    padding: 12px;"
                                "    font-size: 14px;"
                                "    text-align: left;"
                                "}"
                                "QPushButton:hover { background-color: #3A3A3A; }"
                                "QPushButton:pressed { background-color: #1A1A1A; }"
                            );

                            // Extract SSID and security flag
                            QString ssid = network;
                            bool secured = false;
                            if (ssid.endsWith(" (Secured)")) {
                                ssid.chop(QString(" (Secured)").size());
                                secured = true;
                            } else if (ssid.endsWith(" (Open)")) {
                                ssid.chop(QString(" (Open)").size());
                            }

                            wifiButton->setProperty("ssid", ssid);
                            wifiButton->setProperty("secured", secured);
                        
                            connect(wifiButton, &QPushButton::clicked, this, [=]() {
                                const QString ssidClicked = wifiButton->property("ssid").toString();
                                const bool needPassword = wifiButton->property("secured").toBool();
                                if (needPassword) {
                                    passwordLabel->setText("Enter password for " + ssidClicked + ":");
                                    passwordLabel->setProperty("ssid", ssidClicked);
                                    passwordContainer->show();
                                } else {
                                    connectionStatus->setText("Connecting to " + ssidClicked + "...");
                                    connectionStatus->setStyleSheet("color: #FFAA00; font-size: 16px;");
                                    
                                    QProcess *connectProcess = new QProcess(this);
                                    connect(connectProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [=](int exitCode) {
                                        if (exitCode == 0) {
                                            connectionStatus->setText("Connected to " + ssidClicked + ". Checking internet...");
                                            checkInternetConnectivity(connectionStatus, continueButton);
                                        } else {
                                            connectionStatus->setText("Failed to connect to " + ssidClicked);
                                            connectionStatus->setStyleSheet("color: #FF6B6B; font-size: 16px;");
                                        }
                                        connectProcess->deleteLater();
                                    });
                                    connectProcess->start("nmcli", QStringList() << "dev" << "wifi" << "connect" << ssidClicked);
                                }
                            });
                        
                            wifiListLayout->addWidget(wifiButton);
                        }
                    }
                });
                nmcliProcess->start("nmcli", args);
            }
        }; // <<<<<<<<<< IMPORTANT: close the lambda with semicolon

        // Connect refresh timer to check internet connection
        connect(refreshTimer, &QTimer::timeout, this, [this, checkInternetConnection]() {
            checkInternetConnection();
        });
        
        // Connect password input
        connect(connectButton, &QPushButton::clicked, this, [=]() {
            if (!passwordInput->text().isEmpty()) {
                QString selectedNetwork = passwordLabel->property("ssid").toString();
                if (selectedNetwork.isEmpty()) {
                    // fallback to previous logic
                    selectedNetwork = passwordLabel->text().split(" ").last().replace(":", "");
                }
                connectionStatus->setText("Connecting to " + selectedNetwork + "...");
                connectionStatus->setStyleSheet("color: #FFAA00; font-size: 16px;");
                
                QProcess *connectProcess = new QProcess(this);
                connect(connectProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [=](int exitCode) {
                    if (exitCode == 0) {
                        connectionStatus->setText("Connected to " + selectedNetwork + ". Checking internet...");
                        passwordContainer->hide();
                        checkInternetConnectivity(connectionStatus, continueButton);
                    } else {
                        connectionStatus->setText("Failed to connect to " + selectedNetwork + ". Check password.");
                        connectionStatus->setStyleSheet("color: #FF6B6B; font-size: 16px;");
                    }
                    connectProcess->deleteLater();
                });
                connectProcess->start("nmcli", QStringList() << "dev" << "wifi" << "connect" << selectedNetwork << "password" << passwordInput->text());
            }
        });
        
        // Connect continue button to enable GitHub page
        connect(continueButton, &QPushButton::clicked, this, [=]() {
            menuButtons[2]->setEnabled(true);  // Enable GitHub button
            menuButtons[2]->setChecked(true);  // Select GitHub button
            contentStack->setCurrentIndex(2);  // Navigate to GitHub page
        });
        
        // Start connection check after a short delay - but don't auto-start timer yet
        // QTimer::singleShot(1000, checkInternetConnection);
            
        QWidget *githubPage = createPage("GitHub", 
            "Connect your GitHub account to sync your dotfiles and configurations. "
            "This step is optional but recommended for developers.");
            
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
        connect(letsStartButton, &QPushButton::clicked, this, [=]() {
            menuButtons[1]->setEnabled(true);  // Enable Internet Connection button
            menuButtons[1]->setChecked(true);  // Select Internet Connection button
            contentStack->setCurrentIndex(1);  // Navigate to Internet Connection page
            // Kick off the first check immediately on entering the page
            QTimer::singleShot(200, this, [=]() {
                if (refreshTimer && !refreshTimer->isActive()) {
                    refreshTimer->start(); // Start auto-refresh
                }
            });
        });
        
        // Connect menu button selection to content stack
        connect(menuButtonGroup, &QButtonGroup::buttonClicked, this,
                [=](QAbstractButton* button) {
                    int index = menuButtonGroup->id(button);
                    contentStack->setCurrentIndex(index);
                    
                    // Start auto-refresh when Internet Connection page is shown
                    if (index == 1) { // Internet Connection page
                        if (!refreshTimer->isActive()) {
                            QTimer::singleShot(500, this, [=]() {
                                refreshTimer->start();
                            });
                        }
                    } else {
                        refreshTimer->stop(); // Stop refresh on other pages
                        // Reset states when leaving internet page
                        lastEthernetState = false;
                        lastInternetState = false;
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
