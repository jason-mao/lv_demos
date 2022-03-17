/**
 * @file lv_demo_music_main.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "lv_demo_music_main.h"
#if LV_USE_DEMO_MUSIC

#include "lv_demo_music_list.h"

/*********************
 *      DEFINES
 *********************/
#define ACTIVE_TRACK_CNT    3
#define INTRO_TIME          1500
#define BAR_COLOR1          lv_color_hex(0xe9dbfc)
#define BAR_COLOR2          lv_color_hex(0x6f8af6)
#define BAR_COLOR3          lv_color_hex(0xffffff)
#define BAR_COLOR1_STOP     160
#define BAR_COLOR2_STOP     200
#define BAR_COLOR3_STOP     (2 * LV_HOR_RES / 3)
#define BAR_CNT             20
#define DEG_STEP            (180/BAR_CNT)
#define BAND_CNT            4
#define BAR_PER_BAND_CNT    (BAR_CNT / BAND_CNT)


/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static lv_obj_t * create_cont(lv_obj_t * parent);
static void create_wave_images(lv_obj_t * parent);
static lv_obj_t * create_title_box(lv_obj_t * parent);
static lv_obj_t * create_icon_box(lv_obj_t * parent);
static lv_obj_t * create_ctrl_box(lv_obj_t * parent);
static lv_obj_t * create_handle(lv_obj_t * parent);


static void sdcard_event_click_cb(lv_event_t * e);
static void fm_event_click_cb(lv_event_t * e);
static void setting_event_click_cb(lv_event_t * e);
static void volume_dec_event_click_cb(lv_event_t * e);
static void volume_inc_event_click_cb(lv_event_t * e);

static void play_event_click_cb(lv_event_t * e);
static void prev_click_event_cb(lv_event_t * e);
static void next_click_event_cb(lv_event_t * e);
static void timer_cb(lv_timer_t * t);
static void track_load(uint32_t id);
static int32_t get_cos(int32_t deg, int32_t a);
static int32_t get_sin(int32_t deg, int32_t a);

/**********************
 *  STATIC VARIABLES
 **********************/
static lv_obj_t * main_cont;
static lv_obj_t * title_label;
static lv_obj_t * artist_label;
static lv_obj_t * genre_label;
static lv_obj_t * time_obj;
static lv_obj_t * slider_obj;
static lv_style_t style_icon;
static uint32_t spectrum_i = 0;
static uint32_t spectrum_i_pause = 0;
static uint32_t bar_ofs = 0;
static uint32_t spectrum_lane_ofs_start = 0;
static uint32_t bar_rot = 0;
static uint32_t time;
static lv_timer_t *  sec_counter_timer;
static const lv_font_t * font_small;
static const lv_font_t * font_large;
static uint32_t track_id;
static bool playing;
static lv_obj_t * play_obj;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

