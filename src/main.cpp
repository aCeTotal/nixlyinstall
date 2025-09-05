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
                "As long as you use NixlyOS, you don't have to do anything with this yourself, as long as you follow the steps below."
            );
            intro->setWordWrap(true);
            intro->setAlignment(Qt::AlignCenter);
            intro->setStyleSheet("color: #cccccc; font-size: 15px; line-height: 1.5;");
            ghLayout->addWidget(intro);
            ghLayout->addSpacing(24); // Extra space between intro text and Step 1

            // Step 1 card
            QWidget *step1Card = new QWidget();
            step1Card->setStyleSheet("QWidget { background-color: #232323; border: 1px solid #3A3A3A; border-radius: 8px; }");
            QVBoxLayout *step1Layout = new QVBoxLayout(step1Card);
            step1Layout->setContentsMargins(16, 16, 16, 16);
            step1Layout->setSpacing(10);

            QLabel *step1 = new QLabel("If you don't already have a GitHub account, you'll need to create one first. If you already have an account, you can skip to step 2.");
            step1->setStyleSheet("color: #cccccc; font-size: 15px; line-height: 1.5;");
            step1->setWordWrap(true);
            step1->setAlignment(Qt::AlignCenter);
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

            // Step 2 card
            QWidget *step2Card = new QWidget();
            step2Card->setStyleSheet("QWidget { background-color: #232323; border: 1px solid #3A3A3A; border-radius: 8px; }");
            QVBoxLayout *step2Layout = new QVBoxLayout(step2Card);
            step2Layout->setContentsMargins(16, 16, 16, 16);
            step2Layout->setSpacing(10);

            QLabel *step2 = new QLabel("We now need to go through a Device Activation so that NixlyInstall gets temporary access to create a private repository for storing your entire NixlyOS system. When you click ‘Github Login’, we will display your one-time code, copy it to your clipboard, and automatically open the activation page in your browser.");
            step2->setStyleSheet("color: #cccccc; font-size: 15px; line-height: 1.5; margin-top: 2px;");
            step2->setWordWrap(true);
            step2->setAlignment(Qt::AlignCenter);
            step2Layout->addWidget(step2);

            // Status row with small spinner
            QLabel *status = new QLabel("Klar til å logge inn med GitHub CLI.");
            status->setStyleSheet("color: #cccccc; font-size: 14px;");
            status->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

            QLabel *spinnerLbl = new QLabel("");
            spinnerLbl->setFixedWidth(16);
            spinnerLbl->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
            spinnerLbl->hide();

            QHBoxLayout *statusRow = new QHBoxLayout();
            statusRow->addStretch();
            statusRow->addWidget(spinnerLbl);
            statusRow->addSpacing(8);
            statusRow->addWidget(status);
            statusRow->addStretch();
            // Per new UX, do not add statusRow to the UI

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
            QPushButton *ghLoginBtn = new QPushButton("Github Login");
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

            // Hint about gh CLI
            QLabel *hint = new QLabel("Requires ‘gh’ (GitHub CLI) in PATH.");
            hint->setStyleSheet("color: #777777; font-size: 12px;");
            hint->setAlignment(Qt::AlignCenter);
            step2Layout->addWidget(hint);

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

            // Step 3 card (guidance to continue)
            QWidget *step3Card = new QWidget();
            step3Card->setStyleSheet("QWidget { background-color: #232323; border: 1px solid #3A3A3A; border-radius: 8px; }");
            QVBoxLayout *step3Layout = new QVBoxLayout(step3Card);
            step3Layout->setContentsMargins(16, 16, 16, 16);
            step3Layout->setSpacing(10);
            QLabel *step3 = new QLabel("After completing activation, continue to Select Drive to choose the installation target for NixlyOS.");
            step3->setStyleSheet("color: #cccccc; font-size: 15px; line-height: 1.5;");
            step3->setWordWrap(true);
            step3->setAlignment(Qt::AlignCenter);
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
            connect(continueToDriveBtn, &QPushButton::clicked, this, [=]() {
                if (menuButtons.size() > 3) {
                    menuButtons[3]->setEnabled(true);
                    menuButtons[3]->setChecked(true);
                    contentStack->setCurrentIndex(3);
                }
            });

            // Open signup page
            connect(createAccountBtn, &QPushButton::clicked, this, [=]() {
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
            QObject::connect(ghState->spin, &QTimer::timeout, this, [=]() mutable {
                ghState->spinnerIndex = (ghState->spinnerIndex + 1) % spinnerFrames.size();
                spinnerLbl->setText(spinnerFrames[ghState->spinnerIndex]);
            });

            // Generate device code without CLI using GitHub OAuth device flow
            connect(genCodeBtn, &QPushButton::clicked, this, [=]() mutable {
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
                connect(rep, &QNetworkReply::finished, this, [=]() mutable {
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
                        connect(openDevicePageBtn, &QPushButton::clicked, this, [=]() {
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
            connect(copyCodeBtn, &QPushButton::clicked, this, [=]() {
                if (!deviceCodeEdit->text().isEmpty()) {
                    QClipboard *cb = QGuiApplication::clipboard();
                    cb->setText(deviceCodeEdit->text());
                    status->setText("Koden er kopiert til utklippstavlen.");
                    status->setStyleSheet("color: #cccccc; font-size: 14px;");
                }
            });

            // Open browser to device page; ensure code is visible/copied
            connect(openDevicePageBtn, &QPushButton::clicked, this, [=]() {
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
            QObject::connect(ghState->devicePoll, &QTimer::timeout, this, [=]() mutable {
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
                QObject::connect(rep, &QNetworkReply::finished, this, [=]() mutable {
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
            connect(copyTokenBtn, &QPushButton::clicked, this, [=]() {
                if (!tokenEdit->text().isEmpty()) {
                    QGuiApplication::clipboard()->setText(tokenEdit->text());
                    status->setText("Token kopiert til utklippstavlen.");
                    status->setStyleSheet("color: #cccccc; font-size: 14px;");
                }
            });

            // Cancel ongoing login or device flow
            connect(ghCancelBtn, &QPushButton::clicked, this, [=]() mutable {
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
            QObject::connect(ghState->poll, &QTimer::timeout, this, [=]() mutable {
                if (!ghState->inProgress || ghState->statusCheckRunning) return;
                ghState->statusCheckRunning = true;
                QProcess *chk = new QProcess(githubPage);
                QObject::connect(chk, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this,
                                 [=](int exitCode, QProcess::ExitStatus) mutable {
                    if (!ghState->inProgress) { chk->deleteLater(); ghState->statusCheckRunning = false; return; }
                    // Do not show success here; wait for the login process to finish
                    ghState->statusCheckRunning = false;
                    chk->deleteLater();
                });
                chk->start("gh", QStringList() << "auth" << "status" << "--hostname" << "github.com");
            });

            // Trigger gh auth login (device flow) and capture device code
            connect(ghLoginBtn, &QPushButton::clicked, this, [=]() mutable {
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
                auto parseOutput = [=]() mutable {
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
                QObject::connect(proc, &QProcess::started, this, [=]() mutable {
                    proc->write("\n");
                    QTimer::singleShot(400, githubPage, [=]() mutable { if (proc) proc->write("\n"); });
                });

                QObject::connect(proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this,
                                 [=](int exitCode, QProcess::ExitStatus es) {
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
                                 [=](int code, QProcess::ExitStatus) mutable {
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
