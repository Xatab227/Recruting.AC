/**
 * @file imgui.h
 * @brief ImGui заголовочный файл (заглушка)
 * 
 * ВАЖНО: Для сборки проекта необходимо скачать полную версию ImGui
 * с https://github.com/ocornut/imgui
 * 
 * Этот файл является заглушкой и показывает необходимые объявления.
 * Замените его на реальный imgui.h из репозитория.
 */

#ifndef IMGUI_H
#define IMGUI_H

// ImGui version
#define IMGUI_VERSION "1.89.9"
#define IMGUI_VERSION_NUM 18909

#include <float.h>
#include <stdarg.h>
#include <stddef.h>
#include <string.h>

// Forward declarations
struct ImDrawChannel;
struct ImDrawCmd;
struct ImDrawData;
struct ImDrawList;
struct ImDrawListSharedData;
struct ImDrawListSplitter;
struct ImDrawVert;
struct ImFont;
struct ImFontAtlas;
struct ImFontConfig;
struct ImFontGlyph;
struct ImFontGlyphRangesBuilder;
struct ImColor;
struct ImGuiContext;
struct ImGuiIO;
struct ImGuiInputTextCallbackData;
struct ImGuiListClipper;
struct ImGuiOnceUponAFrame;
struct ImGuiPayload;
struct ImGuiSizeCallbackData;
struct ImGuiStorage;
struct ImGuiStyle;
struct ImGuiTableSortSpecs;
struct ImGuiTableColumnSortSpecs;
struct ImGuiTextBuffer;
struct ImGuiTextFilter;
struct ImGuiViewport;

// Typedefs
typedef int ImGuiCol;
typedef int ImGuiCond;
typedef int ImGuiDataType;
typedef int ImGuiDir;
typedef int ImGuiKey;
typedef int ImGuiMouseButton;
typedef int ImGuiMouseCursor;
typedef int ImGuiSortDirection;
typedef int ImGuiStyleVar;
typedef int ImGuiTableBgTarget;
typedef int ImGuiTableColumnFlags;
typedef int ImGuiTableFlags;
typedef int ImGuiTableRowFlags;
typedef int ImGuiTreeNodeFlags;
typedef int ImGuiViewportFlags;
typedef int ImGuiWindowFlags;

typedef void* ImTextureID;
typedef unsigned int ImGuiID;
typedef signed char ImS8;
typedef unsigned char ImU8;
typedef signed short ImS16;
typedef unsigned short ImU16;
typedef signed int ImS32;
typedef unsigned int ImU32;
typedef signed long long ImS64;
typedef unsigned long long ImU64;

// Vec2/Vec4
struct ImVec2 {
    float x, y;
    constexpr ImVec2() : x(0.0f), y(0.0f) {}
    constexpr ImVec2(float _x, float _y) : x(_x), y(_y) {}
};

struct ImVec4 {
    float x, y, z, w;
    constexpr ImVec4() : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {}
    constexpr ImVec4(float _x, float _y, float _z, float _w) : x(_x), y(_y), z(_z), w(_w) {}
};

