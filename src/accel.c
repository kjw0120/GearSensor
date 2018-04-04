#include "accel.h"
#include <sensor.h>

#define SENSITIVITY 5

typedef struct
{
	sensor_h sensor; /**< Sensor handle */
	sensor_listener_h sensor_listener;
} sensorinfo;

typedef struct appdata {
	Evas_Object *win;
	Evas_Object *conform;
	Evas_Object *box;
	Evas_Object *box2;
	Evas_Object *label0;
	Evas_Object *label1;
	Evas_Object *label2;
	Evas_Object *label3;
	Evas_Object *list;
	Evas_Object *list_difficulty;
	Evas_Object *text; //tap to move
	Evas_Object *text2; //bubble count

	Evas_Object *rect[25];
//	Evas_Object *list;
	sensorinfo sensor_info;
	float max_value[3];
	float prev_accel[3];
	//float speed[3];
	char pos[10];
	int position;
	int position_com;
	int sensor_interval;
	int bubble_status[25]; //0: initial, 1: yours, 2: com's
	int bubble_count;
	int bubble_count_you;
	int bubble_count_com;
	int game_mode; //1: single, 2: vs com
	int difficulty;
	int game_end;

	Ecore_Timer *timer;
	//int timer_count;
} appdata_s;



static void
win_delete_request_cb(void *data, Evas_Object *obj, void *event_info)
{
	ui_app_exit();
}

static void
win_back_cb(void *data, Evas_Object *obj, void *event_info)
{
	appdata_s *ad = data;
	/* Let window go to hide state. */
	elm_win_lower(ad->win);
}

static void show_is_supported(appdata_s *ad)
{
	char buf[PATH_MAX];
	bool is_supported = false;
	sensor_is_supported(SENSOR_ACCELEROMETER, &is_supported);
	sprintf(buf, "Acceleration Sensor is %s", is_supported ? "support" : "not support");
	elm_object_text_set(ad->label0, buf);
}

