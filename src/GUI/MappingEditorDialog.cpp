#include "MappingEditorDialog.h"

#include "CurvePreview.h"
#include "ProfileStore.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>
#include <QTabWidget>
#include <QTableWidget>
#include <QVBoxLayout>

#include <algorithm>
#include <cmath>
#include <tuple>

namespace
{

enum Column { ColControl, ColMessage, ColChannel, ColNumber, ColCount };

QString const hatNames[] = { "X", "Y", "Up", "Right", "Down", "Left" };

struct MmcCommand
{
    int code;
    char const *name;
};

MmcCommand const mmcCommands[] = {
    { 0x01, QT_TRANSLATE_NOOP("MappingEditorDialog", "Stop") },
    { 0x02, QT_TRANSLATE_NOOP("MappingEditorDialog", "Play") },
    { 0x03, QT_TRANSLATE_NOOP("MappingEditorDialog", "Deferred play") },
    { 0x04, QT_TRANSLATE_NOOP("MappingEditorDialog", "Fast forward") },
    { 0x05, QT_TRANSLATE_NOOP("MappingEditorDialog", "Rewind") },
    { 0x06, QT_TRANSLATE_NOOP("MappingEditorDialog", "Record strobe (punch in)") },
    { 0x07, QT_TRANSLATE_NOOP("MappingEditorDialog", "Record exit (punch out)") },
    { 0x08, QT_TRANSLATE_NOOP("MappingEditorDialog", "Record pause") },
    { 0x09, QT_TRANSLATE_NOOP("MappingEditorDialog", "Pause") },
};

QString mmcCommandName(int code)
{
    for (MmcCommand const &c : mmcCommands)
    {
        if (c.code == code)
        {
            return QObject::tr(c.name);
        }
    }
    return QString();
}

QString kindName(MidiTargetKind kind)
{
    switch (kind)
    {
    case MidiTargetKind::ControlChange: return QObject::tr("Control change");
    case MidiTargetKind::Note: return QObject::tr("Note");
    case MidiTargetKind::PitchBend: return QObject::tr("Pitch bend");
    case MidiTargetKind::ProgramChange: return QObject::tr("Program change");
    case MidiTargetKind::ChannelAftertouch: return QObject::tr("Channel aftertouch");
    case MidiTargetKind::Mmc: return QObject::tr("MMC");
    }
    return QString();
}

std::string const *findLabel(Profile const &p, ControlType type, int index)
{
    for (ControlLabel const &l : p.labels)
    {
        if (l.type == type && l.index == index)
        {
            return &l.text;
        }
    }
    return nullptr;
}

void setLabel(Profile &p, ControlType type, int index, std::string const &text)
{
    for (auto it = p.labels.begin(); it != p.labels.end(); ++it)
    {
        if (it->type == type && it->index == index)
        {
            if (text.empty())
            {
                p.labels.erase(it);
            }
            else
            {
                it->text = text;
            }
            return;
        }
    }

    if (!text.empty())
    {
        ControlLabel l;
        l.type = type;
        l.index = index;
        l.text = text;
        p.labels.push_back(l);
    }
}

QDoubleSpinBox *makeDoubleSpin(double min, double max, double step, int decimals)
{
    QDoubleSpinBox *s = new QDoubleSpinBox();
    s->setRange(min, max);
    s->setSingleStep(step);
    s->setDecimals(decimals);
    return s;
}

QSpinBox *makeSpin(int min, int max)
{
    QSpinBox *s = new QSpinBox();
    s->setRange(min, max);
    return s;
}

} // namespace

MappingEditorDialog::MappingEditorDialog(std::shared_ptr<ProfileLibrary> library, ProfileStore &store,
                                         std::function<void()> onChanged, std::string const &initialGuid,
                                         QWidget *parent) :
    QDialog(parent),
    m_library(std::move(library)),
    m_store(store),
    m_onChanged(std::move(onChanged)),
    m_updating(false)
{
    setWindowTitle(tr("Mapping presets"));
    resize(1180, 680);

    m_applyTimer.setSingleShot(true);
    m_applyTimer.setInterval(100);
    connect(&m_applyTimer, &QTimer::timeout, this, &MappingEditorDialog::applyNow);

    m_clock.start();
    m_flashTimer.setInterval(50);
    connect(&m_flashTimer, &QTimer::timeout, this, &MappingEditorDialog::updateFlashes);

    for (TypeRecord const &r : m_library->records())
    {
        m_original[r.type.guid] = r;
        m_working[r.type.guid] = r;
    }

    buildUi();

    std::vector<TypeRecord> sorted;
    for (auto const &entry : m_working)
    {
        sorted.push_back(entry.second);
    }
    std::sort(sorted.begin(), sorted.end(), [](TypeRecord const &a, TypeRecord const &b) {
        return a.type.name < b.type.name;
    });

    int initialRow = 0;
    for (size_t i = 0; i < sorted.size(); ++i)
    {
        TypeRecord const &t = sorted[i];
        QListWidgetItem *item = new QListWidgetItem(
            tr("%1  (%2 axes, %3 buttons, %4 hats)")
                .arg(QString::fromStdString(t.type.name)).arg(t.type.numAxes).arg(t.type.numButtons).arg(t.type.numHats));
        item->setData(Qt::UserRole, QString::fromStdString(t.type.guid));
        m_typeList->addItem(item);
        if (t.type.guid == initialGuid)
        {
            initialRow = static_cast<int>(i);
        }
    }

    if (m_typeList->count() > 0)
    {
        m_typeList->setCurrentRow(initialRow);
    }
    else
    {
        updateFormState();
    }
}

