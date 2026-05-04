#pragma once
#include "esphome.h"
//https://community.home-assistant.io/t/global-variables-within-lambdas/438234
using namespace esphome;

class Canvas {
  public:
    const int columns_2 = 2;
    const int columns_3 = 3;
    const int columns_4 = 4;
    const int columns_5 = 5;
    const int rows_2 = 2;
    const int rows_3 = 3;
    const int rows_4 = 4;
    const int rows_5 = 5;

    int padding;
    int bottom_trim;
    int width;
    int height;

    struct cell_t {
        int width;
        int height;
    } cell;

    struct time_t {
        int x;
        int y;
        int width;
        int height;
    } time;

    struct date_t {
        int x;
        int y;
        int width;
        int height;
    } date;

    struct qr_t {
        int x;
        int y;
        int width;
        int height;
        int x_wifi_off;
        int y_wifi_off;
        int scale_factor;
        int icon_width;
        int icon_height;
    } qr;

    struct credentials_t {
        int x;
        int y;
        int width;
        int height;
    } credentials;

    struct current_weather_t {
        int x;
        int y;
        int width;
        int height;
    } current_weather;

    struct forecast_sunset_t {
        int x;
        int y;
        int width;
        int height;
    } forecast_sunset;

    struct forecast_weather_t {
        int x;
        int y;
        int width;
        int height;
    } forecast_weather;

   struct forecast_info_t {
        float temp_h[3];
        std::string time_h[3];
        std::string condition_h[3];
    } forecast_info;

} canvas;