static void
_new_sensor_value(sensor_h sensor, sensor_event_s *sensor_data, void *user_data)
{
	float x = sensor_data->values[0];
	float y = sensor_data->values[1];
	float z = sensor_data->values[2];


	 appdata_s *ad = user_data;

	 float px = ad->prev_accel[0];
	 float py = ad->prev_accel[1];
//	 float pz = ad->prev_accel[2];

	 char buf[1024];
	 if (sensor_data->value_count < 3)
	 {
		 elm_object_text_set(ad->label2, "Gathering data...");
		 return;
	 }
	 snprintf(buf, sizeof(buf ), "<font_size = 10>X:%0.1f/Y:%0.1f/Z:%0.1f</font_size>", x, y, z);
	 elm_object_text_set(ad->label1, buf);
	 /*
	 for (int i = 0; i < 3; i++){
		 if(fabsf(sensor_data->values[i]) > fabsf(ad->max_value[i]))
				 ad->max_value[i] = sensor_data->values[i];
	 }
	 snprintf(buf, sizeof(buf ), "<font_size = 10>X:%0.1f/Y:%0.1f/Z:%0.1f</font_size>",
			 ad->max_value[0], ad->max_value[1], ad->max_value[2]);
	 elm_object_text_set(ad->label2, buf);
	 */

	 for(int i = 0; i < 25; i++){
		 if(ad->bubble_status[i] == 2 && i != ad->position)
			 evas_object_color_set(ad->rect[i], 0, 0, 100, 100);
	 }


	if (x >= SENSITIVITY && x * px > 0 && fabsf(y) < SENSITIVITY && fabsf(z) < SENSITIVITY && strlen(ad->pos) < 2){
		if(ad->position % 5 != 4){
			snprintf(buf, sizeof(buf ), "<font_size = 10>X:%0.1f/Y:%0.1f/Z:%0.1f</font_size>", x, y, z);
			elm_object_text_set(ad->label2, buf);
			sprintf(ad->pos, "Right");
			elm_object_text_set(ad->label3, "Right");
			evas_object_color_set(ad->rect[ad->position], 100, 0, 80, 100);
			evas_object_color_set(ad->rect[ad->position+1], 100, 0, 0, 100);

			ad->position = ad->position + 1;
			if(ad->bubble_status[ad->position] == 0){
				ad->bubble_status[ad->position] = 1;
				if(ad->game_mode == 1)
					ad->bubble_count++;
				else if(ad->game_mode == 2)
					ad->bubble_count_you++;

				char buf[20];
				if(ad->game_mode == 1)
					sprintf(buf, "Bubbles: %d/25", ad->bubble_count);
				else if(ad->game_mode == 2)
					sprintf(buf, "YOU: %d / COM: %d", ad->bubble_count_you, ad->bubble_count_com);
				evas_object_text_text_set(ad->text2, buf);
			}
			evas_object_show(ad->text);
			ad->sensor_interval = 1000;
		}
	}
	else if (x <= -SENSITIVITY && x * px > 0 && fabsf(y) < SENSITIVITY && fabsf(z) < SENSITIVITY && strlen(ad->pos) < 2){
		if(ad->position % 5 != 0){
			snprintf(buf, sizeof(buf ), "<font_size = 10>X:%0.1f/Y:%0.1f/Z:%0.1f</font_size>", x, y, z);
			elm_object_text_set(ad->label2, buf);
			sprintf(ad->pos, "Left");
			elm_object_text_set(ad->label3, "Left");
			evas_object_color_set(ad->rect[ad->position], 100, 0, 80, 100);
			evas_object_color_set(ad->rect[ad->position-1], 100, 0, 0, 100);
			ad->position = ad->position - 1;
			if(ad->bubble_status[ad->position] == 0){
				ad->bubble_status[ad->position] = 1;
				if(ad->game_mode == 1)
					ad->bubble_count++;
				else if(ad->game_mode == 2)
					ad->bubble_count_you++;
				char buf[20];
				if(ad->game_mode == 1)
					sprintf(buf, "Bubbles: %d/25", ad->bubble_count);
				else if(ad->game_mode == 2)
					sprintf(buf, "YOU: %d / COM: %d", ad->bubble_count_you, ad->bubble_count_com);
				evas_object_text_text_set(ad->text2, buf);
			}
			evas_object_show(ad->text);
			ad->sensor_interval = 1000;
		}
	}
	else if (y >= SENSITIVITY * 0.7 && y * py > 0 && fabsf(x) < SENSITIVITY && fabsf(z) < SENSITIVITY && strlen(ad->pos) < 2){
		if(ad->position / 5 != 0){
			snprintf(buf, sizeof(buf ), "<font_size = 10>X:%0.1f/Y:%0.1f/Z:%0.1f</font_size>", x, y, z);
			elm_object_text_set(ad->label2, buf);
			sprintf(ad->pos, "Up");
			elm_object_text_set(ad->label3, "Up");
			evas_object_color_set(ad->rect[ad->position], 100, 0, 80, 100);
			evas_object_color_set(ad->rect[ad->position-5], 100, 0, 0, 100);
			ad->position = ad->position - 5;
			if(ad->bubble_status[ad->position] == 0){
				ad->bubble_status[ad->position] = 1;
				if(ad->game_mode == 1)
					ad->bubble_count++;
				else if(ad->game_mode == 2)
					ad->bubble_count_you++;
				char buf[20];
				if(ad->game_mode == 1)
					sprintf(buf, "Bubbles: %d/25", ad->bubble_count);
				else if(ad->game_mode == 2)
					sprintf(buf, "YOU: %d / COM: %d", ad->bubble_count_you, ad->bubble_count_com);
				evas_object_text_text_set(ad->text2, buf);
			}
			evas_object_show(ad->text);
			ad->sensor_interval = 1000;
		}
	}
	else if (y <= -SENSITIVITY && y * py > 0 && fabsf(x) < SENSITIVITY && fabsf(z) < SENSITIVITY && strlen(ad->pos) < 2){
		if(ad->position / 5 != 4){
			snprintf(buf, sizeof(buf ), "<font_size = 10>X:%0.1f/Y:%0.1f/Z:%0.1f</font_size>", x, y, z);
			elm_object_text_set(ad->label2, buf);
			sprintf(ad->pos, "Down");
			elm_object_text_set(ad->label3, "Down");
			evas_object_color_set(ad->rect[ad->position], 100, 0, 80, 100);
			evas_object_color_set(ad->rect[ad->position+5], 100, 0, 0, 100);
			ad->position = ad->position + 5;
			if(ad->bubble_status[ad->position] == 0){
				ad->bubble_status[ad->position] = 1;
				if(ad->game_mode == 1)
					ad->bubble_count++;
				else if(ad->game_mode == 2)
					ad->bubble_count_you++;
				char buf[20];
				if(ad->game_mode == 1)
					sprintf(buf, "Bubbles: %d/25", ad->bubble_count);
				else if(ad->game_mode == 2)
					sprintf(buf, "YOU: %d / COM: %d", ad->bubble_count_you, ad->bubble_count_com);
				evas_object_text_text_set(ad->text2, buf);
			}
			evas_object_show(ad->text);
			ad->sensor_interval = 1000;
		}
	}
	else if(fabsf(z) >= 10 && fabsf(x) <= SENSITIVITY && fabsf(y) <= SENSITIVITY && strlen(ad->pos) < 2){
		snprintf(buf, sizeof(buf ), "<font_size = 10>X:%0.1f/Y:%0.1f/Z:%0.1f</font_size>", x, y, z);
		elm_object_text_set(ad->label2, buf);
		sprintf(ad->pos, "Jump");
		elm_object_text_set(ad->label3, "Jump");
		evas_object_color_set(ad->rect[ad->position], 100, 100, 0, 100);
		evas_object_show(ad->text);
		ad->sensor_interval = 1000;
	}

	if(ad->bubble_count == 25){
		evas_object_text_text_set(ad->text, "Clear!");
		//sensor_listener_stop(ad->sensor_info.sensor_listener);
		//ecore_timer_del(ad->timer);
		ad->game_end = 1;
	}
	if(ad->bubble_count_you >= 13){
		evas_object_text_text_set(ad->text, "You Win!");
		//sensor_listener_stop(ad->sensor_info.sensor_listener);
		ecore_timer_del(ad->timer);
		ad->game_end = 1;
	}
	if(ad->bubble_count_com >= 13){
		evas_object_text_text_set(ad->text, "You Lose!");
		//sensor_listener_stop(ad->sensor_info.sensor_listener);
		ecore_timer_del(ad->timer);
		ad->game_end = 1;
	}

	ad->prev_accel[0] = x;
	ad->prev_accel[1] = y;
	ad->prev_accel[2] = z;
}

