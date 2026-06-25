#include <KPropertiesDialogPlugin>

#include <KPluginFactory>

#include <kio_version.h>

#include <QByteArray>
#include <QComboBox>
#include <QFile>
#include <QFileInfo>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QLayout>
#include <QLocale>
#include <QLoggingCategory>
#include <QProcess>
#include <QPushButton>
#include <QStandardPaths>
#include <QString>
#include <QUrl>
#include <QVariantList>
#include <QVBoxLayout>
#include <QWidget>

#include <cstdint>

namespace {

Q_LOGGING_CATEGORY(LOG_BOX64_PROFILE_ACTION, "box64.profile.propertiesplugin", QtWarningMsg)

struct ProfileAction {
    const char *name;
    const char *label;
    const char *zhLabel;
    const char *description;
    const char *zhDescription;
};

constexpr ProfileAction PROFILE_ACTIONS[] = {
    {"default",
     "Default",
     "默认",
     "The default settings that most programs runs fine with a handy performance.",
     "大多数程序都能正常运行且性能适中的默认设置。"},
    {"safest",
     "Safest",
     "最安全",
     "Profile with all the unsafe DynaRec optimizations disabled.",
     "禁用所有不安全的优化。"},
    {"safe",
     "Safe",
     "安全",
     "Well, less safer than safest.",
     "仅启用小部分不安全的优化。"},
    {"fast",
     "Fast",
     "快速",
     "Enable many unsafe optimizations, but also enable strongmem emulation.",
     "启用许多不安全的优化，但同时启用强内存模型模拟。"},
    {"fastest",
     "Fastest",
     "最快",
     "Enable many unsafe optimizations to have a better performance.",
     "启用许多不安全的优化以获得更好的性能。"},
};

enum class ExecutableSupport {
    Unsupported,
    SupportedExecutable,
    SharedLibrary,
};

bool useChineseLanguage()
{
    return QLocale::system().language() == QLocale::Chinese;
}

QString localizedText(const char *english, const char *chinese)
{
    return useChineseLanguage() ? QString::fromUtf8(chinese) : QString::fromLatin1(english);
}

QString configuratorCommand()
{
    const QString command = QStandardPaths::findExecutable(QStringLiteral("box64-configurator"));
    return command.isEmpty() ? QStringLiteral("box64-configurator") : command;
}

QString normalizeProfile(QString value)
{
    value = value.trimmed().toLower();
    for (const ProfileAction &profile : PROFILE_ACTIONS) {
        if (value == QLatin1String(profile.name)) {
            return value;
        }
    }
    return QStringLiteral("default");
}

QString profileLabel(const QString &name)
{
    for (const ProfileAction &profile : PROFILE_ACTIONS) {
        if (name == QLatin1String(profile.name)) {
            return localizedText(profile.label, profile.zhLabel);
        }
    }
    return localizedText("Default", "默认");
}

QString profileDescription(const QString &name)
{
    for (const ProfileAction &profile : PROFILE_ACTIONS) {
        if (name == QLatin1String(profile.name)) {
            return localizedText(profile.description, profile.zhDescription);
        }
    }
    return localizedText(
        "The default settings that most programs runs fine with a handy performance.",
        "大多数程序都能正常运行且性能适中的默认设置。");
}

QString currentProfile(const QString &command, const QString &path)
{
    QProcess process;
    process.setProgram(command);
    process.setArguments({QStringLiteral("--get-profile"), path});
    process.setProcessChannelMode(QProcess::MergedChannels);
    process.start();
    if (!process.waitForStarted(500) || !process.waitForFinished(2000)) {
        return QStringLiteral("default");
    }
    if (process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0) {
        return QStringLiteral("default");
    }
    return normalizeProfile(QString::fromUtf8(process.readAllStandardOutput()));
}

uint16_t read16(const QByteArray &data, int offset, bool littleEndian)
{
    const auto first = static_cast<uint8_t>(data.at(offset));
    const auto second = static_cast<uint8_t>(data.at(offset + 1));
    if (littleEndian) {
        return static_cast<uint16_t>(first | (second << 8));
    }
    return static_cast<uint16_t>((first << 8) | second);
}

uint32_t read32Le(const QByteArray &data, int offset)
{
    return static_cast<uint32_t>(static_cast<uint8_t>(data.at(offset)))
        | (static_cast<uint32_t>(static_cast<uint8_t>(data.at(offset + 1))) << 8)
        | (static_cast<uint32_t>(static_cast<uint8_t>(data.at(offset + 2))) << 16)
        | (static_cast<uint32_t>(static_cast<uint8_t>(data.at(offset + 3))) << 24);
}

ExecutableSupport classifyX86Executable(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        return ExecutableSupport::Unsupported;
    }

