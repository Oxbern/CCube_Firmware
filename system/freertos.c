#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "queue.h"

#include "stm32f4xx_hal.h"

#include "GUI.h"
#include "BUTTON.h"
#include "TREEVIEW.h"
#include "DIALOG.h"
#include "MULTIPAGE.h"
#include "TEXT.h"
#include "WM.h"

#include "usbd_cdc.h"
#include "usbd_cdc_if.h"
#include "usb_device.h"
#include "ltdc.h"
#include "sdram.h"
#include "dma2d.h"
#include "i2c.h"
#include "crc.h"
#include "sd.h"
#include "spi.h"
#include "tim.h"
#include "adc.h"
#include "fatfs.h"
#include "json.h"
#include "json-builder.h"
#include "led.h"
#include "console.h"
#include "database_utils.h"
#include "database_structures.h"





#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_POINTS 9*9*9
#define BUFFER_MAX_INDEX 64

/*
 * Firmware Tasks
 */

osThreadId initTaskHandle;
void StartInitTask(void const * argument);

osThreadId touchTaskHandle;
extern void StartTouchTask(void const * argument);

osThreadId pwmTaskHandle;
extern void StartPwmTask(void const * argument);

osThreadId ledTaskHandle;
extern void StartLedTask(void const * argument);

osThreadId fsTaskHandle;
extern void StartFsTask(void const * argument);

osThreadId guiTaskHandle;
extern void StartGuiTask(void const * argument);

osThreadId blinkTaskHandle;
extern void StartBlinkTask(void const * argument);

osThreadId CDC_receptionTaskHandle;
extern void StartCDCReceptionTask(void const * argument);

osThreadId CDC_transmissionTaskHandle;
extern void StartCDCTransmissionTask(void const * argument);


QueueHandle_t receptionQueue = 0;
QueueHandle_t transmissionQueue = 0;

Reception_Task_Args receptionTaskArgs;
Transmission_Task_Args transmissionTaskArgs;
    

/**
 * FreeRTOS Initialisation function
 */
void FREERTOS_Init(void)
{
	osThreadDef(initTask, StartInitTask, osPriorityHigh, 0, 8192);
	initTaskHandle = osThreadCreate(osThread(initTask), NULL);

}


void StartInitTask(void const * argument)
{
	console_init();

	osThreadDef(touchTask, StartTouchTask, osPriorityHigh, 0, 8192);
	touchTaskHandle = osThreadCreate(osThread(touchTask), NULL);

	osThreadDef(pwmTask, StartPwmTask, osPriorityNormal, 0, 8192);
	pwmTaskHandle = osThreadCreate(osThread(pwmTask), NULL);

	osThreadDef(ledTask, StartLedTask, osPriorityNormal, 0, 8192);
	ledTaskHandle = osThreadCreate(osThread(ledTask), NULL);

	osThreadDef(fsTask, StartFsTask, osPriorityNormal, 0, 32768);
	fsTaskHandle = osThreadCreate(osThread(fsTask), NULL);

	osThreadDef(guiTask, StartGuiTask, osPriorityNormal, 0, 8192);
	guiTaskHandle = osThreadCreate(osThread(guiTask), NULL);

	osThreadDef(blinkTask, StartBlinkTask, osPriorityNormal, 0, 8192);
	blinkTaskHandle = osThreadCreate(osThread(blinkTask), NULL);


	receptionQueue = xQueueCreate(10, BUFFER_MAX_INDEX*sizeof(uint8_t));
	transmissionQueue = xQueueCreate(10, BUFFER_MAX_INDEX*sizeof(uint8_t));

	receptionTaskArgs.receptionQueue = receptionQueue;
	transmissionTaskArgs.transmissionQueue = transmissionQueue;
	
	osThreadDef(cdcReceptionTask, StartCDCReceptionTask, osPriorityNormal, 0, 8192);
	CDC_receptionTaskHandle = osThreadCreate(osThread(cdcReceptionTask),
						 (void *)&receptionTaskArgs);

	osThreadDef(cdcTransmissionTask, StartCDCTransmissionTask, osPriorityNormal, 0, 8192);
	CDC_transmissionTaskHandle = osThreadCreate(osThread(cdcTransmissionTask),
						    (void *)&transmissionTaskArgs);

	
	vTaskDelete(initTaskHandle);
}



