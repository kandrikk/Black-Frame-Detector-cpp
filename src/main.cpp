#include <QApplication>
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDir>
#include <QFileDialog>
#include "../include/BlackFrameDetector.h"
#include <string>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    QFont font;
    font.setFamily("Segoe UI");
    font.setBold(true);

    QWidget window;
    window.setWindowTitle("Black Frame Detector");
    window.resize(600, 800);
    window.setStyleSheet("background: #010202");


    QLabel *logoLabel = new QLabel();
    QPixmap logoPixmap("../photo/logo.png");

    if (logoPixmap.isNull()) {
        logoLabel->setText("<center><b>Black Frame Detector</b></center>");
    } else {
        logoLabel->setPixmap(logoPixmap.scaledToHeight(300, Qt::SmoothTransformation));
    }
    logoLabel->setAlignment(Qt::AlignCenter);
    
    QLineEdit *pathEdit = new QLineEdit();
    pathEdit->setPlaceholderText("Video file path....");
    pathEdit->setStyleSheet(
    "QLineEdit {"
    "    padding: 8px 12px;"               /* отступы внутри */
    "    border: 1px solid #cccccc;"        /* тонкая серая рамка */
    "    border-radius: 6px;"               /* скруглённые углы */
    "    background-color: #a5a5a5;"          /* белый фон */
    "    selection-background-color: #535d5e;" /* цвет выделения текста */
    "    selection-color: white;"
    "    font-size: 14px;"
    "}"
    "QLineEdit:focus {"
    "    border: 1.5px solid #000000ff;"      /* подсветка при фокусе */
    "    outline: none;"
    "}"
);

    QPushButton *analyzeButton = new QPushButton("");
    QIcon analyzeIcon;
    analyzeIcon.addPixmap(QPixmap("../photo/start-analysis-logo.png"), QIcon::Normal, QIcon::Off);
    analyzeIcon.addPixmap(QPixmap("../photo/analysis-clicked.png"), QIcon::Normal, QIcon::On);

    analyzeButton->setIcon(analyzeIcon);
    analyzeButton->setIconSize(QSize(200, 200));
    analyzeButton->setCheckable(true);
    

    QPushButton *browseButton = new QPushButton("");
    QIcon icon;
    icon.addPixmap(QPixmap("../photo/browse.png"), QIcon::Normal, QIcon::Off);
    icon.addPixmap(QPixmap("../photo/browse-clicked.png"), QIcon::Normal, QIcon::On);
    browseButton->setIcon(icon);
    browseButton->setIconSize(QSize(100, 100));
    browseButton->setCheckable(true);

    QLabel *resultLabel = new QLabel("The result will appear here");
    resultLabel->setWordWrap(true);
    resultLabel->setFont(font);

    QHBoxLayout *pathLayout = new QHBoxLayout();
    pathLayout->addWidget(pathEdit);
    pathLayout->addWidget(browseButton);
    

    auto *mainLayout = new QVBoxLayout();
    mainLayout->addWidget(logoLabel);
    mainLayout->addLayout(pathLayout);
    mainLayout->addWidget(analyzeButton);
    mainLayout->addWidget(resultLabel);

    QObject::connect(analyzeButton, &QPushButton::clicked, [pathEdit, resultLabel]() {
        std::string videoPath = pathEdit->text().trimmed().toStdString();
        QString res = QString::fromStdString(findBlackFrames(videoPath, 5));

        resultLabel->setText("Result:\n" + res);
    });

    QObject::connect(browseButton, &QPushButton::clicked, [pathEdit]() {
        QString filepath = QFileDialog::getOpenFileName(
            nullptr,
            "Select a video file",
            QDir::homePath(),
            "Video files (*.mp4 *.avi *.mov *.mkv);;All files (*)"
        );

        if (!filepath.isEmpty()) {
            pathEdit->setText(filepath);
        }
    });

    window.setLayout(mainLayout);
    window.show();

    return qApp->exec();
}