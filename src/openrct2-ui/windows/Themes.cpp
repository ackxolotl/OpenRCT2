/*****************************************************************************
 * Copyright (c) 2014-2020 OpenRCT2 developers
 *
 * For a complete list of all authors, please refer to contributors.md
 * Interested in contributing? Visit https://github.com/OpenRCT2/OpenRCT2
 *
 * OpenRCT2 is licensed under the GNU General Public License version 3.
 *****************************************************************************/

#include "../interface/Theme.h"

#include <openrct2-ui/interface/Dropdown.h>
#include <openrct2-ui/interface/Widget.h>
#include <openrct2-ui/windows/Window.h>
#include <openrct2/Context.h>
#include <openrct2/Game.h>
#include <openrct2/Input.h>
#include <openrct2/config/Config.h>
#include <openrct2/drawing/Drawing.h>
#include <openrct2/localisation/Localisation.h>
#include <openrct2/sprites.h>
#include <openrct2/util/Util.h>

// clang-format off
enum {
    WINDOW_THEMES_TAB_SETTINGS,
    WINDOW_THEMES_TAB_MAIN_UI,
    WINDOW_THEMES_TAB_PARK,
    WINDOW_THEMES_TAB_TOOLS,
    WINDOW_THEMES_TAB_RIDES_PEEPS,
    WINDOW_THEMES_TAB_EDITORS,
    WINDOW_THEMES_TAB_MISC,
    WINDOW_THEMES_TAB_PROMPTS,
    WINDOW_THEMES_TAB_FEATURES,
    WINDOW_THEMES_TAB_COUNT
};

static void window_themes_mouseup(rct_window *w, rct_widgetindex widgetIndex);
static void window_themes_resize(rct_window *w);
static void window_themes_mousedown(rct_window *w, rct_widgetindex widgetIndex, rct_widget* widget);
static void window_themes_dropdown(rct_window *w, rct_widgetindex widgetIndex, int32_t dropdownIndex);
static void window_themes_update(rct_window *w);
static void window_themes_scrollgetsize(rct_window *w, int32_t scrollIndex, int32_t *width, int32_t *height);
static void window_themes_scrollmousedown(rct_window *w, int32_t scrollIndex, const ScreenCoordsXY& screenCoords);
static void window_themes_scrollmouseover(rct_window *w, int32_t scrollIndex, const ScreenCoordsXY& screenCoords);
static void window_themes_textinput(rct_window *w, rct_widgetindex widgetIndex, char *text);
static void window_themes_invalidate(rct_window *w);
static void window_themes_paint(rct_window *w, rct_drawpixelinfo *dpi);
static void window_themes_scrollpaint(rct_window *w, rct_drawpixelinfo *dpi, int32_t scrollIndex);
static void window_themes_draw_tab_images(rct_drawpixelinfo *dpi, rct_window *w);

static rct_window_event_list window_themes_events = {
    nullptr,
    window_themes_mouseup,
    window_themes_resize,
    window_themes_mousedown,
    window_themes_dropdown,
    nullptr,
    window_themes_update,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    window_themes_scrollgetsize,
    window_themes_scrollmousedown,
    nullptr,
    window_themes_scrollmouseover,
    window_themes_textinput,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    window_themes_invalidate,
    window_themes_paint,
    window_themes_scrollpaint,
};

enum WINDOW_STAFF_LIST_WIDGET_IDX {
    WIDX_THEMES_BACKGROUND,
    WIDX_THEMES_TITLE,
    WIDX_THEMES_CLOSE,
    WIDX_THEMES_TAB_CONTENT_PANEL,
    WIDX_THEMES_SETTINGS_TAB,
    WIDX_THEMES_MAIN_UI_TAB,
    WIDX_THEMES_PARK_TAB,
    WIDX_THEMES_TOOLS_TAB,
    WIDX_THEMES_RIDE_PEEPS_TAB,
    WIDX_THEMES_EDITORS_TAB,
    WIDX_THEMES_MISC_TAB,
    WIDX_THEMES_PROMPTS_TAB,
    WIDX_THEMES_FEATURES_TAB,
    WIDX_THEMES_HEADER_WINDOW,
    WIDX_THEMES_HEADER_PALETTE,
    WIDX_THEMES_PRESETS,
    WIDX_THEMES_PRESETS_DROPDOWN,
    WIDX_THEMES_DUPLICATE_BUTTON,
    WIDX_THEMES_DELETE_BUTTON,
    WIDX_THEMES_RENAME_BUTTON,
    WIDX_THEMES_COLOURBTN_MASK,
    WIDX_THEMES_LIST,
    WIDX_THEMES_RCT1_RIDE_LIGHTS,
    WIDX_THEMES_RCT1_PARK_LIGHTS,
    WIDX_THEMES_RCT1_SCENARIO_FONT,
    WIDX_THEMES_RCT1_BOTTOM_TOOLBAR
};

static constexpr const rct_string_id WINDOW_TITLE = STR_THEMES_TITLE;
static constexpr const int32_t WW = 320;
static constexpr const int32_t WH = 107;