// Window Flags
enum ImGuiWindowFlags_ {
    ImGuiWindowFlags_None                   = 0,
    ImGuiWindowFlags_NoTitleBar             = 1 << 0,
    ImGuiWindowFlags_NoResize               = 1 << 1,
    ImGuiWindowFlags_NoMove                 = 1 << 2,
    ImGuiWindowFlags_NoScrollbar            = 1 << 3,
    ImGuiWindowFlags_NoScrollWithMouse      = 1 << 4,
    ImGuiWindowFlags_NoCollapse             = 1 << 5,
    ImGuiWindowFlags_AlwaysAutoResize       = 1 << 6,
    ImGuiWindowFlags_NoBackground           = 1 << 7,
    ImGuiWindowFlags_NoSavedSettings        = 1 << 8,
    ImGuiWindowFlags_NoMouseInputs          = 1 << 9,
    ImGuiWindowFlags_MenuBar                = 1 << 10,
    ImGuiWindowFlags_HorizontalScrollbar    = 1 << 11,
    ImGuiWindowFlags_NoFocusOnAppearing     = 1 << 12,
    ImGuiWindowFlags_NoBringToFrontOnFocus  = 1 << 13,
    ImGuiWindowFlags_AlwaysVerticalScrollbar= 1 << 14,
    ImGuiWindowFlags_AlwaysHorizontalScrollbar = 1 << 15,
    ImGuiWindowFlags_AlwaysUseWindowPadding = 1 << 16,
    ImGuiWindowFlags_NoNavInputs            = 1 << 18,
    ImGuiWindowFlags_NoNavFocus             = 1 << 19,
    ImGuiWindowFlags_UnsavedDocument        = 1 << 20,
    ImGuiWindowFlags_NoNav                  = ImGuiWindowFlags_NoNavInputs | ImGuiWindowFlags_NoNavFocus,
    ImGuiWindowFlags_NoDecoration           = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse,
    ImGuiWindowFlags_NoInputs               = ImGuiWindowFlags_NoMouseInputs | ImGuiWindowFlags_NoNavInputs | ImGuiWindowFlags_NoNavFocus,
};

// Config Flags
enum ImGuiConfigFlags_ {
    ImGuiConfigFlags_None                   = 0,
    ImGuiConfigFlags_NavEnableKeyboard      = 1 << 0,
    ImGuiConfigFlags_NavEnableGamepad       = 1 << 1,
    ImGuiConfigFlags_NavEnableSetMousePos   = 1 << 2,
    ImGuiConfigFlags_NavNoCaptureKeyboard   = 1 << 3,
    ImGuiConfigFlags_NoMouse                = 1 << 4,
    ImGuiConfigFlags_NoMouseCursorChange    = 1 << 5,
    ImGuiConfigFlags_DockingEnable          = 1 << 6,
    ImGuiConfigFlags_ViewportsEnable        = 1 << 10,
    ImGuiConfigFlags_DpiEnableScaleViewports = 1 << 14,
    ImGuiConfigFlags_DpiEnableScaleFonts    = 1 << 15,
    ImGuiConfigFlags_IsSRGB                 = 1 << 20,
    ImGuiConfigFlags_IsTouchScreen          = 1 << 21,
};

// Col enum
enum ImGuiCol_ {
    ImGuiCol_Text,
    ImGuiCol_TextDisabled,
    ImGuiCol_WindowBg,
    ImGuiCol_ChildBg,
    ImGuiCol_PopupBg,
    ImGuiCol_Border,
    ImGuiCol_BorderShadow,
    ImGuiCol_FrameBg,
    ImGuiCol_FrameBgHovered,
    ImGuiCol_FrameBgActive,
    ImGuiCol_TitleBg,
    ImGuiCol_TitleBgActive,
    ImGuiCol_TitleBgCollapsed,
    ImGuiCol_MenuBarBg,
    ImGuiCol_ScrollbarBg,
    ImGuiCol_ScrollbarGrab,
    ImGuiCol_ScrollbarGrabHovered,
    ImGuiCol_ScrollbarGrabActive,
    ImGuiCol_CheckMark,
    ImGuiCol_SliderGrab,
    ImGuiCol_SliderGrabActive,
    ImGuiCol_Button,
    ImGuiCol_ButtonHovered,
    ImGuiCol_ButtonActive,
    ImGuiCol_Header,
    ImGuiCol_HeaderHovered,
    ImGuiCol_HeaderActive,
    ImGuiCol_Separator,
    ImGuiCol_SeparatorHovered,
    ImGuiCol_SeparatorActive,
    ImGuiCol_ResizeGrip,
    ImGuiCol_ResizeGripHovered,
    ImGuiCol_ResizeGripActive,
    ImGuiCol_Tab,
    ImGuiCol_TabHovered,
    ImGuiCol_TabActive,
    ImGuiCol_TabUnfocused,
    ImGuiCol_TabUnfocusedActive,
    ImGuiCol_PlotLines,
    ImGuiCol_PlotLinesHovered,
    ImGuiCol_PlotHistogram,
    ImGuiCol_PlotHistogramHovered,
    ImGuiCol_TableHeaderBg,
    ImGuiCol_TableBorderStrong,
    ImGuiCol_TableBorderLight,
    ImGuiCol_TableRowBg,
    ImGuiCol_TableRowBgAlt,
    ImGuiCol_TextSelectedBg,
    ImGuiCol_DragDropTarget,
    ImGuiCol_NavHighlight,
    ImGuiCol_NavWindowingHighlight,
    ImGuiCol_NavWindowingDimBg,
    ImGuiCol_ModalWindowDimBg,
    ImGuiCol_COUNT
};