static void
start_acceleration_sensor(appdata_s *ad)
{
	sensor_error_e err = SENSOR_ERROR_NONE;
	//err = sensor_get_default_sensor(SENSOR_ACCELEROMETER, &ad->sensor_info.sensor);
	err = sensor_get_default_sensor(SENSOR_LINEAR_ACCELERATION, &ad->sensor_info.sensor);
	if (err != SENSOR_ERROR_NONE)
	goto error_check;
	err = sensor_create_listener(ad->sensor_info.sensor, &ad->sensor_info.sensor_listener);
	if (err != SENSOR_ERROR_NONE)
	goto error_check;
	sensor_listener_set_event_cb(ad->sensor_info.sensor_listener, ad->sensor_interval, _new_sensor_value, ad); //INTERVAL
	sensor_listener_start(ad->sensor_info.sensor_listener);
	error_check:
	if (err != SENSOR_ERROR_NONE)
	{
		const char *msg;
		char fullmsg[1024];
		switch (err)
		{
			case SENSOR_ERROR_IO_ERROR: msg = "I/O error"; break;
			case SENSOR_ERROR_INVALID_PARAMETER: msg = "Invalid parameter"; break;
			case SENSOR_ERROR_NOT_SUPPORTED: msg = "The sensor type is not supported in the current device"; break;
			case SENSOR_ERROR_PERMISSION_DENIED: msg = "Permission denied"; break;
			case SENSOR_ERROR_OUT_OF_MEMORY: msg = "Out of memory"; break;
			case SENSOR_ERROR_NOT_NEED_CALIBRATION: msg = "Sensor doesn't need calibration"; break;
			case SENSOR_ERROR_OPERATION_FAILED: msg = "Operation failed"; break;
			default: msg = "Unknown error"; break;
		}
		snprintf(fullmsg, sizeof(fullmsg), "<align=center>An error occurred:<br/>%s</>", msg);
		elm_object_text_set(ad->label1, "No data");
		elm_object_text_set(ad->label2, fullmsg);
	}


}