static rct_widget window_themes_widgets[] = {
    WINDOW_SHIM(WINDOW_TITLE, WW, WH),
    MakeWidget({  0, 43}, {320,  64}, WWT_RESIZE,       WindowColour::Secondary                                                                                     ), // tab content panel
    MakeTab   ({  3, 17},                                                                                                        STR_THEMES_TAB_SETTINGS_TIP        ), // settings tab
    MakeTab   ({ 34, 17},                                                                                                        STR_THEMES_TAB_MAIN_TIP            ), // main ui tab
    MakeTab   ({ 65, 17},                                                                                                        STR_THEMES_TAB_PARK_TIP            ), // park tab
    MakeTab   ({ 96, 17},                                                                                                        STR_THEMES_TAB_TOOLS_TIP           ), // tools tab
    MakeTab   ({127, 17},                                                                                                        STR_THEMES_TAB_RIDES_AND_GUESTS_TIP), // rides and peeps tab
    MakeTab   ({158, 17},                                                                                                        STR_THEMES_TAB_EDITORS_TIP         ), // editors tab
    MakeTab   ({189, 17},                                                                                                        STR_THEMES_TAB_MISC_TIP            ), // misc tab
    MakeTab   ({220, 17},                                                                                                        STR_THEMES_TAB_PROMPTS_TIP         ), // prompts tab
    MakeTab   ({251, 17},                                                                                                        STR_THEMES_TAB_FEATURES_TIP        ), // features tab
    MakeWidget({  5, 46}, {214,  15}, WWT_TABLE_HEADER, WindowColour::Secondary, STR_THEMES_HEADER_WINDOW                                                           ), // Window header
    MakeWidget({219, 46}, { 97,  15}, WWT_TABLE_HEADER, WindowColour::Secondary, STR_THEMES_HEADER_PALETTE                                                          ), // Palette header
    MakeWidget({125, 60}, {175,  12}, WWT_DROPDOWN,     WindowColour::Secondary                                                                                     ), // Preset colour schemes
    MakeWidget({288, 61}, { 11,  10}, WWT_BUTTON,       WindowColour::Secondary, STR_DROPDOWN_GLYPH                                                                 ),
    MakeWidget({ 10, 82}, { 91,  12}, WWT_BUTTON,       WindowColour::Secondary, STR_TITLE_EDITOR_ACTION_DUPLICATE,              STR_THEMES_ACTION_DUPLICATE_TIP    ), // Duplicate button
    MakeWidget({110, 82}, { 91,  12}, WWT_BUTTON,       WindowColour::Secondary, STR_TRACK_MANAGE_DELETE,                        STR_THEMES_ACTION_DELETE_TIP       ), // Delete button
    MakeWidget({210, 82}, { 91,  12}, WWT_BUTTON,       WindowColour::Secondary, STR_TRACK_MANAGE_RENAME,                        STR_THEMES_ACTION_RENAME_TIP       ), // Rename button
    MakeWidget({  0,  0}, {  1,   1}, WWT_COLOURBTN,    WindowColour::Secondary                                                                                     ), // colour button mask
    MakeWidget({  3, 60}, {314,  44}, WWT_SCROLL,       WindowColour::Secondary, SCROLL_VERTICAL                                                                    ), // staff list
    MakeWidget({ 10, 54}, {290,  12}, WWT_CHECKBOX,     WindowColour::Secondary, STR_THEMES_OPTION_RCT1_RIDE_CONTROLS                                               ), // rct1 ride lights
    MakeWidget({ 10, 69}, {290,  12}, WWT_CHECKBOX,     WindowColour::Secondary, STR_THEMES_OPTION_RCT1_PARK_CONTROLS                                               ), // rct1 park lights
    MakeWidget({ 10, 84}, {290,  12}, WWT_CHECKBOX,     WindowColour::Secondary, STR_THEMES_OPTION_RCT1_SCENARIO_SELECTION_FONT                                     ), // rct1 scenario font
    MakeWidget({ 10, 99}, {290,  12}, WWT_CHECKBOX,     WindowColour::Secondary, STR_THEMES_OPTION_RCT1_BOTTOM_TOOLBAR                                              ), // rct1 bottom toolbar
    { WIDGETS_END },
};

static int32_t window_themes_tab_animation_loops[] = {
    32,
    32,
    1,
    1,
    64,
    32,
    8,
    14,
    38
};
static int32_t window_themes_tab_animation_divisor[] = {
    4,
    4,
    1,
    1,
    4,
    2,
    2,
    2,
    2
};
static int32_t window_themes_tab_sprites[] = {
    SPR_TAB_PAINT_0,
    SPR_TAB_KIOSKS_AND_FACILITIES_0,
    SPR_TAB_PARK_ENTRANCE,
    SPR_G2_TAB_LAND,
    SPR_TAB_RIDE_0,
    SPR_TAB_WRENCH_0,
    SPR_TAB_GEARS_0,
    SPR_TAB_STAFF_OPTIONS_0,
    SPR_TAB_FINANCES_MARKETING_0
};

static rct_windowclass window_themes_tab_1_classes[] = {
    WC_TOP_TOOLBAR,
    WC_BOTTOM_TOOLBAR,
    WC_EDITOR_SCENARIO_BOTTOM_TOOLBAR,
    WC_EDITOR_TRACK_BOTTOM_TOOLBAR,
    WC_TITLE_MENU,
    WC_TITLE_EXIT,
    WC_TITLE_OPTIONS,
    WC_SCENARIO_SELECT
};

static rct_windowclass window_themes_tab_2_classes[] = {
    WC_PARK_INFORMATION,
    WC_FINANCES,
    WC_NEW_CAMPAIGN,
    WC_RESEARCH,
    WC_MAP,
    WC_VIEWPORT,
    WC_RECENT_NEWS
};

static rct_windowclass window_themes_tab_3_classes[] = {
    WC_LAND,
    WC_WATER,
    WC_CLEAR_SCENERY,
    WC_LAND_RIGHTS,
    WC_SCENERY,
    WC_SCENERY_SCATTER,
    WC_FOOTPATH,
    WC_RIDE_CONSTRUCTION,
    WC_TRACK_DESIGN_PLACE,
    WC_CONSTRUCT_RIDE,
    WC_TRACK_DESIGN_LIST
};