// Table Flags
enum ImGuiTableFlags_ {
    ImGuiTableFlags_None                       = 0,
    ImGuiTableFlags_Resizable                  = 1 << 0,
    ImGuiTableFlags_Reorderable                = 1 << 1,
    ImGuiTableFlags_Hideable                   = 1 << 2,
    ImGuiTableFlags_Sortable                   = 1 << 3,
    ImGuiTableFlags_NoSavedSettings            = 1 << 4,
    ImGuiTableFlags_ContextMenuInBody          = 1 << 5,
    ImGuiTableFlags_RowBg                      = 1 << 6,
    ImGuiTableFlags_BordersInnerH              = 1 << 7,
    ImGuiTableFlags_BordersOuterH              = 1 << 8,
    ImGuiTableFlags_BordersInnerV              = 1 << 9,
    ImGuiTableFlags_BordersOuterV              = 1 << 10,
    ImGuiTableFlags_BordersH                   = ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_BordersOuterH,
    ImGuiTableFlags_BordersV                   = ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_BordersOuterV,
    ImGuiTableFlags_BordersInner               = ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_BordersInnerH,
    ImGuiTableFlags_BordersOuter               = ImGuiTableFlags_BordersOuterV | ImGuiTableFlags_BordersOuterH,
    ImGuiTableFlags_Borders                    = ImGuiTableFlags_BordersInner | ImGuiTableFlags_BordersOuter,
};

// Table Column Flags
enum ImGuiTableColumnFlags_ {
    ImGuiTableColumnFlags_None                 = 0,
    ImGuiTableColumnFlags_Disabled             = 1 << 0,
    ImGuiTableColumnFlags_DefaultHide          = 1 << 1,
    ImGuiTableColumnFlags_DefaultSort          = 1 << 2,
    ImGuiTableColumnFlags_WidthStretch         = 1 << 3,
    ImGuiTableColumnFlags_WidthFixed           = 1 << 4,
    ImGuiTableColumnFlags_NoResize             = 1 << 5,
    ImGuiTableColumnFlags_NoReorder            = 1 << 6,
    ImGuiTableColumnFlags_NoHide               = 1 << 7,
    ImGuiTableColumnFlags_NoClip               = 1 << 8,
    ImGuiTableColumnFlags_NoSort               = 1 << 9,
    ImGuiTableColumnFlags_NoSortAscending      = 1 << 10,
    ImGuiTableColumnFlags_NoSortDescending     = 1 << 11,
    ImGuiTableColumnFlags_NoHeaderLabel        = 1 << 12,
    ImGuiTableColumnFlags_NoHeaderWidth        = 1 << 13,
    ImGuiTableColumnFlags_PreferSortAscending  = 1 << 14,
    ImGuiTableColumnFlags_PreferSortDescending = 1 << 15,
    ImGuiTableColumnFlags_IndentEnable         = 1 << 16,
    ImGuiTableColumnFlags_IndentDisable        = 1 << 17,
};

