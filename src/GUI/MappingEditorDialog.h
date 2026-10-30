#ifndef MAPPINGEDITORDIALOG_H_
#define MAPPINGEDITORDIALOG_H_

#include "core/ProfileLibrary.h"

#include <QDialog>
#include <QElapsedTimer>
#include <QTimer>

#include <functional>
#include <map>
#include <memory>
#include <set>

class CurvePreview;
class ProfileStore;
class QCheckBox;
class QComboBox;
class QDoubleSpinBox;
class QFormLayout;
class QGroupBox;
class QLabel;
class QLineEdit;
class QListWidget;
class QPushButton;
class QSpinBox;
class QTableWidget;
class QTabWidget;

// Edits the presets of all known joystick types. Edits are applied to the library while typing
// (onChanged is then called) and written to disk on OK; Cancel restores the previous state.
class MappingEditorDialog : public QDialog
{
    Q_OBJECT

public:
    MappingEditorDialog(std::shared_ptr<ProfileLibrary> library, ProfileStore &store,
                        std::function<void()> onChanged, std::string const &initialGuid, QWidget *parent);

    void accept() override;
    void reject() override;

public slots:
    // Highlights the row of the control that was moved on the device type being edited.
    void onControlActivity(QString const &guid, int type, int index, float value);

private slots:
    void updateFlashes();
    void onTypeChanged();
    void onPresetChanged();
    void onControlChanged();
    void formChanged();

    void newPreset();
    void duplicatePreset();
    void renamePreset();
    void deletePreset();
    void activatePreset();
    void importPreset();
    void exportPreset();

    void onStickChanged();
    void stickFormChanged();
    void addStick();
    void removeStick();
    void copyStickSettings(bool xToY);

    void applyNow();

private:
    struct ControlKey
    {
        ControlType type = ControlType::Axis;
        int index = 0;
        HatComponent hat = HatComponent::X;
    };

    void buildUi();
    TypeRecord *currentRecord();
    Profile *currentPreset();
    int currentPresetIndex() const;
    ControlKey currentKey() const;

    void markDirty();
    void flashRow(int row, bool scroll);
    void rebuildPresetList(int selectRow);
    void refreshTable();
    void refreshStickList();
    void loadForm();
    void loadStickForm();
    void updateFormState();

    QString controlName(Profile const &preset, ControlKey const &key) const;
    QString mappingText(Mapping const *mapping) const;
    std::string uniqueName(std::string const &base) const;
    int nextFreeNumber(Profile const &preset, MidiTargetKind kind, int channel) const;

    static Mapping *findMapping(Profile &preset, ControlKey const &key);
    static bool isAnalog(ControlKey const &key);

    std::shared_ptr<ProfileLibrary> m_library;
    ProfileStore &m_store;
    std::function<void()> m_onChanged;

    std::map<std::string, TypeRecord> m_original;
    std::map<std::string, TypeRecord> m_working;
    std::set<std::string> m_dirty;
    std::string m_guid;
    std::vector<ControlKey> m_rows;
    bool m_updating;
    QTimer m_applyTimer;
    QTimer m_flashTimer;
    QElapsedTimer m_clock;
    std::map<int, qint64> m_flashes;   // row -> time of last activity (ms)
    std::map<int, float> m_lastAxis;
    std::map<int, int> m_lastHat;

    QListWidget *m_typeList;
    QListWidget *m_presetList;
    QTabWidget *m_tabs;
    QTableWidget *m_controlTable;

    QGroupBox *m_detailGroup;
    QLineEdit *m_labelEdit;
    QCheckBox *m_mappedCheck;
    QCheckBox *m_invertCheck;
    QComboBox *m_kindCombo;
    QSpinBox *m_channelSpin;
    QFormLayout *m_commonForm;
    QLabel *m_numberLabel;
    QSpinBox *m_numberSpin;
    QLabel *m_mmcLabel;
    QComboBox *m_mmcCombo;
    QLabel *m_deviceIdLabel;
    QSpinBox *m_deviceIdSpin;

    QGroupBox *m_discreteGroup;
    QLabel *m_onLabel;
    QSpinBox *m_onSpin;
    QLabel *m_offLabel;
    QSpinBox *m_offSpin;

    QGroupBox *m_analogGroup;
    QComboBox *m_rangeCombo;
    QComboBox *m_curveCombo;
    QDoubleSpinBox *m_amountSpin;
    QDoubleSpinBox *m_deadZoneSpin;
    QDoubleSpinBox *m_smoothingSpin;
    QSpinBox *m_rateSpin;
    CurvePreview *m_preview;

    QListWidget *m_stickList;
    QWidget *m_stickForm;
    QLineEdit *m_stickLabelEdit;
    QComboBox *m_stickXCombo;
    QComboBox *m_stickYCombo;
    QDoubleSpinBox *m_stickRadialSpin;
    QLabel *m_stickWarning;
};

#endif /* MAPPINGEDITORDIALOG_H_ */