void MappingEditorDialog::buildUi()
{
    QVBoxLayout *outer = new QVBoxLayout(this);
    QHBoxLayout *top = new QHBoxLayout();
    outer->addLayout(top, 1);

    // Left: types and presets
    QVBoxLayout *left = new QVBoxLayout();
    left->addWidget(new QLabel(tr("Joystick types seen so far")));
    m_typeList = new QListWidget();
    left->addWidget(m_typeList, 1);
    left->addWidget(new QLabel(tr("Presets of the selected type")));
    m_presetList = new QListWidget();
    left->addWidget(m_presetList, 1);

    QGridLayout *presetButtons = new QGridLayout();
    struct { char const *text; void (MappingEditorDialog::*slot)(); } const buttons[] = {
        { "New", &MappingEditorDialog::newPreset },
        { "Duplicate", &MappingEditorDialog::duplicatePreset },
        { "Rename", &MappingEditorDialog::renamePreset },
        { "Delete", &MappingEditorDialog::deletePreset },
        { "Set active", &MappingEditorDialog::activatePreset },
        { "Import...", &MappingEditorDialog::importPreset },
        { "Export...", &MappingEditorDialog::exportPreset },
    };
    int n = 0;
    for (auto const &b : buttons)
    {
        QPushButton *button = new QPushButton(tr(b.text));
        connect(button, &QPushButton::clicked, this, b.slot);
        presetButtons->addWidget(button, n / 2, n % 2);
        ++n;
    }
    left->addLayout(presetButtons);
    top->addLayout(left, 2);

    // Right: tabs
    m_tabs = new QTabWidget();
    top->addWidget(m_tabs, 5);

    // Controls tab
    QWidget *controlsTab = new QWidget();
    QHBoxLayout *controlsLayout = new QHBoxLayout(controlsTab);

    m_controlTable = new QTableWidget(0, ColCount);
    m_controlTable->setHorizontalHeaderLabels(QStringList() << tr("Control") << tr("MIDI message") << tr("Channel") << tr("Number"));
    m_controlTable->horizontalHeader()->setSectionResizeMode(ColControl, QHeaderView::Stretch);
    m_controlTable->horizontalHeader()->setSectionResizeMode(ColMessage, QHeaderView::ResizeToContents);
    m_controlTable->verticalHeader()->hide();
    m_controlTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_controlTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_controlTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    controlsLayout->addWidget(m_controlTable, 3);

    QVBoxLayout *detail = new QVBoxLayout();
    controlsLayout->addLayout(detail, 2);

    QFormLayout *top2 = new QFormLayout();
    m_labelEdit = new QLineEdit();
    m_labelEdit->setPlaceholderText(tr("Optional name for this control"));
    m_mappedCheck = new QCheckBox(tr("Send MIDI for this control"));
    top2->addRow(tr("Label"), m_labelEdit);
    top2->addRow(m_mappedCheck);
    detail->addLayout(top2);

    m_detailGroup = new QGroupBox(tr("MIDI message"));
    QVBoxLayout *groupLayout = new QVBoxLayout(m_detailGroup);
    QFormLayout *common = new QFormLayout();
    m_kindCombo = new QComboBox();
    m_channelSpin = makeSpin(1, 16);
    m_numberLabel = new QLabel(tr("Number"));
    m_numberSpin = makeSpin(0, 127);
    m_mmcLabel = new QLabel(tr("Command"));
    m_mmcCombo = new QComboBox();
    for (MmcCommand const &c : mmcCommands)
    {
        m_mmcCombo->addItem(tr(c.name), c.code);
    }
    m_deviceIdLabel = new QLabel(tr("Device ID"));
    m_deviceIdSpin = makeSpin(0, 127);
    m_deviceIdSpin->setValue(127);
    m_deviceIdSpin->setToolTip(tr("127 addresses all devices"));
    m_invertCheck = new QCheckBox(tr("Invert"));
    common->addRow(tr("Message"), m_kindCombo);
    common->addRow(tr("Channel"), m_channelSpin);
    common->addRow(m_numberLabel, m_numberSpin);
    common->addRow(m_mmcLabel, m_mmcCombo);
    common->addRow(m_deviceIdLabel, m_deviceIdSpin);
    common->addRow(m_invertCheck);
    m_commonForm = common;
    groupLayout->addLayout(common);

    m_discreteGroup = new QGroupBox(tr("Button values"));
    QFormLayout *discrete = new QFormLayout(m_discreteGroup);
    m_onLabel = new QLabel();
    m_offLabel = new QLabel(tr("Value when released"));
    m_onSpin = makeSpin(0, 127);
    m_offSpin = makeSpin(0, 127);
    discrete->addRow(m_onLabel, m_onSpin);
    discrete->addRow(m_offLabel, m_offSpin);
    groupLayout->addWidget(m_discreteGroup);

    m_analogGroup = new QGroupBox(tr("Response"));
    QFormLayout *analog = new QFormLayout(m_analogGroup);
    m_rangeCombo = new QComboBox();
    m_rangeCombo->addItem(tr("Centered (stick, -1..1)"), static_cast<int>(AxisRange::Bipolar));
    m_rangeCombo->addItem(tr("Single direction (pedal, trigger, 0..1)"), static_cast<int>(AxisRange::Unipolar));
    m_curveCombo = new QComboBox();
    m_curveCombo->addItem(tr("Linear"), static_cast<int>(CurveType::Linear));
    m_curveCombo->addItem(tr("Exponential (finer near center)"), static_cast<int>(CurveType::Exponential));
    m_curveCombo->addItem(tr("Logarithmic (more sensitive near center)"), static_cast<int>(CurveType::Logarithmic));
    m_amountSpin = makeDoubleSpin(0.1, 10.0, 0.1, 1);
    m_deadZoneSpin = makeDoubleSpin(0.0, 0.9, 0.01, 2);
    m_smoothingSpin = makeDoubleSpin(0.0, 500.0, 1.0, 0);
    m_smoothingSpin->setSuffix(tr(" ms"));
    m_rateSpin = makeSpin(0, 1000);
    m_rateSpin->setSuffix(tr(" Hz"));
    m_rateSpin->setSpecialValueText(tr("Unlimited"));
    m_preview = new CurvePreview();
    analog->addRow(tr("Range"), m_rangeCombo);
    analog->addRow(tr("Curve"), m_curveCombo);
    analog->addRow(tr("Curve amount"), m_amountSpin);
    analog->addRow(tr("Dead zone"), m_deadZoneSpin);
    analog->addRow(tr("Smoothing"), m_smoothingSpin);
    analog->addRow(tr("Max. message rate"), m_rateSpin);
    analog->addRow(m_preview);
    groupLayout->addWidget(m_analogGroup);

    detail->addWidget(m_detailGroup);
    detail->addStretch(1);

    m_tabs->addTab(controlsTab, tr("Controls"));

    // Sticks tab
    QWidget *sticksTab = new QWidget();
    QHBoxLayout *sticksLayout = new QHBoxLayout(sticksTab);

    QVBoxLayout *stickLeft = new QVBoxLayout();
    m_stickList = new QListWidget();
    stickLeft->addWidget(m_stickList, 1);
    QHBoxLayout *stickButtons = new QHBoxLayout();
    QPushButton *addButton = new QPushButton(tr("Add stick"));
    QPushButton *removeButton = new QPushButton(tr("Remove stick"));
    stickButtons->addWidget(addButton);
    stickButtons->addWidget(removeButton);
    stickLeft->addLayout(stickButtons);
    sticksLayout->addLayout(stickLeft, 2);

    m_stickForm = new QWidget();
    QVBoxLayout *stickFormLayout = new QVBoxLayout(m_stickForm);
    QFormLayout *stickFields = new QFormLayout();
    m_stickLabelEdit = new QLineEdit();
    m_stickXCombo = new QComboBox();
    m_stickYCombo = new QComboBox();
    m_stickRadialSpin = makeDoubleSpin(0.0, 0.9, 0.01, 2);
    m_stickRadialSpin->setSpecialValueText(tr("Off"));
    stickFields->addRow(tr("Name"), m_stickLabelEdit);
    stickFields->addRow(tr("X axis"), m_stickXCombo);
    stickFields->addRow(tr("Y axis"), m_stickYCombo);
    stickFields->addRow(tr("Radial dead zone"), m_stickRadialSpin);
    stickFormLayout->addLayout(stickFields);
    m_stickWarning = new QLabel(tr("X and Y must be different axes."));
    m_stickWarning->setStyleSheet("color: #c00;");
    m_stickWarning->hide();
    stickFormLayout->addWidget(m_stickWarning);
    QPushButton *xToY = new QPushButton(tr("Copy X axis response to Y axis"));
    QPushButton *yToX = new QPushButton(tr("Copy Y axis response to X axis"));
    stickFormLayout->addWidget(xToY);
    stickFormLayout->addWidget(yToX);
    stickFormLayout->addStretch(1);
    sticksLayout->addWidget(m_stickForm, 3);

    m_tabs->addTab(sticksTab, tr("Sticks"));

    QDialogButtonBox *box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(box, &QDialogButtonBox::accepted, this, &MappingEditorDialog::accept);
    connect(box, &QDialogButtonBox::rejected, this, &MappingEditorDialog::reject);
    outer->addWidget(box);

    // Connections
    connect(m_typeList, &QListWidget::currentRowChanged, this, &MappingEditorDialog::onTypeChanged);
    connect(m_presetList, &QListWidget::currentRowChanged, this, &MappingEditorDialog::onPresetChanged);
    connect(m_controlTable, &QTableWidget::currentCellChanged, this, &MappingEditorDialog::onControlChanged);

    connect(m_labelEdit, &QLineEdit::textEdited, this, &MappingEditorDialog::formChanged);
    connect(m_mappedCheck, &QCheckBox::toggled, this, &MappingEditorDialog::formChanged);
    connect(m_invertCheck, &QCheckBox::toggled, this, &MappingEditorDialog::formChanged);
    connect(m_kindCombo, &QComboBox::currentIndexChanged, this, &MappingEditorDialog::formChanged);
    connect(m_channelSpin, &QSpinBox::valueChanged, this, &MappingEditorDialog::formChanged);
    connect(m_numberSpin, &QSpinBox::valueChanged, this, &MappingEditorDialog::formChanged);
    connect(m_mmcCombo, &QComboBox::currentIndexChanged, this, &MappingEditorDialog::formChanged);
    connect(m_deviceIdSpin, &QSpinBox::valueChanged, this, &MappingEditorDialog::formChanged);
    connect(m_onSpin, &QSpinBox::valueChanged, this, &MappingEditorDialog::formChanged);
    connect(m_offSpin, &QSpinBox::valueChanged, this, &MappingEditorDialog::formChanged);
    connect(m_rangeCombo, &QComboBox::currentIndexChanged, this, &MappingEditorDialog::formChanged);
    connect(m_curveCombo, &QComboBox::currentIndexChanged, this, &MappingEditorDialog::formChanged);
    connect(m_amountSpin, &QDoubleSpinBox::valueChanged, this, &MappingEditorDialog::formChanged);
    connect(m_deadZoneSpin, &QDoubleSpinBox::valueChanged, this, &MappingEditorDialog::formChanged);
    connect(m_smoothingSpin, &QDoubleSpinBox::valueChanged, this, &MappingEditorDialog::formChanged);
    connect(m_rateSpin, &QSpinBox::valueChanged, this, &MappingEditorDialog::formChanged);

    connect(m_stickList, &QListWidget::currentRowChanged, this, &MappingEditorDialog::onStickChanged);
    connect(addButton, &QPushButton::clicked, this, &MappingEditorDialog::addStick);
    connect(removeButton, &QPushButton::clicked, this, &MappingEditorDialog::removeStick);
    connect(m_stickLabelEdit, &QLineEdit::textEdited, this, &MappingEditorDialog::stickFormChanged);
    connect(m_stickXCombo, &QComboBox::currentIndexChanged, this, &MappingEditorDialog::stickFormChanged);
    connect(m_stickYCombo, &QComboBox::currentIndexChanged, this, &MappingEditorDialog::stickFormChanged);
    connect(m_stickRadialSpin, &QDoubleSpinBox::valueChanged, this, &MappingEditorDialog::stickFormChanged);
    connect(xToY, &QPushButton::clicked, this, [this]() { copyStickSettings(true); });
    connect(yToX, &QPushButton::clicked, this, [this]() { copyStickSettings(false); });
}