    const QByteArray header = file.read(64);
    if (header.size() >= 20
        && static_cast<uint8_t>(header.at(0)) == 0x7F
        && header.at(1) == 'E'
        && header.at(2) == 'L'
        && header.at(3) == 'F') {
        const auto elfClass = static_cast<uint8_t>(header.at(4));
        const auto elfData = static_cast<uint8_t>(header.at(5));
        if (elfData != 1 && elfData != 2) {
            return ExecutableSupport::Unsupported;
        }

        const uint16_t type = read16(header, 16, elfData == 1);
        const uint16_t machine = read16(header, 18, elfData == 1);
        if (!((elfClass == 1 && machine == 3) || (elfClass == 2 && machine == 62))) {
            return ExecutableSupport::Unsupported;
        }
        return (type == 2 || type == 3)
            ? ExecutableSupport::SupportedExecutable
            : ExecutableSupport::Unsupported;
    }

    if (header.size() >= 64 && header.startsWith("MZ")) {
        const uint32_t peOffset = read32Le(header, 0x3C);
        if (peOffset > 1024 * 1024) {
            return ExecutableSupport::Unsupported;
        }
        if (!file.seek(peOffset)) {
            return ExecutableSupport::Unsupported;
        }
        const QByteArray peHeader = file.read(24);
        if (peHeader.size() < 24 || peHeader.at(0) != 'P' || peHeader.at(1) != 'E'
            || peHeader.at(2) != '\0' || peHeader.at(3) != '\0') {
            return ExecutableSupport::Unsupported;
        }

        const uint16_t machine = read16(peHeader, 4, true);
        if (machine != 0x014C && machine != 0x8664) {
            return ExecutableSupport::Unsupported;
        }

        const uint16_t characteristics = read16(peHeader, 22, true);
        if (characteristics & 0x2000) {
            return ExecutableSupport::SharedLibrary;
        }
        return ExecutableSupport::SupportedExecutable;
    }

    return ExecutableSupport::Unsupported;
}

bool isSupportedExecutable(const KFileItem &item)
{
    if (!item.isLocalFile() || !item.isFile()) {
        return false;
    }

    const QString localPath = item.localPath();
    const QFileInfo localInfo(localPath);
    if (localPath.isEmpty() || !localInfo.isFile()) {
        return false;
    }

    const QString fileName = localInfo.fileName();
    if (item.mimetype() == QLatin1String("application/x-sharedlib")
        || fileName.endsWith(QLatin1String(".so"), Qt::CaseInsensitive)
        || fileName.contains(QLatin1String(".so."), Qt::CaseInsensitive)
        || fileName.endsWith(QLatin1String(".dll"), Qt::CaseInsensitive)) {
        qCDebug(LOG_BOX64_PROFILE_ACTION) << "skipping shared library for Box64 profile page" << localPath;
        return false;
    }

    const ExecutableSupport support = classifyX86Executable(localPath);
    if (support != ExecutableSupport::SupportedExecutable) {
        qCDebug(LOG_BOX64_PROFILE_ACTION) << "unsupported executable for Box64 profile page" << localPath << static_cast<int>(support);
        return false;
    }

    return true;
}

}

