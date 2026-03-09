#include "camera.hpp"
#include "renderer.hpp"

namespace Graphics
{
    //TRI MESH
    VertexBuffer* _quadMesh = 0;
    VertexBuffer* _triangleMesh = 0;
    VertexBuffer* _circleMesh = 0;

    //only for gizmos
    //CANNOT BE DRAWN WITH MDM_TRI
    VertexBuffer* _boxMesh = 0;
    VertexBuffer* _lineMesh = 0;

    std::unordered_map<std::string, Texture*> _textureBuffer{};
 
    struct RenderCommand 
    {
        RenderData data;
        PrimitiveType type;
        std::string text; //if using text renderer

        //sorting operator
        bool operator<(const RenderCommand& other) const 
        {
            //primary layer sort
            if (data.layer != other.data.layer)
                return data.layer < other.data.layer;

            //secondary sort with same layer
            if (data.sortOrder != other.data.sortOrder)
                return data.sortOrder < other.data.sortOrder;

            //sort by texture, prevents additional texture set calls
            return data.texture < other.data.texture;
        }
    };
}

namespace Graphics //init mesh functions and texture handlers
{
    void MeshStart() { AEGfxMeshStart(); }

    void AddTriangle(float2 p0, float2 p1, float2 p2, Color c, float2 uv0, float2 uv1, float2 uv2) 
    {
        u32 hexColor = c.hex();
        AEGfxTriAdd(
            p0.x, p0.y, hexColor, uv0.x, uv0.y,
            p1.x, p1.y, hexColor, uv1.x, uv1.y,
            p2.x, p2.y, hexColor, uv2.x, uv2.y
        );
    }

    void AddTriangle(float2 p0, float2 p1, float2 p2, float2 uv0, float2 uv1, float2 uv2) 
    {
        AEGfxTriAdd(
            p0.x, p0.y, 0xFF'FF'FF'FF, uv0.x, uv0.y,
            p1.x, p1.y, 0xFF'FF'FF'FF, uv1.x, uv1.y,
            p2.x, p2.y, 0xFF'FF'FF'FF, uv2.x, uv2.y
        );
    }

    void AddVertex(float2 p, Color c, float2 uv)
    {
        AEGfxVertexAdd(p.x, p.y, c.hex(), uv.x, uv.y);
    }

    void AddVertex(float2 p, float2 uv)
    {
        AEGfxVertexAdd(p.x, p.y, 0xFF'FF'FF'FF, uv.x, uv.y);
    }

    VertexBuffer* MeshEnd() { return AEGfxMeshEnd(); }

    void InitQuad() 
    {
        MeshStart();

        AddTriangle
        (
            { 0.5f ,-0.5f },
            {-0.5f , 0.5f },
            {-0.5f ,-0.5f },
            { 1.0f , 1.0f },
            { 0.0f , 0.0f },
            { 0.0f , 1.0f }
        );

        AddTriangle
        (
            {  0.5f ,-0.5f },
            {  0.5f , 0.5f },
            { -0.5f , 0.5f },
            {  1.0f , 1.0f },
            {  1.0f , 0.0f },
            {  0.0f , 0.0f }
        );

        _quadMesh = MeshEnd();
    }

    void InitTri() {
        MeshStart();

        AddTriangle
        (
            { 0.0f,  0.57735f },
            {-0.5f, -0.28867f },
            { 0.5f, -0.28867f },
            { 0.5f, 0.0f },
            { 0.0f, 1.0f },
            { 1.0f, 1.0f }
        );

        _triangleMesh = MeshEnd();
    }

    void InitCircle() {
        MeshStart();

        const int segments = 32; //resolution of circle
        const f32 radius = 0.5f; //produces a unit circle (dia = 1)
        const f32 angleStep = (2.0f * PI) / segments;

        for (int i = 0; i < segments; i++)
        {
            f32 theta1 = i * angleStep;
            f32 theta2 = (i + 1) * angleStep;

            //vertices
            float2 v0 = { 0.0f, 0.0f }; // center
            float2 v1 = { cosf(theta1) * radius, sinf(theta1) * radius };
            float2 v2 = { cosf(theta2) * radius, sinf(theta2) * radius };

            float2 uv0 = { 0.5f, 0.5f };
            float2 uv1 = { (v1.x + 0.5f), (v1.y + 0.5f) };
            float2 uv2 = { (v2.x + 0.5f), (v2.y + 0.5f) };

            AddTriangle(v0, v1, v2, uv0, uv1, uv2);
        }

        _circleMesh = MeshEnd();
    }

    void InitBoxMesh()
    {
        MeshStart();

        //top line
        AddVertex(float2(-0.5, 0.5), float2(0, 0));
        AddVertex(float2(0.5, 0.5), float2(0, 0));

        //left
        AddVertex(float2(-0.5, 0.5), float2(0, 0));
        AddVertex(float2(-0.5, -0.5), float2(0, 0));

        //right
        AddVertex(float2(0.5, 0.5), float2(0, 0));
        AddVertex(float2(0.5, -0.5), float2(0, 0));

        //bottom
        AddVertex(float2(-0.5, -0.5),float2(0, 0));
        AddVertex(float2(0.5, -0.5), float2(0, 0));

        _boxMesh = MeshEnd();
    }

    void InitLineMesh()
    {
        MeshStart();

        AddVertex({ 0.f,0.f }, { 0.f,0.f });
        AddVertex({ 1.f,0.f }, { 0.f,0.f });

        _lineMesh = MeshEnd();
    }

    // Loads textures used
    Texture* LoadTexture(std::string fileName)
    {        
        if (fileName.empty()) {
            return nullptr;
        }
        std::string filePath = "Assets/" + fileName;
        if (auto search = _textureBuffer.find(filePath); search != _textureBuffer.end()){
            return search->second;
        }
        _textureBuffer.insert({ filePath, AEGfxTextureLoad(filePath.c_str()) });
        return _textureBuffer.find(filePath)->second;
    }

    void UnloadTextures()
    {
        for (const auto& t : _textureBuffer) {
            AEGfxTextureUnload(t.second);
        }
    }

    VertexBuffer* QuadMesh() { return _quadMesh; }
}


namespace Graphics
{
    namespace
    {
        std::vector<RenderCommand> _commandBuffer;