// Structures
struct ImGuiStyle {
    float       Alpha;
    float       DisabledAlpha;
    ImVec2      WindowPadding;
    float       WindowRounding;
    float       WindowBorderSize;
    ImVec2      WindowMinSize;
    ImVec2      WindowTitleAlign;
    int         WindowMenuButtonPosition;
    float       ChildRounding;
    float       ChildBorderSize;
    float       PopupRounding;
    float       PopupBorderSize;
    ImVec2      FramePadding;
    float       FrameRounding;
    float       FrameBorderSize;
    ImVec2      ItemSpacing;
    ImVec2      ItemInnerSpacing;
    ImVec2      CellPadding;
    ImVec2      TouchExtraPadding;
    float       IndentSpacing;
    float       ColumnsMinSpacing;
    float       ScrollbarSize;
    float       ScrollbarRounding;
    float       GrabMinSize;
    float       GrabRounding;
    float       LogSliderDeadzone;
    float       TabRounding;
    float       TabBorderSize;
    float       TabMinWidthForCloseButton;
    int         ColorButtonPosition;
    ImVec2      ButtonTextAlign;
    ImVec2      SelectableTextAlign;
    ImVec2      DisplayWindowPadding;
    ImVec2      DisplaySafeAreaPadding;
    float       MouseCursorScale;
    bool        AntiAliasedLines;
    bool        AntiAliasedLinesUseTex;
    bool        AntiAliasedFill;
    float       CurveTessellationTol;
    float       CircleTessellationMaxError;
    ImVec4      Colors[ImGuiCol_COUNT];
};

struct ImGuiViewport {
    ImGuiID             ID;
    int                 Flags;
    ImVec2              Pos;
    ImVec2              Size;
    ImVec2              WorkPos;
    ImVec2              WorkSize;
    float               DpiScale;
    ImGuiID             ParentViewportId;
    void*               RendererUserData;
    void*               PlatformUserData;
    void*               PlatformHandle;
    void*               PlatformHandleRaw;
    bool                PlatformRequestMove;
    bool                PlatformRequestResize;
    bool                PlatformRequestClose;
};

struct ImFontAtlas {
    // Font atlas methods
    ImFont* AddFontDefault(const ImFontConfig* font_cfg = nullptr);
    ImFont* AddFontFromFileTTF(const char* filename, float size_pixels, const ImFontConfig* font_cfg = nullptr, const unsigned short* glyph_ranges = nullptr);
    const unsigned short* GetGlyphRangesCyrillic();
    const unsigned short* GetGlyphRangesDefault();
    
    // Members
    bool Locked;
    int Flags;
    void* TexID;
    int TexDesiredWidth;
    int TexGlyphPadding;
    bool TexReady;
    unsigned char* TexPixelsAlpha8;
    unsigned int* TexPixelsRGBA32;
    int TexWidth;
    int TexHeight;
    ImVec2 TexUvScale;
    ImVec2 TexUvWhitePixel;
    
    // ImVector<ImFont*> Fonts - simplified as raw pointer
    ImFont** Fonts;
    int FontsCount;
};

struct ImGuiIO {
    ImGuiConfigFlags ConfigFlags;
    ImGuiConfigFlags BackendFlags;
    ImVec2 DisplaySize;
    float DeltaTime;
    float IniSavingRate;
    const char* IniFilename;
    const char* LogFilename;
    float MouseDoubleClickTime;
    float MouseDoubleClickMaxDist;
    float MouseDragThreshold;
    float KeyRepeatDelay;
    float KeyRepeatRate;
    void* UserData;
    
    ImFontAtlas* Fonts;
    float FontGlobalScale;
    bool FontAllowUserScaling;
    ImFont* FontDefault;
    ImVec2 DisplayFramebufferScale;
    
    // Other IO members...
    bool WantCaptureMouse;
    bool WantCaptureKeyboard;
    bool WantTextInput;
    bool WantSetMousePos;
    bool WantSaveIniSettings;
    bool NavActive;
    bool NavVisible;
    float Framerate;
    int MetricsRenderVertices;
    int MetricsRenderIndices;
    int MetricsRenderWindows;
    int MetricsActiveWindows;
    int MetricsActiveAllocations;
    ImVec2 MouseDelta;
};

struct ImFont {
    float FontSize;
    float Scale;
    ImVec2 DisplayOffset;
    ImFontAtlas* ContainerAtlas;
    float Ascent, Descent;
};