// ---- State access ----

TypeRecord *MappingEditorDialog::currentRecord()
{
    auto it = m_working.find(m_guid);
    return it == m_working.end() ? nullptr : &it->second;
}

int MappingEditorDialog::currentPresetIndex() const
{
    return m_presetList->currentRow();
}

Profile *MappingEditorDialog::currentPreset()
{
    TypeRecord *r = currentRecord();
    int const i = currentPresetIndex();
    if (!r || i < 0 || i >= static_cast<int>(r->presets.size()))
    {
        return nullptr;
    }
    return &r->presets[i];
}

MappingEditorDialog::ControlKey MappingEditorDialog::currentKey() const
{
    int const row = m_controlTable->currentRow();
    return (row >= 0 && row < static_cast<int>(m_rows.size())) ? m_rows[row] : ControlKey();
}

Mapping *MappingEditorDialog::findMapping(Profile &preset, ControlKey const &key)
{
    for (Mapping &m : preset.mappings)
    {
        if (m.source.type == key.type && m.source.index == key.index
            && (key.type != ControlType::Hat || m.hatComponent == key.hat))
        {
            return &m;
        }
    }
    return nullptr;
}

bool MappingEditorDialog::isAnalog(ControlKey const &key)
{
    return key.type == ControlType::Axis
        || (key.type == ControlType::Hat && (key.hat == HatComponent::X || key.hat == HatComponent::Y));
}