        Texture* _currentTex = nullptr;
        RenderMode _currentMode = (RenderMode)-1;
        Font _currentFont;
        

        void ApplyStates(const RenderData& data)
        {
            //only set mode if required
            if (data.renderMode != _currentMode)
            {
                AEGfxSetRenderMode(data.renderMode);
                _currentMode = data.renderMode;
            }

            AEGfxSetBlendMode(data.blendMode);
            AEGfxSetColorToMultiply(data.color.r, data.color.g, data.color.b, 1.0f);
            AEGfxSetTransparency(data.color.a);

            if (data.renderMode == AE_GFX_RM_TEXTURE && data.texture != _currentTex) 
            {
                AEGfxTextureSet(data.texture, 0.f, 0.f);
                _currentTex = data.texture;
            }
        }
   
        MTX GetAlignmentMatrix(Alignment alignment)
        {
            MTX align{};
            switch (alignment) {
            case Alignment::TL:
                AEMtx33Trans(&align, 0.5f, -0.5f);
                break;
            case Alignment::TC:
                AEMtx33Trans(&align, 0.f, -0.5f);
                break;
            case Alignment::TR:
                AEMtx33Trans(&align, -0.5f, -0.5f);
                break;
            case Alignment::ML:
                AEMtx33Trans(&align, 0.5f, 0.f);
                break;
            case Alignment::MC:
                AEMtx33Trans(&align, 0.f, 0.f);
                break;
            case Alignment::MR:
                AEMtx33Trans(&align, -0.5f, 0.f);
                break;
            case Alignment::BL:
                AEMtx33Trans(&align, 0.5f, 0.5f);
                break;
            case Alignment::BC:
                AEMtx33Trans(&align, 0.f, 0.5f);
                break;
            case Alignment::BR:
                AEMtx33Trans(&align, -0.5f, 0.5f);
                break;                
            }

            return align;
        }
    
        void GetTransformMTX(float2 t, float2 s, f32 r, Alignment mode, MTX& mtx, bool scrnSpace)
        {
            AEMtx33Identity(&mtx);
            MTX align = GetAlignmentMatrix(mode);
            AEMtx33Concat(&mtx, &align, &mtx);

            MTX scale{};
            AEMtx33Scale(&scale, s.x, s.y);
            MTX rotate{};
            AEMtx33RotDeg(&rotate, r);
            MTX translate{};
            AEMtx33Trans(&translate, t.x, t.y);

            if (scrnSpace) {
                AEMtx33Concat(&scale, &CameraData::camScaleM, &scale);
                AEMtx33Concat(&rotate, &CameraData::camRotateM, &rotate);
                AEMtx33Concat(&translate, &CameraData::camTransM, &translate);
            }

            AEMtx33Concat(&mtx, &scale, &mtx);
            AEMtx33Concat(&mtx, &rotate, &mtx);
            AEMtx33Concat(&mtx, &translate, &mtx);
        }
    
        void DrawMesh(VertexBuffer* mesh, DrawMode mode, Texture* texture)
        {
            AEGfxTextureSet(texture, 0, 0);
            AEGfxMeshDraw(mesh, mode);
        }
        