// Main API namespace
namespace ImGui {
    // Context
    ImGuiContext* CreateContext(ImFontAtlas* shared_font_atlas = nullptr);
    void DestroyContext(ImGuiContext* ctx = nullptr);
    ImGuiContext* GetCurrentContext();
    void SetCurrentContext(ImGuiContext* ctx);
    
    // IO
    ImGuiIO& GetIO();
    ImGuiStyle& GetStyle();
    
    // Main
    void NewFrame();
    void EndFrame();
    void Render();
    ImDrawData* GetDrawData();
    
    // Windows
    bool Begin(const char* name, bool* p_open = nullptr, ImGuiWindowFlags flags = 0);
    void End();
    
    // Child Windows
    bool BeginChild(const char* str_id, const ImVec2& size = ImVec2(0, 0), bool border = false, ImGuiWindowFlags flags = 0);
    void EndChild();
    
    // Viewport
    ImGuiViewport* GetMainViewport();
    
    // Window utilities
    float GetWindowWidth();
    float GetWindowHeight();
    void SetNextWindowPos(const ImVec2& pos, ImGuiCond cond = 0, const ImVec2& pivot = ImVec2(0, 0));
    void SetNextWindowSize(const ImVec2& size, ImGuiCond cond = 0);
    void SetWindowFontScale(float scale);
    
    // Cursor
    void SetCursorPosX(float local_x);
    
    // Text
    void Text(const char* fmt, ...);
    void TextColored(const ImVec4& col, const char* fmt, ...);
    void TextWrapped(const char* fmt, ...);
    ImVec2 CalcTextSize(const char* text, const char* text_end = nullptr, bool hide_text_after_double_hash = false, float wrap_width = -1.0f);
    
    // Widgets: Main
    bool Button(const char* label, const ImVec2& size = ImVec2(0, 0));
    void ProgressBar(float fraction, const ImVec2& size_arg = ImVec2(-FLT_MIN, 0), const char* overlay = nullptr);
    
    // Widgets: Trees
    bool CollapsingHeader(const char* label, ImGuiTreeNodeFlags flags = 0);
    
    // Widgets: Tabs
    bool BeginTabBar(const char* str_id, int flags = 0);
    void EndTabBar();
    bool BeginTabItem(const char* label, bool* p_open = nullptr, int flags = 0);
    void EndTabItem();
    
    // Tables
    bool BeginTable(const char* str_id, int column, int flags = 0, const ImVec2& outer_size = ImVec2(0.0f, 0.0f), float inner_width = 0.0f);
    void EndTable();
    void TableSetupColumn(const char* label, int flags = 0, float init_width_or_weight = 0.0f, unsigned int user_id = 0);
    void TableHeadersRow();
    void TableNextRow(int row_flags = 0, float min_row_height = 0.0f);
    bool TableSetColumnIndex(int column_n);
    
    // Columns (legacy)
    void Columns(int count = 1, const char* id = nullptr, bool border = true);
    void NextColumn();
    
    // Layout
    void Separator();
    void SameLine(float offset_from_start_x = 0.0f, float spacing = -1.0f);
    void Spacing();
    void Indent(float indent_w = 0.0f);
    void Unindent(float indent_w = 0.0f);
    
    // ID
    void PushID(int int_id);
    void PopID();
    
    // Styling
    void PushStyleColor(ImGuiCol idx, const ImVec4& col);
    void PopStyleColor(int count = 1);
    void PushFont(ImFont* font);
    void PopFont();
    
    // Disabled
    void BeginDisabled(bool disabled = true);
    void EndDisabled();
}

// Version check
#define IMGUI_CHECKVERSION() ImGui::DebugCheckVersionAndDataLayout(IMGUI_VERSION, sizeof(ImGuiIO), sizeof(ImGuiStyle), sizeof(ImVec2), sizeof(ImVec4), sizeof(ImDrawVert), sizeof(unsigned int))

namespace ImGui {
    bool DebugCheckVersionAndDataLayout(const char* version, size_t sz_io, size_t sz_style, size_t sz_vec2, size_t sz_vec4, size_t sz_vert, size_t sz_idx);
}

// For backend
#define IMGUI_IMPL_API

#endif // IMGUI_H
