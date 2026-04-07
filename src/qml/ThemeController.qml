import QtQuick

QtObject {
    id: root

    property string currentThemeKey: "dark"
    readonly property bool isDark: currentThemeKey === "dark"

    property var themeModel: ListModel {
        ListElement { key: "dark"; name: "深色" }
        ListElement { key: "light"; name: "浅色" }
    }

    function setThemeByKey(key) {
        var k = String(key);
        for (var i = 0; i < themeModel.count; i++) {
            if (themeModel.get(i).key === k) {
                currentThemeKey = k;
                return;
            }
        }
    }

    function toggleTheme() {
        currentThemeKey = isDark ? "light" : "dark";
    }

    readonly property var palettes: ({
        "dark": {
            windowGradientTop: "#0b0f19",
            windowGradientMid: "#0a1020",
            windowGradientBottom: "#060912",
            windowOverlayColor: "#000000",
            windowOverlayOpacity: 0.12,

            surfaceStrong: "#111827",
            surface: "#0f172a",
            surfaceAlt: "#0b1220",
            border: "#2a3446",
            borderStrong: "#22304a",
            separator: "#1f2937",

            textPrimary: "#e5e7eb",
            textSecondary: "#cbd5e1",
            textMuted: "#9ca3af",

            accent: "#6d28d9",
            accentSoft: "#2e1065",
            accentBorder: "#a78bfa",
            success: "#22c55e",
            warning: "#f59e0b",
            danger: "#ef4444",
            dangerSoft: "#7f1d1d",
            dangerText: "#fecaca",

            chipBg: "#111827",
            chipBorder: "#2a3446",

            modeTileHoverTop: "#131d34",
            modeTileHoverBottom: "#0b1223",

            boardSceneBg: "#0b1220",
            boardCellBg: "#111827",
            boardCellTop: "#0f172a",
            boardCellBottom: "#0b1220",
            boardCellInner: "#0b1220",

            cubeEmpty: "#111827",
            cubeFilled: "#6d28d9",

            axisX: "#ef4444",
            axisY: "#3b82f6",
            axisZ: "#22c55e",

            comboMaterialBg: "#111827",
            comboBg: "#0c1423",
            comboPopupBg: "#101826"
        },
        "light": {
            windowGradientTop: "#f7fafc",
            windowGradientMid: "#eef3f9",
            windowGradientBottom: "#e6edf7",
            windowOverlayColor: "#ffffff",
            windowOverlayOpacity: 0.06,

            surfaceStrong: "#ffffff",
            surface: "#f8fbff",
            surfaceAlt: "#f1f5fb",
            border: "#c9d6e8",
            borderStrong: "#b6c6dc",
            separator: "#d6e0ee",

            textPrimary: "#1f2937",
            textSecondary: "#334155",
            textMuted: "#64748b",

            accent: "#2563eb",
            accentSoft: "#dbeafe",
            accentBorder: "#60a5fa",
            success: "#16a34a",
            warning: "#d97706",
            danger: "#dc2626",
            dangerSoft: "#fee2e2",
            dangerText: "#991b1b",

            chipBg: "#eef2f7",
            chipBorder: "#c8d2e0",

            modeTileHoverTop: "#edf4ff",
            modeTileHoverBottom: "#e6effb",

            boardSceneBg: "#f3f7fd",
            boardCellBg: "#e6edf7",
            boardCellTop: "#f7fbff",
            boardCellBottom: "#eaf1fb",
            boardCellInner: "#fdfefe",

            cubeEmpty: "#d9e4f2",
            cubeFilled: "#2563eb",

            axisX: "#ef4444",
            axisY: "#3b82f6",
            axisZ: "#22c55e",

            comboMaterialBg: "#ffffff",
            comboBg: "#ffffff",
            comboPopupBg: "#ffffff"
        }
    })

    readonly property var activePalette: palettes[currentThemeKey] ? palettes[currentThemeKey] : palettes.dark

    function paletteValue(name) {
        if (activePalette[name] !== undefined)
            return activePalette[name];
        return palettes.dark[name];
    }

    // Window / global surfaces
    readonly property color windowGradientTop: paletteValue("windowGradientTop")
    readonly property color windowGradientMid: paletteValue("windowGradientMid")
    readonly property color windowGradientBottom: paletteValue("windowGradientBottom")
    readonly property color windowOverlayColor: paletteValue("windowOverlayColor")
    readonly property real windowOverlayOpacity: paletteValue("windowOverlayOpacity")

    readonly property color surfaceStrong: paletteValue("surfaceStrong")
    readonly property color surface: paletteValue("surface")
    readonly property color surfaceAlt: paletteValue("surfaceAlt")
    readonly property color border: paletteValue("border")
    readonly property color borderStrong: paletteValue("borderStrong")
    readonly property color separator: paletteValue("separator")

    // Typography
    readonly property color textPrimary: paletteValue("textPrimary")
    readonly property color textSecondary: paletteValue("textSecondary")
    readonly property color textMuted: paletteValue("textMuted")

    // Accent / status
    readonly property color accent: paletteValue("accent")
    readonly property color accentSoft: paletteValue("accentSoft")
    readonly property color accentBorder: paletteValue("accentBorder")
    readonly property color success: paletteValue("success")
    readonly property color warning: paletteValue("warning")
    readonly property color danger: paletteValue("danger")
    readonly property color dangerSoft: paletteValue("dangerSoft")
    readonly property color dangerText: paletteValue("dangerText")

    // Cards / chips
    readonly property color chipBg: paletteValue("chipBg")
    readonly property color chipBorder: paletteValue("chipBorder")

    // Mode tile hover gradient
    readonly property color modeTileHoverTop: paletteValue("modeTileHoverTop")
    readonly property color modeTileHoverBottom: paletteValue("modeTileHoverBottom")

    // Board specific
    readonly property color boardSceneBg: paletteValue("boardSceneBg")
    readonly property color boardCellBg: paletteValue("boardCellBg")
    readonly property color boardCellTop: paletteValue("boardCellTop")
    readonly property color boardCellBottom: paletteValue("boardCellBottom")
    readonly property color boardCellInner: paletteValue("boardCellInner")

    // 3D
    readonly property color cubeEmpty: paletteValue("cubeEmpty")
    readonly property color cubeFilled: paletteValue("cubeFilled")

    // Key hint axis colors
    readonly property color axisX: paletteValue("axisX")
    readonly property color axisY: paletteValue("axisY")
    readonly property color axisZ: paletteValue("axisZ")

    // ComboBox themed surfaces
    readonly property color comboMaterialBg: paletteValue("comboMaterialBg")
    readonly property color comboBg: paletteValue("comboBg")
    readonly property color comboPopupBg: paletteValue("comboPopupBg")
}
