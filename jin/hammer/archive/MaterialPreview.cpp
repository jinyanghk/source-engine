#include "MaterialPreview.h"
#include <QPainter>

// Source Engine headers
#include "materialsystem/imaterialsystem.h"
#include "materialsystem/imaterial.h"
#include "bitmap/imageformat.h"

QMaterialPreview::QMaterialPreview(QWidget *parent)
    : QWidget(parent)
{
    // Give the widget a default background look while no material is loaded
    setBackgroundRole(QPalette::Dark);
    setAutoFillBackground(true);
}

bool QMaterialPreview::SetMaterial(const QString &materialName, const QString &groupName)
{
    m_currentMaterial = materialName;

    // 1. Look for the requested material to preview
    IMaterial *pMaterial = g_pMaterialSystem->FindMaterial( materialName.toUtf8().constData(), 
            groupName.toUtf8().constData(),
            true );
    if ( !pMaterial )
    {
        m_previewImage = QImage(); // Clear out old preview if it fails
        update();
        return false;
    }
    
    // 2. Get dimensions of the image to query required memory
    int width = 0, height = 0;
    ImageFormat eFormat = IMAGE_FORMAT_UNKNOWN;
    bool bIsTranslucent = false;

    PreviewImageRetVal_t retVal = pMaterial->GetPreviewImageProperties( 
        &width, 
        &height, 
        &eFormat, 
        &bIsTranslucent  );

    if ( retVal != MATERIAL_PREVIEW_IMAGE_OK )
    {
        m_previewImage = QImage();
        update();
        return false;
    }

    // 3. Query required memory (uses the linked bitmap library from your fix earlier)
    int bytes = ImageLoader::GetMemRequired( width, height, 1, IMAGE_FORMAT_RGB888, false );

    unsigned char *pData = new unsigned char[bytes];
    if ( pMaterial->GetPreviewImage( pData, width, height, IMAGE_FORMAT_RGB888 ) != MATERIAL_PREVIEW_IMAGE_OK )
    {
        delete[] pData;
        m_previewImage = QImage();
        update();
        return false;
    }

    // 4. Safely construct the temporary Qt representation and perform a deep copy
    QImage temporaryImage( pData, width, height, QImage::Format_RGB888 );
    m_previewImage = temporaryImage.copy(); // Cached into widget memory context

    // 5. Clean up raw heap allocations safely
    delete[] pData;

    // Trigger an asynchronous repaint event for the UI layout
    update();
    return true;
}

void QMaterialPreview::paintEvent(QPaintEvent *event)
{
    // Let the base widget draw background configurations if the image is empty
    QWidget::paintEvent(event);

    if ( m_previewImage.isNull() )
    {
        QPainter painter(this);
        painter.setPen(Qt::white);
        painter.drawText(rect(), Qt::AlignCenter, "No Preview Available");
        return;
    }

    QPainter painter(this);
    
    // Enable clean scaling adjustments if the material is stretched
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    // Scale the material preview cleanly to match the widget bounding box while preserving aspect ratio
    QImage scaledImage = m_previewImage.scaled(rect().size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);

    // Center the texture within the canvas container layout bounds
    int x = (rect().width() - scaledImage.width()) / 2;
    int y = (rect().height() - scaledImage.height()) / 2;

    painter.drawImage(x, y, scaledImage);
}