bool correct_extention(char * filename)
{
	char * point;
	if((point = strrchr(filename, '.')) != NULL)
	{
		if ((strcmp(point, ".ccdb") == 0) || (strcmp(point, ".lua") == 0))
		{
			return true;
		} else {
			return false;
		}	
	} else {
		return false;
	}
}


typedef struct myfd {
	int type;
	char * path;
} myfd_t;

FRESULT scan_files (
	char* path,        /* Start node to be scanned (also used as work area) */
	TREEVIEW_Handle *hpath
)
{
	printf("calling scan_files(%s)\n", path);
    FRESULT res;
    FILINFO fno;
    DIR dir;
    int i;
    char *fn;   /* This function assumes non-Unicode configuration */
    static char lfn[_MAX_LFN + 1];   /* Buffer to store the LFN */
    fno.lfname = lfn;
    fno.lfsize = sizeof lfn;


    res = f_opendir(&dir, path);                       /* Open the directory */
    if (res == FR_OK) {
		// Create directory in treeview
		TREEVIEW_ITEM_Handle node =  TREEVIEW_InsertItem(	*hpath,
															TREEVIEW_ITEM_TYPE_NODE,
															0,
															0,
															(const char *)path
														);
		myfd_t * this_fd = malloc(sizeof(struct myfd));
		this_fd->type = 0;
		char * file_path = malloc(strlen(path)+1);
		sprintf(file_path, "%s", path);
		//printf("%s\n", file_path);
		this_fd->path = file_path;
		TREEVIEW_ITEM_SetUserData(node, (uint32_t)this_fd);

		int nb_child = 0;
		TREEVIEW_ITEM_Handle last_item = NULL;
        i = strlen(path);
        for (;;) {
            res = f_readdir(&dir, &fno);                   /* Read a directory item */
            if (res != FR_OK || fno.fname[0] == 0) break;  /* Break on error or end of dir */
            if (fno.fname[0] == '.') 
			{
					continue;             /* Ignore dot entry */
			}
            fn = *fno.lfname ? fno.lfname : fno.fname;
            if (fno.fattrib & AM_DIR) {                    /* It is a directory */
				int j = strlen(fn);
				char * dirname = malloc(j);
				strcpy(dirname, fn);
				char newpath[i+j]; // "/0" du i renplacé par "/" plus bas
				newpath[0] = '\0';
				strcat(newpath, path);
				strcat(newpath, "/");
				strcat(newpath, fn);
				TREEVIEW_Handle child_tree = TREEVIEW_CreateEx(0,0,0,0,0,0,0,0);
                res = scan_files(newpath, &child_tree);
				TREEVIEW_ITEM_Handle child_tree_first_node = TREEVIEW_GetItem(	child_tree,
																				0,
																				TREEVIEW_GET_FIRST
																				);
				if (child_tree_first_node != 0)
				{
						child_tree_first_node = TREEVIEW_ITEM_SetText(	child_tree_first_node,
								dirname);
						TREEVIEW_AttachItem(	*hpath,
								child_tree_first_node,
								nb_child ? last_item : node,
								nb_child ? TREEVIEW_INSERT_BELOW : TREEVIEW_INSERT_FIRST_CHILD
								);
						last_item = child_tree_first_node;
				}
				free(dirname);
                if (res != FR_OK) break;
				nb_child++;
            } else {                                       /* It is a file. */
				if (correct_extention(fn))
				{
					last_item = TREEVIEW_InsertItem(	*hpath,
														TREEVIEW_ITEM_TYPE_LEAF,
														nb_child ? last_item : node,
														nb_child ? TREEVIEW_INSERT_BELOW : TREEVIEW_INSERT_FIRST_CHILD,
														(const char *) fn
													);
					nb_child++;
					myfd_t * this_fd = malloc(sizeof(struct myfd));
					this_fd->type = 1;
					char * file_path = malloc(strlen(path)+strlen(fn)+2);
					sprintf(file_path, "%s/%s", path, fn);
					//printf("%s\n", file_path);
					this_fd->path = file_path;
					TREEVIEW_ITEM_SetUserData(last_item, (uint32_t)this_fd);
				}
            }
        }
        f_closedir(&dir);
    }

    return res;
}

database_t * db = NULL;
int current_motif_index;
char current_motif_title[256];
char * current_motif_desc;
TEXT_Handle motif_name_widget;
MULTIEDIT_HANDLE motif_desc_widget;
point_t * points2blink = NULL;

void clear_points2blink(void)
{
	points2blink = NULL;
}

