#include "./tap.h"
#include "mtstate.h"
#include "guesture_manager.h"
#include <assert.h>

static void on_start(void *user_data,const struct Touch *touches,int touch_bit){
    GUESTURE_DEBUG("[tap]on_start\n");
}

static void on_update(void *user_data,const struct Touch *touches,int touch_bit){
    GUESTURE_DEBUG("[tap]on_update\n");
    struct tap_guesture_s *guesture = user_data;

    int count = 0;
    
    guesture->sum_x = 0;
    guesture->sum_y = 0;
    const struct Touch *ptouch;
    int i; foreach_bit(i,touch_bit){
        ptouch = touches + i;
        guesture->sum_x += ptouch->total_dx;
        guesture->sum_y += ptouch->total_dy;
        count++;
    }

    guesture->sum_x /= count;
    guesture->sum_y /= count;
            
    /** should have within a certain amount of time */
    int64_t diff_ms = time_diff_ms(&ptouch->update_time,&ptouch->create_time);
    uint64_t dist2 = math_dist2(guesture->sum_x,guesture->sum_y);
    if( diff_ms > TAP_TIME_MAX_HOLD_TIME){
        GUESTURE_DEBUG("[tap] diff_ms > TAP_TIME_MAX_HOLD_TIME\n");
       // not a tap
        guesture_set_match(&guesture->guesture,GUESTURE_MATCH_NO);
    }else if( dist2 > TAP_MOVE_DIST2){
        GUESTURE_DEBUG("[tap] dist2 > TAP_MOVE_DIST2\n");
        // not a tap
        guesture_set_match(&guesture->guesture,GUESTURE_MATCH_NO);
    }
    return ;
}

static bool on_end(void *user_data,bool is_cancel,int touch_count){
    GUESTURE_DEBUG("[tap]on_end\n");
    struct tap_guesture_s *guesture = user_data;
    struct timeval time;
 	gettimeofday(&time, NULL);
    
    GUESTURE_DEBUG("TAP %d Recognized\n",guesture->guesture.props.required_touches);
    guesture_set_match(&guesture->guesture,GUESTURE_MATCH_OK);
    int tap_button = -1;
    
    if( guesture->guesture.props.required_touches == 1 ){
        tap_button = TAP_1_BUTTON;
    }else if( guesture->guesture.props.required_touches == 2 ){
        tap_button = TAP_2_BUTTON;
    }else if( guesture->guesture.props.required_touches == 3 ){
        tap_button = TAP_3_BUTTON;
    }else if( guesture->guesture.props.required_touches == 4 ){
        tap_button = TAP_4_BUTTON;
    }else{
        tap_button = -1;
    }

    if( tap_button >= 0 ){
        /** it's a tap */
        guesture_post_button_down_and_up(&guesture->guesture,tap_button,TAP_UP_TIME);
    }
    return TRUE;
}

static struct guesture_callbacks_s s_callbacks = {
    .on_start   = on_start,
    .on_update  = on_update,
    .on_end     = on_end
};

void tap_guesture_init(struct tap_guesture_s *guesture,int touch_count){
    guesture_init(&guesture->guesture,"tap",&s_callbacks,guesture);
    sprintf(guesture->guesture.name+strlen(guesture->guesture.name),"%d",touch_count);
    guesture->guesture.props.required_touches = touch_count;
}