static rct_windowclass window_themes_tab_4_classes[] = {
    WC_RIDE,
    WC_RIDE_LIST,
    WC_PEEP,
    WC_GUEST_LIST,
    WC_STAFF,
    WC_STAFF_LIST,
    WC_BANNER
};

static rct_windowclass window_themes_tab_5_classes[] = {
    WC_EDITOR_OBJECT_SELECTION,
    WC_EDITOR_INVENTION_LIST,
    WC_EDITOR_SCENARIO_OPTIONS,
    WC_EDTIOR_OBJECTIVE_OPTIONS,
    WC_MAPGEN,
    WC_MANAGE_TRACK_DESIGN,
    WC_INSTALL_TRACK
};

static rct_windowclass window_themes_tab_6_classes[] = {
    WC_CHEATS,
    WC_TILE_INSPECTOR,
    WC_VIEW_CLIPPING,
    WC_THEMES,
    WC_TITLE_EDITOR,
    WC_OPTIONS,
    WC_KEYBOARD_SHORTCUT_LIST,
    WC_CHANGE_KEYBOARD_SHORTCUT,
    WC_LOADSAVE,
    WC_CHANGELOG,
    WC_SERVER_LIST,
    WC_MULTIPLAYER,
    WC_PLAYER,
    WC_CHAT,
    WC_CONSOLE,
};

static rct_windowclass window_themes_tab_7_classes[] = {
    WC_SAVE_PROMPT,
    WC_DEMOLISH_RIDE_PROMPT,
    WC_FIRE_PROMPT,
    WC_TRACK_DELETE_PROMPT,
    WC_LOADSAVE_OVERWRITE_PROMPT,
    WC_NETWORK_STATUS,
};

static rct_windowclass *window_themes_tab_classes[] = {
    nullptr,
    window_themes_tab_1_classes,
    window_themes_tab_2_classes,
    window_themes_tab_3_classes,
    window_themes_tab_4_classes,
    window_themes_tab_5_classes,
    window_themes_tab_6_classes,
    window_themes_tab_7_classes,
};
// clang-format on

static uint8_t _selected_tab = 0;
static int16_t _colour_index_1 = -1;
static int8_t _colour_index_2 = -1;
static constexpr const uint8_t _row_height = 32;
static constexpr const uint8_t _button_offset_x = 220;
static constexpr const uint8_t _button_offset_y = 3;
static constexpr const uint8_t _check_offset_y = 3 + 12 + 2;

static void window_themes_init_vars()
{
    _selected_tab = WINDOW_THEMES_TAB_SETTINGS;
}

static rct_windowclass get_window_class_tab_index(int32_t index)
{
    rct_windowclass* classes = window_themes_tab_classes[_selected_tab];
    return classes[index];
}

static int32_t get_colour_scheme_tab_count()
{
    switch (_selected_tab)
    {
        case 1:
            return sizeof(window_themes_tab_1_classes);
        case 2:
            return sizeof(window_themes_tab_2_classes);
        case 3:
            return sizeof(window_themes_tab_3_classes);
        case 4:
            return sizeof(window_themes_tab_4_classes);
        case 5:
            return sizeof(window_themes_tab_5_classes);
        case 6:
            return sizeof(window_themes_tab_6_classes);
        case 7:
            return sizeof(window_themes_tab_7_classes);
    }
    return 0;
}

static void window_themes_draw_tab_images(rct_drawpixelinfo* dpi, rct_window* w)
{
    for (int32_t i = 0; i < WINDOW_THEMES_TAB_COUNT; i++)
    {
        int32_t sprite_idx = window_themes_tab_sprites[i];
        if (_selected_tab == i)
            sprite_idx += w->frame_no / window_themes_tab_animation_divisor[_selected_tab];
        gfx_draw_sprite(
            dpi, sprite_idx,
            w->windowPos
                + ScreenCoordsXY{ w->widgets[WIDX_THEMES_SETTINGS_TAB + i].left, w->widgets[WIDX_THEMES_SETTINGS_TAB + i].top },
            0);
    }
}

rct_window* window_themes_open()
{
    rct_window* window;

    // Check if window is already open
    window = window_bring_to_front_by_class(WC_THEMES);
    if (window != nullptr)
        return window;

    window = window_create_auto_pos(320, 107, &window_themes_events, WC_THEMES, WF_10 | WF_RESIZABLE);
    window->widgets = window_themes_widgets;
    window->enabled_widgets = (1 << WIDX_THEMES_CLOSE) | (1 << WIDX_THEMES_SETTINGS_TAB) | (1 << WIDX_THEMES_MAIN_UI_TAB)
        | (1 << WIDX_THEMES_PARK_TAB) | (1 << WIDX_THEMES_TOOLS_TAB) | (1 << WIDX_THEMES_RIDE_PEEPS_TAB)
        | (1 << WIDX_THEMES_EDITORS_TAB) | (1 << WIDX_THEMES_MISC_TAB) | (1 << WIDX_THEMES_PROMPTS_TAB)
        | (1 << WIDX_THEMES_FEATURES_TAB) | (1 << WIDX_THEMES_COLOURBTN_MASK) | (1 << WIDX_THEMES_PRESETS)
        | (1 << WIDX_THEMES_PRESETS_DROPDOWN) | (1 << WIDX_THEMES_DUPLICATE_BUTTON) | (1 << WIDX_THEMES_DELETE_BUTTON)
        | (1 << WIDX_THEMES_RENAME_BUTTON) | (1 << WIDX_THEMES_RCT1_RIDE_LIGHTS) | (1 << WIDX_THEMES_RCT1_PARK_LIGHTS)
        | (1 << WIDX_THEMES_RCT1_SCENARIO_FONT) | (1 << WIDX_THEMES_RCT1_BOTTOM_TOOLBAR);

    window_themes_init_vars();

    window_init_scroll_widgets(window);
    window->list_information_type = 0;
    _colour_index_1 = -1;
    _colour_index_2 = -1;
    window->min_width = 320;
    window->min_height = 107;
    window->max_width = 320;
    window->max_height = 107;

    return window;
}

