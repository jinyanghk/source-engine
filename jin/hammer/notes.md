2026年10月7日

项目：基于 nillerusr 泄漏版 Source 引擎的 Hammer 风格编辑器
平台：Linux (Mesa Intel UHD 630, OpenGL 4.6 compatibility)
编译：GCC + waf build，链接 SDL2 / Source 引擎接口
main.cpp 约 2000 行，当前可编译运行

【已经实现的功能】
1. 加载渲染 Alyx / Barney 等 MDL 模型
2. 手写 OpenGL 绘制包围盒 / Gizmo / Brush
3. 多 entity 系统（entity 列表 + 选中）
4. 相机：Fly (WASD/QE) + Pan (RMB/MMB) + Zoom (滚轮/Z) + Orbit (LMB 空白)
5. Gizmo：Translate (箭头) / Rotate (圆环)，1/2/3 切换模式
6. 点击拾取切换选中 entity
7. Brush 系统：地板 + 两面墙 + 棋盘格/砖墙 placeholder 纹理
8. info_player_start 绿色胶囊

【关键技术点】（非常重要！）
- 【核心坑】nillerusr 泄漏版 Source 引擎的 OpenGL 后端的视图矩阵是 D3D 风格的 Y 朝下
  → 从 pRenderContext->GetMatrix(MATERIAL_VIEW) 拿到的矩阵给 OpenGL 用时要翻转 Y 行：
    glView[1] = -glView[1]; glView[5] = -glView[5]; glView[9] = -glView[9]; glView[13] = -glView[13];
- IMesh 接口不全（无 CreateStaticMesh/Position3f），所以 Brush/Gizmo 用手写 glBegin/glEnd
- DrawModel 的模型变换不能通过 LoadMatrix 传递，必须手动应用到每根骨骼矩阵
- 引擎的 GetMatrix 返回值是行主序，给 OpenGL 用要先转置
- Msg() 会被引擎日志系统吞掉，用 printf + fflush

【当前 main.cpp 关键结构】
- CEntity { m_iType, m_hMdl, m_szName, m_vecPos, m_angRot, m_vecBBoxMins/Maxs }
- CBrush { m_vecPos, m_vecSize, m_angRot, m_iTexId }
- g_entities / g_brushes / g_iSelectedEntity
- DrawBrush / DrawBrushEdges / DrawEntityBBox / DrawPlayerStartModel / DrawTranslateGizmo / DrawRotationRing
- UpdateEntityBBox / RenderEntityModel / WorldToScreen / RayToAxisSegmentDistSq / BuildGLProjectionMatrix / BuildGLViewMatrix

【下一步要做的】
1. Gizmo 拖动改进：屏幕空间增量法（防漂移、抗丢帧）
2. Gizmo 悬停检测：射线-圆柱相交（代替射线-线段）

【即将收到完整 main.cpp】