        void DrawTextMesh(const char* text,const RenderData& data)
        {
            f32 winW = (f32)AEGfxGetWindowWidth();
            f32 winH = (f32)AEGfxGetWindowHeight();
            f32 halfW = winW * 0.5f;
            f32 halfH = winH * 0.5f;

            float2 screenPixelPos;
            if (data.isScreenSpace)
            {
                //set final position without
                screenPixelPos = data.pos;
            }
            else //apply cam mtx if worldspace
            {
                AEMtx33 worldMtx;
                AEMtx33Trans(&worldMtx, data.pos.x, data.pos.y);

                AEMtx33 viewPosMtx;
                //AEMtx33Concat(&viewPosMtx, &CameraData::camMatrix, &worldMtx);

                screenPixelPos.x = viewPosMtx.m[0][2];
                screenPixelPos.y = viewPosMtx.m[1][2];
            }

            f32 aeX = screenPixelPos.x / halfW;
            f32 aeY = screenPixelPos.y / halfH;

            f32 textW, textH;
            AEGfxGetPrintSize(_currentFont, text, data.scale.x, &textW, &textH);

            f32 offX = 0, offY = 0;
            switch (data.alignment)
            {
            case Alignment::TL:
                offX = 0;           offY = textH;           break;
            case Alignment::TC:
                offX = textW * 0.5f; offY = textH;           break;
            case Alignment::TR:
                offX = textW;        offY = textH;           break;

            case Alignment::ML:
                offX = 0;           offY = textH * 0.5f; break;
            case Alignment::MC:
                offX = textW * 0.5f; offY = textH * 0.5f; break;
            case Alignment::MR:
                offX = textW;        offY = textH * 0.5f; break;

            case Alignment::BL:
                offX = 0;           offY = 0;        break;
            case Alignment::BC:
                offX = textW * 0.5f; offY = 0;        break;
            case Alignment::BR:
                offX = textW;        offY = 0;        break;
            }

            AEGfxPrint(_currentFont, text, aeX - offX, aeY - offY,
                data.scale.x, data.color.r, data.color.g, data.color.b, data.color.a);
        }
    }

   

    void Initialize()
    {
        InitQuad();
        InitTri();
        InitCircle();
        InitBoxMesh();
        InitLineMesh();

        _currentFont = AEGfxCreateFont("./Assets/liberation-mono.ttf", 72);
    }

    void Flush()
    {
        _commandBuffer.clear();
    }

    void Exit()
    {
        _commandBuffer.clear(); //clear all buffer
        //unload all data
        AEGfxMeshFree(_quadMesh);
        AEGfxMeshFree(_triangleMesh);
        AEGfxMeshFree(_circleMesh);

        //line meshes
        AEGfxMeshFree(_boxMesh);
        AEGfxMeshFree(_lineMesh);

        //unload textures
        UnloadTextures();
        //unload font
        AEGfxDestroyFont(_currentFont);
    }

    void Submit(const RenderData& data, PrimitiveType type, const char* text)
    {
        //construct command
        RenderCommand cmd;
        cmd.data = data;
        cmd.type = type;
        if (text) cmd.text = text;

        //pushes into buffer
        _commandBuffer.push_back(cmd);
    }

    void SubmitArrow(const RenderData& data)
    {
        RenderData stem = data;
        stem.scale = { data.scale.x, data.scale.y * 0.2f };
        stem.alignment = Alignment::ML;
        stem.layer = data.layer;
        stem.sortOrder = data.sortOrder;
        Submit(stem, PrimitiveType::QUAD);

        RenderData tip = data;
        float angleRad = data.rot * (3.14159f / 180.0f);
        tip.pos.x += cosf(angleRad) * data.scale.x;
        tip.pos.y += sinf(angleRad) * data.scale.x;
        tip.scale = { data.scale.y, data.scale.y };
        tip.rot -= 90.0f;
        tip.alignment = Alignment::MC;
        tip.layer = data.layer;
        tip.sortOrder = data.sortOrder;
        Submit(tip, PrimitiveType::TRIANGLE);
    }

    void Execute() //executes this frame's commands
    {
        //sort the commands
        std::sort(_commandBuffer.begin(), _commandBuffer.end());

        //reset cache
        _currentTex = nullptr;
        _currentMode = (AEGfxRenderMode)-1;

        //dispatch commands
        for (const auto& cmd : _commandBuffer) 
        {
            const RenderData& d = cmd.data;
            //apply states
            ApplyStates(d);

            //set matrix
            MTX transform{};
            GetTransformMTX(d.pos,d.scale,d.rot,d.alignment,transform, d.isScreenSpace);
            AEGfxSetTransform(transform.m);

            //draw based off primitive type
            switch (cmd.type) 
            {
            case PrimitiveType::QUAD:
                DrawMesh(_quadMesh, d.drawMode, d.texture);
                break;
            case PrimitiveType::TRIANGLE:
                DrawMesh(_triangleMesh, d.drawMode, d.texture);
                break;
            case PrimitiveType::CIRCLE:
                DrawMesh(_circleMesh, d.drawMode, d.texture);
                break;
            case PrimitiveType::BOX:
                DrawMesh(_boxMesh, d.drawMode, d.texture);
                break;
            case PrimitiveType::TEXT:
                DrawTextMesh(cmd.text.c_str(), d);
                break;
            }
        }

        //clears buffer for nxt frame
        _commandBuffer.clear();
    }
}