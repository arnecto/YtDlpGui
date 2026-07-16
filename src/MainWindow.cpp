#include "MainWindow.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QProgressBar>
#include <QPlainTextEdit>
#include <QCheckBox>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QDir>

namespace {
// індекси відповідають пунктам formatCombo
enum FormatPreset {
    PresetBestMp4 = 0,
    PresetBestAny,
    PresetAudioMp3,
    PresetCustom
};

// індекси відповідають пунктам codecCombo
enum CodecChoice {
    CodecNone = 0,
    CodecH264Prefer,
    CodecH264Force,
    CodecH265Prefer,
    CodecVp9Prefer
};
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    buildUi();
    loadSettings();

    m_process = new QProcess(this);
    connect(m_process, &QProcess::readyReadStandardOutput, this, &MainWindow::onProcessReadyReadStdout);
    connect(m_process, &QProcess::readyReadStandardError, this, &MainWindow::onProcessReadyReadStderr);
    connect(m_process, &QProcess::errorOccurred, this, &MainWindow::onProcessErrorOccurred);
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &MainWindow::onProcessFinished);
}

MainWindow::~MainWindow()
{
    saveSettings();
    if (m_process && m_process->state() != QProcess::NotRunning) {
        m_process->kill();
        m_process->waitForFinished(2000);
    }
}