void MappingEditorDialog::markDirty()
{
    if (!m_guid.empty())
    {
        m_dirty.insert(m_guid);
        m_applyTimer.start();
    }
}

void MappingEditorDialog::applyNow()
{
    if (m_dirty.empty())
    {
        return;
    }

    for (std::string const &guid : m_dirty)
    {
        m_library->setRecord(m_working[guid]);
    }
    m_onChanged();
}

void MappingEditorDialog::accept()
{
    m_applyTimer.stop();
    applyNow();

    for (std::string const &guid : m_dirty)
    {
        QString error;
        if (!m_store.save(guid, error))
        {
            QMessageBox::warning(this, tr("Cannot save presets"), error);
        }
    }

    QDialog::accept();
}

void MappingEditorDialog::reject()
{
    m_applyTimer.stop();

    if (!m_dirty.empty())
    {
        for (std::string const &guid : m_dirty)
        {
            m_library->setRecord(m_original[guid]);
        }
        m_onChanged();
    }

    QDialog::reject();
}

// ---- Naming ----

QString MappingEditorDialog::controlName(Profile const &preset, ControlKey const &key) const
{
    std::string const *label = findLabel(preset, key.type, key.index);
    QString const custom = label ? QString::fromStdString(*label) : QString();

    switch (key.type)
    {
    case ControlType::Axis:
        if (!custom.isEmpty())
        {
            return custom;
        }
        for (Stick const &s : preset.sticks)
        {
            if (s.xAxis == key.index || s.yAxis == key.index)
            {
                QString const name = s.label.empty() ? tr("Stick") : QString::fromStdString(s.label);
                return name + (s.xAxis == key.index ? " X" : " Y");
            }
        }
        return tr("Axis %1").arg(key.index + 1);

    case ControlType::Button:
        return custom.isEmpty() ? tr("Button %1").arg(key.index + 1) : custom;

    case ControlType::Hat:
        return (custom.isEmpty() ? tr("Hat %1").arg(key.index + 1) : custom) + " "
            + hatNames[static_cast<int>(key.hat)];
    }
    return QString();
}

QString MappingEditorDialog::mappingText(Mapping const *mapping) const
{
    if (mapping && mapping->target.kind == MidiTargetKind::Mmc)
    {
        return kindName(mapping->target.kind) + ": " + mmcCommandName(mapping->target.number);
    }
    return mapping ? kindName(mapping->target.kind) : QString::fromUtf8("\xE2\x80\x94");
}

std::string MappingEditorDialog::uniqueName(std::string const &base) const
{
    auto it = m_working.find(m_guid);
    if (it == m_working.end())
    {
        return base;
    }

    auto exists = [&](std::string const &name) {
        for (Profile const &p : it->second.presets)
        {
            if (p.name == name)
            {
                return true;
            }
        }
        return false;
    };

    std::string name = base;
    for (int n = 2; exists(name); ++n)
    {
        name = base + " " + std::to_string(n);
    }
    return name;
}

int MappingEditorDialog::nextFreeNumber(Profile const &preset, MidiTargetKind kind, int channel) const
{
    if (kind != MidiTargetKind::ControlChange && kind != MidiTargetKind::Note)
    {
        return 0;
    }

    std::set<int> used;
    for (Mapping const &m : preset.mappings)
    {
        if (m.target.kind == kind && m.target.channel == channel)
        {
            used.insert(m.target.number);
        }
    }

    int n = kind == MidiTargetKind::Note ? 36 : 16;
    while (n < 127 && used.count(n))
    {
        ++n;
    }
    return n;
}

// ---- Type and preset selection ----

void MappingEditorDialog::onTypeChanged()
{
    QListWidgetItem *item = m_typeList->currentItem();
    m_guid = item ? item->data(Qt::UserRole).toString().toStdString() : std::string();
    m_lastAxis.clear();
    m_lastHat.clear();

    TypeRecord *r = currentRecord();
    if (!r)
    {
        rebuildPresetList(-1);
        return;
    }

    if (r->presets.empty())
    {
        Profile p = makeDefaultProfile(r->type);
        r->activePreset = p.name;
        r->presets.push_back(p);
        markDirty();
    }

    m_updating = true;
    m_stickXCombo->clear();
    m_stickYCombo->clear();
    for (int i = 0; i < r->type.numAxes; ++i)
    {
        m_stickXCombo->addItem(tr("Axis %1").arg(i + 1));
        m_stickYCombo->addItem(tr("Axis %1").arg(i + 1));
    }
    m_updating = false;

    int selected = 0;
    for (size_t i = 0; i < r->presets.size(); ++i)
    {
        if (r->presets[i].name == r->activePreset)
        {
            selected = static_cast<int>(i);
        }
    }
    rebuildPresetList(selected);
}