void handle_option(option_t * o)
{
	switch (o->type)
	{
		case BLINK:
			new_point_queue(
						((blink_params_t*)o->params)->x,
						((blink_params_t*)o->params)->y,
						((blink_params_t*)o->params)->z,
						&points2blink
						);
			break;
		case DUPLICATE:
			break;
	}
}

void update_motif(void)
{
	motif_t * m = db->motifs;
	for (uint32_t i = 1; i < current_motif_index; i++)
		m = m->next;
	sprintf(current_motif_title, "%i/%i -- %s", (int)current_motif_index, (int)db->nb_motifs, m->name);
	current_motif_desc = m->desc;
	MULTIEDIT_SetText(motif_desc_widget, current_motif_desc);
	TEXT_SetText(motif_name_widget, current_motif_title);

	led_clear();
	point_t * p = m->points;
	while (p != NULL)
	{	
		led_set(p->x, p->y, p->z);
		p = p->next;
	}

	clear_points2blink();
	
	option_t * o = m->options;
	while (o != NULL)
	{
		handle_option(o);
		o = o->next;
	}
}

void _cbrundb(WM_MESSAGE * pMsg)
{
	int NCode;
	int Id;
	
	switch(pMsg->MsgId)
	{
		case WM_NOTIFY_PARENT:
			NCode = pMsg->Data.v;
			Id = WM_GetId(pMsg->hWinSrc);
			if		(Id == GUI_ID_BUTTON0) // Prev
			{
				switch (NCode)
				{
					case WM_NOTIFICATION_RELEASED:
						{
							if (current_motif_index > 1)
							{
								current_motif_index--;
								update_motif();
							}
						}
						break;
				}
				
			}
			else if (Id == GUI_ID_BUTTON1) // Next
			{
				switch(NCode)
				{
					case WM_NOTIFICATION_RELEASED:
						{
							if (current_motif_index < db->nb_motifs)
							{
								current_motif_index++;
								update_motif();
							}
						}
						break;
				}
			}
			break;
		case WM_PAINT:
			GUI_SetColor(GUI_WHITE);
			GUI_Clear();
		default:
			WM_DefaultProc(pMsg);
	}
	
}

extern GUI_CONST_STORAGE GUI_BITMAP bmarrowrdefault;
extern GUI_CONST_STORAGE GUI_BITMAP bmarrowdefault;
extern GUI_CONST_STORAGE GUI_BITMAP bmarrowrclicked;
extern GUI_CONST_STORAGE GUI_BITMAP bmarrowclicked;


void run_db(void)
{
/*	
	printf("Database name : %s\n", db->name);
	printf("Database nb children : %i\n", (int)db->nb_motifs);
	for (uint32_t i = 0; i < db->nb_motifs; i++)
	{
		motif_t * m = db->motifs;
		printf("    motif %i:\n", (int)i);
		printf("        name : %s\n", m->name);
		printf("        desc : %s\n", m->desc);
		printf("        img  : %s\n", m->image);
	}
*/
	char error_404[] = "No Motifs Found in Database!";


	if (db->motifs == NULL)
	{
		strcpy(current_motif_title, error_404);
	} else {
		current_motif_index = 1;
		//sprintf(current_motif_title, "%i/%i -- %s", (int)current_motif_index, (int)db->nb_motifs, db->motifs->name);
		//current_motif_desc = db->motifs->desc;
		update_motif();
	}

	WM_HWIN		db_nav_win = WINDOW_CreateEx(	0, 0,
												800, 480,
												WM_HBKWIN, WM_CF_SHOW,
												0, GUI_ID_USER+0,
												_cbrundb);

	WINDOW_SetBkColor(db_nav_win, GUI_WHITE);

	motif_name_widget = TEXT_CreateEx(	0, 0,
													800, 100,
													db_nav_win, WM_CF_SHOW,
													TEXT_CF_HCENTER | TEXT_CF_VCENTER, GUI_ID_TEXT0,
													(const char *)current_motif_title);

	
	motif_desc_widget = MULTIEDIT_CreateEx(	100, 100,
																600, 380,
																db_nav_win, WM_CF_SHOW,
																MULTIEDIT_CF_AUTOSCROLLBAR_V, GUI_ID_MULTIEDIT1,
																5000,
																(const char *) current_motif_desc
															);

	BUTTON_Handle bt_prev_widget = BUTTON_CreateEx(	0, 100,
													100, 380,
													db_nav_win, WM_CF_SHOW,
													0,  GUI_ID_BUTTON0);


	BUTTON_Handle bt_next_widget = BUTTON_CreateEx(	700, 100,
													100, 380,
													db_nav_win, WM_CF_SHOW,
													0,  GUI_ID_BUTTON1);
	
	TEXT_SetFont(motif_name_widget, GUI_FONT_24_1);
	BUTTON_SetFont(bt_prev_widget, GUI_FONT_24_1);
	BUTTON_SetFont(bt_next_widget, GUI_FONT_24_1);
	MULTIEDIT_SetFont(motif_desc_widget, GUI_FONT_24_1);
	MULTIEDIT_SetWrapWord(motif_desc_widget);

	BUTTON_SetBitmap(bt_prev_widget, BUTTON_BI_UNPRESSED, &bmarrowdefault);
	BUTTON_SetBitmap(bt_next_widget, BUTTON_BI_UNPRESSED, &bmarrowrdefault);
	BUTTON_SetBitmap(bt_prev_widget, BUTTON_BI_PRESSED  , &bmarrowclicked);
	BUTTON_SetBitmap(bt_next_widget, BUTTON_BI_PRESSED  , &bmarrowrclicked);

}