static void window_themes_mouseup(rct_window* w, rct_widgetindex widgetIndex)
{
    size_t activeAvailableThemeIndex;
    const utf8* activeThemeName;

    switch (widgetIndex)
    {
        case WIDX_THEMES_CLOSE:
            window_close(w);
            break;
        case WIDX_THEMES_DUPLICATE_BUTTON:;
            activeAvailableThemeIndex = theme_manager_get_active_available_theme_index();
            activeThemeName = theme_manager_get_available_theme_name(activeAvailableThemeIndex);
            window_text_input_open(
                w, widgetIndex, STR_TITLE_EDITOR_ACTION_DUPLICATE, STR_THEMES_PROMPT_ENTER_THEME_NAME, STR_STRING,
                reinterpret_cast<uintptr_t>(activeThemeName), 64);
            break;
        case WIDX_THEMES_DELETE_BUTTON:
            if (theme_get_flags() & UITHEME_FLAG_PREDEFINED)
            {
                context_show_error(STR_THEMES_ERR_CANT_CHANGE_THIS_THEME, STR_NONE);
            }
            else
            {
                theme_delete();
            }
            break;
        case WIDX_THEMES_RENAME_BUTTON:
            if (theme_get_flags() & UITHEME_FLAG_PREDEFINED)
            {
                context_show_error(STR_THEMES_ERR_CANT_CHANGE_THIS_THEME, STR_NONE);
            }
            else
            {
                activeAvailableThemeIndex = theme_manager_get_active_available_theme_index();
                activeThemeName = theme_manager_get_available_theme_name(activeAvailableThemeIndex);
                window_text_input_open(
                    w, widgetIndex, STR_TRACK_MANAGE_RENAME, STR_THEMES_PROMPT_ENTER_THEME_NAME, STR_STRING,
                    reinterpret_cast<uintptr_t>(activeThemeName), 64);
            }
            break;
    }
}

static void window_themes_resize(rct_window* w)
{
    if (_selected_tab == WINDOW_THEMES_TAB_SETTINGS)
    {
        w->min_width = 320;
        w->min_height = 107;
        w->max_width = 320;
        w->max_height = 107;

        if (w->width < w->min_width)
        {
            w->width = w->min_width;
            gfx_invalidate_screen();
        }
        if (w->height < w->min_height)
        {
            w->height = w->min_height;
            gfx_invalidate_screen();
        }
        if (w->width > w->max_width)
        {
            w->width = w->max_width;
            gfx_invalidate_screen();
        }
        if (w->height > w->max_height)
        {
            w->height = w->max_height;
            gfx_invalidate_screen();
        }
    }
    else if (_selected_tab == WINDOW_THEMES_TAB_FEATURES)
    {
        w->min_width = 320;
        w->min_height = 122;
        w->max_width = 320;
        w->max_height = 122;

        if (w->width < w->min_width)
        {
            w->width = w->min_width;
            gfx_invalidate_screen();
        }
        if (w->height < w->min_height)
        {
            w->height = w->min_height;
            gfx_invalidate_screen();
        }
        if (w->width > w->max_width)
        {
            w->width = w->max_width;
            gfx_invalidate_screen();
        }
        if (w->height > w->max_height)
        {
            w->height = w->max_height;
            gfx_invalidate_screen();
        }
    }
    else
    {
        w->min_width = 320;
        w->min_height = 270;
        w->max_width = 320;
        w->max_height = 450;

        if (w->width < w->min_width)
        {
            w->width = w->min_width;
            w->Invalidate();
        }
        if (w->height < w->min_height)
        {
            w->height = w->min_height;
            w->Invalidate();
        }
        if (w->width > w->max_width)
        {
            w->width = w->max_width;
            w->Invalidate();
        }
        if (w->height > w->max_height)
        {
            w->height = w->max_height;
            w->Invalidate();
        }
    }
}