void MappingEditorDialog::rebuildPresetList(int selectRow)
{
    TypeRecord *r = currentRecord();

    m_presetList->blockSignals(true);
    m_presetList->clear();
    if (r)
    {
        for (Profile const &p : r->presets)
        {
            QString text = QString::fromStdString(p.name);
            if (p.name == r->activePreset)
            {
                text += tr("  (active)");
            }
            m_presetList->addItem(text);
        }
    }
    m_presetList->setCurrentRow(selectRow);
    m_presetList->blockSignals(false);

    onPresetChanged();
}

void MappingEditorDialog::onPresetChanged()
{
    refreshTable();
    refreshStickList();

    m_controlTable->blockSignals(true);
    m_controlTable->setCurrentCell(m_controlTable->rowCount() > 0 ? 0 : -1, 0);
    m_controlTable->blockSignals(false);

    loadForm();
}

// ---- Controls table ----

void MappingEditorDialog::refreshTable()
{
    Profile *p = currentPreset();
    TypeRecord *r = currentRecord();
    int const keep = m_controlTable->currentRow();

    m_rows.clear();
    if (p && r)
    {
        for (int i = 0; i < r->type.numAxes; ++i)
        {
            ControlKey k;
            k.type = ControlType::Axis;
            k.index = i;
            m_rows.push_back(k);
        }
        for (int i = 0; i < r->type.numButtons; ++i)
        {
            ControlKey k;
            k.type = ControlType::Button;
            k.index = i;
            m_rows.push_back(k);
        }
        for (int h = 0; h < r->type.numHats; ++h)
        {
            for (int c = 0; c < 6; ++c)
            {
                ControlKey k;
                k.type = ControlType::Hat;
                k.index = h;
                k.hat = static_cast<HatComponent>(c);
                m_rows.push_back(k);
            }
        }
    }

    // Flag messages that are produced by more than one control.
    std::map<std::tuple<int, int, int>, int> uses;
    if (p)
    {
        for (Mapping const &m : p->mappings)
        {
            if (m.target.kind == MidiTargetKind::ControlChange || m.target.kind == MidiTargetKind::Note)
            {
                ++uses[std::make_tuple(static_cast<int>(m.target.kind), m.target.channel, m.target.number)];
            }
        }
    }

    m_controlTable->blockSignals(true);
    m_controlTable->setRowCount(static_cast<int>(m_rows.size()));
    for (size_t row = 0; row < m_rows.size(); ++row)
    {
        Mapping const *m = findMapping(*p, m_rows[row]);

        QString channel;
        QString number;
        bool duplicate = false;
        if (m)
        {
            channel = QString::number(m->target.channel + 1);
            if (m->target.kind == MidiTargetKind::Mmc)
            {
                channel.clear();
            }
            if (m->target.kind == MidiTargetKind::ControlChange || m->target.kind == MidiTargetKind::Note
                || m->target.kind == MidiTargetKind::ProgramChange)
            {
                number = QString::number(m->target.number);
            }
            duplicate = uses[std::make_tuple(static_cast<int>(m->target.kind), m->target.channel, m->target.number)] > 1
                && m->target.kind != MidiTargetKind::ProgramChange
                && m->target.kind != MidiTargetKind::ChannelAftertouch
                && m->target.kind != MidiTargetKind::PitchBend;
        }

        QString const texts[ColCount] = { controlName(*p, m_rows[row]), mappingText(m), channel, number };
        for (int col = 0; col < ColCount; ++col)
        {
            QTableWidgetItem *item = new QTableWidgetItem(texts[col]);
            item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
            if (duplicate && col == ColNumber)
            {
                item->setForeground(QBrush(Qt::red));
                item->setToolTip(tr("Another control sends the same message."));
            }
            m_controlTable->setItem(static_cast<int>(row), col, item);
        }
    }

    if (keep >= 0 && keep < m_controlTable->rowCount())
    {
        m_controlTable->setCurrentCell(keep, 0);
    }
    m_controlTable->blockSignals(false);
}

void MappingEditorDialog::onControlChanged()
{
    loadForm();
}

// ---- Activity indicator ----

void MappingEditorDialog::onControlActivity(QString const &guid, int type, int index, float value)
{
    if (guid.toStdString() != m_guid)
    {
        return;
    }

    auto findRow = [this](ControlType t, int i, HatComponent hat) {
        for (size_t row = 0; row < m_rows.size(); ++row)
        {
            ControlKey const &k = m_rows[row];
            if (k.type == t && k.index == i && (t != ControlType::Hat || k.hat == hat))
            {
                return static_cast<int>(row);
            }
        }
        return -1;
    };

    switch (static_cast<ControlType>(type))
    {
    case ControlType::Axis:
    {
        // The first value only sets the baseline, small jitter is ignored.
        auto it = m_lastAxis.find(index);
        if (it == m_lastAxis.end())
        {
            m_lastAxis[index] = value;
        }
        else if (std::abs(value - it->second) > 0.05f)
        {
            it->second = value;
            flashRow(findRow(ControlType::Axis, index, HatComponent::X), false);
        }
        break;
    }
    case ControlType::Button:
        flashRow(findRow(ControlType::Button, index, HatComponent::X), value > 0.5f);
        break;
    case ControlType::Hat:
    {
        int const mask = static_cast<int>(value);
        int const changed = mask ^ m_lastHat[index];
        m_lastHat[index] = mask;

        struct { int bit; HatComponent component; } const dirs[] = {
            { HatUp, HatComponent::Up }, { HatRight, HatComponent::Right },
            { HatDown, HatComponent::Down }, { HatLeft, HatComponent::Left } };
        for (auto const &d : dirs)
        {
            if (changed & d.bit)
            {
                flashRow(findRow(ControlType::Hat, index, d.component), (mask & d.bit) != 0);
            }
        }
        if (changed & (HatLeft | HatRight))
        {
            flashRow(findRow(ControlType::Hat, index, HatComponent::X), false);
        }
        if (changed & (HatUp | HatDown))
        {
            flashRow(findRow(ControlType::Hat, index, HatComponent::Y), false);
        }
        break;
    }
    }
}

