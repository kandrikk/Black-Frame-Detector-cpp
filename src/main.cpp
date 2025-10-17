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
#include <string>
#include <QtConcurrent>
#include <QFutureWatcher>
#include <QMainWindow>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    QMainWindow window;
    window.setWindowTitle("Black Frame Detector");
    window.resize(400, 600);
    window.setStyleSheet("background: #010202");
    
    QLineEdit *pathEdit = new QLineEdit();
    pathEdit->setPlaceholderText("Video file path....");
    pathEdit->setStyleSheet(
        "QLineEdit {"
        "    padding: 8px 12px;"               /* отступы внутри */
        "    border: 1.5px solid #515b5c;"        /* тонкая серая рамка */
        "    border-radius: 6px;"               /* скруглённые углы */
        "    background-color: #a5a5a5;"          /* белый фон */
        "    selection-background-color: #142838;" /* цвет выделения текста */
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
    analyzeButton->setFixedSize(250, 100); // чтобы не растягивалась
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

    QObject::connect(analyzeButton, &QPushButton::clicked, [styleAnalyzeBotton, analyzeButton, pathEdit, resultTextEdit]() {
        QElapsedTimer timer;
        timer.start();

        QString videoPath = pathEdit->text().trimmed();

        // --- Проверка файла ---
        if (videoPath.isEmpty()) {
            QString errorMsg = "No file selected";
            resultTextEdit->setHtml(
                "<div style='text-align: center; color: #FF6347; font-weight: bold; font-size: 16px;'>"
                + errorMsg.toHtmlEscaped() +
                "</div>"
            );

            return;
        }

        QFileInfo checkFileExist(videoPath);
        if (!checkFileExist.exists()) {
            QString errorMsg = "File not found.";
            resultTextEdit->setHtml(
                "<div style='text-align: center; color: #FF6347; font-weight: bold; font-size: 16px;'>"
                + errorMsg.toHtmlEscaped() +
                "</div>"
            );

            return;
        } else if (!checkFileExist.isFile()) {
            QString errorMsg = "Is not file.";
            resultTextEdit->setHtml(
                "<div style='text-align: center; color: #FF6347; font-weight: bold; font-size: 16px;'>"
                + errorMsg.toHtmlEscaped() +
                "</div>"
            );

            return;
        }

        std::vector<std::string> info = infoVideoFile(videoPath.toStdString());
        QString filename = QString::fromStdString(info[0]);
        QString duration = QString::fromStdString(info[1]);
        QString fps = QString::fromStdString(info[2]);
        QString frames = QString::fromStdString(info[3]);
        // --- СРАЗУ показываем информацию о файле ---
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

        QFuture<QString> future = QtConcurrent::run([videoPath]() -> QString {
            std::string result = findBlackFrames(videoPath.toStdString(), 5.0);
            return QString::fromStdString(result);
        });

        QFutureWatcher<QString>* watcher = new QFutureWatcher<QString>();
        watcher->setFuture(future);

        QObject::connect(watcher, &QFutureWatcher<QString>::finished,
            [watcher, analyzeButton, resultTextEdit, styleAnalyzeBotton, timer, filename, duration, fps, frames]() mutable {
                analyzeButton->setEnabled(true);
                analyzeButton->setStyleSheet(styleAnalyzeBotton);

                double elapsedSec = timer.elapsed() / 1000.0;
                QString timeInfo = QString("Processing time: %1 seconds").arg(elapsedSec, 0, 'f', 2);

                QString blackFrameResult = watcher->result();

                // Формируем красивый HTML
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
                .arg(blackFrameResult.toHtmlEscaped())  // сохраняет переносы строк
                .arg(timeInfo.toHtmlEscaped());

                resultTextEdit->setHtml(html);
                watcher->deleteLater();
            }
        );
    });

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