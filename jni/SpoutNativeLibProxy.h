/*
 * SpoutNativeLibProxy.h
 *
 *  Created on: Oct 16, 2015
 *      Author: exl
 */

#ifndef SPOUTNATIVELIBPROXY_H_
#define SPOUTNATIVELIBPROXY_H_

#include <android/log.h>

enum KeyCodes {
	KEY_LEFT	=	0x01,
	KEY_RIGHT	=	0x02,
	KEY_UP		=	0x03,
	KEY_DOWN	=	0x04,
	KEY_FIRE	=	0x05,
	KEY_QUIT	=	0x06,
	KEY_PAUSE	=	0x07,
	KEY_UNKNOWN	=	0x08
};

#define LOG_TAG "Spout_App"
#define LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)

#endif /* SPOUTNATIVELIBPROXY_H_ */
