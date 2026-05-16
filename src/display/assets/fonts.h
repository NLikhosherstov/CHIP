#pragma once
// Заголовок шрифтовых ресурсов.
// Шрифты хранятся как const-указатели на GFXfont.
// Пока реальные данные не добавлены — все указатели равны nullptr.
// При nullptr TFT_eSPI откатывается к встроенному шрифту (GLCD/FONT2 и т.д.).
//
// Как заполнить данные:
//   1. Скачать TTF: Aldrich и Roboto Mono Bold с Google Fonts.
//   2. Собрать утилиту fontconvert из Adafruit_GFX (tools/fontconvert/).
//   3. Выполнить для каждого нужного размера:
//        ./fontconvert Aldrich-Regular.ttf 34 > src/display/assets/font_aldrich_34.h
//   4. В fonts.cpp заменить nullptr на &Font_Aldrich_34_Data, включив нужный .h-файл.
//
// Для TFT_eSPI шрифты — структуры GFXfont. Включение TFT_eSPI.h обеспечивает нужный тип.

#include <TFT_eSPI.h>

// ─── Aldrich (основной шрифт чисел и режимов) ─────────────────────────────────
extern const GFXfont* const Font_Aldrich_70;  // Температура теплообменника (крупно)
extern const GFXfont* const Font_Aldrich_34;  // Температура/влажность (мелко)
extern const GFXfont* const Font_Aldrich_28;  // Цифра скорости
extern const GFXfont* const Font_Aldrich_24;  // Название режима, значения виджетов

// ─── Roboto Mono Bold (подписи и единицы измерения) ───────────────────────────
extern const GFXfont* const Font_RobotoMono_40; // Символ "°" рядом с большой температурой
extern const GFXfont* const Font_RobotoMono_12; // Мелкие подписи ("режим", "л/ч", "сек")
