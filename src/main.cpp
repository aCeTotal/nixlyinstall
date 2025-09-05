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

class MainWindow : public QMainWindow
{
public:
    MainWindow(QWidget *parent = nullptr) : QMainWindow(parent)
    {
        // Set window title
        setWindowTitle("NixlyInstall");
        
        // Set window flags for tiling window managers
        setWindowFlags(Qt::Window | Qt::WindowTitleHint | 
                      Qt::WindowSystemMenuHint | Qt::WindowMinMaxButtonsHint);
        
        // Disable resizing by the user (let the tiling WM handle it)
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        
        // Set window attributes
        setAttribute(Qt::WA_AcceptTouchEvents);
        setAttribute(Qt::WA_OpaquePaintEvent);
        setAttribute(Qt::WA_NoSystemBackground);
        
        // Set initial size - will be managed by tiling WM
        resize(1000, 700);
        
        // Create central widget and main layout
        QWidget *centralWidget = new QWidget(this);
        QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
        mainLayout->setContentsMargins(0, 0, 0, 0);
        mainLayout->setSpacing(0);
        
        // Create top menu panel (60px high with dark background)
        QWidget *topPanel = new QWidget();
        topPanel->setFixedHeight(60);
        topPanel->setAutoFillBackground(true);
        
        // Set top panel background color to #121212 (RGB: 18, 18, 18)
        QPalette topPalette = topPanel->palette();
        topPalette.setColor(QPalette::Window, QColor(18, 18, 18));
        topPanel->setPalette(topPalette);
        
        // Create horizontal layout for menu buttons
        QHBoxLayout *topLayout = new QHBoxLayout(topPanel);
        topLayout->setContentsMargins(10, 5, 10, 5);
        topLayout->setSpacing(5);
        
        // Create button group for exclusive selection
        QButtonGroup *menuButtonGroup = new QButtonGroup(this);
        
        // Create menu buttons
        QStringList menuItems = {
            "Welcome to NixlyOS",
            "Internet Connection", 
            "Github",
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
            
            // Set button style
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
        
        // Create content area with slightly lighter background
        QWidget *contentPanel = new QWidget();
        contentPanel->setAutoFillBackground(true);
        
        // Set content panel background color to #1A1A1A (RGB: 26, 26, 26)
        QPalette contentPalette = contentPanel->palette();
        contentPalette.setColor(QPalette::Window, QColor(26, 26, 26));
        contentPanel->setPalette(contentPalette);
        
        // Create stacked widget for content pages
        QStackedWidget *contentStack = new QStackedWidget();
        
        // Function to create a content page with a title and description
        auto createPage = [](const QString &title, const QString &description) {
            QWidget *page = new QWidget();
            QVBoxLayout *layout = new QVBoxLayout(page);
            layout->setContentsMargins(40, 40, 40, 40);
            layout->setSpacing(20);
            
            // Add title
            QLabel *titleLabel = new QLabel(title);
            titleLabel->setStyleSheet("color: white; font-size: 28px; font-weight: bold;");
            titleLabel->setAlignment(Qt::AlignCenter);
            layout->addWidget(titleLabel);
            
            // Add description
            QLabel *descLabel = new QLabel(description);
            descLabel->setStyleSheet("color: #cccccc; font-size: 16px; line-height: 1.5;");
            descLabel->setWordWrap(true);
            descLabel->setAlignment(Qt::AlignCenter);
            layout->addWidget(descLabel);
            
            layout->addStretch();
            return page;
        };
        
        // Create pages for each menu item
        // Special handling for Welcome page with logo
        QWidget *welcomePage = new QWidget();
        QVBoxLayout *welcomeLayout = new QVBoxLayout(welcomePage);
        welcomeLayout->setContentsMargins(40, 40, 40, 40);
        welcomeLayout->setSpacing(30);
        
        // Add logo
        QLabel *logoLabel = new QLabel();
        // Try multiple possible paths for the logo
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
            // Scale logo to appropriate size (max 320px width, maintain aspect ratio)
            QPixmap scaledLogo = logo.scaled(320, 320, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            logoLabel->setPixmap(scaledLogo);
        } else {
            // Fallback if logo can't be loaded
            logoLabel->setText("NixlyOS");
            logoLabel->setStyleSheet("color: #0078D4; font-size: 32px; font-weight: bold;");
        }
        logoLabel->setAlignment(Qt::AlignCenter);
        welcomeLayout->addWidget(logoLabel);
        
        // Add title
        QLabel *welcomeTitleLabel = new QLabel("Welcome to NixlyOS");
        welcomeTitleLabel->setStyleSheet("color: white; font-size: 28px; font-weight: bold;");
        welcomeTitleLabel->setAlignment(Qt::AlignCenter);
        welcomeLayout->addWidget(welcomeTitleLabel);
        
        // Add description
        QLabel *welcomeDescLabel = new QLabel("Welcome to the NixlyOS installation wizard. This installer will guide you through "
            "setting up your new NixlyOS system with all the necessary configurations. ");
        welcomeDescLabel->setStyleSheet("color: #cccccc; font-size: 16px; line-height: 1.5;");
        welcomeDescLabel->setWordWrap(true);
        welcomeDescLabel->setAlignment(Qt::AlignCenter);
        welcomeLayout->addWidget(welcomeDescLabel);
        
        // Add "Let's start" button
        QPushButton *letsStartButton = new QPushButton("Let's start");
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
        
        // Center the button
        QHBoxLayout *buttonLayout = new QHBoxLayout();
        buttonLayout->addStretch();
        buttonLayout->addWidget(letsStartButton);
        buttonLayout->addStretch();
        welcomeLayout->addLayout(buttonLayout);
        
        welcomeLayout->addStretch();
        
        // Create other pages
        QWidget *internetPage = createPage("Internet Connection", 
            "Configure your internet connection settings. Ensure you have a stable internet "
            "connection before proceeding with the installation.");
            
        QWidget *githubPage = createPage("Github", 
            "Connect your Github account to sync your dotfiles and configurations. "
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
        connect(letsStartButton, &QPushButton::clicked, [=]() {
            menuButtons[1]->setEnabled(true);  // Enable Internet Connection button
            menuButtons[1]->setChecked(true);  // Select Internet Connection button
            contentStack->setCurrentIndex(1);  // Navigate to Internet Connection page
        });
        
        // Connect menu button selection to content stack
        connect(menuButtonGroup, QOverload<QAbstractButton*>::of(&QButtonGroup::buttonClicked),
                [=](QAbstractButton* button) {
                    int index = menuButtonGroup->id(button);
                    contentStack->setCurrentIndex(index);
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
    
    // Always use Wayland platform
    qInfo("Setting up Wayland platform exclusively");
    
    // Clear any existing QT_QPA_PLATFORM setting
    qunsetenv("QT_QPA_PLATFORM");
    
    // Find Qt plugins directory
    const char* qtPluginPath = getenv("QT_PLUGIN_PATH");
    if (!qtPluginPath || strlen(qtPluginPath) == 0) {
        // If not set, try to find it in a standard location
        qputenv("QT_PLUGIN_PATH", "/nix/store/l9kvcx3wna1bla7xpy1629hawjqmna4y-qtbase-6.9.0/lib/qt-6/plugins");
        qInfo("Set QT_PLUGIN_PATH to Qt6 plugins directory");
    }
    
    // Set Wayland platform and shell integration paths
    qputenv("QT_QPA_PLATFORM", "wayland");
    qputenv("QT_WAYLAND_DISABLE_WINDOWDECORATION", "1");
    
    // Set the shell integration path to the one we found
    qputenv("QT_WAYLAND_SHELL_INTEGRATION", "xdg-shell"); // Explicitly request xdg-shell
    qputenv("QT_WAYLAND_SHELL_INTEGRATION_PLUGIN_PATH", "/nix/store/53d22iacdvx0whdbg4qb88bbhlm6lw2f-qtwayland-6.9.0/lib/qt-6/plugins/wayland-shell-integration");
    
    // Also set general plugin path to include qtwayland
    const char* existingPluginPath = getenv("QT_PLUGIN_PATH");
    QString pluginPath;
    if (existingPluginPath) {
        pluginPath = QString(existingPluginPath) + ":/nix/store/53d22iacdvx0whdbg4qb88bbhlm6lw2f-qtwayland-6.9.0/lib/qt-6/plugins";
    } else {
        pluginPath = "/nix/store/53d22iacdvx0whdbg4qb88bbhlm6lw2f-qtwayland-6.9.0/lib/qt-6/plugins";
    }
    qputenv("QT_PLUGIN_PATH", pluginPath.toUtf8());
    qInfo("Set QT_PLUGIN_PATH to include Wayland plugins: %s", qgetenv("QT_PLUGIN_PATH").constData());
    
    // Enable debugging for Qt platform plugins
    qputenv("QT_DEBUG_PLUGINS", "1");
    
    // Set application attributes before creating QApplication
    // Note: High-DPI support is enabled by default in Qt6
    
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
