#pragma once

#include <QWidget>
#include <QString>
#include <QImage>
#include <QPaintEvent>

class QMaterialPreview : public QWidget
{
    Q_OBJECT

public:
    explicit QMaterialPreview(QWidget *parent = nullptr);
    virtual ~QMaterialPreview() = default;

    // Call this to dynamically change the material shown by the preview widget
    bool SetMaterial(const QString &materialName, const QString &groupName);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QImage m_previewImage;
    QString m_currentMaterial;
};