void MainWindow::buildUi()
{
    setWindowTitle("YtDlpGui — обгортка для yt-dlp");

    auto *central = new QWidget(this);
    setCentralWidget(central);
    auto *mainLayout = new QVBoxLayout(central);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(16);

    // --- Група: шляхи до інструментів ---
    auto *pathsBox = new QGroupBox("Інструменти", central);
    auto *pathsLayout = new QFormLayout(pathsBox);
    pathsLayout->setSpacing(10);
    pathsLayout->setContentsMargins(14, 16, 14, 14);

    auto *ytDlpRow = new QWidget(pathsBox);
    auto *ytDlpRowLayout = new QHBoxLayout(ytDlpRow);
    ytDlpRowLayout->setContentsMargins(0, 0, 0, 0);
    m_ytDlpPathEdit = new QLineEdit(ytDlpRow);
    m_ytDlpPathEdit->setPlaceholderText("Шлях до yt-dlp.exe (або просто 'yt-dlp', якщо він у PATH)");
    auto *ytDlpBrowseBtn = new QPushButton("Огляд...", ytDlpRow);
    auto *checkVersionBtn = new QPushButton("Перевірити", ytDlpRow);
    ytDlpRowLayout->addWidget(m_ytDlpPathEdit);
    ytDlpRowLayout->addWidget(ytDlpBrowseBtn);
    ytDlpRowLayout->addWidget(checkVersionBtn);
    pathsLayout->addRow("yt-dlp:", ytDlpRow);

    auto *ffmpegRow = new QWidget(pathsBox);
    auto *ffmpegRowLayout = new QHBoxLayout(ffmpegRow);
    ffmpegRowLayout->setContentsMargins(0, 0, 0, 0);
    m_ffmpegPathEdit = new QLineEdit(ffmpegRow);
    m_ffmpegPathEdit->setPlaceholderText("Папка з ffmpeg.exe (необов'язково, якщо ffmpeg у PATH)");
    auto *ffmpegBrowseBtn = new QPushButton("Огляд...", ffmpegRow);
    ffmpegRowLayout->addWidget(m_ffmpegPathEdit);
    ffmpegRowLayout->addWidget(ffmpegBrowseBtn);
    pathsLayout->addRow("ffmpeg:", ffmpegRow);

    mainLayout->addWidget(pathsBox);

    // --- Група: завантаження ---
    auto *downloadBox = new QGroupBox("Завантаження", central);
    auto *downloadLayout = new QFormLayout(downloadBox);
    downloadLayout->setSpacing(10);
    downloadLayout->setContentsMargins(14, 16, 14, 14);

    m_urlEdit = new QLineEdit(downloadBox);
    m_urlEdit->setPlaceholderText("https://www.youtube.com/watch?v=...");
    downloadLayout->addRow("URL відео:", m_urlEdit);

    auto *outDirRow = new QWidget(downloadBox);
    auto *outDirRowLayout = new QHBoxLayout(outDirRow);
    outDirRowLayout->setContentsMargins(0, 0, 0, 0);
    m_outputDirEdit = new QLineEdit(outDirRow);
    auto *outDirBrowseBtn = new QPushButton("Огляд...", outDirRow);
    outDirRowLayout->addWidget(m_outputDirEdit);
    outDirRowLayout->addWidget(outDirBrowseBtn);
    downloadLayout->addRow("Папка збереження:", outDirRow);

    m_formatCombo = new QComboBox(downloadBox);
    m_formatCombo->addItem("Найкраща якість (mp4, відео+аудіо)");
    m_formatCombo->addItem("Найкраща якість (будь-який контейнер)");
    m_formatCombo->addItem("Тільки аудіо (mp3)");
    m_formatCombo->addItem("Власні аргументи");
    downloadLayout->addRow("Формат:", m_formatCombo);

    m_codecCombo = new QComboBox(downloadBox);
    m_codecCombo->addItem("Без переваг (як завантажиться)");
    m_codecCombo->addItem("H.264/AVC — перевага (швидко, без перекодування)");
    m_codecCombo->addItem("H.264/AVC — примусово перекодувати (сумісно з Adobe, повільно)");
    m_codecCombo->addItem("H.265/HEVC — перевага");
    m_codecCombo->addItem("VP9 — перевага");
    downloadLayout->addRow("Відеокодек:", m_codecCombo);

    m_customArgsEdit = new QLineEdit(downloadBox);
    m_customArgsEdit->setPlaceholderText("напр.: -f 137+140 --embed-thumbnail");
    m_customArgsEdit->setEnabled(false);
    downloadLayout->addRow("Власні аргументи:", m_customArgsEdit);

    m_cookiesCombo = new QComboBox(downloadBox);
    m_cookiesCombo->addItem("Не використовувати (без входу)");
    m_cookiesCombo->addItem("Chrome");
    m_cookiesCombo->addItem("Firefox");
    m_cookiesCombo->addItem("Edge");
    m_cookiesCombo->addItem("Brave");
    m_cookiesCombo->addItem("Opera");
    m_cookiesCombo->addItem("Vivaldi");
    downloadLayout->addRow("Куки з браузера:", m_cookiesCombo);

    auto *cookiesFileRow = new QWidget(downloadBox);
    auto *cookiesFileRowLayout = new QHBoxLayout(cookiesFileRow);
    cookiesFileRowLayout->setContentsMargins(0, 0, 0, 0);
    m_cookiesFileEdit = new QLineEdit(cookiesFileRow);
    m_cookiesFileEdit->setPlaceholderText("(необов'язково) шлях до cookies.txt — має пріоритет над 'Куки з браузера'");
    auto *cookiesFileBrowseBtn = new QPushButton("Огляд...", cookiesFileRow);
    cookiesFileRowLayout->addWidget(m_cookiesFileEdit);
    cookiesFileRowLayout->addWidget(cookiesFileBrowseBtn);
    downloadLayout->addRow("Файл cookies.txt:", cookiesFileRow);

    m_extraArgsEdit = new QLineEdit(downloadBox);
    m_extraArgsEdit->setPlaceholderText("напр.: --remote-components ejs:github (додається завжди, незалежно від формату)");
    downloadLayout->addRow("Додаткові аргументи:", m_extraArgsEdit);

    auto *checksRow = new QWidget(downloadBox);
    auto *checksRowLayout = new QHBoxLayout(checksRow);
    checksRowLayout->setContentsMargins(0, 0, 0, 0);
    m_subsCheck = new QCheckBox("Завантажити субтитри", checksRow);
    m_playlistCheck = new QCheckBox("Дозволити плейлист (за замовч. лише одне відео)", checksRow);
    checksRowLayout->addWidget(m_subsCheck);
    checksRowLayout->addWidget(m_playlistCheck);
    checksRowLayout->addStretch();
    downloadLayout->addRow("", checksRow);

    mainLayout->addWidget(downloadBox);

    // --- Кнопки керування ---
    auto *buttonsRow = new QWidget(central);
    auto *buttonsLayout = new QHBoxLayout(buttonsRow);
    buttonsLayout->setContentsMargins(0, 0, 0, 0);
    m_downloadButton = new QPushButton("Завантажити", buttonsRow);
    m_downloadButton->setObjectName("downloadButton");
    m_downloadButton->setMinimumHeight(40);
    m_cancelButton = new QPushButton("Скасувати", buttonsRow);
    m_cancelButton->setObjectName("cancelButton");
    m_cancelButton->setMinimumHeight(40);
    m_cancelButton->setEnabled(false);
    buttonsLayout->addWidget(m_downloadButton);
    buttonsLayout->addWidget(m_cancelButton);
    mainLayout->addWidget(buttonsRow);

    m_progressBar = new QProgressBar(central);
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);
    mainLayout->addWidget(m_progressBar);

    mainLayout->addWidget(new QLabel("Лог:", central));
    m_logEdit = new QPlainTextEdit(central);
    m_logEdit->setReadOnly(true);
    m_logEdit->setMaximumBlockCount(5000);
    mainLayout->addWidget(m_logEdit, 1);

    // --- З'єднання сигналів ---
    connect(ytDlpBrowseBtn, &QPushButton::clicked, this, &MainWindow::onBrowseYtDlp);
    connect(ffmpegBrowseBtn, &QPushButton::clicked, this, &MainWindow::onBrowseFfmpeg);
    connect(cookiesFileBrowseBtn, &QPushButton::clicked, this, &MainWindow::onBrowseCookiesFile);
    connect(outDirBrowseBtn, &QPushButton::clicked, this, &MainWindow::onBrowseOutputDir);
    connect(checkVersionBtn, &QPushButton::clicked, this, &MainWindow::onCheckVersion);
    connect(m_formatCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onFormatChanged);
    connect(m_downloadButton, &QPushButton::clicked, this, &MainWindow::onDownloadClicked);
    connect(m_cancelButton, &QPushButton::clicked, this, &MainWindow::onCancelClicked);
}