void MappingEditorDialog::flashRow(int row, bool scroll)
{
    if (row < 0 || row >= m_controlTable->rowCount())
    {
        return;
    }

    m_flashes[row] = m_clock.elapsed();
    for (int col = 0; col < ColCount; ++col)
    {
        if (QTableWidgetItem *item = m_controlTable->item(row, col))
        {
            item->setBackground(QBrush(QColor(80, 200, 80)));
        }
    }
    if (scroll)
    {
        m_controlTable->scrollToItem(m_controlTable->item(row, ColControl));
    }
    if (!m_flashTimer.isActive())
    {
        m_flashTimer.start();
    }
}

void MappingEditorDialog::updateFlashes()
{
    qint64 const now = m_clock.elapsed();
    for (auto it = m_flashes.begin(); it != m_flashes.end();)
    {
        if (now - it->second > 400)
        {
            for (int col = 0; col < ColCount; ++col)
            {
                if (QTableWidgetItem *item = m_controlTable->item(it->first, col))
                {
                    item->setBackground(QBrush());
                }
            }
            it = m_flashes.erase(it);
        }
        else
        {
            ++it;
        }
    }
    if (m_flashes.empty())
    {
        m_flashTimer.stop();
    }
}

void MappingEditorDialog::loadForm()
{
    Profile *p = currentPreset();
    int const row = m_controlTable->currentRow();

    m_updating = true;

    if (p && row >= 0 && row < static_cast<int>(m_rows.size()))
    {
        ControlKey const key = m_rows[row];
        Mapping const *m = findMapping(*p, key);
        std::string const *label = findLabel(*p, key.type, key.index);

        m_labelEdit->setText(label ? QString::fromStdString(*label) : QString());
        m_mappedCheck->setChecked(m != nullptr);

        m_kindCombo->clear();
        m_kindCombo->addItem(kindName(MidiTargetKind::ControlChange), static_cast<int>(MidiTargetKind::ControlChange));
        if (!isAnalog(key))
        {
            m_kindCombo->addItem(kindName(MidiTargetKind::Note), static_cast<int>(MidiTargetKind::Note));
        }
        else
        {
            m_kindCombo->addItem(kindName(MidiTargetKind::PitchBend), static_cast<int>(MidiTargetKind::PitchBend));
        }
        m_kindCombo->addItem(kindName(MidiTargetKind::ProgramChange), static_cast<int>(MidiTargetKind::ProgramChange));
        m_kindCombo->addItem(kindName(MidiTargetKind::ChannelAftertouch), static_cast<int>(MidiTargetKind::ChannelAftertouch));
        if (!isAnalog(key))
        {
            m_kindCombo->addItem(kindName(MidiTargetKind::Mmc), static_cast<int>(MidiTargetKind::Mmc));
        }

        if (m)
        {
            m_kindCombo->setCurrentIndex(std::max(0, m_kindCombo->findData(static_cast<int>(m->target.kind))));
            m_channelSpin->setValue(m->target.channel + 1);
            m_numberSpin->setValue(m->target.number);
            m_mmcCombo->setCurrentIndex(std::max(0, m_mmcCombo->findData(m->target.number)));
            m_deviceIdSpin->setValue(m->target.deviceId);
            m_onSpin->setValue(m->target.onValue);
            m_offSpin->setValue(m->target.offValue);
            m_invertCheck->setChecked(m->invert);
            m_rangeCombo->setCurrentIndex(std::max(0, m_rangeCombo->findData(static_cast<int>(m->range))));
            m_curveCombo->setCurrentIndex(std::max(0, m_curveCombo->findData(static_cast<int>(m->curve.type))));
            m_amountSpin->setValue(m->curve.amount);
            m_deadZoneSpin->setValue(m->deadZone);
            m_smoothingSpin->setValue(m->smoothing);
            m_rateSpin->setValue(static_cast<int>(m->maxRateHz));
        }
    }
    else
    {
        m_labelEdit->clear();
        m_mappedCheck->setChecked(false);
        m_kindCombo->clear();
    }

    m_updating = false;
    updateFormState();
}

void MappingEditorDialog::updateFormState()
{
    Profile *p = currentPreset();
    int const row = m_controlTable->currentRow();
    bool const has = p && row >= 0 && row < static_cast<int>(m_rows.size());

    m_labelEdit->setEnabled(has);
    m_mappedCheck->setEnabled(has);
    m_detailGroup->setEnabled(has && m_mappedCheck->isChecked());
    if (!has)
    {
        return;
    }

    ControlKey const key = m_rows[row];
    bool const analog = isAnalog(key);
    m_analogGroup->setVisible(analog);
    m_discreteGroup->setVisible(!analog);

    MidiTargetKind const kind = static_cast<MidiTargetKind>(m_kindCombo->currentData().toInt());
    bool const mmc = kind == MidiTargetKind::Mmc;
    m_discreteGroup->setVisible(!analog && !mmc);
    m_commonForm->setRowVisible(m_channelSpin, !mmc);
    m_commonForm->setRowVisible(m_numberSpin, !mmc);
    m_commonForm->setRowVisible(m_mmcCombo, mmc);
    m_commonForm->setRowVisible(m_deviceIdSpin, mmc);
    bool const hasNumber = kind == MidiTargetKind::ControlChange || kind == MidiTargetKind::Note
        || kind == MidiTargetKind::ProgramChange;
    m_numberSpin->setEnabled(hasNumber);
    m_numberLabel->setText(kind == MidiTargetKind::ControlChange ? tr("Controller")
                           : kind == MidiTargetKind::Note ? tr("Note")
                           : kind == MidiTargetKind::ProgramChange ? tr("Program") : tr("Number"));

    m_onLabel->setText(kind == MidiTargetKind::Note ? tr("Velocity") : tr("Value when pressed"));
    m_onSpin->setEnabled(kind != MidiTargetKind::ProgramChange);
    m_offSpin->setEnabled(kind == MidiTargetKind::ControlChange || kind == MidiTargetKind::ChannelAftertouch);
    m_offLabel->setEnabled(m_offSpin->isEnabled());

    m_amountSpin->setEnabled(m_curveCombo->currentData().toInt() != static_cast<int>(CurveType::Linear));

    Mapping const *m = findMapping(*p, key);
    if (analog && m)
    {
        m_preview->setMapping(*m);
    }
}