static void window_themes_mousedown(rct_window* w, rct_widgetindex widgetIndex, rct_widget* widget)
{
    int16_t newSelectedTab;
    int32_t num_items;

    switch (widgetIndex)
    {
        case WIDX_THEMES_SETTINGS_TAB:
        case WIDX_THEMES_MAIN_UI_TAB:
        case WIDX_THEMES_PARK_TAB:
        case WIDX_THEMES_TOOLS_TAB:
        case WIDX_THEMES_RIDE_PEEPS_TAB:
        case WIDX_THEMES_EDITORS_TAB:
        case WIDX_THEMES_MISC_TAB:
        case WIDX_THEMES_PROMPTS_TAB:
        case WIDX_THEMES_FEATURES_TAB:
            newSelectedTab = widgetIndex - WIDX_THEMES_SETTINGS_TAB;
            if (_selected_tab == newSelectedTab)
                break;
            _selected_tab = static_cast<uint8_t>(newSelectedTab);
            w->scrolls[0].v_top = 0;
            w->frame_no = 0;
            window_event_resize_call(w);
            w->Invalidate();
            break;
        case WIDX_THEMES_PRESETS_DROPDOWN:
            theme_manager_load_available_themes();
            num_items = static_cast<int32_t>(theme_manager_get_num_available_themes());

            widget--;
            for (int32_t i = 0; i < num_items; i++)
            {
                gDropdownItemsFormat[i] = STR_OPTIONS_DROPDOWN_ITEM;
                gDropdownItemsArgs[i] = reinterpret_cast<uintptr_t>(theme_manager_get_available_theme_name(i));
            }

            window_dropdown_show_text_custom_width(
                { w->windowPos.x + widget->left, w->windowPos.y + widget->top }, widget->height() + 1, w->colours[1], 0,
                DROPDOWN_FLAG_STAY_OPEN, num_items, widget->width() - 3);

            dropdown_set_checked(static_cast<int32_t>(theme_manager_get_active_available_theme_index()), true);
            break;
        case WIDX_THEMES_RCT1_RIDE_LIGHTS:
            if (theme_get_flags() & UITHEME_FLAG_PREDEFINED)
            {
                context_show_error(STR_THEMES_ERR_CANT_CHANGE_THIS_THEME, STR_NONE);
            }
            else
            {
                theme_set_flags(theme_get_flags() ^ UITHEME_FLAG_USE_LIGHTS_RIDE);
                theme_save();
                window_invalidate_all();
            }
            break;
        case WIDX_THEMES_RCT1_PARK_LIGHTS:
            if (theme_get_flags() & UITHEME_FLAG_PREDEFINED)
            {
                context_show_error(STR_THEMES_ERR_CANT_CHANGE_THIS_THEME, STR_NONE);
            }
            else
            {
                theme_set_flags(theme_get_flags() ^ UITHEME_FLAG_USE_LIGHTS_PARK);
                theme_save();
                window_invalidate_all();
            }
            break;
        case WIDX_THEMES_RCT1_SCENARIO_FONT:
            if (theme_get_flags() & UITHEME_FLAG_PREDEFINED)
            {
                context_show_error(STR_THEMES_ERR_CANT_CHANGE_THIS_THEME, STR_NONE);
            }
            else
            {
                theme_set_flags(theme_get_flags() ^ UITHEME_FLAG_USE_ALTERNATIVE_SCENARIO_SELECT_FONT);
                theme_save();
                window_invalidate_all();
            }
            break;
        case WIDX_THEMES_RCT1_BOTTOM_TOOLBAR:
            if (theme_get_flags() & UITHEME_FLAG_PREDEFINED)
            {
                context_show_error(STR_THEMES_ERR_CANT_CHANGE_THIS_THEME, STR_NONE);
            }
            else
            {
                theme_set_flags(theme_get_flags() ^ UITHEME_FLAG_USE_FULL_BOTTOM_TOOLBAR);
                theme_save();
                window_invalidate_all();
            }
    }
}

static void window_themes_dropdown(rct_window* w, rct_widgetindex widgetIndex, int32_t dropdownIndex)
{
    switch (widgetIndex)
    {
        case WIDX_THEMES_LIST:
            if (dropdownIndex != -1)
            {
                rct_windowclass wc = get_window_class_tab_index(_colour_index_1);
                uint8_t colour = theme_get_colour(wc, _colour_index_2);
                colour = (colour & COLOUR_FLAG_TRANSLUCENT) | dropdownIndex;
                theme_set_colour(wc, _colour_index_2, colour);
                colour_scheme_update_all();
                window_invalidate_all();
                _colour_index_1 = -1;
                _colour_index_2 = -1;
            }
            break;
        case WIDX_THEMES_PRESETS_DROPDOWN:
            if (dropdownIndex != -1)
            {
                theme_manager_set_active_available_theme(dropdownIndex);
            }
            break;
    }
}

void window_themes_update(rct_window* w)
{
    w->frame_no++;
    if (w->frame_no >= window_themes_tab_animation_loops[_selected_tab])
        w->frame_no = 0;

    widget_invalidate(w, WIDX_THEMES_SETTINGS_TAB + _selected_tab);
}

void window_themes_scrollgetsize(rct_window* w, int32_t scrollIndex, int32_t* width, int32_t* height)
{
    if (_selected_tab == WINDOW_THEMES_TAB_SETTINGS || _selected_tab == WINDOW_THEMES_TAB_FEATURES)
        return;

    int32_t scrollHeight = get_colour_scheme_tab_count() * _row_height;
    int32_t i = scrollHeight - window_themes_widgets[WIDX_THEMES_LIST].bottom + window_themes_widgets[WIDX_THEMES_LIST].top
        + 21;
    if (i < 0)
        i = 0;
    if (i < w->scrolls[0].v_top)
    {
        w->scrolls[0].v_top = i;
        w->Invalidate();
    }

    *width = 420;
    *height = scrollHeight;
}