void MainWindow::loadSettings()
{
    QSettings settings;
    m_ytDlpPathEdit->setText(settings.value("ytDlpPath", "yt-dlp").toString());
    m_ffmpegPathEdit->setText(settings.value("ffmpegPath", "").toString());

    QString defaultDir = QStandardPaths::writableLocation(QStandardPaths::MoviesLocation);
    if (defaultDir.isEmpty())
        defaultDir = QDir::homePath();
    m_outputDirEdit->setText(settings.value("outputDir", defaultDir).toString());

    m_formatCombo->setCurrentIndex(settings.value("formatPreset", 0).toInt());
    m_codecCombo->setCurrentIndex(settings.value("codecChoice", 0).toInt());
    m_cookiesCombo->setCurrentIndex(settings.value("cookiesBrowser", 0).toInt());
    m_cookiesFileEdit->setText(settings.value("cookiesFile", "").toString());
    m_extraArgsEdit->setText(settings.value("extraArgs", "").toString());
}

void MainWindow::saveSettings()
{
    QSettings settings;
    settings.setValue("ytDlpPath", m_ytDlpPathEdit->text());
    settings.setValue("ffmpegPath", m_ffmpegPathEdit->text());
    settings.setValue("outputDir", m_outputDirEdit->text());
    settings.setValue("formatPreset", m_formatCombo->currentIndex());
    settings.setValue("codecChoice", m_codecCombo->currentIndex());
    settings.setValue("cookiesBrowser", m_cookiesCombo->currentIndex());
    settings.setValue("cookiesFile", m_cookiesFileEdit->text());
    settings.setValue("extraArgs", m_extraArgsEdit->text());
}

void MainWindow::onBrowseYtDlp()
{
    QString file = QFileDialog::getOpenFileName(this, "Вкажи yt-dlp.exe", QString(),
#ifdef Q_OS_WIN
        "Виконувані файли (*.exe);;Усі файли (*)"
#else
        "Усі файли (*)"
#endif
    );
    if (!file.isEmpty())
        m_ytDlpPathEdit->setText(file);
}