lv_obj_t * _lv_demo_music_main_create(lv_obj_t * parent)
{
#if LV_DEMO_MUSIC_LARGE
    font_small = &lv_font_montserrat_22;
    font_large = &lv_font_montserrat_32;
#else
    font_small = &lv_font_montserrat_12;
    font_large = &lv_font_montserrat_16;
#endif

    /*Create the content of the music player*/
    lv_obj_t * cont = create_cont(parent);
    create_wave_images(cont);
    lv_obj_t * icon_box = create_icon_box(cont);
    lv_obj_t * title_box = create_title_box(cont);
    lv_obj_t * ctrl_box = create_ctrl_box(cont);
    lv_obj_t * handle_box = create_handle(cont);

    /*Arrange the content into a grid*/
#if LV_DEMO_MUSIC_LANDSCAPE == 0
    static const lv_coord_t grid_cols[] = {LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
    static const lv_coord_t grid_rows[] = {LV_DEMO_MUSIC_HANDLE_SIZE,     /*Spacing*/
                                           LV_GRID_FR(1),   /*Spacer*/
                                           LV_GRID_CONTENT, /*Title box*/
                                           LV_GRID_FR(1),   /*Spacer*/
                                           LV_GRID_CONTENT, /*Icon box*/
                                           LV_GRID_FR(1),   /*Spacer*/
                                           LV_GRID_CONTENT, /*Control box*/
                                           LV_GRID_FR(1),   /*Spacer*/
                                           LV_GRID_CONTENT, /*Handle box*/
                                           LV_GRID_FR(1),   /*Spacer*/
                                           LV_DEMO_MUSIC_HANDLE_SIZE,     /*Spacing*/
                                           LV_GRID_TEMPLATE_LAST};

    lv_obj_set_grid_dsc_array(cont, grid_cols, grid_rows);
    lv_obj_set_style_grid_row_align(cont, LV_GRID_ALIGN_SPACE_BETWEEN, 0);

    lv_obj_set_grid_cell(icon_box, LV_GRID_ALIGN_STRETCH, 0, 1, LV_ALIGN_CENTER, 2, 1);
    lv_obj_set_grid_cell(title_box, LV_GRID_ALIGN_STRETCH, 0, 1, LV_ALIGN_CENTER, 4, 1);
    lv_obj_set_grid_cell(ctrl_box, LV_GRID_ALIGN_STRETCH, 0, 1, LV_ALIGN_CENTER, 6, 1);
    lv_obj_set_grid_cell(handle_box, LV_GRID_ALIGN_STRETCH, 0, 1, LV_ALIGN_CENTER, 8, 1);
#else
    /*Arrange the content into a grid*/
    static const lv_coord_t grid_cols[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
    static const lv_coord_t grid_rows[] = {LV_DEMO_MUSIC_HANDLE_SIZE,     /*Spacing*/
                                           LV_GRID_FR(1),   /*Spacer*/
                                           LV_GRID_CONTENT, /*Title box*/
                                           LV_GRID_FR(1),   /*Spacer*/
                                           LV_GRID_CONTENT, /*Icon box*/
                                           LV_GRID_FR(3),   /*Spacer*/
                                           LV_GRID_CONTENT, /*Control box*/
                                           LV_GRID_FR(1),   /*Spacer*/
                                           LV_GRID_CONTENT, /*Handle box*/
                                           LV_GRID_FR(1),   /*Spacer*/
                                           LV_DEMO_MUSIC_HANDLE_SIZE,     /*Spacing*/
                                           LV_GRID_TEMPLATE_LAST};

    lv_obj_set_grid_dsc_array(cont, grid_cols, grid_rows);
    lv_obj_set_style_grid_row_align(cont, LV_GRID_ALIGN_SPACE_BETWEEN, 0);
    lv_obj_set_grid_cell(title_box, LV_GRID_ALIGN_STRETCH, 0, 1, LV_ALIGN_CENTER, 2, 1);
    lv_obj_set_grid_cell(icon_box, LV_GRID_ALIGN_STRETCH, 0, 1, LV_ALIGN_CENTER, 4, 1);
    lv_obj_set_grid_cell(ctrl_box, LV_GRID_ALIGN_STRETCH, 0, 1, LV_ALIGN_CENTER, 6, 1);
    lv_obj_set_grid_cell(handle_box, LV_GRID_ALIGN_STRETCH, 0, 2, LV_ALIGN_CENTER, 8, 1);
    lv_obj_set_grid_cell(spectrum_obj, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_CENTER, 1, 9);
#endif
    sec_counter_timer = lv_timer_create(timer_cb, 1000, NULL);
    lv_timer_pause(sec_counter_timer);
#if 1
    lv_obj_fade_in(title_box, 1000, INTRO_TIME + 300);
    lv_obj_fade_in(icon_box, 1000, INTRO_TIME + 300);
    lv_obj_fade_in(ctrl_box, 1000, INTRO_TIME + 300);
    lv_obj_fade_in(handle_box, 1000, INTRO_TIME + 300);

    /* Create an intro from a logo + label */
    LV_IMG_DECLARE(img_lv_music_logo);
    lv_obj_t * logo = lv_img_create(lv_scr_act());
    lv_img_set_src(logo, &img_lv_music_logo);
    lv_obj_move_foreground(logo);
    lv_obj_fade_out(logo, 300, INTRO_TIME);

    lv_obj_t * title = lv_label_create(lv_scr_act());
    lv_label_set_text(title, "Espressif Music player");
    lv_obj_set_style_text_align(title, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_font(title, font_large, 0);
    lv_obj_set_style_text_color(title, lv_color_hex(0xFF3034), 0);
    lv_obj_set_style_text_line_space(title, 8, 0);
    lv_obj_fade_out(title, 300, INTRO_TIME);
    lv_obj_align_to(logo, NULL, LV_ALIGN_CENTER, 0, -30);
    lv_obj_align_to(title, logo, LV_ALIGN_OUT_BOTTOM_MID, 0, 20);

#endif
    lv_obj_update_layout(main_cont);

    return main_cont;
}

void _lv_demo_music_album_next(bool next)
{
    uint32_t id = track_id;
    if(next) {
        id++;
        if(id >= ACTIVE_TRACK_CNT) id = 0;
    } else {
        if(id == 0) {
            id = ACTIVE_TRACK_CNT - 1;
        } else {
            id--;
        }
    }

    if(playing) {
        _lv_demo_music_play(id);
    } else {
        track_load(id);
    }
}

void _lv_demo_music_play(uint32_t id)
{
    track_load(id);

    _lv_demo_music_resume();
}

void _lv_demo_music_resume(void)
{
    playing = true;
    LV_LOG_USER("star resume");
    lv_timer_resume(sec_counter_timer);
    lv_slider_set_range(slider_obj, 0, _lv_demo_music_get_track_length(track_id));
    lv_obj_add_state(play_obj, LV_STATE_CHECKED);
}

void _lv_demo_music_pause(void)
{
    playing = false;
    spectrum_i_pause = spectrum_i;
    spectrum_i = 0;
    lv_timer_pause(sec_counter_timer);
    lv_obj_clear_state(play_obj, LV_STATE_CHECKED);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static lv_obj_t * create_cont(lv_obj_t * parent)
{
    /*A transparent container in which the player section will be scrolled*/
    main_cont = lv_obj_create(parent);
    lv_obj_clear_flag(main_cont, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(main_cont, LV_OBJ_FLAG_SCROLL_ELASTIC);
    lv_obj_remove_style_all(main_cont);                            /*Make it transparent*/
    lv_obj_set_size(main_cont, lv_pct(100), lv_pct(100));
    lv_obj_set_scroll_snap_y(main_cont, LV_SCROLL_SNAP_CENTER);    /*Snap the children to the center*/

    /*Create a container for the player*/
    lv_obj_t * player = lv_obj_create(main_cont);
    lv_obj_set_y(player, - LV_DEMO_MUSIC_HANDLE_SIZE);
    lv_obj_set_size(player, LV_HOR_RES, LV_VER_RES + LV_DEMO_MUSIC_HANDLE_SIZE * 2);

    lv_obj_set_style_bg_color(player, lv_color_hex(0xffffff), 0);
    lv_obj_set_style_border_width(player, 0, 0);
    lv_obj_set_style_pad_all(player, 0, 0);
    lv_obj_set_scroll_dir(player, LV_DIR_VER);

    /* A transparent placeholder below the player container
     * It is used only to snap it to center.*/
    lv_obj_t * placeholder = lv_obj_create(main_cont);
    lv_obj_remove_style_all(placeholder);
    lv_obj_set_size(placeholder, lv_pct(100), LV_VER_RES -2 *  LV_DEMO_MUSIC_HANDLE_SIZE);
    lv_obj_set_y(placeholder, LV_VER_RES + LV_DEMO_MUSIC_HANDLE_SIZE);
    lv_obj_clear_flag(placeholder, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_update_layout(main_cont);

    return player;
}

static void create_wave_images(lv_obj_t * parent)
{
    LV_IMG_DECLARE(img_lv_demo_music_wave_top);
    LV_IMG_DECLARE(img_lv_demo_music_wave_bottom);
    lv_obj_t * wave_top = lv_img_create(parent);
    lv_img_set_src(wave_top,&img_lv_demo_music_wave_top);
    lv_obj_set_width(wave_top, LV_HOR_RES);
    lv_obj_align(wave_top, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_add_flag(wave_top, LV_OBJ_FLAG_IGNORE_LAYOUT);

    lv_obj_t * wave_bottom = lv_img_create(parent);
    lv_img_set_src(wave_bottom,&img_lv_demo_music_wave_bottom);
    lv_obj_set_width(wave_bottom, LV_HOR_RES);
    lv_obj_align(wave_bottom, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_add_flag(wave_bottom, LV_OBJ_FLAG_IGNORE_LAYOUT);

    LV_IMG_DECLARE(img_lv_demo_music_corner_left);
    LV_IMG_DECLARE(img_lv_demo_music_corner_right);
    lv_obj_t * wave_corner = lv_img_create(parent);
    lv_img_set_src(wave_corner, &img_lv_demo_music_corner_left);
    lv_obj_align(wave_corner, LV_ALIGN_BOTTOM_LEFT, 0, 0);
    lv_obj_add_flag(wave_corner, LV_OBJ_FLAG_IGNORE_LAYOUT);

    wave_corner = lv_img_create(parent);
    lv_img_set_src(wave_corner, &img_lv_demo_music_corner_right);
    lv_obj_align(wave_corner, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
    lv_obj_add_flag(wave_corner, LV_OBJ_FLAG_IGNORE_LAYOUT);
}

static lv_obj_t * create_title_box(lv_obj_t * parent)
{

    /*Create the titles*/
    lv_obj_t * cont = lv_obj_create(parent);
    lv_obj_remove_style_all(cont);
    lv_obj_set_height(cont, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    title_label = lv_label_create(cont);
    lv_obj_set_style_text_font(title_label, font_large, 0);
    lv_obj_set_style_text_color(title_label, lv_color_hex(0x504d6d), 0);
    lv_label_set_text(title_label, _lv_demo_music_get_title(track_id));
    lv_obj_set_height(title_label, lv_font_get_line_height(font_large) * 3 / 2);

    artist_label = lv_label_create(cont);
    lv_obj_set_style_text_font(artist_label, font_small, 0);
    lv_obj_set_style_text_color(artist_label, lv_color_hex(0x504d6d), 0);
    lv_label_set_text(artist_label, _lv_demo_music_get_artist(track_id));

    genre_label = lv_label_create(cont);
    lv_obj_set_style_text_font(genre_label, font_small, 0);
    lv_obj_set_style_text_color(genre_label, lv_color_hex(0x8a86b8), 0);
    lv_label_set_text(genre_label, _lv_demo_music_get_genre(track_id));

    return cont;
}

static lv_obj_t * create_icon_box(lv_obj_t * parent)
{
    lv_obj_t * cont = lv_obj_create(parent);
    lv_obj_remove_style_all(cont);
    lv_obj_set_height(cont, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    //LV_SYMBOL_SD_CARD
    lv_obj_t * sdcard_icon = lv_label_create(cont);
    lv_obj_add_style(sdcard_icon, &style_icon, 0);
    lv_label_set_text(sdcard_icon, LV_SYMBOL_SD_CARD);
    lv_obj_add_event_cb(sdcard_icon, sdcard_event_click_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_add_flag(sdcard_icon, LV_OBJ_FLAG_CLICKABLE);

    LV_IMG_DECLARE(img_lv_music_fm_icon);
    lv_obj_t* fm_icon = lv_img_create(cont);
    lv_img_set_src(fm_icon, &img_lv_music_fm_icon);
    lv_obj_add_event_cb(fm_icon, fm_event_click_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_add_flag(fm_icon, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_t * setting_icn = lv_label_create(cont);
    lv_obj_add_style(setting_icn, &style_icon, 0);
    lv_label_set_text(setting_icn, LV_SYMBOL_SETTINGS);
    lv_obj_add_event_cb(setting_icn, setting_event_click_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_add_flag(setting_icn, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_t * volume_dec_icn = lv_label_create(cont);
    lv_obj_add_style(volume_dec_icn, &style_icon, 0);
    lv_label_set_text(volume_dec_icn, LV_SYMBOL_VOLUME_MID);
    lv_obj_add_event_cb(volume_dec_icn, volume_dec_event_click_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_add_flag(volume_dec_icn, LV_OBJ_FLAG_CLICKABLE);


    lv_obj_t * volume_inc_icn = lv_label_create(cont);
    lv_obj_add_style(volume_inc_icn, &style_icon, 0);
    lv_label_set_text(volume_inc_icn, LV_SYMBOL_VOLUME_MAX);
    lv_obj_add_event_cb(volume_inc_icn, volume_inc_event_click_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_add_flag(volume_inc_icn, LV_OBJ_FLAG_CLICKABLE);

    return cont;
}

static lv_obj_t * create_ctrl_box(lv_obj_t * parent)
{
    /*Create the control box*/
    lv_obj_t * cont = lv_obj_create(parent);
    lv_obj_remove_style_all(cont);
    lv_obj_set_height(cont, LV_SIZE_CONTENT);
#if LV_DEMO_MUSIC_LARGE
    lv_obj_set_style_pad_bottom(cont, 17, 0);
#else
    lv_obj_set_style_pad_bottom(cont, 8, 0);
#endif
    static const lv_coord_t grid_col[] = {LV_GRID_FR(2), LV_GRID_FR(3),LV_GRID_FR(5), LV_GRID_FR(5), LV_GRID_FR(5), LV_GRID_FR(3), LV_GRID_FR(2), LV_GRID_TEMPLATE_LAST};
    static const lv_coord_t grid_row[] = {LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};
    lv_obj_set_grid_dsc_array(cont, grid_col, grid_row);

    LV_IMG_DECLARE(img_lv_demo_music_btn_loop);
    LV_IMG_DECLARE(img_lv_demo_music_btn_rnd);
    LV_IMG_DECLARE(img_lv_demo_music_btn_next);
    LV_IMG_DECLARE(img_lv_demo_music_btn_prev);
    LV_IMG_DECLARE(img_lv_demo_music_btn_play);
    LV_IMG_DECLARE(img_lv_demo_music_btn_pause);

    lv_obj_t * icon;
    icon = lv_img_create(cont);
    lv_img_set_src(icon, &img_lv_demo_music_btn_rnd);
    lv_obj_set_grid_cell(icon, LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_CENTER, 0, 1);

    icon = lv_img_create(cont);
    lv_img_set_src(icon, &img_lv_demo_music_btn_loop);
    lv_obj_set_grid_cell(icon, LV_GRID_ALIGN_END, 5, 1, LV_GRID_ALIGN_CENTER, 0, 1);

    icon = lv_img_create(cont);
    lv_img_set_src(icon, &img_lv_demo_music_btn_prev);
    lv_obj_set_grid_cell(icon, LV_GRID_ALIGN_CENTER, 2, 1, LV_GRID_ALIGN_CENTER, 0, 1);
    lv_obj_add_event_cb(icon, prev_click_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_add_flag(icon, LV_OBJ_FLAG_CLICKABLE);

    play_obj = lv_imgbtn_create(cont);
    lv_imgbtn_set_src(play_obj, LV_IMGBTN_STATE_RELEASED, NULL, &img_lv_demo_music_btn_play, NULL);
    lv_imgbtn_set_src(play_obj, LV_IMGBTN_STATE_CHECKED_RELEASED, NULL, &img_lv_demo_music_btn_pause, NULL);
    lv_obj_add_flag(play_obj, LV_OBJ_FLAG_CHECKABLE);
    lv_obj_set_grid_cell(play_obj, LV_GRID_ALIGN_CENTER, 3, 1, LV_GRID_ALIGN_CENTER, 0, 1);

    lv_obj_add_event_cb(play_obj, play_event_click_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_add_flag(play_obj, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_width(play_obj, img_lv_demo_music_btn_play.header.w);

    icon = lv_img_create(cont);
    lv_img_set_src(icon, &img_lv_demo_music_btn_next);
    lv_obj_set_grid_cell(icon, LV_GRID_ALIGN_CENTER, 4, 1, LV_GRID_ALIGN_CENTER, 0, 1);
    lv_obj_add_event_cb(icon, next_click_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_add_flag(icon, LV_OBJ_FLAG_CLICKABLE);

    LV_IMG_DECLARE(img_lv_demo_music_slider_knob);
    slider_obj = lv_slider_create(cont);
    lv_obj_set_style_anim_time(slider_obj, 100, 0);
    lv_obj_add_flag(slider_obj, LV_OBJ_FLAG_CLICKABLE); /*No input from the slider*/

#if LV_DEMO_MUSIC_LARGE == 0
    lv_obj_set_height(slider_obj, 3);
#else
    lv_obj_set_height(slider_obj, 6);
#endif
    lv_obj_set_grid_cell(slider_obj, LV_GRID_ALIGN_STRETCH, 1, 4, LV_GRID_ALIGN_CENTER, 1, 1);

    lv_obj_set_style_bg_img_src(slider_obj, &img_lv_demo_music_slider_knob, LV_PART_KNOB);
    lv_obj_set_style_bg_opa(slider_obj, LV_OPA_TRANSP, LV_PART_KNOB);
    lv_obj_set_style_pad_all(slider_obj, 20, LV_PART_KNOB);
    lv_obj_set_style_bg_grad_dir(slider_obj, LV_GRAD_DIR_HOR, LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(slider_obj, lv_color_hex(0x569af8), LV_PART_INDICATOR);
    lv_obj_set_style_bg_grad_color(slider_obj, lv_color_hex(0xa666f1), LV_PART_INDICATOR);
    lv_obj_set_style_outline_width(slider_obj, 0, 0);

    time_obj = lv_label_create(cont);
    lv_obj_set_style_text_font(time_obj, font_small, 0);
    lv_obj_set_style_text_color(time_obj, lv_color_hex(0x8a86b8), 0);
    lv_label_set_text(time_obj, "0:00");
    lv_obj_set_grid_cell(time_obj, LV_GRID_ALIGN_END, 5, 1, LV_GRID_ALIGN_CENTER, 1, 1);

    return cont;
}

static lv_obj_t * create_handle(lv_obj_t * parent)
{
    lv_obj_t * cont = lv_obj_create(parent);
    lv_obj_remove_style_all(cont);

    lv_obj_set_size(cont, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(cont, 8, 0);

    /*A handle to scroll to the track list*/
    lv_obj_t * handle_label = lv_label_create(cont);
    lv_label_set_text(handle_label, "ALL TRACKS");
    lv_obj_set_style_text_font(handle_label, font_small, 0);
    lv_obj_set_style_text_color(handle_label, lv_color_hex(0x8a86b8), 0);

    lv_obj_t * handle_rect = lv_obj_create(cont);
#if LV_DEMO_MUSIC_LARGE
    lv_obj_set_size(handle_rect, 40, 3);
#else
    lv_obj_set_size(handle_rect, 20, 2);
#endif

    lv_obj_set_style_bg_color(handle_rect, lv_color_hex(0x8a86b8), 0);
    lv_obj_set_style_border_width(handle_rect, 0, 0);

    return cont;
}

static void track_load(uint32_t id)
{
    LV_LOG_ERROR("track_load----->>>>>");
    spectrum_i = 0;
    time = 0;
    spectrum_i_pause = 0;
    lv_slider_set_value(slider_obj, 0, LV_ANIM_OFF);
    lv_label_set_text(time_obj, "0:00");

    if(id == track_id) return;
    bool next = false;
    if((track_id + 1) % ACTIVE_TRACK_CNT == id) next = true;

    _lv_demo_music_list_btn_check(track_id, false);

    track_id = id;

    _lv_demo_music_list_btn_check(id, true);

    lv_label_set_text(title_label, _lv_demo_music_get_title(track_id));
    lv_label_set_text(artist_label, _lv_demo_music_get_artist(track_id));
    lv_label_set_text(genre_label, _lv_demo_music_get_genre(track_id));

    // lv_obj_fade_out(album_img_obj, 500, 0);
}

static void sdcard_event_click_cb(lv_event_t * e)
{
    LV_LOG_USER("IS here");
}

static void fm_event_click_cb(lv_event_t * e)
{
    LV_LOG_USER("IS here");
}

static void setting_event_click_cb(lv_event_t * e)
{
    LV_LOG_USER("IS here");
}

static void volume_dec_event_click_cb(lv_event_t * e)
{
    LV_LOG_USER("IS here");
}

static void volume_inc_event_click_cb(lv_event_t * e)
{
    LV_LOG_USER("IS here");
}

static void play_event_click_cb(lv_event_t * e)
{
    lv_obj_t * obj = lv_event_get_target(e);
    if(lv_obj_has_state(obj, LV_STATE_CHECKED)) {
        _lv_demo_music_resume();
    } else {
        _lv_demo_music_pause();
    }
}

static void prev_click_event_cb(lv_event_t * e)
{
    LV_LOG_USER("prev_click_event_cb prev");
}

static void next_click_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    LV_LOG_USER("next_click_event_cb next");
    if(code == LV_EVENT_CLICKED) {
    }
}

static void timer_cb(lv_timer_t * t)
{
    time++;
    lv_label_set_text_fmt(time_obj, "%d:%02d", time / 60, time % 60);
    lv_slider_set_value(slider_obj, time, LV_ANIM_ON);
}



#if 0
static void slider_event_cb(lv_event_t * e);
static lv_obj_t * slider_label;

/**
 * A default slider with a label displaying the current value
 */
void lv_example_slider_12(void)
{
    /*Create a slider in the center of the display*/
    lv_obj_t * slider = lv_slider_create(lv_scr_act());
    lv_obj_center(slider);
    lv_obj_add_event_cb(slider, slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    /*Create a label below the slider*/
    slider_label = lv_label_create(lv_scr_act());
    lv_label_set_text(slider_label, "0%");

    lv_obj_align_to(slider_label, slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
}

static void slider_event_cb(lv_event_t * e)
{
    lv_obj_t * slider = lv_event_get_target(e);
    char buf[8];
    lv_snprintf(buf, sizeof(buf), "%d%%", (int)lv_slider_get_value(slider));
    lv_label_set_text(slider_label, buf);
    lv_obj_align_to(slider_label, slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
}
#endif

#endif /*LV_USE_DEMO_MUSIC*/