void MappingEditorDialog::formChanged()
{
    if (m_updating)
    {
        return;
    }

    Profile *p = currentPreset();
    int const row = m_controlTable->currentRow();
    if (!p || row < 0 || row >= static_cast<int>(m_rows.size()))
    {
        return;
    }

    ControlKey const key = m_rows[row];
    setLabel(*p, key.type, key.index, m_labelEdit->text().trimmed().toStdString());

    Mapping *m = findMapping(*p, key);
    bool const wantMapped = m_mappedCheck->isChecked();

    if (wantMapped && !m)
    {
        Mapping created;
        created.source.type = key.type;
        created.source.index = key.index;
        created.hatComponent = key.hat;
        if (isAnalog(key))
        {
            created.target.kind = MidiTargetKind::ControlChange;
            created.deadZone = 0.05f;
        }
        else
        {
            created.target.kind = MidiTargetKind::Note;
            created.target.onValue = 100;
        }
        created.target.number = static_cast<uint8_t>(nextFreeNumber(*p, created.target.kind, 0));
        p->mappings.push_back(created);

        markDirty();
        refreshTable();
        loadForm();
        return;
    }

    if (!wantMapped && m)
    {
        p->mappings.erase(p->mappings.begin() + (m - p->mappings.data()));

        markDirty();
        refreshTable();
        loadForm();
        return;
    }

    if (m)
    {
        m->target.kind = static_cast<MidiTargetKind>(m_kindCombo->currentData().toInt());
        m->target.channel = static_cast<uint8_t>(m_channelSpin->value() - 1);
        m->target.number = static_cast<uint8_t>(m_numberSpin->value());
        if (m->target.kind == MidiTargetKind::Mmc)
        {
            m->target.number = static_cast<uint8_t>(m_mmcCombo->currentData().toInt());
        }
        m->target.deviceId = static_cast<uint8_t>(m_deviceIdSpin->value());
        m->target.onValue = static_cast<uint8_t>(m_onSpin->value());
        m->target.offValue = static_cast<uint8_t>(m_offSpin->value());
        m->invert = m_invertCheck->isChecked();

        if (isAnalog(key))
        {
            m->range = static_cast<AxisRange>(m_rangeCombo->currentData().toInt());
            m->curve.type = static_cast<CurveType>(m_curveCombo->currentData().toInt());
            m->curve.amount = static_cast<float>(m_amountSpin->value());
            m->deadZone = static_cast<float>(m_deadZoneSpin->value());
            m->smoothing = static_cast<float>(m_smoothingSpin->value());
            m->maxRateHz = static_cast<float>(m_rateSpin->value());
        }
    }

    markDirty();
    refreshTable();
    updateFormState();
}

// ---- Presets ----

void MappingEditorDialog::newPreset()
{
    TypeRecord *r = currentRecord();
    if (!r)
    {
        return;
    }

    bool ok = false;
    QString const name = QInputDialog::getText(this, tr("New preset"), tr("Name:"), QLineEdit::Normal, tr("New preset"), &ok);
    if (!ok || name.trimmed().isEmpty())
    {
        return;
    }

    Profile p = makeDefaultProfile(r->type);
    p.name = uniqueName(name.trimmed().toStdString());
    r->presets.push_back(p);

    markDirty();
    rebuildPresetList(static_cast<int>(r->presets.size()) - 1);
}

void MappingEditorDialog::duplicatePreset()
{
    TypeRecord *r = currentRecord();
    Profile *p = currentPreset();
    if (!r || !p)
    {
        return;
    }

    Profile copy = *p;
    copy.name = uniqueName(p->name + " copy");
    r->presets.push_back(copy);

    markDirty();
    rebuildPresetList(static_cast<int>(r->presets.size()) - 1);
}

void MappingEditorDialog::renamePreset()
{
    TypeRecord *r = currentRecord();
    Profile *p = currentPreset();
    if (!r || !p)
    {
        return;
    }

    bool ok = false;
    QString const text = QInputDialog::getText(this, tr("Rename preset"), tr("Name:"), QLineEdit::Normal,
                                               QString::fromStdString(p->name), &ok);
    std::string const name = text.trimmed().toStdString();
    if (!ok || name.empty() || name == p->name)
    {
        return;
    }

    for (Profile const &other : r->presets)
    {
        if (other.name == name)
        {
            QMessageBox::warning(this, tr("Cannot rename"), tr("A preset with this name already exists."));
            return;
        }
    }

    std::string const oldName = p->name;
    if (r->activePreset == oldName)
    {
        r->activePreset = name;
    }
    for (auto &entry : r->instancePresets)
    {
        if (entry.second == oldName)
        {
            entry.second = name;
        }
    }
    p->name = name;

    int const row = currentPresetIndex();
    markDirty();
    rebuildPresetList(row);
}

void MappingEditorDialog::deletePreset()
{
    TypeRecord *r = currentRecord();
    Profile *p = currentPreset();
    if (!r || !p)
    {
        return;
    }

    if (r->presets.size() <= 1)
    {
        QMessageBox::information(this, tr("Cannot delete"), tr("A joystick type needs at least one preset."));
        return;
    }

    if (QMessageBox::question(this, tr("Delete preset"),
                              tr("Delete preset \"%1\"?").arg(QString::fromStdString(p->name))) != QMessageBox::Yes)
    {
        return;
    }

    int const row = currentPresetIndex();
    std::string const removed = p->name;
    r->presets.erase(r->presets.begin() + row);

    if (r->activePreset == removed)
    {
        r->activePreset = r->presets.front().name;
    }
    for (auto it = r->instancePresets.begin(); it != r->instancePresets.end();)
    {
        it = it->second == removed ? r->instancePresets.erase(it) : std::next(it);
    }

    markDirty();
    rebuildPresetList(std::min(row, static_cast<int>(r->presets.size()) - 1));
}

void MappingEditorDialog::activatePreset()
{
    TypeRecord *r = currentRecord();
    Profile *p = currentPreset();
    if (!r || !p)
    {
        return;
    }

    r->activePreset = p->name;
    markDirty();
    rebuildPresetList(currentPresetIndex());
}

