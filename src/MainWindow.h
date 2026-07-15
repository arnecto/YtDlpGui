#pragma once

#include <QMainWindow>
#include <QProcess>

class QLineEdit;
class QComboBox;
class QPushButton;
class QProgressBar;
class QPlainTextEdit;
class QCheckBox;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void onBrowseYtDlp();
    void onBrowseOutputDir();
    void onBrowseFfmpeg();
    void onBrowseCookiesFile();
    void onFormatChanged(int index);
    void onCheckVersion();
    void onDownloadClicked();
    void onCancelClicked();

    void onProcessReadyReadStdout();
    void onProcessReadyReadStderr();
    void onProcessFinished(int exitCode, QProcess::ExitStatus status);
    void onProcessErrorOccurred(QProcess::ProcessError error);

private:
    void buildUi();
    void loadSettings();
    void saveSettings();
    QStringList buildArguments() const;
    void appendLog(const QString &text, bool isError = false);
    void parseProgressLine(const QString &line);
    void setDownloadingState(bool downloading);

    // widgets
    QLineEdit *m_ytDlpPathEdit = nullptr;
    QLineEdit *m_ffmpegPathEdit = nullptr;
    QLineEdit *m_urlEdit = nullptr;
    QLineEdit *m_outputDirEdit = nullptr;
    QComboBox *m_formatCombo = nullptr;
    QComboBox *m_codecCombo = nullptr;
    QLineEdit *m_customArgsEdit = nullptr;
    QComboBox *m_cookiesCombo = nullptr;
    QLineEdit *m_cookiesFileEdit = nullptr;
    QLineEdit *m_extraArgsEdit = nullptr;
    QCheckBox *m_subsCheck = nullptr;
    QCheckBox *m_playlistCheck = nullptr;
    QPushButton *m_downloadButton = nullptr;
    QPushButton *m_cancelButton = nullptr;
    QProgressBar *m_progressBar = nullptr;
    QPlainTextEdit *m_logEdit = nullptr;

    QProcess *m_process = nullptr;
};