void window_themes_scrollmousedown(rct_window* w, int32_t scrollIndex, const ScreenCoordsXY& screenCoords)
{
    if (screenCoords.y / _row_height < get_colour_scheme_tab_count())
    {
        int32_t y2 = screenCoords.y % _row_height;
        _colour_index_1 = screenCoords.y / _row_height;
        _colour_index_2 = ((screenCoords.x - _button_offset_x) / 12);

        rct_windowclass wc = get_window_class_tab_index(_colour_index_1);
        int32_t numColours = theme_desc_get_num_colours(wc);
        if (_colour_index_2 < numColours)
        {
            if (screenCoords.x >= _button_offset_x && screenCoords.x < _button_offset_x + 12 * 6 && y2 >= _button_offset_y
                && y2 < _button_offset_y + 11)
            {
                if (theme_get_flags() & UITHEME_FLAG_PREDEFINED)
                {
                    context_show_error(STR_THEMES_ERR_CANT_CHANGE_THIS_THEME, STR_THEMES_DESC_CANT_CHANGE_THIS_THEME);
                }
                else
                {
                    window_themes_widgets[WIDX_THEMES_COLOURBTN_MASK].type = WWT_COLOURBTN;
                    window_themes_widgets[WIDX_THEMES_COLOURBTN_MASK].left = _button_offset_x + _colour_index_2 * 12
                        + window_themes_widgets[WIDX_THEMES_LIST].left;
                    window_themes_widgets[WIDX_THEMES_COLOURBTN_MASK].top = _colour_index_1 * _row_height + _button_offset_y
                        - w->scrolls[0].v_top + window_themes_widgets[WIDX_THEMES_LIST].top;
                    window_themes_widgets[WIDX_THEMES_COLOURBTN_MASK].right = window_themes_widgets[WIDX_THEMES_COLOURBTN_MASK]
                                                                                  .left
                        + 12;
                    window_themes_widgets[WIDX_THEMES_COLOURBTN_MASK].bottom = window_themes_widgets[WIDX_THEMES_COLOURBTN_MASK]
                                                                                   .top
                        + 12;

                    uint8_t colour = theme_get_colour(wc, _colour_index_2);
                    window_dropdown_show_colour(w, &(window_themes_widgets[WIDX_THEMES_COLOURBTN_MASK]), w->colours[1], colour);
                    widget_invalidate(w, WIDX_THEMES_LIST);
                }
            }
            else if (
                screenCoords.x >= _button_offset_x && screenCoords.x < _button_offset_x + 12 * 6 - 1 && y2 >= _check_offset_y
                && y2 < _check_offset_y + 11)
            {
                if (theme_get_flags() & UITHEME_FLAG_PREDEFINED)
                {
                    context_show_error(STR_THEMES_ERR_CANT_CHANGE_THIS_THEME, STR_THEMES_DESC_CANT_CHANGE_THIS_THEME);
                }
                else
                {
                    uint8_t colour = theme_get_colour(wc, _colour_index_2);
                    if (colour & COLOUR_FLAG_TRANSLUCENT)
                    {
                        colour &= ~COLOUR_FLAG_TRANSLUCENT;
                    }
                    else
                    {
                        colour |= COLOUR_FLAG_TRANSLUCENT;
                    }
                    theme_set_colour(wc, _colour_index_2, colour);
                    colour_scheme_update_all();
                    window_invalidate_all();
                }
            }
        }
    }
}

void window_themes_scrollmouseover(rct_window* w, int32_t scrollIndex, const ScreenCoordsXY& screenCoords)
{
}

static void window_themes_textinput(rct_window* w, rct_widgetindex widgetIndex, char* text)
{
    if (text == nullptr || text[0] == 0)
        return;

    switch (widgetIndex)
    {
        case WIDX_THEMES_DUPLICATE_BUTTON:
        case WIDX_THEMES_RENAME_BUTTON:
            if (filename_valid_characters(text))
            {
                if (theme_get_index_for_name(text) == SIZE_MAX)
                {
                    if (widgetIndex == WIDX_THEMES_DUPLICATE_BUTTON)
                    {
                        theme_duplicate(text);
                    }
                    else
                    {
                        theme_rename(text);
                    }
                    w->Invalidate();
                }
                else
                {
                    context_show_error(STR_THEMES_ERR_NAME_ALREADY_EXISTS, STR_NONE);
                }
            }
            else
            {
                context_show_error(STR_ERROR_INVALID_CHARACTERS, STR_NONE);
            }
            break;
    }
}

