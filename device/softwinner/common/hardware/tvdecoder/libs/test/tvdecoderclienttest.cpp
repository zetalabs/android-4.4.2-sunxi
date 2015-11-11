/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "TVDecoderServiceTest"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <ui/GraphicBuffer.h>
#include <binder/IPCThreadState.h>
#include <binder/IServiceManager.h>
#include <binder/ProcessState.h>
#include <utils/KeyedVector.h>
#include <utils/Log.h>
#include <utils/Vector.h>
#include <utils/threads.h>
#include "../../include/ITVDecoderService.h"

using namespace android;

int main(int argc, char **argv)
{
	printf("TVDecoderService start\n");
	sp<IServiceManager> sm = defaultServiceManager();
	sp<IBinder> binder = sm->getService(String16("media.tvd"));
	sp<ITVDecoderService> tvds = interface_cast<ITVDecoderService>(binder);
	tvds->connect(getpid(),1,0,1,0);
	tvds->startDecoder();
	printf("TVDecoderService finished\n");
}
