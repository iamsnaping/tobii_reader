#include <tobii.h>
#include <tobii_streams.h>
#include <stdio.h>
#include <assert.h>
#include <string>
#include <string.h>
#include <vector>
#include<iostream>
#include<zmq.h>
#include<json.hpp>
#include<iostream>
#include<ctime>
#include<windows.h>
#include<chrono>
#pragma comment(lib, "User32.lib")
#define KEY_DOWN(VK_NONAME) ((GetAsyncKeyState(VK_NONAME) & 0x8000) ? 1:0) //必要的，要背下来

#pragma comment(lib, "tobii_stream_engine.lib")
#pragma warning(disable:4996)
using json = nlohmann::json;
using namespace std;
int gaze[2];
void* ctx = zmq_ctx_new();
void* publisher = zmq_socket(ctx, ZMQ_PUB);
FILE* out = fopen("record.csv", "w");
int weight;
int height;

int trackers_order[2] = { 0,0 };

char tmp[32] = { NULL };
/*
    回调函数，获得tobii的数据并转化为json发送出去
*/
void gaze_point_callback_1(tobii_gaze_point_t const* gaze_point, void* )
{
    POINT pt;
    GetCursorPos(&pt);
    // Check that the data is valid before using it
    //cout << gaze_point->position_xy[0] << " " << gaze_point->position_xy[1] << endl;
    if (gaze_point->validity == TOBII_VALIDITY_VALID) //判断是否成功获取数据
    {
        //printf("Gaze point: %f, %f\n",            gaze_point->position_xy[0] * 1920,            gaze_point->position_xy[1] * 1080);
        gaze[0] = gaze_point->position_xy[0]*weight;
        gaze[1] = gaze_point->position_xy[1]*height;

        //cout<<gaze_point->position_xy[0]<<" 1 "<<gaze_point->position_xy[1] << gaze[0] << " " << gaze[1] << " 1" << endl;
        //cout << gaze[0] << " " << gaze[1] << " 1" << endl;
        //pd为判断鼠标是否按下，按下为0，否则为3
        int pd = 3;
        if (KEY_DOWN(MOUSE_MOVED))
            pd = 0;
        time_t t = time(0);
        //获得时间戳
        strftime(tmp, sizeof(tmp), "%H:%M:%S", localtime(&t));
        fprintf(out,"%d,%d,%d,%d,%d\n", gaze[0],gaze[1],pt.x,pt.y,pd);
        //建立json
        char  number[10];
        if (trackers_order[0] == 0)
            strcpy(number, "1");
        else
            strcpy(number, "2");
        //cout << number << endl;
        char mouse_x[10] = {0};
        char mouse_y[10] = { 0 };
        char pd_str[10] = { 0 };
        itoa(pt.x, mouse_x, 10);
        itoa(pt.y, mouse_y, 10);
        itoa(pd,pd_str,10);
        //cout << mouse_x << " " << mouse_y << endl;
        //cout << "gaze 1" << endl;
        cout << number << endl;
        //cout << number << endl;
        json j{
                    {"command","eye"},
                    {"actualX", int(gaze_point->position_xy[0]*weight)},
                    {"actualY", int(gaze_point->position_xy[1]*height)},
                    {"id",number},
                    {"content", "1"},
                    {"time",tmp},
                    {"x",mouse_x},
                    {"y",mouse_y},
                    {"pd",pd_str}
        };
        char update[100];
        std::string s = j.dump();
        strcpy(update, s.c_str());
        //如果正常获得数据就用zmq发送出去
        if (gaze[0] != 0)
        {
            //std::cout << pd << " str " << pd_str << endl;
            zmq_send(publisher, update, strlen(update), 0);
        }
            
        //std::cout << gaze_point->position_xy[0] << " " << gaze_point->position_xy[1] << std::endl;
    }
}
//和上面的一样，但是因为是两个tobii所以分开写了
void gaze_point_callback_2(tobii_gaze_point_t const* gaze_point, void*)
{
    //cout << gaze_point->position_xy[0] << " " << gaze_point->position_xy[1] << endl;
    // Check that the data is valid before using it
    POINT pt;
    GetCursorPos(&pt);
    if (gaze_point->validity == TOBII_VALIDITY_VALID)
    {
        //printf("Gaze point: %f, %f\n",            gaze_point->position_xy[0] * 1920,            gaze_point->position_xy[1] * 1080);
        gaze[0] = gaze_point->position_xy[0] * weight;
        gaze[1] = gaze_point->position_xy[1] * height;
        //cout << gaze[0] << " " << gaze[1] <<" 2" << endl;
        int pd = 3;
        if (KEY_DOWN(MOUSE_MOVED))
            pd = 0;
        time_t t = time(0);
        strftime(tmp, sizeof(tmp), "%H:%M:%S", localtime(&t));
        char  number[10];
        if (trackers_order[1] == 0)
            strcpy(number, "1");
        else
            strcpy(number, "2");
        //cout << number << < endl;
        char mouse_x[10] = { 0 };
        char mouse_y[10] = { 0 };
        char pd_str[10] = { 0 };
        itoa(pt.x, mouse_x, 10);
        itoa(pt.y, mouse_y, 10);
        itoa(pd, pd_str, 10);

        cout << number << endl;
        //cout << mouse_x << " " << mouse_y << endl;
       // cout << "gaze 2" << endl;
        json j{
                    {"command","eye"},
                    {"actualX", int(gaze_point->position_xy[0] * weight)},
                    {"actualY", int(gaze_point->position_xy[1] * height)},
                    {"id", number},
                    {"content", number},
                    {"time",tmp},
                    {"x",mouse_x},
                    {"y",mouse_y},
                    {"pd",pd_str}
        };
        char update[100];
        std::string s = j.dump();
        strcpy(update, s.c_str());
        //std::cout<<"gaze 0 :" << gaze[0] << endl;
        if (gaze[0] != 0)
        {
            //std::cout<<"gaze 2 2 2 2" << gaze[0] << "wsmbf" << std::endl;
            //std::cout << pd << " str " << pd_str << endl;
            zmq_send(publisher, update, strlen(update), 0);
        }
            //std::cout << gaze_point->position_xy[0] << " " << gaze_point->position_xy[1] << std::endl;
    }
}
std::vector<std::string> str;
//回调函数,获得所有tobii的地址
void url_receiver(char const* url, void* user_data)
{
    char* buffer = (char*)user_data;
    str.push_back(std::string(url));
    if (strlen(url) < 256)
        strcpy(buffer, url);
}