static void vs_com_cb(void *data, Evas_Object *obj, void *event_info);
static void restart(appdata_s *ad);


static void tap_to_move_cb(void* data, Evas *e, Evas_Object *obj, void *event_info){
	appdata_s *ad = data;

	if(ad->game_end == 0){
		if (strlen(ad->pos) >= 2) {
			sprintf(ad->pos, "?");
			elm_object_text_set(ad->label3, "?");
			evas_object_color_set(ad->rect[ad->position], 0, 100, 0, 100);
			evas_object_hide(ad->text);
			ad->sensor_interval = 50;
		}
	}
	else if(ad->game_end == 1){
		sprintf(ad->pos, "?");
		evas_object_hide(ad->text);
		evas_object_hide(ad->text2);
		restart(ad);
	}
}

static void single_cb(void *data, Evas_Object *obj, void *event_info){
	appdata_s *ad = data;

	ad->game_mode = 1;

	evas_object_hide(ad->box);

	evas_object_event_callback_add(ad->conform, EVAS_CALLBACK_MOUSE_DOWN, tap_to_move_cb, ad);

	/* Text1 - tab to move */
	ad->text = evas_object_text_add(evas_object_evas_get(ad->win));
	evas_object_text_text_set(ad->text, "Tap To Move");
	evas_object_text_font_set(ad->text, "DejaVu", 30);
	evas_object_color_set(ad->text, 100,100,100,100);
	evas_object_move(ad->text, 100, 30);
	evas_object_hide(ad->text);


	/* Text2 - bubble count */
	ad->text2 = evas_object_text_add(evas_object_evas_get(ad->win));
	char buf[20];
	sprintf(buf, "Bubbles: %d/25", ad->bubble_count);
	evas_object_text_text_set(ad->text2, buf);
	evas_object_text_font_set(ad->text2, "DejaVu", 30);
	evas_object_color_set(ad->text2, 100,100,100,100);
	evas_object_move(ad->text2, 80, 60);
	evas_object_show(ad->text2);

	for(int i = 0; i < 25; i++){
		ad->rect[i] = evas_object_rectangle_add(evas_object_evas_get(ad->conform));
		evas_object_resize(ad->rect[i], 35, 35);
		evas_object_color_set(ad->rect[i], 100, 100, 100, 100);
		evas_object_move(ad->rect[i], i%5 * 40 + 80, i/5 * 40 + 120);
		evas_object_show(ad->rect[i]);
	}

	for(int i = 0; i < 25; i++){
		ad->bubble_status[i] = 0;
	}

	ad->position = 20;
	evas_object_color_set(ad->rect[20], 0, 100, 0, 100);
	ad->bubble_status[20] = 1;
	ad->bubble_count = 1;

}

