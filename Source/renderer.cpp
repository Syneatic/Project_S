/*
Author: Jia Xi
Co-Author: Yan Chun
*/
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

        bool operator<(const RenderCommand& other) const
        {
            return data.sortKey < other.data.sortKey;
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

    // Defaut values for meshes
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

    // Defaut values for meshes
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

    // Defaut values for meshes
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

    // Defaut values for meshes
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
        std::string filePath = EngineCTX::GetAbsPath("Assets/" + fileName);
        if (auto search = _textureBuffer.find(filePath); search != _textureBuffer.end()){
            return search->second;
        }
        _textureBuffer.insert({ filePath, AEGfxTextureLoad(filePath.c_str()) });
        return _textureBuffer.find(filePath)->second;
    }
    // Unload all textures
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
        std::vector<RenderCommand> _radixTemp;

        Texture* _currentTex = nullptr;
        RenderMode _currentMode = (RenderMode)-1;
        Font _currentFont;
        
        void RadixSort(std::vector<RenderCommand>& buf) 
        {
            PROFILE_SCOPE(__func__);
            const size_t n = buf.size();
            if (n < 2) return;

            _radixTemp.resize(n);

            // 8 passes over 8 bytes of the u64 key (LSB to MSB)
            for (int pass = 0; pass < 8; ++pass)
            {
                const int shift = pass * 8;

                // Count occurrences of each byte value
                uint32_t count[256] = {};
                for (size_t i = 0; i < n; ++i)
                    ++count[(buf[i].data.sortKey >> shift) & 0xFF];

                // Prefix sum -> starting output index per bucket
                uint32_t prefix[256] = {};
                for (int b = 1; b < 256; ++b)
                    prefix[b] = prefix[b - 1] + count[b - 1];

                // Scatter into temp buffer
                for (size_t i = 0; i < n; ++i)
                {
                    uint8_t bucket = (buf[i].data.sortKey >> shift) & 0xFF;
                    _radixTemp[prefix[bucket]++] = buf[i];
                }

                // Swap back
                std::swap(buf, _radixTemp);
            }
            // After 8 (even) passes, result is always back in buf
        }

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
   
        float2 GetAlignmentOffset(Alignment alignment)
        {
            switch (alignment)
            {
            case Alignment::TL: return { 0.5f, -0.5f };
            case Alignment::TC: return { 0.0f, -0.5f };
            case Alignment::TR: return { -0.5f, -0.5f };
            case Alignment::ML: return { 0.5f,  0.0f };
            case Alignment::MC: return { 0.0f,  0.0f };
            case Alignment::MR: return { -0.5f,  0.0f };
            case Alignment::BL: return { 0.5f,  0.5f };
            case Alignment::BC: return { 0.0f,  0.5f };
            case Alignment::BR: return { -0.5f,  0.5f };
            default:            return { 0.0f,  0.0f };
            }
        }

        // Get matrix to render by applying camera matrix to transform
        void GetTransformMTX(float2 t, float2 s, f32 r, Alignment mode, MTX& mtx, bool notScrnSpace)
        {
            struct CamCache
            {
                f32 c00, c01, c02; // row 0
                f32 c10, c11, c12; // row 1
            } cam;

            cam.c00 = CameraData::camM.m[0][0];
            cam.c01 = CameraData::camM.m[0][1];
            cam.c02 = CameraData::camM.m[0][2];
            cam.c10 = CameraData::camM.m[1][0];
            cam.c11 = CameraData::camM.m[1][1];
            cam.c12 = CameraData::camM.m[1][2];


            float2 align = GetAlignmentOffset(mode);

            const f32 rotRad = r * (PI / 180.f);
            const f32 c = cosf(rotRad);
            const f32 si = sinf(rotRad);

            const f32 csx = c * s.x;
            const f32 ssx = si * s.x;
            const f32 csy = c * s.y;
            const f32 ssy = si * s.y;

            const f32 L00 = csx;
            const f32 L01 = -ssy;
            const f32 L02 = t.x + csx * align.x - ssy * align.y;
            const f32 L10 = ssx;
            const f32 L11 = csy;
            const f32 L12 = t.y + ssx * align.x + csy * align.y;

            if (notScrnSpace)
            {
                mtx.m[0][0] = cam.c00 * L00 + cam.c01 * L10;
                mtx.m[0][1] = cam.c00 * L01 + cam.c01 * L11;
                mtx.m[0][2] = cam.c00 * L02 + cam.c01 * L12 + cam.c02;

                mtx.m[1][0] = cam.c10 * L00 + cam.c11 * L10;
                mtx.m[1][1] = cam.c10 * L01 + cam.c11 * L11;
                mtx.m[1][2] = cam.c10 * L02 + cam.c11 * L12 + cam.c12;
            }
            else
            {
                mtx.m[0][0] = L00;  mtx.m[0][1] = L01;  mtx.m[0][2] = L02;
                mtx.m[1][0] = L10;  mtx.m[1][1] = L11;  mtx.m[1][2] = L12;
            }

            mtx.m[2][0] = 0.f;
            mtx.m[2][1] = 0.f;
            mtx.m[2][2] = 1.f;
        }

        // Helper function to check if object is within margin of viewport, for render culling
        bool InViewport(const RenderData& d)
        {
            if (d.isScreenSpace) return true; // always render screenspace objects

            f32 winW = (f32)AEGfxGetWindowWidth();
            f32 winH = (f32)AEGfxGetWindowHeight();

            float2 screenPos;
            screenPos.x = CameraData::camM.m[0][0] * d.pos.x + CameraData::camM.m[0][1] * d.pos.y + CameraData::camM.m[0][2];
            screenPos.y = CameraData::camM.m[1][0] * d.pos.x + CameraData::camM.m[1][1] * d.pos.y + CameraData::camM.m[1][2];

            f32 objW = fabsf(d.scale.x) * 0.5f;
            f32 objH = fabsf(d.scale.y) * 0.5f;

            // buffer set slightly outside
            f32 boundX = winW * 0.5f * 1.1f;
            f32 boundY = winH * 0.5f * 1.1f;

            // AABB
            return (screenPos.x + objW > -boundX) && (screenPos.x - objW < boundX) &&
                   (screenPos.y + objH > -boundY) && (screenPos.y - objH < boundY);
        }
    
        void DrawMesh(VertexBuffer* mesh, DrawMode mode, Texture* texture)
        {
            AEGfxTextureSet(texture, 0, 0);
            AEGfxMeshDraw(mesh, mode);
        }
        
        // Draw text objects with offset
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
                AEMtx33Concat(&viewPosMtx, &CameraData::camM, &worldMtx);

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
    
        u64 BuildSortKey(const RenderData& d)
        {
            // Layer: 16 bits (RenderLayer values fit easily)
            u64 layerBits = (uint64_t)(uint16_t)d.layer;

            // sortOrder: reinterpret float bits as u32.
            // For non-negative floats, IEEE 754 bit order == numeric order.
            // Negative floats are uncommon for sort order but handled by flipping sign bit.
            u32 sortBits;
            std::memcpy(&sortBits, &d.sortOrder, sizeof(float));
            if (sortBits & 0x8000'0000u)          // negative: flip all bits
                sortBits ^= 0xFFFF'FFFFu;
            else                                   // positive: flip sign bit only
                sortBits ^= 0x8000'0000u;

            // Texture: hash pointer to 16 bits
            u64 texBits = (u64)(uintptr_t)d.texture & 0xFFFFu;

            return (layerBits << 48) | ((u64)sortBits << 16) | texBits;
        }
    }

   
    // Initialise all meshes
    void Initialize()
    {
        InitQuad();
        InitTri();
        InitCircle();
        InitBoxMesh();
        InitLineMesh();

        std::string fontPath = EngineCTX::GetAbsPath("Assets/Ubuntu-Light.ttf");
        _currentFont = AEGfxCreateFont(fontPath.c_str(), 72);
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

    // Submit shape into render buffer
    void Submit(const RenderData& data, PrimitiveType type, const char* text)
    {
        if (!InViewport(data)) return;

        //construct command
        RenderCommand cmd;
        cmd.data = data;
        cmd.type = type;
        cmd.data.sortKey = BuildSortKey(data);
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
        PROFILE_SCOPE("Renderer");
        //Debug::ScopedTimer t("Render:Execute");

        //sort the commands
        //swaps algorithm based off buffer size
        {   
            PROFILE_SCOPE("Sort");
            if (_commandBuffer.size() < 1500)
                std::sort(_commandBuffer.begin(), _commandBuffer.end());
            else    
                RadixSort(_commandBuffer);          
        }

        //reset cache
        _currentTex = nullptr;
        _currentMode = (AEGfxRenderMode)-1;

        //dispatch commands
        for (const auto& cmd : _commandBuffer) 
        {
            PROFILE_SCOPE("Dispatch");
            const RenderData& d = cmd.data;
            //apply states
            ApplyStates(d);

            //set matrix
            MTX transform{};
            GetTransformMTX(d.pos,d.scale,d.rot,d.alignment,transform, !d.isScreenSpace);
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

        //FrameProfiler::CommitFrame();
        //if (FrameProfiler::IsReady())
            //FrameProfiler::FlushToFile("render_profile.txt");
    }
}