void window_themes_invalidate(rct_window* w)
{
    int32_t pressed_widgets = w->pressed_widgets
        & ~((1LL << WIDX_THEMES_SETTINGS_TAB) | (1LL << WIDX_THEMES_MAIN_UI_TAB) | (1LL << WIDX_THEMES_PARK_TAB)
            | (1LL << WIDX_THEMES_TOOLS_TAB) | (1LL << WIDX_THEMES_RIDE_PEEPS_TAB) | (1LL << WIDX_THEMES_EDITORS_TAB)
            | (1LL << WIDX_THEMES_MISC_TAB) | (1LL << WIDX_THEMES_PROMPTS_TAB) | (1LL << WIDX_THEMES_FEATURES_TAB));
    rct_widgetindex widgetIndex = _selected_tab + WIDX_THEMES_SETTINGS_TAB;

    w->pressed_widgets = pressed_widgets | (1 << widgetIndex);

    if (window_find_by_class(WC_DROPDOWN) == nullptr)
    {
        _colour_index_1 = -1;
        _colour_index_2 = -1;
    }

    window_themes_widgets[WIDX_THEMES_BACKGROUND].right = w->width - 1;
    window_themes_widgets[WIDX_THEMES_BACKGROUND].bottom = w->height - 1;
    window_themes_widgets[WIDX_THEMES_TAB_CONTENT_PANEL].right = w->width - 1;
    window_themes_widgets[WIDX_THEMES_TAB_CONTENT_PANEL].bottom = w->height - 1;
    window_themes_widgets[WIDX_THEMES_TITLE].right = w->width - 2;
    window_themes_widgets[WIDX_THEMES_CLOSE].left = w->width - 2 - 0x0B;
    window_themes_widgets[WIDX_THEMES_CLOSE].right = w->width - 2 - 0x0B + 0x0A;
    window_themes_widgets[WIDX_THEMES_LIST].right = w->width - 4;
    window_themes_widgets[WIDX_THEMES_LIST].bottom = w->height - 0x0F;

    if (_selected_tab == WINDOW_THEMES_TAB_SETTINGS)
    {
        window_themes_widgets[WIDX_THEMES_HEADER_WINDOW].type = WWT_EMPTY;
        window_themes_widgets[WIDX_THEMES_HEADER_PALETTE].type = WWT_EMPTY;
        window_themes_widgets[WIDX_THEMES_LIST].type = WWT_EMPTY;
        window_themes_widgets[WIDX_THEMES_RCT1_RIDE_LIGHTS].type = WWT_EMPTY;
        window_themes_widgets[WIDX_THEMES_RCT1_PARK_LIGHTS].type = WWT_EMPTY;
        window_themes_widgets[WIDX_THEMES_RCT1_SCENARIO_FONT].type = WWT_EMPTY;
        window_themes_widgets[WIDX_THEMES_RCT1_BOTTOM_TOOLBAR].type = WWT_EMPTY;
        window_themes_widgets[WIDX_THEMES_DUPLICATE_BUTTON].type = WWT_BUTTON;
        window_themes_widgets[WIDX_THEMES_DELETE_BUTTON].type = WWT_BUTTON;
        window_themes_widgets[WIDX_THEMES_RENAME_BUTTON].type = WWT_BUTTON;
        window_themes_widgets[WIDX_THEMES_PRESETS].type = WWT_DROPDOWN;
        window_themes_widgets[WIDX_THEMES_PRESETS_DROPDOWN].type = WWT_BUTTON;
        window_themes_widgets[WIDX_THEMES_COLOURBTN_MASK].type = WWT_EMPTY;
    }
    else if (_selected_tab == WINDOW_THEMES_TAB_FEATURES)
    {
        window_themes_widgets[WIDX_THEMES_HEADER_WINDOW].type = WWT_EMPTY;
        window_themes_widgets[WIDX_THEMES_HEADER_PALETTE].type = WWT_EMPTY;
        window_themes_widgets[WIDX_THEMES_LIST].type = WWT_EMPTY;
        window_themes_widgets[WIDX_THEMES_RCT1_RIDE_LIGHTS].type = WWT_CHECKBOX;
        window_themes_widgets[WIDX_THEMES_RCT1_PARK_LIGHTS].type = WWT_CHECKBOX;
        window_themes_widgets[WIDX_THEMES_RCT1_SCENARIO_FONT].type = WWT_CHECKBOX;
        window_themes_widgets[WIDX_THEMES_RCT1_BOTTOM_TOOLBAR].type = WWT_CHECKBOX;
        window_themes_widgets[WIDX_THEMES_DUPLICATE_BUTTON].type = WWT_EMPTY;
        window_themes_widgets[WIDX_THEMES_DELETE_BUTTON].type = WWT_EMPTY;
        window_themes_widgets[WIDX_THEMES_RENAME_BUTTON].type = WWT_EMPTY;
        window_themes_widgets[WIDX_THEMES_PRESETS].type = WWT_EMPTY;
        window_themes_widgets[WIDX_THEMES_PRESETS_DROPDOWN].type = WWT_EMPTY;
        window_themes_widgets[WIDX_THEMES_COLOURBTN_MASK].type = WWT_EMPTY;

        widget_set_checkbox_value(w, WIDX_THEMES_RCT1_RIDE_LIGHTS, theme_get_flags() & UITHEME_FLAG_USE_LIGHTS_RIDE);
        widget_set_checkbox_value(w, WIDX_THEMES_RCT1_PARK_LIGHTS, theme_get_flags() & UITHEME_FLAG_USE_LIGHTS_PARK);
        widget_set_checkbox_value(
            w, WIDX_THEMES_RCT1_SCENARIO_FONT, theme_get_flags() & UITHEME_FLAG_USE_ALTERNATIVE_SCENARIO_SELECT_FONT);
        widget_set_checkbox_value(w, WIDX_THEMES_RCT1_BOTTOM_TOOLBAR, theme_get_flags() & UITHEME_FLAG_USE_FULL_BOTTOM_TOOLBAR);
    }
    else
    {
        window_themes_widgets[WIDX_THEMES_HEADER_WINDOW].type = WWT_TABLE_HEADER;
        window_themes_widgets[WIDX_THEMES_HEADER_PALETTE].type = WWT_TABLE_HEADER;
        window_themes_widgets[WIDX_THEMES_LIST].type = WWT_SCROLL;
        window_themes_widgets[WIDX_THEMES_RCT1_RIDE_LIGHTS].type = WWT_EMPTY;
        window_themes_widgets[WIDX_THEMES_RCT1_PARK_LIGHTS].type = WWT_EMPTY;
        window_themes_widgets[WIDX_THEMES_RCT1_SCENARIO_FONT].type = WWT_EMPTY;
        window_themes_widgets[WIDX_THEMES_RCT1_BOTTOM_TOOLBAR].type = WWT_EMPTY;
        window_themes_widgets[WIDX_THEMES_DUPLICATE_BUTTON].type = WWT_EMPTY;
        window_themes_widgets[WIDX_THEMES_DELETE_BUTTON].type = WWT_EMPTY;
        window_themes_widgets[WIDX_THEMES_RENAME_BUTTON].type = WWT_EMPTY;
        window_themes_widgets[WIDX_THEMES_PRESETS].type = WWT_EMPTY;
        window_themes_widgets[WIDX_THEMES_PRESETS_DROPDOWN].type = WWT_EMPTY;
        window_themes_widgets[WIDX_THEMES_COLOURBTN_MASK].type = WWT_EMPTY;
    }
}