static Eina_Bool timer_cb(void *data EINA_UNUSED){
	appdata_s *ad = data;

	srand(time(NULL));

	int pos = rand() % 4; // 0:right, 1:left, 2:up, 3:down

	if(ad->difficulty == 3){
		if(ad->position_com % 5 != 4 && ad->bubble_status[ad->position_com + 1] == 0){
			pos = 0;
		}
		else if(ad->position_com % 5 != 0 && ad->bubble_status[ad->position_com - 1] == 0){
			pos = 1;
		}
		else if(ad->position_com / 5 != 0 && ad->bubble_status[ad->position_com - 5] == 0){
			pos = 2;
		}
		else if(ad->position_com / 5 != 4 && ad->bubble_status[ad->position_com + 5] == 0){
			pos = 3;
		}
	}

	if (pos == 0){
		if(ad->position_com % 5 != 4){
			//evas_object_color_set(ad->rect[ad->position_com], 0, 0, 100, 20);
			//evas_object_color_set(ad->rect[ad->position_com+1], 0, 0, 100, 100);

			ad->position_com = ad->position_com + 1;
			if(ad->bubble_status[ad->position_com] == 0){
				ad->bubble_status[ad->position_com] = 2;
				ad->bubble_count_com++;
				char buf[20];
				sprintf(buf, "YOU: %d / COM: %d", ad->bubble_count_you, ad->bubble_count_com);
				evas_object_text_text_set(ad->text2, buf);
			}
		}
	}
	else if (pos == 1){
		if(ad->position_com % 5 != 0){
			//evas_object_color_set(ad->rect[ad->position_com], 0, 0, 100, 20);
			//evas_object_color_set(ad->rect[ad->position_com+1], 0, 0, 100, 100);

			ad->position_com = ad->position_com - 1;
			if(ad->bubble_status[ad->position_com] == 0){
				ad->bubble_status[ad->position_com] = 2;
				ad->bubble_count_com++;
				char buf[20];
				sprintf(buf, "YOU: %d / COM: %d", ad->bubble_count_you, ad->bubble_count_com);
				evas_object_text_text_set(ad->text2, buf);
			}
		}
	}
	else if (pos == 2){
		if(ad->position_com / 5 != 0){
			//evas_object_color_set(ad->rect[ad->position_com], 0, 0, 100, 20);
			//evas_object_color_set(ad->rect[ad->position_com+1], 0, 0, 100, 100);

			ad->position_com = ad->position_com - 5;
			if(ad->bubble_status[ad->position_com] == 0){
				ad->bubble_status[ad->position_com] = 2;
				ad->bubble_count_com++;
				char buf[20];
				sprintf(buf, "YOU: %d / COM: %d", ad->bubble_count_you, ad->bubble_count_com);
				evas_object_text_text_set(ad->text2, buf);
			}
		}
	}
	else if (pos == 3){
		if(ad->position_com / 5 != 4){
			//evas_object_color_set(ad->rect[ad->position_com], 0, 0, 100, 20);
			//evas_object_color_set(ad->rect[ad->position_com+1], 0, 0, 100, 100);

			ad->position_com = ad->position_com + 5;
			if(ad->bubble_status[ad->position_com] == 0){
				ad->bubble_status[ad->position_com] = 2;
				ad->bubble_count_com++;
				char buf[20];
				sprintf(buf, "YOU: %d / COM: %d", ad->bubble_count_you, ad->bubble_count_com);
				evas_object_text_text_set(ad->text2, buf);
			}
		}
	}

	return ECORE_CALLBACK_RENEW;
}



static void vs_com_start(appdata_s *ad, double timer_interval){
	ad->game_end = 0;

	evas_object_event_callback_add(ad->conform, EVAS_CALLBACK_MOUSE_DOWN, tap_to_move_cb, ad);

	/* Text1 - tab to move */
	ad->text = evas_object_text_add(evas_object_evas_get(ad->win));
	evas_object_text_text_set(ad->text, "Tap To Move");
	evas_object_text_font_set(ad->text, "DejaVu", 30);
	evas_object_color_set(ad->text, 100,100,100,100);
	evas_object_move(ad->text, 100, 30);
	evas_object_hide(ad->text);


	/* Text2 - bubble count */
	ad->text2 = evas_object_text_add(evas_object_evas_get(ad->win));
	char buf[20];
	sprintf(buf, "YOU: %d / COM: %d", ad->bubble_count_you, ad->bubble_count_com);
	evas_object_text_text_set(ad->text2, buf);
	evas_object_text_font_set(ad->text2, "DejaVu", 30);
	evas_object_color_set(ad->text2, 100,100,100,100);
	evas_object_move(ad->text2, 80, 60);
	evas_object_show(ad->text2);

	for(int i = 0; i < 25; i++){
		ad->rect[i] = evas_object_rectangle_add(evas_object_evas_get(ad->conform));
		evas_object_resize(ad->rect[i], 35, 35);
		evas_object_color_set(ad->rect[i], 100, 100, 100, 100);
		evas_object_move(ad->rect[i], i%5 * 40 + 80, i/5 * 40 + 120);
		evas_object_show(ad->rect[i]);
	}

	for(int i = 0; i < 25; i++){
		ad->bubble_status[i] = 0;
	}

	ad->position = 20;
	evas_object_color_set(ad->rect[20], 0, 100, 0, 100);
	ad->bubble_status[20] = 1;

	ad->position_com = 4;
	evas_object_color_set(ad->rect[4], 0, 0, 100, 100);
	ad->bubble_status[4] = 2;

	//ad->bubble_count = 1;
	ad->bubble_count_you = 1;
	ad->bubble_count_com = 1;

	ad->timer = ecore_timer_add(timer_interval, timer_cb, ad);

	//show_is_supported(ad);
	ad->sensor_interval = 50;

}

