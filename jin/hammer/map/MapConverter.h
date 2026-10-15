#ifndef MAP_CONVERTER_H
#define MAP_CONVERTER_H

class MapDocument;

// 把编辑器的 g_brushes / g_entities 转换到 MapDocument
// 第一版：只处理轴对齐 brush（忽略 m_angRot），entity 只导出 classname/origin/angles
void ConvertEditorToDocument(MapDocument& doc);

// 反向：从 MapDocument 恢复到 g_brushes / g_entities
void ConvertDocumentToEditor(const MapDocument& doc);

#endif