void window_themes_paint(rct_window* w, rct_drawpixelinfo* dpi)
{
    // Widgets
    window_draw_widgets(w, dpi);
    window_themes_draw_tab_images(dpi, w);

    if (_selected_tab == WINDOW_THEMES_TAB_SETTINGS)
    {
        size_t activeAvailableThemeIndex = theme_manager_get_active_available_theme_index();
        const utf8* activeThemeName = theme_manager_get_available_theme_name(activeAvailableThemeIndex);
        Formatter::Common().Add<const utf8*>(activeThemeName);
        gfx_draw_string_left(
            dpi, STR_THEMES_LABEL_CURRENT_THEME, nullptr, w->colours[1],
            w->windowPos + ScreenCoordsXY{ 10, window_themes_widgets[WIDX_THEMES_PRESETS].top + 1 });
        gfx_draw_string_left_clipped(
            dpi, STR_STRING, gCommonFormatArgs, w->colours[1],
            w->windowPos
                + ScreenCoordsXY{ window_themes_widgets[WIDX_THEMES_PRESETS].left + 1,
                                  window_themes_widgets[WIDX_THEMES_PRESETS].top },
            w->windowPos.x + window_themes_widgets[WIDX_THEMES_PRESETS_DROPDOWN].left
                - window_themes_widgets[WIDX_THEMES_PRESETS].left - 4);
    }
}

/**
 *
 *  rct2: 0x006BD785
 */
void window_themes_scrollpaint(rct_window* w, rct_drawpixelinfo* dpi, int32_t scrollIndex)
{
    ScreenCoordsXY screenCoords;

    if (_selected_tab == WINDOW_THEMES_TAB_SETTINGS || _selected_tab == WINDOW_THEMES_TAB_FEATURES)
        return;

    if ((w->colours[1] & 0x80) == 0)
        // gfx_fill_rect(dpi, dpi->x, dpi->y, dpi->x + dpi->width - 1, dpi->y + dpi->height - 1,
        // ColourMapA[w->colours[1]].mid_light);
        gfx_clear(dpi, ColourMapA[w->colours[1]].mid_light);
    screenCoords.y = 0;
    for (int32_t i = 0; i < get_colour_scheme_tab_count(); i++)
    {
        if (screenCoords.y > dpi->y + dpi->height)
        {
            break;
        }
        if (screenCoords.y + _row_height >= dpi->y)
        {
            if (i + 1 < get_colour_scheme_tab_count())
            {
                int32_t colour = w->colours[1];
                if (colour & COLOUR_FLAG_TRANSLUCENT)
                {
                    translucent_window_palette windowPalette = TranslucentWindowPalettes[BASE_COLOUR(colour)];

                    gfx_filter_rect(
                        dpi, 0, screenCoords.y + _row_height - 2, window_themes_widgets[WIDX_THEMES_LIST].right,
                        screenCoords.y + _row_height - 2, windowPalette.highlight);
                    gfx_filter_rect(
                        dpi, 0, screenCoords.y + _row_height - 1, window_themes_widgets[WIDX_THEMES_LIST].right,
                        screenCoords.y + _row_height - 1, windowPalette.shadow);
                }
                else
                {
                    colour = ColourMapA[w->colours[1]].mid_dark;
                    gfx_fill_rect(
                        dpi,
                        { { 0, screenCoords.y + _row_height - 2 },
                          { window_themes_widgets[WIDX_THEMES_LIST].right, screenCoords.y + _row_height - 2 } },
                        colour);
                    colour = ColourMapA[w->colours[1]].lightest;
                    gfx_fill_rect(
                        dpi,
                        { { 0, screenCoords.y + _row_height - 1 },
                          { window_themes_widgets[WIDX_THEMES_LIST].right, screenCoords.y + _row_height - 1 } },
                        colour);
                }
            }

            rct_windowclass wc = get_window_class_tab_index(i);
            int32_t numColours = theme_desc_get_num_colours(wc);
            for (uint8_t j = 0; j < numColours; j++)
            {
                gfx_draw_string_left(dpi, theme_desc_get_name(wc), nullptr, w->colours[1], { 2, screenCoords.y + 4 });

                uint8_t colour = theme_get_colour(wc, j);
                uint32_t image = SPRITE_ID_PALETTE_COLOUR_1(colour & ~COLOUR_FLAG_TRANSLUCENT) | SPR_PALETTE_BTN;
                if (i == _colour_index_1 && j == _colour_index_2)
                {
                    image = SPRITE_ID_PALETTE_COLOUR_1(colour & ~COLOUR_FLAG_TRANSLUCENT) | SPR_PALETTE_BTN_PRESSED;
                }
                gfx_draw_sprite(dpi, image, { _button_offset_x + 12 * j, screenCoords.y + _button_offset_y }, 0);

                gfx_fill_rect_inset(
                    dpi, _button_offset_x + 12 * j, screenCoords.y + _check_offset_y, _button_offset_x + 12 * j + 9,
                    screenCoords.y + _check_offset_y + 10, w->colours[1], INSET_RECT_F_E0);
                if (colour & COLOUR_FLAG_TRANSLUCENT)
                {
                    gCurrentFontSpriteBase = FONT_SPRITE_BASE_MEDIUM_DARK;
                    gfx_draw_string(
                        dpi, static_cast<const char*>(CheckBoxMarkString), w->colours[1] & 0x7F,
                        { _button_offset_x + 12 * j, screenCoords.y + _check_offset_y });
                }
            }
        }

        screenCoords.y += _row_height;
    }
}