static void restart(appdata_s *ad){
	ad->game_end = 0;

	//evas_object_event_callback_add(ad->conform, EVAS_CALLBACK_MOUSE_DOWN, tap_to_move_cb, ad);

	/* Text1 - tab to move */
	ad->text = evas_object_text_add(evas_object_evas_get(ad->win));
	evas_object_text_text_set(ad->text, "Tap To Move");
	evas_object_text_font_set(ad->text, "DejaVu", 30);
	evas_object_color_set(ad->text, 100,100,100,100);
	evas_object_move(ad->text, 100, 30);2
	evas_object_hide(ad->text);

	ad->bubble_count = 1;
	ad->bubble_count_you = 1;
	ad->bubble_count_com = 1;


	/* Text2 - bubble count */
	ad->text2 = evas_object_text_add(evas_object_evas_get(ad->win));
	char buf[20];
	if(ad->game_mode == 1)
		sprintf(buf, "Bubbles: %d/25", ad->bubble_count);
	else if(ad->game_mode == 2)
		sprintf(buf, "YOU: %d / COM: %d", ad->bubble_count_you, ad->bubble_count_com);
	evas_object_text_text_set(ad->text2, buf);
	evas_object_text_font_set(ad->text2, "DejaVu", 30);
	evas_object_color_set(ad->text2, 100,100,100,100);
	evas_object_move(ad->text2, 80, 60);
	evas_object_show(ad->text2);

	for(int i = 0; i < 25; i++){
		//ad->rect[i] = evas_object_rectangle_add(evas_object_evas_get(ad->conform));
		//evas_object_resize(ad->rect[i], 35, 35);
		evas_object_color_set(ad->rect[i], 100, 100, 100, 100);
		//evas_object_move(ad->rect[i], i%5 * 40 + 80, i/5 * 40 + 120);
		//evas_object_show(ad->rect[i]);
	}

	for(int i = 0; i < 25; i++){
		ad->bubble_status[i] = 0;
	}

	if(ad->game_mode == 1){
		ad->position = 20;
		evas_object_color_set(ad->rect[20], 0, 100, 0, 100);
		ad->bubble_status[20] = 1;
		ad->bubble_count = 1;
	}
	else if(ad->game_mode == 2){
		ad->position = 20;
		evas_object_color_set(ad->rect[20], 0, 100, 0, 100);
		ad->bubble_status[20] = 1;

		ad->position_com = 4;
		evas_object_color_set(ad->rect[4], 0, 0, 100, 100);
		ad->bubble_status[4] = 2;

		//ad->bubble_count = 1;


		double timer_interval = 3.0;

		if(ad->difficulty == 1)
			timer_interval = 3.0;
		else if(ad->difficulty == 2)
			timer_interval = 2.0;
		else if(ad->difficulty == 3)
			timer_interval = 2.0;

		ad->timer = ecore_timer_add(timer_interval, timer_cb, ad);
	}

	//show_is_supported(ad);
	ad->sensor_interval = 50;

}