void MainWindow::onBrowseFfmpeg()
{
    QString dir = QFileDialog::getExistingDirectory(this, "Вкажи папку з ffmpeg.exe");
    if (!dir.isEmpty())
        m_ffmpegPathEdit->setText(dir);
}

void MainWindow::onBrowseCookiesFile()
{
    QString file = QFileDialog::getOpenFileName(this, "Вкажи файл cookies.txt", QString(),
        "Текстові файли (*.txt);;Усі файли (*)");
    if (!file.isEmpty())
        m_cookiesFileEdit->setText(file);
}

void MainWindow::onBrowseOutputDir()
{
    QString dir = QFileDialog::getExistingDirectory(this, "Папка для збереження відео", m_outputDirEdit->text());
    if (!dir.isEmpty())
        m_outputDirEdit->setText(dir);
}

void MainWindow::onFormatChanged(int index)
{
    m_customArgsEdit->setEnabled(index == PresetCustom);
}

void MainWindow::onCheckVersion()
{
    QProcess check;
    check.start(m_ytDlpPathEdit->text(), {"--version"});
    if (!check.waitForFinished(5000)) {
        QMessageBox::warning(this, "yt-dlp", "Не вдалося запустити yt-dlp. Перевір шлях у полі вище.");
        return;
    }
    QString output = QString::fromUtf8(check.readAllStandardOutput()).trimmed();
    QString errOutput = QString::fromUtf8(check.readAllStandardError()).trimmed();
    if (check.exitCode() == 0 && !output.isEmpty()) {
        QMessageBox::information(this, "yt-dlp", "Версія yt-dlp: " + output);
    } else {
        QMessageBox::warning(this, "yt-dlp",
            "yt-dlp не відповів коректно.\n" + (errOutput.isEmpty() ? output : errOutput));
    }
}

QStringList MainWindow::buildArguments() const
{
    QStringList args;

    // ffmpeg location, якщо вказана
    if (!m_ffmpegPathEdit->text().trimmed().isEmpty()) {
        args << "--ffmpeg-location" << m_ffmpegPathEdit->text().trimmed();
    }

    // шаблон імені файлу
    QString outTemplate = m_outputDirEdit->text().trimmed();
    if (!outTemplate.isEmpty() && !outTemplate.endsWith('/') && !outTemplate.endsWith('\\'))
        outTemplate += "/";
    outTemplate += "%(title)s.%(ext)s";
    args << "-o" << outTemplate;

    if (!m_playlistCheck->isChecked())
        args << "--no-playlist";

    QString cookiesFile = m_cookiesFileEdit->text().trimmed();
    if (!cookiesFile.isEmpty()) {
        args << "--cookies" << cookiesFile;
    } else if (m_cookiesCombo->currentIndex() > 0) {
        static const QStringList browserIds = {
            "", "chrome", "firefox", "edge", "brave", "opera", "vivaldi"
        };
        args << "--cookies-from-browser" << browserIds.at(m_cookiesCombo->currentIndex());
    }

    if (m_subsCheck->isChecked())
        args << "--write-subs" << "--write-auto-subs" << "--sub-langs" << "uk,en";

    switch (m_formatCombo->currentIndex()) {
    case PresetBestMp4:
        args << "-f" << "bestvideo[ext=mp4]+bestaudio[ext=m4a]/best[ext=mp4]/best"
             << "--merge-output-format" << "mp4";
        break;
    case PresetBestAny:
        args << "-f" << "bestvideo+bestaudio/best";
        break;
    case PresetAudioMp3:
        args << "-x" << "--audio-format" << "mp3";
        break;
    case PresetCustom: {
        QString custom = m_customArgsEdit->text().trimmed();
        if (!custom.isEmpty()) {
            args << QProcess::splitCommand(custom);
        }
        break;
    }
    }

    switch (m_codecCombo->currentIndex()) {
    case CodecNone:
        break;
    case CodecH264Prefer:
        args << "--format-sort" << "vcodec:h264";
        break;
    case CodecH264Force:
        args << "--format-sort" << "vcodec:h264"
             << "--recode-video" << "mp4"
             << "--postprocessor-args" << "VideoConvertor:-c:v libx264 -c:a aac -crf 18 -preset medium";
        break;
    case CodecH265Prefer:
        args << "--format-sort" << "vcodec:h265";
        break;
    case CodecVp9Prefer:
        args << "--format-sort" << "vcodec:vp9";
        break;
    }

    // додаткові аргументи, які користувач вписав вручну (завжди застосовуються)
    QString extra = m_extraArgsEdit->text().trimmed();
    if (!extra.isEmpty()) {
        args << QProcess::splitCommand(extra);
    }

    // прогрес у машинно-читаному вигляді, лінія за лінією
    args << "--newline";

    args << m_urlEdit->text().trimmed();

    return args;
}

