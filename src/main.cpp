
#include "mbed.h"
#include "lvgl.h"

#include "Drivers/BSP/STM32F429I-Discovery/stm32f429i_discovery.h"
#include "Drivers/BSP/STM32F429I-Discovery/stm32f429i_discovery_lcd.h"
#include "Drivers/BSP/STM32F429I-Discovery/stm32f429i_discovery_ts.h"


#include "UI/ui.h"

#define WIDTH 240
#define HEIGHT 320

#define BUFF_SIZE (240 * 10)

#define LVGL_TICK 5                             //Time tick value for lvgl in ms (1-10msa)

TS_StateTypeDef TS_State; 

FileHandle *mbed::mbed_override_console(int)
{
    static BufferedSerial custom_target_console(USBTX, USBRX);
    return &custom_target_console;
}

Ticker ticker;                                  //Initialize your system tick                    
DigitalOut led(LED1);

void lv_ticker_func();
lv_disp_t* display_init(void);
void touchpad_init(void);
static void my_disp_flush_cb(lv_disp_drv_t* disp_drv, const lv_area_t* area, lv_color_t* color_p);
void touchpad_read(lv_indev_drv_t *indev, lv_indev_data_t *data);


static void btn_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * btn = lv_event_get_target(e);
    if(code == LV_EVENT_CLICKED) {
        debug("btn click!");
        static uint8_t cnt = 0;
        cnt++;

        /*Get the first child of the button which is the label and change its text*/
        lv_obj_t * label = lv_obj_get_child(btn, 0);
        lv_label_set_text_fmt(label, "Button: %d", cnt);
    }
}

void lv_example_get_started_1(void)
{
    lv_obj_t * btn = lv_btn_create(lv_scr_act());     /*Add a button the current screen*/
    lv_obj_set_pos(btn, 10, 10);                            /*Set its position*/
    lv_obj_set_size(btn, 120, 50);                          /*Set its size*/
    lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_ALL, NULL);           /*Assign a callback to the button*/

    lv_obj_t * label = lv_label_create(btn);          /*Add a label to the button*/
    lv_label_set_text(label, "Button");                     /*Set the labels text*/
    lv_obj_center(label);
}

uint8_t my_prinnt_line = 0;

void my_print(const char * buf)
{
    debug(buf);
}

int main()
{
    display_init();
    touchpad_init();

    lv_log_register_print_cb( my_print );

    debug("LittlevGL DEMO");
    // lv_example_get_started_1();
    
    ui_init();

    while (true) {
        lv_ticker_func();
        thread_sleep_for(10);
    }
}


void lv_ticker_func() {
    lv_tick_inc(LVGL_TICK); 
    //Call lv_tick_inc(x) every x milliseconds in a Timer or Task (x should be between 1 and 10). 
    //It is required for the internal timing of LittlevGL.
    lv_task_handler(); 
    //Call lv_task_handler() periodically every few milliseconds. 
    //It will redraw the screen if required, handle input devices etc.
}

lv_disp_t* display_init(void){
    //Init the touch screen display via the BSP driver. Based on ST's example.
    BSP_LCD_Init();
    BSP_LCD_LayerDefaultInit(0, LCD_FRAME_BUFFER); // LCD_FB_START_ADDRESS
    BSP_LCD_DisplayOn();
    BSP_LCD_SelectLayer(0);
    BSP_LCD_Clear(LCD_COLOR_TRANSPARENT);
    BSP_LCD_SetFont(&LCD_DEFAULT_FONT);
    BSP_LCD_SetBackColor(LCD_COLOR_WHITE);
    BSP_LCD_SetTextColor(LCD_COLOR_RED);
    
    lv_init();       
    
    static lv_disp_draw_buf_t disp_buf;

    /*Static or global buffer(s). The second buffer is optional*/
    static lv_color_t buf_1[BUFF_SIZE]; //TODO: Declare your own BUFF_SIZE appropriate to your system.
    static lv_color_t buf_2[BUFF_SIZE];

    lv_disp_draw_buf_init(&disp_buf, buf_1, buf_2, BUFF_SIZE);

    static lv_disp_drv_t disp_drv;          /*A variable to hold the drivers. Must be static or global.*/
    lv_disp_drv_init(&disp_drv);            /*Basic initialization*/
    disp_drv.draw_buf = &disp_buf;          /*Set an initialized buffer*/
    disp_drv.flush_cb = my_disp_flush_cb;        /*Set a flush callback to draw to the display*/
    disp_drv.hor_res = WIDTH;                 /*Set the horizontal resolution in pixels*/
    disp_drv.ver_res = HEIGHT;                 /*Set the vertical resolution in pixels*/

    return lv_disp_drv_register(&disp_drv); /*Register the driver and save the created display objects*/
}


void my_disp_flush_cb(lv_disp_drv_t* disp_drv, const lv_area_t* area, lv_color_t* color_p)
{
    //The most simple case (but also the slowest) to put all pixels to the screen one-by-one
    uint16_t x, y;
    for(y = area->y1; y <= area->y2; y++) {
        for(x = area->x1; x <= area->x2; x++) {
            //put_px(x, y, *color_p)
            BSP_LCD_DrawPixel(x, y, color_p->full);
            color_p++;
        }
    }
    
    //IMPORTANT!!!* Inform the graphics library that you are ready with the flushing
    lv_disp_flush_ready(disp_drv);
}


void touchpad_init(void){
    BSP_TS_Init(WIDTH, HEIGHT);

    lv_indev_drv_t indev_drv;                       //Descriptor of an input device driver
    lv_indev_drv_init(&indev_drv);                  //Basic initialization
    indev_drv.type = LV_INDEV_TYPE_POINTER;         //The touchpad is pointer type device
    indev_drv.read_cb = touchpad_read;              //Set the touchpad_read function
    lv_indev_drv_register(&indev_drv);              //Register touch driver in LvGL
}

void touchpad_read(lv_indev_drv_t *indev, lv_indev_data_t *data){
    // Read your touchpad
    // BSP_TS_GetState(&TS_State);
    debug("click!");
    // if(TS_State.TouchDetected) {
        // data->point.x = TS_State.X;
        // data->point.y = TS_State.Y;
        // data->state = LV_INDEV_STATE_PR;
    // } else {
        // data->point.x = 0;
        // data->point.y = 0;
        // data->state = LV_INDEV_STATE_REL;
    // }
    // return false;   //false: no more data to read because we are no buffering
}