static void easy_cb(void *data, Evas_Object *obj, void *event_info){
	appdata_s *ad = data;
	evas_object_hide(ad->list_difficulty);
	evas_object_hide(ad->text);
	evas_object_hide(ad->text2);
	ad->difficulty = 1;
	vs_com_start(ad, 3.0);
}
static void normal_cb(void *data, Evas_Object *obj, void *event_info){
	appdata_s *ad = data;
	evas_object_hide(ad->list_difficulty);
	evas_object_hide(ad->text);
	evas_object_hide(ad->text2);
	ad->difficulty = 2;
	vs_com_start(ad, 2.0);
}
static void hard_cb(void *data, Evas_Object *obj, void *event_info){
	appdata_s *ad = data;
	evas_object_hide(ad->list_difficulty);
	evas_object_hide(ad->text);
	evas_object_hide(ad->text2);
	ad->difficulty = 3;
	vs_com_start(ad, 2.0);
}

static void vs_com_cb(void *data, Evas_Object *obj, void *event_info){
	appdata_s *ad = data;

	ad->game_mode = 2;

	evas_object_hide(ad->list);

	//ad->box2 = evas_object_box_add(ad->conform);
	//evas_object_size_hint_weight_set(ad->box2, EVAS_HINT_EXPAND,	EVAS_HINT_EXPAND);
	//elm_object_content_set(ad->conform, ad->box2);
	//evas_object_show(ad->box2);

	ad->list_difficulty = elm_list_add(ad->box);
	/* Set the list size */
	evas_object_size_hint_weight_set(ad->list_difficulty, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(ad->list_difficulty, EVAS_HINT_FILL, EVAS_HINT_FILL);

	/* Add an item to the list */
	elm_list_item_append(ad->list_difficulty, "EASY", NULL, NULL, easy_cb, ad);
	elm_list_item_append(ad->list_difficulty, "NORMAL", NULL, NULL, normal_cb, ad);
	elm_list_item_append(ad->list_difficulty, "HARD", NULL, NULL, hard_cb, ad);

	evas_object_show(ad->list_difficulty);
	elm_box_pack_end(ad->box, ad->list_difficulty);

}

static void
create_base_gui(appdata_s *ad)
{
	/* Window */
	ad->win = elm_win_util_standard_add(PACKAGE, PACKAGE);
	elm_win_autodel_set(ad->win, EINA_TRUE);

	if (elm_win_wm_rotation_supported_get(ad->win)) {
		int rots[4] = { 0, 90, 180, 270 };
		elm_win_wm_rotation_available_rotations_set(ad->win, (const int *)(&rots), 4);
	}

	evas_object_smart_callback_add(ad->win, "delete,request", win_delete_request_cb, NULL);
	eext_object_event_callback_add(ad->win, EEXT_CALLBACK_BACK, win_back_cb, ad);

	/* Conformant */
	ad->conform = elm_conformant_add(ad->win);
	elm_win_indicator_mode_set(ad->win, ELM_WIN_INDICATOR_SHOW);
	elm_win_indicator_opacity_set(ad->win, ELM_WIN_INDICATOR_OPAQUE);
	evas_object_size_hint_weight_set(ad->conform, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_win_resize_object_add(ad->win, ad->conform);
	evas_object_show(ad->conform);



	/* child object - indent to how relationship */

	/* A box to put things in verticallly - default mode for box */
	ad->box = elm_box_add(ad->conform);
	evas_object_size_hint_weight_set(ad->box, EVAS_HINT_EXPAND,	EVAS_HINT_EXPAND);
	elm_object_content_set(ad->conform, ad->box);
	evas_object_show(ad->box); //hide


	/* child object - indent to how relationship */
	/*
	// Label-0
	ad->label0 = elm_label_add(ad->box);
	elm_object_text_set(ad->label0, "Msg - ");
	elm_box_pack_end(ad->box, ad->label0);
	evas_object_hide(ad->label0);

	//Label-1 current value
	ad->label1 = elm_label_add(ad->box);
	elm_object_text_set(ad->label1, "Value - ");
	elm_box_pack_end(ad->box, ad->label1);
	evas_object_hide(ad->label1);

	// Label-2 max value
	ad->label2 = elm_label_add(ad->box);
	elm_object_text_set(ad->label2, "MAX Value - ");
	elm_box_pack_end(ad->box, ad->label2);
	evas_object_hide(ad->label2);

	// Label-3 position
	ad->label3 = elm_label_add(ad->box);
	elm_object_text_set(ad->label3, "?");
	elm_box_pack_end(ad->box, ad->label3);
	evas_object_hide(ad->label3);
	*/

	/* Create the list */
	ad->list = elm_list_add(ad->box);
	/* Set the list size */
	evas_object_size_hint_weight_set(ad->list, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(ad->list, EVAS_HINT_FILL, EVAS_HINT_FILL);

	/* Add an item to the list */
	elm_list_item_append(ad->list, "SINGLE", NULL, NULL, single_cb, ad);
	elm_list_item_append(ad->list, "VS COM", NULL, NULL, vs_com_cb, ad);

	evas_object_show(ad->list);
	elm_box_pack_end(ad->box, ad->list);



	/* Show window after base gui is set up */

	show_is_supported(ad);
	start_acceleration_sensor(ad);


	evas_object_show(ad->win);
}

static bool
app_create(void *data)
{
	/* Hook to take necessary actions before main event loop starts
		Initialize UI resources and application's data
		If this function returns true, the main loop of application starts
		If this function returns false, the application is terminated */
	appdata_s *ad = data;

	create_base_gui(ad);

	return true;
}

static void
app_control(app_control_h app_control, void *data)
{
	/* Handle the launch request. */
}

static void
app_pause(void *data)
{
	/* Take necessary actions when application becomes invisible. */
}

static void
app_resume(void *data)
{
	/* Take necessary actions when application becomes visible. */
}

static void
app_terminate(void *data)
{
	/* Release all resources. */
}

static void
ui_app_lang_changed(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_LANGUAGE_CHANGED*/
	char *locale = NULL;
	system_settings_get_value_string(SYSTEM_SETTINGS_KEY_LOCALE_LANGUAGE, &locale);
	elm_language_set(locale);
	free(locale);
	return;
}

static void
ui_app_orient_changed(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_DEVICE_ORIENTATION_CHANGED*/
	return;
}

static void
ui_app_region_changed(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_REGION_FORMAT_CHANGED*/
}

static void
ui_app_low_battery(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_LOW_BATTERY*/
}

static void
ui_app_low_memory(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_LOW_MEMORY*/
}

int
main(int argc, char *argv[])
{
	appdata_s ad = {0,};
	int ret = 0;

	ui_app_lifecycle_callback_s event_callback = {0,};
	app_event_handler_h handlers[5] = {NULL, };

	event_callback.create = app_create;
	event_callback.terminate = app_terminate;
	event_callback.pause = app_pause;
	event_callback.resume = app_resume;
	event_callback.app_control = app_control;

	ui_app_add_event_handler(&handlers[APP_EVENT_LOW_BATTERY], APP_EVENT_LOW_BATTERY, ui_app_low_battery, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_LOW_MEMORY], APP_EVENT_LOW_MEMORY, ui_app_low_memory, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_DEVICE_ORIENTATION_CHANGED], APP_EVENT_DEVICE_ORIENTATION_CHANGED, ui_app_orient_changed, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_LANGUAGE_CHANGED], APP_EVENT_LANGUAGE_CHANGED, ui_app_lang_changed, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_REGION_FORMAT_CHANGED], APP_EVENT_REGION_FORMAT_CHANGED, ui_app_region_changed, &ad);
	ui_app_remove_event_handler(handlers[APP_EVENT_LOW_MEMORY]);

	ret = ui_app_main(argc, argv, &event_callback, &ad);
	if (ret != APP_ERROR_NONE) {
		dlog_print(DLOG_ERROR, LOG_TAG, "app_main() is failed. err = %d", ret);
	}

	return ret;
}