void _cbHBKWIN(WM_MESSAGE * pMsg)
{
	int NCode;
	TREEVIEW_ITEM_Handle Sel;
	int Id;
	char pBuffer[128];
	
	switch (pMsg->MsgId)
	{
		case WM_NOTIFY_PARENT:
			NCode = pMsg->Data.v;
			Id = WM_GetId(pMsg->hWinSrc);
			if (Id == GUI_ID_TREEVIEW0)
			{
				switch (NCode)
				{
					case WM_NOTIFICATION_CLICKED:
						//printf("switching focus to treeview\n");
						WM_SetFocus(pMsg->hWinSrc);
						break;
					case WM_NOTIFICATION_RELEASED:
						Sel = TREEVIEW_GetSel(pMsg->hWinSrc);
						TREEVIEW_ITEM_GetText(Sel, (U8*)pBuffer, 128);
						printf("Selecting \"%s\"\n", pBuffer);
						
						myfd_t * selfd = (myfd_t *) TREEVIEW_ITEM_GetUserData(Sel);
						printf("Path to selection : %s\n", selfd->path);
						if (selfd->type == 0)
						{
							printf("is a directory\n");
						} else {
							printf("is a file\n");
							char * file_str = file2string(selfd->path);
							db = string2database(file_str);
							if (db)
							{
								run_db();
							} else {
								printf("Error loading database\n");
							}
						}
						
						break;
					case WM_NOTIFICATION_SEL_CHANGED:
						printf("Selection changed\n");
						break;
				}
			}
			else if (Id == GUI_ID_MULTIEDIT0)
			{
				switch (NCode)
				{
					case WM_NOTIFICATION_CLICKED:
						//printf("switching focus to terminal\n");
						WM_SetFocus(pMsg->hWinSrc);
						break;
					case WM_NOTIFICATION_VALUE_CHANGED:
						//printf("WM_NOTIFICATION_VALUE_CHANGED\n");
						break;
				}
			}
			else if (Id == GUI_ID_USER+0)
			{
				switch (NCode)
				{
					case WM_NOTIFICATION_CLICKED:
						WM_SetFocus(pMsg->hWinSrc);
						break;
				}
			}
			break;
		
		case WM_PAINT:
			GUI_SetColor(GUI_WHITE);
			GUI_Clear();
			break;

		default:
			WM_DefaultProc(pMsg);
	}
}

void StartFsTask(void const * argument)
{
	printf("Fs task started\n");
	taskENTER_CRITICAL();
	TREEVIEW_Handle filesystem  = TREEVIEW_CreateEx(	500, 0,
														300, 480,
														0, WM_CF_SHOW,
														TREEVIEW_CF_AUTOSCROLLBAR_V |
														TREEVIEW_CF_AUTOSCROLLBAR_H |
														TREEVIEW_CF_ROWSELECT,
														GUI_ID_TREEVIEW0
													);
	TREEVIEW_SetFont(filesystem, GUI_FONT_24_ASCII);
	scan_files("", &filesystem);
	TREEVIEW_ITEM_ExpandAll(TREEVIEW_GetItem(filesystem, 0, TREEVIEW_GET_FIRST));
	WM_SetCallback(WM_HBKWIN, _cbHBKWIN);
	WM_SetFocus(filesystem);
	taskEXIT_CRITICAL();

    while(1)
    {
		osDelay(5000);
    }
}