void MappingEditorDialog::importPreset()
{
    TypeRecord *r = currentRecord();
    if (!r)
    {
        return;
    }

    QString const path = QFileDialog::getOpenFileName(this, tr("Import preset"), QString(), tr("Presets (*.json)"));
    if (path.isEmpty())
    {
        return;
    }

    Profile p;
    QString error;
    if (!ProfileStore::readProfileFile(path, p, error))
    {
        QMessageBox::warning(this, tr("Cannot import preset"), error);
        return;
    }

    p.deviceGuid = r->type.guid;
    p.name = uniqueName(p.name.empty() ? std::string("Imported") : p.name);
    r->presets.push_back(p);

    markDirty();
    rebuildPresetList(static_cast<int>(r->presets.size()) - 1);
}

void MappingEditorDialog::exportPreset()
{
    Profile *p = currentPreset();
    if (!p)
    {
        return;
    }

    QString const path = QFileDialog::getSaveFileName(this, tr("Export preset"), QString::fromStdString(p->name) + ".json",
                                                      tr("Presets (*.json)"));
    if (path.isEmpty())
    {
        return;
    }

    QString error;
    if (!ProfileStore::writeProfileFile(path, *p, error))
    {
        QMessageBox::warning(this, tr("Cannot export preset"), error);
    }
}

// ---- Sticks ----

void MappingEditorDialog::refreshStickList()
{
    Profile *p = currentPreset();
    int const keep = m_stickList->currentRow();

    m_stickList->blockSignals(true);
    m_stickList->clear();
    if (p)
    {
        for (Stick const &s : p->sticks)
        {
            QString const name = s.label.empty() ? tr("Stick") : QString::fromStdString(s.label);
            m_stickList->addItem(tr("%1 (axes %2 / %3)").arg(name).arg(s.xAxis + 1).arg(s.yAxis + 1));
        }
    }
    m_stickList->setCurrentRow(std::min(keep, m_stickList->count() - 1));
    m_stickList->blockSignals(false);

    loadStickForm();
}

void MappingEditorDialog::onStickChanged()
{
    loadStickForm();
}

void MappingEditorDialog::loadStickForm()
{
    Profile *p = currentPreset();
    int const row = m_stickList->currentRow();
    bool const has = p && row >= 0 && row < static_cast<int>(p->sticks.size());

    m_stickForm->setEnabled(has);
    m_stickWarning->hide();
    if (!has)
    {
        return;
    }

    Stick const &s = p->sticks[row];
    m_updating = true;
    m_stickLabelEdit->setText(QString::fromStdString(s.label));
    m_stickXCombo->setCurrentIndex(s.xAxis);
    m_stickYCombo->setCurrentIndex(s.yAxis);
    m_stickRadialSpin->setValue(s.radialDeadZone);
    m_updating = false;
}

void MappingEditorDialog::stickFormChanged()
{
    if (m_updating)
    {
        return;
    }

    Profile *p = currentPreset();
    int const row = m_stickList->currentRow();
    if (!p || row < 0 || row >= static_cast<int>(p->sticks.size()))
    {
        return;
    }

    Stick &s = p->sticks[row];
    s.label = m_stickLabelEdit->text().trimmed().toStdString();
    s.radialDeadZone = static_cast<float>(m_stickRadialSpin->value());

    bool const distinct = m_stickXCombo->currentIndex() != m_stickYCombo->currentIndex();
    m_stickWarning->setVisible(!distinct);
    if (distinct)
    {
        s.xAxis = m_stickXCombo->currentIndex();
        s.yAxis = m_stickYCombo->currentIndex();
    }

    markDirty();
    refreshStickList();
    refreshTable();
}

void MappingEditorDialog::addStick()
{
    TypeRecord *r = currentRecord();
    Profile *p = currentPreset();
    if (!r || !p)
    {
        return;
    }

    if (r->type.numAxes < 2)
    {
        QMessageBox::information(this, tr("Cannot add stick"), tr("This joystick has fewer than two axes."));
        return;
    }

    std::set<int> used;
    for (Stick const &s : p->sticks)
    {
        used.insert(s.xAxis);
        used.insert(s.yAxis);
    }

    Stick s;
    s.xAxis = 0;
    s.yAxis = 1;
    for (int i = 0; i + 1 < r->type.numAxes; ++i)
    {
        if (!used.count(i) && !used.count(i + 1))
        {
            s.xAxis = i;
            s.yAxis = i + 1;
            break;
        }
    }
    s.label = tr("Stick %1").arg(p->sticks.size() + 1).toStdString();
    p->sticks.push_back(s);

    markDirty();
    m_stickList->blockSignals(true);
    m_stickList->setCurrentRow(-1);
    m_stickList->blockSignals(false);
    refreshStickList();
    m_stickList->setCurrentRow(m_stickList->count() - 1);
    refreshTable();
}

void MappingEditorDialog::removeStick()
{
    Profile *p = currentPreset();
    int const row = m_stickList->currentRow();
    if (!p || row < 0 || row >= static_cast<int>(p->sticks.size()))
    {
        return;
    }

    p->sticks.erase(p->sticks.begin() + row);
    markDirty();
    refreshStickList();
    refreshTable();
}

void MappingEditorDialog::copyStickSettings(bool xToY)
{
    Profile *p = currentPreset();
    int const row = m_stickList->currentRow();
    if (!p || row < 0 || row >= static_cast<int>(p->sticks.size()))
    {
        return;
    }

    Stick const &s = p->sticks[row];
    ControlKey from;
    from.type = ControlType::Axis;
    from.index = xToY ? s.xAxis : s.yAxis;
    ControlKey to = from;
    to.index = xToY ? s.yAxis : s.xAxis;

    Mapping const *source = findMapping(*p, from);
    Mapping *target = findMapping(*p, to);
    if (!source || !target)
    {
        QMessageBox::information(this, tr("Cannot copy"), tr("Both axes of the stick need a MIDI mapping."));
        return;
    }

    target->invert = source->invert;
    target->range = source->range;
    target->curve = source->curve;
    target->deadZone = source->deadZone;
    target->smoothing = source->smoothing;
    target->maxRateHz = source->maxRateHz;

    markDirty();
    loadForm();
}