class Box64ProfilePropertiesPlugin : public KPropertiesDialogPlugin {
    Q_OBJECT

public:
    explicit Box64ProfilePropertiesPlugin(QObject *parent = nullptr)
#if KIO_VERSION_MAJOR < 6
        : KPropertiesDialogPlugin(qobject_cast<KPropertiesDialog *>(parent))
#else
        : KPropertiesDialogPlugin(parent)
#endif
        , m_command(configuratorCommand())
    {
        const KFileItemList items = properties->items();
        if (items.size() != 1) {
            return;
        }

        const KFileItem item = items.constFirst();
        if (!isSupportedExecutable(item)) {
            return;
        }

        m_path = item.localPath();
        m_currentProfile = currentProfile(m_command, m_path);
        qCDebug(LOG_BOX64_PROFILE_ACTION) << "adding Box64 profile page for" << m_path << "profile" << m_currentProfile;

        auto *page = new QWidget();
        auto *layout = new QVBoxLayout(page);
        layout->setSpacing(12);

        auto *description = new QLabel(localizedText(
            "Select the Box64 compatibility profile for this executable.",
            "为此可执行文件选择 Box64 兼容性预设。"));
        description->setWordWrap(true);
        layout->addWidget(description);

        auto *profileGrid = new QGridLayout();
        profileGrid->setSpacing(8);
        profileGrid->setColumnStretch(1, 1);

        auto *profileLabelWidget = new QLabel(localizedText("Profile:", "预设："));
        profileGrid->addWidget(profileLabelWidget, 0, 0, Qt::AlignRight | Qt::AlignVCenter);

        m_profileCombo = new QComboBox(page);
        for (const ProfileAction &profile : PROFILE_ACTIONS) {
            const QString profileName = QString::fromLatin1(profile.name);
            m_profileCombo->addItem(profileLabel(profileName), profileName);
        }
        m_profileCombo->setCurrentIndex(profileIndex(m_currentProfile));
        connect(m_profileCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() {
            updateProfileDescription();
            updateDirtyState();
        });

        m_applyButton = new QPushButton(localizedText("Apply", "应用"), page);
        m_applyButton->setEnabled(false);
        connect(m_applyButton, &QPushButton::clicked, this, [this]() {
            if (applySelectedProfile()) {
                m_currentProfile = selectedProfile();
                updateDirtyState();
            }
        });

        auto *comboRow = new QHBoxLayout();
        comboRow->setSpacing(8);
        comboRow->addWidget(m_profileCombo, 1);
        comboRow->addWidget(m_applyButton);
        profileGrid->addLayout(comboRow, 0, 1);

        m_profileDescription = new QLabel(page);
        m_profileDescription->setWordWrap(true);
        m_profileDescription->setStyleSheet(QStringLiteral("QLabel { color: palette(text); opacity: 0.75; }"));
        updateProfileDescription();
        profileGrid->addWidget(m_profileDescription, 1, 1);

        layout->addLayout(profileGrid);

        layout->addSpacing(12);

        auto *openButton = new QPushButton(QIcon::fromTheme(QStringLiteral("application-x-executable")), localizedText(
            "Open in Box64 Configurator",
            "在 Box64 配置器中打开"), page);
        connect(openButton, &QPushButton::clicked, this, [this]() {
            if (!QProcess::startDetached(m_command, {m_path})) {
                qCWarning(LOG_BOX64_PROFILE_ACTION) << "failed to start" << m_command << "for" << m_path;
            }
        });
        layout->addWidget(openButton);

        layout->addStretch();

        properties->addPage(page, localizedText("Box64", "Box64"));
    }

    Box64ProfilePropertiesPlugin(QObject *parent, const QVariantList &)
        : Box64ProfilePropertiesPlugin(parent)
    {
    }

    void applyChanges() override
    {
        if (m_path.isEmpty()) {
            return;
        }

        if (applySelectedProfile()) {
            m_currentProfile = selectedProfile();
            updateDirtyState();
        }
    }

private:
    int profileIndex(const QString &name) const
    {
        const QString normalized = normalizeProfile(name);
        for (int i = 0; i < m_profileCombo->count(); ++i) {
            if (m_profileCombo->itemData(i).toString() == normalized) {
                return i;
            }
        }
        return 0;
    }

    QString selectedProfile() const
    {
        if (!m_profileCombo) {
            return QStringLiteral("default");
        }
        return normalizeProfile(m_profileCombo->currentData().toString());
    }

    void updateProfileDescription()
    {
        if (m_profileDescription) {
            m_profileDescription->setText(profileDescription(selectedProfile()));
        }
    }

    void updateDirtyState()
    {
        const bool profileChanged = selectedProfile() != m_currentProfile;
        setDirty(profileChanged);
        if (m_applyButton) {
            m_applyButton->setEnabled(profileChanged);
        }
        if (profileChanged) {
            emit changed();
        }
    }

    bool applySelectedProfile()
    {
        const QString profile = selectedProfile();
        if (profile == m_currentProfile) {
            return false;
        }

        qCDebug(LOG_BOX64_PROFILE_ACTION) << "applying Box64 profile" << profile << "for" << m_path;
        if (!QProcess::startDetached(m_command, {QStringLiteral("--set-profile"), profile, m_path})) {
            qCWarning(LOG_BOX64_PROFILE_ACTION) << "failed to start" << m_command << "to set profile" << profile << "for" << m_path;
            return false;
        }
        return true;
    }

    QString m_command;
    QString m_path;
    QString m_currentProfile;
    QComboBox *m_profileCombo = nullptr;
    QPushButton *m_applyButton = nullptr;
    QLabel *m_profileDescription = nullptr;
};

K_PLUGIN_CLASS_WITH_JSON(Box64ProfilePropertiesPlugin, "box64_profile_propertiesplugin.json")

#include "box64_profile_propertiesplugin.moc"
