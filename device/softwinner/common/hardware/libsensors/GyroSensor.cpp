/*
 * Copyright (C) 2008 The Android Open Source Project
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

#define LOG_TAG "GyroSensor"
#include <fcntl.h>
#include <errno.h>
#include <math.h>
#include <poll.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/select.h>
#include <cutils/log.h>
#include <utils/BitSet.h>
#include <cutils/properties.h>

#include "GyroSensor.h"
#include "MEMSAlgLib_Fusion.h"

#define EVENT_TYPE_GYRO_X           ABS_X
#define EVENT_TYPE_GYRO_Y           ABS_Y
#define EVENT_TYPE_GYRO_Z           ABS_Z

// conversion of gyro data to SI units (radian/sec)
#define RANGE_GYRO                  (2000.0f*(float)M_PI/180.0f)
#define CONVERT_GYRO                ((70.0f / 1000.0f) * ((float)M_PI / 180.0f))
#define CONVERT_GYRO_X              (CONVERT_GYRO)
#define CONVERT_GYRO_Y              (CONVERT_GYRO)
#define CONVERT_GYRO_Z              (CONVERT_GYRO)

#define FETCH_FULL_EVENT_BEFORE_RETURN 1
#define IGNORE_EVENT_TIME 350000000

#define INPUT_SYSFS_PATH_GYRO "/sys/class/i2c-adapter/i2c-2/2-006a/"

/*****************************************************************************/

GyroSensor::GyroSensor() : SensorBase(NULL, "l3gd20_gyr"),
    mEnabled(0),
    mInputReader(4)
{
    memset(&mPendingEvent, 0, sizeof(mPendingEvent));

    mPendingEvent.version = sizeof(sensors_event_t);
    mPendingEvent.sensor = ID_GY;
    mPendingEvent.type = SENSOR_TYPE_GYROSCOPE;
    mPendingEvent.gyro.status = SENSOR_STATUS_ACCURACY_HIGH;

    if (data_fd >= 0) {
        sprintf(gyrSensorInfo.classPath, "%s/%s", gyrSensorInfo.classPath, "device");
        ALOGD("gyrSensorInfo.classPath:%s", gyrSensorInfo.classPath);
    }
}

GyroSensor::~GyroSensor() {
    if (mEnabled) {
        setEnable(ID_GY, 0);
    }
}

int GyroSensor::setEnable(int32_t handle, int en) {
    char buf[2];
    int err = -1 ;

    if(gyrSensorInfo.classPath[0] == '\0')
        return -1;

    int bytes = sprintf(buf, "%d", en);

    if (en != mEnabled) {
        err = set_sysfs_input_attr(gyrSensorInfo.classPath,"enable_device",buf,bytes);
        mEnabled = en;
    }
    return 0;
}

int GyroSensor::setDelay(int32_t handle, int64_t delay_ns)
{
    char buf[80];

    int bytes = sprintf(buf, "%lld", delay_ns/1000 / 1000);
    int err = set_sysfs_input_attr(gyrSensorInfo.classPath,"pollrate_ms",buf,bytes);

    return err;
}

int GyroSensor::readEvents(sensors_event_t* data, int count)
{
    static int64_t prev_time;
    int64_t time;

    if (count < 1)
        return -EINVAL;

    if(data_fd < 0)
        return 0;

    ssize_t n = mInputReader.fill(data_fd);
    if (n < 0)
        return n;

    int numEventReceived = 0;
    input_event const* event;
    float gyrox, gyroy, gyroz;

#if FETCH_FULL_EVENT_BEFORE_RETURN
again:
#endif
    while (count && mInputReader.readEvent(&event)) {
        //ALOGD("GyroSensor::readEvents() coutn = %d, event->value = %f", count, event->value);
        int type = event->type;

        if (type == EV_ABS) {
            float value = event->value;
            if (event->code == EVENT_TYPE_GYRO_X) {
                gyrox = value;
            } else if (event->code == EVENT_TYPE_GYRO_Y) {
                gyroy = value;
            } else if (event->code == EVENT_TYPE_GYRO_Z) {
                gyroz = value;
            }
        }else if (type == EV_SYN) {
            NineAxisTypeDef nineInput;
            nineInput.ax =  1;
            nineInput.ay =  1;
            nineInput.az =  1000;
            nineInput.mx =  300;
            nineInput.my =  300;
            nineInput.mz =  300;
            nineInput.gx =  gyrox;
            nineInput.gy =  gyroy;
            nineInput.gz =  gyroz;

            nineInput.time = getTimestamp()/1000000;

            FusionTypeDef fusionData = MEMSAlgLib_Fusion_Update(nineInput);
            float offx, offy, offz;
            MEMSAlgLib_Fusion_Get_GyroOffset(&offx,&offy,&offz);

#ifdef DEBUG_SENSOR
            ALOGD("gyro offset: %f, %f, %f", offx, offy, offz);
#endif
            mPendingEvent.data[0] = (gyrox-offx) * CONVERT_GYRO_X;
            mPendingEvent.data[1] = (gyroy-offy) * CONVERT_GYRO_Y;
            mPendingEvent.data[2] = (gyroz-offz) * CONVERT_GYRO_Z;

            time = timevalToNano(event->time);

            if(mEnabled) {
                mPendingEvent.timestamp = time;

                *data++ = mPendingEvent;
                count--;
                numEventReceived++;
            }

        }else {
            ALOGE("GyroSensor: unknown event (type=%d, code=%d)", type, event->code);
        }
        mInputReader.next();
    }

#if FETCH_FULL_EVENT_BEFORE_RETURN
    /* if we didn't read a complete event, see if we can fill and
       try again instead of returning with nothing and redoing poll. */
    if (numEventReceived == 0 && mEnabled == 1) {
        n = mInputReader.fill(data_fd);
        if (n)
            goto again;
    }
#endif

    return numEventReceived;
}

void GyroSensor::processEvent(int code, int value)
{
}