void MainWindow::onDownloadClicked()
{
    QString url = m_urlEdit->text().trimmed();
    if (url.isEmpty()) {
        QMessageBox::warning(this, "YtDlpGui", "Спочатку встав URL відео.");
        return;
    }
    if (m_ytDlpPathEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "YtDlpGui", "Вкажи шлях до yt-dlp.");
        return;
    }

    QDir outDir(m_outputDirEdit->text());
    if (!outDir.exists()) {
        if (!outDir.mkpath(".")) {
            QMessageBox::warning(this, "YtDlpGui", "Не вдалося створити папку збереження.");
            return;
        }
    }

    saveSettings();

    QStringList args = buildArguments();
    appendLog("Запуск: " + m_ytDlpPathEdit->text() + " " + args.join(' '));

    m_progressBar->setValue(0);
    setDownloadingState(true);

    m_process->setProcessChannelMode(QProcess::SeparateChannels);
    m_process->start(m_ytDlpPathEdit->text(), args);
}

void MainWindow::onCancelClicked()
{
    if (m_process->state() != QProcess::NotRunning) {
        m_process->kill();
        appendLog("Завантаження скасовано користувачем.");
    }
}

void MainWindow::onProcessReadyReadStdout()
{
    while (m_process->canReadLine()) {
        QString line = QString::fromUtf8(m_process->readLine()).trimmed();
        if (line.isEmpty())
            continue;
        parseProgressLine(line);
        appendLog(line);
    }
}

void MainWindow::onProcessReadyReadStderr()
{
    QString data = QString::fromUtf8(m_process->readAllStandardError());
    const QStringList lines = data.split('\n', Qt::SkipEmptyParts);
    for (const QString &line : lines)
        appendLog(line.trimmed(), true);
}

void MainWindow::onProcessFinished(int exitCode, QProcess::ExitStatus status)
{
    setDownloadingState(false);
    if (status == QProcess::CrashExit) {
        appendLog("Процес yt-dlp завершився аварійно.", true);
    } else if (exitCode == 0) {
        m_progressBar->setValue(100);
        appendLog("Готово! Завантаження завершено успішно.");
    } else {
        appendLog(QString("yt-dlp завершився з кодом помилки %1.").arg(exitCode), true);
    }
}

void MainWindow::onProcessErrorOccurred(QProcess::ProcessError error)
{
    Q_UNUSED(error);
    setDownloadingState(false);
    appendLog("Не вдалося запустити yt-dlp. Перевір, чи правильний шлях вказано в полі 'yt-dlp'.", true);
}

void MainWindow::parseProgressLine(const QString &line)
{
    // Приклад рядка yt-dlp: "[download]  45.2% of   10.00MiB at    1.20MiB/s ETA 00:05"
    static const QRegularExpression re(R"(\[download\]\s+(\d{1,3}(?:\.\d+)?)%)");
    QRegularExpressionMatch match = re.match(line);
    if (match.hasMatch()) {
        bool ok = false;
        double percent = match.captured(1).toDouble(&ok);
        if (ok) {
            m_progressBar->setValue(static_cast<int>(percent));
        }
    }
}

void MainWindow::appendLog(const QString &text, bool isError)
{
    if (isError)
        m_logEdit->appendHtml("<span style=\"color:#d9534f;\">" + text.toHtmlEscaped() + "</span>");
    else
        m_logEdit->appendPlainText(text);
}

void MainWindow::setDownloadingState(bool downloading)
{
    m_downloadButton->setEnabled(!downloading);
    m_cancelButton->setEnabled(downloading);
    m_urlEdit->setEnabled(!downloading);
}