int main(int argc,char const *argv[])
{   

    const char * eyeTracker1 = "IS5FF-100202508221";
    const char * eyeTracker2 = "IS5FF-100201544227";
    cout << 1 << endl;
    //获得参数
    if (argc>1)
    {
        weight = atoi(argv[1]);

        height = atoi(argv[2]);
    }
    cout << weight << " " << height << endl;
    weight = 1920;
    height = 1080;
    int now = 0;
    //端口，默认为5566
    char addr[] = "tcp://*:5566";
    if (argc > 3)
    {
        for (int i = 8; i <= 11; ++i)
            addr[i] = argv[3][i - 8];
    }
    //zmq绑定端口
    zmq_bind(publisher, addr);
    int zc = 1;
    //zmq参数，不用管
    zmq_setsockopt(publisher, ZMQ_CONFLATE, &zc, sizeof(zc));
    cout << addr << endl;
    
    // 创建tobii api
    tobii_api_t* api = NULL;
    tobii_error_t result = tobii_api_create(&api, NULL, NULL);
    assert(result == TOBII_ERROR_NO_ERROR);

    
    cout << 3 << endl;
    // 获得两个tobii的地址
    char url_1[256] = { 0 };
    char url_2[256] = { 0 };
    result = tobii_enumerate_local_device_urls(api, url_receiver, url_1);
    cout << 4 << endl;
    //result = tobii_enumerate_local_device_urls(api, url_receiver, url);
    for (auto i : str)
        std::cout << i << std::endl;
    assert(result == TOBII_ERROR_NO_ERROR);
    cout << 5 << endl;
    int tobiiFlag = 0;
    if (str.size() > 0)
    {
        int l = str[0].length();
        for (int i = 0; i < l; ++i)
            url_1[i] = str[0][i];
        url_1[l] = '\0';
    }
    if (str.size() > 1)
    {
        int l = str[1].length();
        for (int i = 0; i < l; ++i)
            url_2[i] = str[1][i];
        url_2[l] = '\0';
    }
    if (*url_1 == '\0')
    {
        printf("Error: No device found\n");
        return 1;
    }
    cout << 6 << endl;
    //通过地址绑定tobii_device_t
    tobii_device_t* device_1 = NULL;
    tobii_device_t* device_2 = NULL;
    //一开始只启动第一个tobii
 
    result = tobii_device_create(api, url_1, TOBII_FIELD_OF_USE_INTERACTIVE, &device_1);
    assert(result == TOBII_ERROR_NO_ERROR);
    result = tobii_gaze_point_subscribe(device_1, gaze_point_callback_1, 0);
    assert(result == TOBII_ERROR_NO_ERROR);

    result = tobii_device_create(api, url_2, TOBII_FIELD_OF_USE_INTERACTIVE, &device_2);
    assert(result == TOBII_ERROR_NO_ERROR);

    // 通过序列号确定是哪个设备 规定 左边的为第一个设备，右边的为第二个
    tobii_device_info_t deivce_info_1;
    result=tobii_get_device_info(device_1, &deivce_info_1);
    assert(error == TOBII_ERROR_NO_ERROR);
    printf("sn1 %s\n", deivce_info_1.serial_number);
    if (strcmp(deivce_info_1.serial_number, eyeTracker1)==0) {
        trackers_order[0] = 0;
        trackers_order[1] = 1;
    // 2 -> left
        tobiiFlag = 1;
        std::cout << " 1 " << endl;
    }
    else {
        trackers_order[0] = 1;
        trackers_order[1] = 0;
        // 1 -> left 
        tobiiFlag = 0;
        std::cout << " 2 " << endl;
    }


    time_t timeStamp = time(nullptr);
    cout << "start" << timeStamp << endl;
    auto now1 = std::chrono::system_clock::now();
    auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now1);
    auto value = now_ms.time_since_epoch().count();
    std::cout << "Milliseconds since epoch: " << value << std::endl;

    cout << "endl" << timeStamp << endl;
    now1 = std::chrono::system_clock::now();
    now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now1);
    value = now_ms.time_since_epoch().count();
    std::cout << "Milliseconds since epoch: " << value << std::endl;



    int times = 0;
    now = tobiiFlag;
    cout << 7 << endl;
    while(1)
    {
        //now1 = std::chrono::system_clock::now();
        //now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now1);
        //value = now_ms.time_since_epoch().count();
        //std::cout << "Att " << value << std::endl;

        gaze[0] = 0, gaze[1] = 1;
        if (now == tobiiFlag)//now为0时使用的是第一个tobii，now为1时使用的是第二个tobii
        {
            //调用tobii获得数据
            result = tobii_wait_for_callbacks(1, &device_1);
            assert(result == TOBII_ERROR_NO_ERROR || result == TOBII_ERROR_TIMED_OUT);
            result = tobii_device_process_callbacks(device_1);
            assert(result == TOBII_ERROR_NO_ERROR);

            times++;
            //如果没有获得数据两次，就关闭第一个tobii，启动第二个tobii
            if (gaze[0] == 0 && gaze[1] == 1 && times>2)
            if (gaze[0] == 0 && gaze[1] == 1 && times>2)
            {
                result = tobii_gaze_point_unsubscribe(device_1);
                assert(result == TOBII_ERROR_NO_ERROR);
                //result = tobii_device_destroy(device_1);
                //assert(result == TOBII_ERROR_NO_ERROR);
                //result = tobii_device_create(api, url_2, TOBII_FIELD_OF_USE_INTERACTIVE, &device_2);
                //assert(result == TOBII_ERROR_NO_ERROR);
                // Subscribe to gaze data
                result = tobii_gaze_point_subscribe(device_2, gaze_point_callback_2, 0);
                assert(result == TOBII_ERROR_NO_ERROR);

                times = 0;
                now = int(tobiiFlag^1);
            }
        }
        else if (now == int(tobiiFlag^1))
        {
            int a1 = 0;
            //调用tobii获得数据
            result = tobii_wait_for_callbacks(1, &device_2);
            result = tobii_wait_for_callbacks(1, &device_2);
            assert(result == TOBII_ERROR_NO_ERROR || result == TOBII_ERROR_TIMED_OUT);

            // Process callbacks on this thread if data is available
            result = tobii_device_process_callbacks(device_2);
            assert(result == TOBII_ERROR_NO_ERROR);
            times ++;
            //如果没有获得数据两次，就关闭第二个tobii，启动第一个tobii
            if (gaze[0] ==0 && gaze[1]==1 && times>2)
            {
                
                result = tobii_gaze_point_unsubscribe(device_2);
                assert(result == TOBII_ERROR_NO_ERROR);
                //result = tobii_device_destroy(device_2);
                //assert(result == TOBII_ERROR_NO_ERROR);

                //result = tobii_device_create(api, url_1, TOBII_FIELD_OF_USE_INTERACTIVE, &device_1);
                //assert(result == TOBII_ERROR_NO_ERROR);
                // Subscribe to gaze data
                result = tobii_gaze_point_subscribe(device_1, gaze_point_callback_1, 0);
                assert(result == TOBII_ERROR_NO_ERROR);
                // This sample will collect 1000 gaze points
                times = 0;
                now = tobiiFlag;
            }
        }
    }

    // Cleanup
    result = tobii_gaze_point_unsubscribe(device_1);
    assert(result == TOBII_ERROR_NO_ERROR);
    result = tobii_device_destroy(device_1);
    assert(result == TOBII_ERROR_NO_ERROR);
    result = tobii_gaze_point_unsubscribe(device_2);
    assert(result == TOBII_ERROR_NO_ERROR);
    result = tobii_device_destroy(device_2);
    assert(result == TOBII_ERROR_NO_ERROR);
    result = tobii_api_destroy(api);
    assert(result == TOBII_ERROR_NO_ERROR);
    return 0;
}
