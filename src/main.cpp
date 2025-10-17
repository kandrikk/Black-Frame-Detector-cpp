#include "../include/BlackFrameDetector.h"
#include <QApplication>
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDir>
#include <QFileDialog>
#include <QTextEdit>
#include <QStatusBar>
#include <QProgressBar>
#include <string>
#include <QtConcurrent>
#include <QFutureWatcher>
#include <QMainWindow>
#include <QMetaObject>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    QMainWindow window;
    window.setWindowTitle("Black Frame Detector");
    window.resize(400, 600);
    window.setStyleSheet("background: #010202");

    // === Статусная строка с текстом и прогресс-баром ===
    QStatusBar *statusBar = new QStatusBar();
    QLabel *statusLabel = new QLabel("Ready");
    statusLabel->setStyleSheet("color: #e0e0e0;");

    QProgressBar *progressBar = new QProgressBar();
    progressBar->setRange(0, 100);
    progressBar->setValue(0);
    progressBar->setFixedWidth(200);
    progressBar->setVisible(false); // Скрыт по умолчанию

    // Стилизация
    statusBar->setStyleSheet(R"(
        QStatusBar {
            background: #000000ff;
            background-color: #000000ff;
            color: #e0e0e0;
            border-top: 1px solid #515b5c;
        }
        QProgressBar {
            border: 1px solid #515b5c;
            border-radius: 4px;
            text-align: center;
            color: white;
            background: #2a2a2a;
        }
        QProgressBar::chunk {
            background: qlineargradient(x1: 0, y1: 0, x2: 1, y2: 0,
                                        stop: 0 #1c7a8d, stop: 1 #4db8ff);
            border-radius: 3px;
        }
    )");

    statusBar->addWidget(statusLabel);
    statusBar->addPermanentWidget(progressBar);
    window.setStatusBar(statusBar);

    // === UI элементы ===
    QLineEdit *pathEdit = new QLineEdit();
    pathEdit->setPlaceholderText("Video file path....");
    pathEdit->setStyleSheet(
        "QLineEdit {"
        "    padding: 8px 12px;"
        "    border: 1.5px solid #515b5c;"
        "    border-radius: 6px;"
        "    background-color: #a5a5a5;"
        "    selection-background-color: #142838;"
        "    selection-color: white;"
        "    font-size: 14px;"
        "    height: 30px;"
        "    color: #161824"
        "}"
    );

    QPushButton *browseButton = new QPushButton("");
    browseButton->setFixedSize(120, 60);
    browseButton->setStyleSheet(
        "QPushButton {"
        "    border: none;"
        "    background-image: url(../icons/browse-logo.png);"
        "    background-repeat: no-repeat;"
        "    background-position: center;"
        "    background-color: transparent;"
        "}"
        "QPushButton:hover {"
        "    background-image: url(../icons/browse-hover.png);"
        "}"
        "QPushButton:pressed {"
        "    background-image: url(../icons/browse-clicked.png);"
        "}"
    );

    QHBoxLayout *pathLayout = new QHBoxLayout();
    pathLayout->addWidget(pathEdit);
    pathLayout->addWidget(browseButton);

    QObject::connect(browseButton, &QPushButton::clicked, [browseButton, pathEdit]() {
        QString filepath = QFileDialog::getOpenFileName(
            nullptr,
            "Select a video file",
            QDir::homePath(),
            "Video files (*.mp4 *.avi *.mov *.mkv);;All files (*)"
        );
        if (!filepath.isEmpty()) {
            pathEdit->setText(filepath);
        }
        browseButton->setChecked(false);
    });

    QPushButton *analyzeButton = new QPushButton();
    analyzeButton->setFixedSize(250, 100);
    QString styleAnalyzeBotton =
        "QPushButton {"
        "    border: none;"
        "    background-image: url(../icons/black-frame-button.png);"
        "    background-repeat: no-repeat;"
        "    background-position: center;"
        "    background-color: transparent;"
        "}"
        "QPushButton:hover {"
        "    background-image: url(../icons/black-frame-button-hover.png);"
        "}"
        "QPushButton:pressed {"
        "    background-image: url(../icons/black-frame-button-analysis.png);"
        "}";
    analyzeButton->setStyleSheet(styleAnalyzeBotton);

    QTextEdit *resultTextEdit = new QTextEdit();
    resultTextEdit->setReadOnly(true);
    resultTextEdit->setPlaceholderText("The result will appear here");
    resultTextEdit->setStyleSheet(
        "QTextEdit{"
        "    background-color: #1c7a8dff;"
        "    color: #e0e0e0;"
        "    border: 1.5px solid #a5a5a5;"
        "    border-radius: 6px;"
        "    padding: 10px;"
        "    font-family: Courier New, monospace;"
        "    font-size: 14px;"
        "}"
    );

    // === Обработчик анализа с прогрессом ===
    QObject::connect(analyzeButton, &QPushButton::clicked, 
        [styleAnalyzeBotton, analyzeButton, pathEdit, resultTextEdit, statusLabel, progressBar]() {

        QString videoPath = pathEdit->text().trimmed();
        if (videoPath.isEmpty()) {
            resultTextEdit->setHtml(
                "<div style='text-align: center; color: #FF6347; font-weight: bold; font-size: 16px;'>"
                "No file selected."
                "</div>"
            );
            statusLabel->setText("Error: no file selected");
            return;
        }

        QFileInfo checkFile(videoPath);
        if (!checkFile.exists() || !checkFile.isFile()) {
            resultTextEdit->setHtml(
                "<div style='text-align: center; color: #FF6347; font-weight: bold; font-size: 16px;'>"
                "File not found or invalid."
                "</div>"
            );
            statusLabel->setText("Error: invalid file");
            return;
        }

        // Получаем информацию о видео
        std::vector<std::string> info = infoVideoFile(videoPath.toStdString());
        if (info.size() != 4) {
            QString errorMsg = QString::fromStdString(info[0]);
            resultTextEdit->setHtml(
                "<div style='text-align: center; color: #FF6347; font-weight: bold; font-size: 16px;'>"
                + errorMsg.toHtmlEscaped() +
                "</div>"
            );
            statusLabel->setText("Error: cannot read video");
            return;
        }

        QString filename = QString::fromStdString(info[0]);
        QString duration = QString::fromStdString(info[1]);
        QString fps = QString::fromStdString(info[2]);
        QString frames = QString::fromStdString(info[3]);

        // Показываем информацию сразу
        QString initialHtml = QString(R"(
            <div style="text-align: center;">
                <div style="display: inline-block; text-align: left; font-family: 'Courier New', monospace; font-size: 14px; line-height: 1.4; color: white;">
                    <div style="font-weight: bold; font-size: 16px; margin-bottom: 10px;">INFO</div>
                    <div>Filename: %1</div>
                    <div>Time: %2</div>
                    <div>FPS: %3</div>
                    <div>Frames: %4</div>
                    <br>
                    <div style="font-weight: bold; font-size: 16px; margin: 15px 0 10px 0; color: #FFD700;">Analyzing black frames...</div>
                </div>
            </div>
        )")
        .arg(filename.toHtmlEscaped())
        .arg(duration.toHtmlEscaped())
        .arg(fps.toHtmlEscaped())
        .arg(frames.toHtmlEscaped());

        resultTextEdit->setHtml(initialHtml);
        analyzeButton->setEnabled(false);
        analyzeButton->setStyleSheet(
            "QPushButton { background-image: url(../icons/black-frame-button-analysis.png); }"
        );

        // Показываем прогресс-бар
        progressBar->setVisible(true);
        progressBar->setValue(0);
        statusLabel->setText("Analyzing...");

        QElapsedTimer timer;
        timer.start();

        // Callback для обновления прогресса
        auto progressCallback = [progressBar, statusLabel](int current, int total) {
            if (total <= 0) return;
            int percent = 100 * current / total;
            QMetaObject::invokeMethod(progressBar, "setValue", Qt::QueuedConnection, Q_ARG(int, percent));
            QMetaObject::invokeMethod(statusLabel, "setText", Qt::QueuedConnection,
                Q_ARG(QString, QString("Processing... %1%").arg(percent)));
        };

        // Запуск в фоне
        QFuture<QString> future = QtConcurrent::run([videoPath, progressCallback]() -> QString {
            return QString::fromStdString(findBlackFrames(videoPath.toStdString(), 5.0, progressCallback));
        });

        QFutureWatcher<QString>* watcher = new QFutureWatcher<QString>();
        watcher->setFuture(future);

        QObject::connect(watcher, &QFutureWatcher<QString>::finished,
            [watcher, analyzeButton, resultTextEdit, styleAnalyzeBotton, timer, 
             filename, duration, fps, frames, statusLabel, progressBar]() mutable {

                analyzeButton->setEnabled(true);
                analyzeButton->setStyleSheet(styleAnalyzeBotton);

                double elapsedSec = timer.elapsed() / 1000.0;
                QString timeInfo = QString("Processing time: %1 seconds").arg(elapsedSec, 0, 'f', 2);
                QString blackFrameResult = watcher->result();

                QString html = QString(R"(
                    <div style="text-align: center;">
                        <div style="display: inline-block; text-align: left; font-family: 'Courier New', monospace; font-size: 14px; line-height: 1.4; color: white;">
                            <div style="font-weight: bold; font-size: 16px; margin-bottom: 10px;">INFO</div>
                            <div>Filename: %1</div>
                            <div>Time: %2</div>
                            <div>FPS: %3</div>
                            <div>Frames: %4</div>
                            <br>
                            <div style="font-weight: bold; font-size: 16px; margin: 15px 0 10px 0;">Black Frames:</div>
                            <div style="white-space: pre;">%5</div>
                            <br>
                            <div style="font-weight: bold; color: #FFD700;">%6</div>
                        </div>
                    </div>
                )")
                .arg(filename.toHtmlEscaped())
                .arg(duration.toHtmlEscaped())
                .arg(fps.toHtmlEscaped())
                .arg(frames.toHtmlEscaped())
                .arg(blackFrameResult.toHtmlEscaped())
                .arg(timeInfo.toHtmlEscaped());

                resultTextEdit->setHtml(html);
                statusLabel->setText("Done");
                progressBar->setVisible(false); // Скрываем после завершения
                watcher->deleteLater();
            }
        );
    });

    // === Layout и отображение ===
    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->addLayout(pathLayout);
    mainLayout->addWidget(analyzeButton, 0, Qt::AlignCenter);
    mainLayout->addWidget(resultTextEdit);

    QWidget *centralWidget = new QWidget();
    centralWidget->setLayout(mainLayout);
    window.setCentralWidget(centralWidget);
    window.show();

    return qApp->exec();
}